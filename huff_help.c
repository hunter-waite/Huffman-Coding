#include "huff_help.h"

/* counts the total number of characters from the given frequency list */
int get_tchars(h_node *n){
    int tchars, i;
    tchars = 0;
    for(i = 0;i<NUM_ELEMENTS;i++){
        tchars += n[i].freq;
    }
    return tchars;
}

/* makes the huffman tree from the given sorted list */
h_node *make_tree(h_node *l) {
    h_node *super;
    if(!(l->next)){
        return l;
    }
    while((l->next->next) != NULL){
        super = make_super(l, l->next);
        l = l->next->next;
        l = insert(l,super);
    }
    super = make_super(l,l->next);
    l = super;
    return l;
}

/* helper for the make_tree, creates a huffman super node of the 
 * given left and right node, reduces redundancy in make_tree */
h_node *make_super(h_node *l, h_node *r){
    h_node *super = calloc(1,sizeof(h_node));
    if(super == NULL){
        perror("Super Node Calloc");
        exit(3);
    }
    super->l_child = l;
    super->r_child = r;
    super->freq = r->freq + l->freq;
    return super;
}

/* insert h_node n into the list l */
h_node *insert(h_node *l, h_node *n){
    h_node *prev = NULL;
    h_node *curr = l;
    while(curr->freq < n->freq){ /* loops until correct node found */
        if(!(curr->next))
            break;
        prev = curr; /* holds a pointer to previous node in list */
        curr = curr->next;
    }
    if(prev == NULL) {  /* inserts n into the head of the list */
        if(curr->freq >= n->freq){  /* n is the new head of list */
            n->next = curr;
            l = n;
        } else {    /* puts n at second item in list */
            curr->next = n;
        }
        return l;
    } else {    /* puts n into middle or end of list */
        if(curr->next) { /* puts n into middle not end */
            prev->next = n;
            n->next = curr;
        } else {    /* puts n in end or second to last */
            if(curr->freq >= n->freq) {     /* n is second to last in list */
                prev->next = n;
                n->next = curr;
            } else {    /* n is the last item of list */
                curr->next = n;
            }
        }
    }
    return l;
}

/* gets a list of all the characters and frequencies in the file 
 * uses unix io */
h_node *get_lists(int fd){
    h_node *l = calloc(NUM_ELEMENTS, sizeof(h_node));    
    unsigned char *buff = calloc((CHAR_READ + 1), 1);
    size_t i, j;
    int ascii_val;
    i = read(fd,buff,CHAR_READ);
    if(l == NULL){
        perror("Get Lists Calloc");
        exit(3);
    }
    while( i != 0 ){
        for(j = 0; j < i; j++){
            ascii_val = *(buff+j);
            if((l + ascii_val)->c == (unsigned char)ascii_val) {
                (l + ascii_val)->freq++;
            } else {
                (l + ascii_val)->c = (unsigned char)ascii_val;
                (l + ascii_val)->freq = 1;
            }
        }
        free(buff);
        buff = calloc((CHAR_READ + 1), 1);
        i = read(fd,buff,CHAR_READ);
    }
    free(buff);
    return l;
}

/* sorts the list of ascii values based on frequency, then links together
 * the nodes that do not have a frequency of zero */
h_node *sort_list(h_node *l){
    int i;
    qsort(l, NUM_ELEMENTS, sizeof(h_node), cmp_freq_then_name);
    for(i=0;i < (NUM_ELEMENTS-2);i++){
        if((l+i+1)->freq != 0) {
            (l+i)->next = (l + i + 1);
        }
    }
    /* only for a full ascii table */
    if((l+i+1) && (l+i+1)->freq != 0){
        (l+i)->next = (l+i+1);
    }
    return l;
}

/* comparison function for the quicksort */
int cmp_freq_then_name(const void *p1, const void *p2){
    h_node *cp1 = (h_node *)p1;
    h_node *cp2 = (h_node *)p2;
    int val = 0;
    if((cp1-> c) && (cp2->c)) {
        if(!(val = cp1->freq - cp2->freq)) {
            return (int)(cp1->c) - (int)(cp2->c);
        }
        return val;
    } else if(!(cp1->c)) {
        return 1;
    } else if(!(cp2->c)) {
        return -1;
    }
    return 0;
}


/* extracts the codes from a given huffman tree */
c_table *extract_codes(h_node *n){
    c_table *codes = calloc(NUM_ELEMENTS, sizeof(c_table)); /* code table */
    char *init_code = calloc(1,1); /* blank initial code for later */
    if(codes ==  NULL || init_code == NULL){
        perror("Code Table Calloc");
        exit(3);
    }
    if(!n){
        return codes;
    }
    extract_codes_helper(n,codes,init_code);
    return codes;
}

/* recursively does a depth first search of the given huffman tree and 
 * creates the codes for all the leafs of the tree */
void extract_codes_helper(h_node *n,c_table *codes,char *curr_code){
    /* creates copies of the code for the left and right side traversal
     * of the tree */
    char *copy1 = calloc(1,strlen(curr_code)+2); /* \0 and new char */
    char *copy2 = calloc(1,strlen(curr_code)+2); /* \0 and new char */   
    copy1 = strcpy(copy1,curr_code);
    copy2 = strcpy(copy2,curr_code);
    if(copy1 == NULL || copy2 == NULL){
        perror("Extract Code Helper Calloc");
        exit(3);
    }
    if(!(n->l_child) && !(n->r_child)) {    /* checks for leaf */
        codes[(unsigned int)n->c].code = curr_code; /* inserts code on leaf */
        codes[(unsigned int)n->c].c = n->c;
    }
    if(n->l_child) {    /* recusrion for left side of tree */
        free(curr_code);
        extract_codes_helper(n->l_child,codes,strcat(copy1,"0"));
    } else {
        free(copy1);
    }
    if(n->r_child) { /* recursion for right side of tree */
        extract_codes_helper(n->r_child,codes,strcat(copy2,"1"));
    } else {
        free(copy2);
    }
}

/* filters and prints the code table to stdout */
c_table *filter_codes(c_table *codes){
    int i;
    for(i = 0;i<NUM_ELEMENTS;i++) {
        if(codes+i && codes[i].code){
            printf("0x%02x: %s\n", codes[i].c,codes[i].code);
        }
        free(codes[i].code);
    }
    return codes;
}


/* used for testing */
void print_list(h_node *list){
    h_node *l = list;
    while(l){
        printf("Character: 0x%x ('%c')\t Frequency: %d\t Ascii Val: %d\n",
        l->c,l->c,l->freq,(int)(l->c));
        l = l->next;
    }
    printf("-----------------------\n");
}

/* frees all the character pointers stored in the code table then
 * frees the code table itself */
void free_codes(c_table *codes){
    int i;
    for(i = 0; i < NUM_ELEMENTS; i++) {
        if(codes[i].code)
            free(codes[i].code);
    }
    free(codes);
}

/* frees all the super nodes made in the tree
 * Checks to make sure that a node won't be free'd if it was
 * initially apart of the list and just has a nul value */
void free_tree(h_node *n,h_node *list){
    if(!n)
        return;
    free_tree(n->l_child,list);
    free_tree(n->r_child,list);
    if(!(n->c) && n != list+255){
        free(n);
    } 
}

void safe_read(int file, void *buff, size_t size) {
    int v = read(file, buff, size);
    if(v < 0) {
        perror("safe_read");
        exit(EXIT_FAILURE);
    }
}

void safe_write(int file, void *buff, size_t size) {
    int v = write(file, buff, size);
    if(v < 0) {
        perror("safe_write");
        exit(EXIT_FAILURE);
    }
}
