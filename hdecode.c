#include "hdecode.h"
/* Decodes the given file that should be a huffman encoded file and 
 * writes the contents out to the file,
 * Note again this is inefficent because it writes to the file for every
 * character(byte) that it comes across in the decoded data, it should be
 * buffered larger then written */
int main(int argc, char *argv[]){
    h_node *freq_list, *freq_holder;
    int tchars;
    int fd_in = STDIN_FILENO;
    int fd_out = STDOUT_FILENO;
    if(argc >= 2){
        fd_in = open(argv[1],O_RDONLY);
    }
    if(argc >= 3){
        fd_out = open(argv[2],O_CREAT|O_WRONLY|O_TRUNC,0644);
    }
    freq_list = read_header(fd_in);
    freq_holder = freq_list;
    freq_list = sort_list(freq_list);
    tchars = get_tchars(freq_list);
    freq_list = make_tree(freq_list);
    decode_file(fd_in,fd_out,freq_list,tchars);
    free(freq_holder);
    close(fd_in);
    close(fd_out);
    return 0;
}

/* Reads the header of the given file */
h_node *read_header(int fd_in){
    h_node *freq_list = calloc(NUM_ELEMENTS, sizeof(h_node));
    uint32_t num_chars = 0;
    uint8_t c;
    uint32_t freq = 0;
    safe_read(fd_in, &num_chars, sizeof(uint32_t));
    if(num_chars == 0 || num_chars > NUM_ELEMENTS)
        exit(3);

    while(num_chars > 0){
        safe_read(fd_in, &c, sizeof(uint8_t));
        safe_read(fd_in, &freq, sizeof(uint32_t));
        freq_list[c].freq = freq;
        freq_list[c].c = (unsigned char)c;
        num_chars--;
    } 
    return freq_list;
}

/* Decodes the body of the given input file */
void decode_file(int fd_in, int fd_out, h_node *n, int tchars){
    h_node *root = n;
    int i = NUM_BITS;
    uint8_t buff,tempbuff;
    uint8_t bitmask = 0x80; /* bit mask for MSB */
    safe_read(fd_in,&buff,sizeof(uint8_t)); /* reads in a byte of data */ 
    while(tchars>0){ /* loops thru the while there are still chars */
        while(i > 0 && tchars > 0){ /* loops thru a byte of data */
            if(!(n->l_child) && !(n->r_child) && tchars > 0){
                /* writes if the current node is a leaf */
                safe_write(fd_out, &(n->c), sizeof(char));
                n = root;
                tchars--;
                continue;
            }
            tempbuff = buff;
            tempbuff = tempbuff & bitmask;
            /* steps through the tree */
            if(tempbuff == 0x80){   /* MSB is set */
                n = n->r_child;    
            }
            else{   /* MSB is not set */
                n = n->l_child;
            }
            i --;
            /* bit shifts the buffer for next piece of date */
            buff <<= 1;
        }
        i = NUM_BITS;
        safe_read(fd_in, &buff, sizeof(uint8_t));
    }
}
