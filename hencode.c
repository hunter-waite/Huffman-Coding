#include "hencode.h"
/* Encodes a given file using the huffman encoding method then writing
 * a header based on the given spec,
 * note that the writing portion of this file is not efficient at all
 * because there is no buffer to be written it just writes to the file for
 * every byte of data */
int main(int argc, char *argv[]){
    int fd_in = open(argv[1],O_RDONLY);
    int fd_out =  STDOUT_FILENO;
    int *out = &fd_out;
    h_node *freq_list,*freq_holder;
    c_table *codes;
    /* checks for arguments */
    if(argc == 3)
        fd_out = open(argv[2], O_CREAT | O_TRUNC | O_WRONLY,0644);

    if(fd_in<0 || fd_out < 0) {
        perror("FD Error");
        exit(1);
    }

    /* gets the frequency list and sorts it, then writes the header */
    freq_list = get_lists(fd_in);
    freq_holder = freq_list;
    fd_out = write_header(fd_out,freq_list);
    freq_list = sort_list(freq_list);
    /* makes the tree then extracts the codes from tree */
    freq_list = make_tree(freq_list);
    codes = extract_codes(freq_list);
    /* frees the tree if it is not size 1 */
    if(freq_list != freq_holder)
        free_tree(freq_list,freq_holder);
    /* frees the initial frequency list */
    free(freq_holder);
    /* resets the file descriptor to the beginning then writes the body */
    lseek(fd_in, 0, SEEK_SET);
    write_body(fd_in,fd_out,codes);
    /* free everything then closes */
    free_codes(codes);
    close(fd_in);
    close(fd_out);
    return 0;
}

/* writes the header for the huffman encoded file based on the given spec */
int write_header(int fd, h_node *freq_list){
    int i;
    uint32_t num_chars = 0;
    uint8_t c;
    uint32_t freq;
    for(i = 0; i < NUM_ELEMENTS; i++) {
        if(freq_list[i].freq != 0)
            num_chars ++;
    }
    safe_write(fd, &num_chars, sizeof(uint32_t));
    for(i = 0; i < NUM_ELEMENTS; i++){
        if(freq_list[i].freq != 0){
            c = (uint8_t)freq_list[i].c;
            freq = (uint32_t)freq_list[i].freq;
            safe_write(fd, &c, sizeof(c));
            safe_write(fd, &freq, sizeof(freq));
        }
    }
    return fd;
}

/* writes the body to the output file based on the given spec, uses the place
 * bit function to write the actual bits */
void write_body(int fd_in, int fd_out, c_table *codes){
    unsigned char *buff = calloc(1,1);
    char *code;
    int i,j,ascii_val;
    while((i=read(fd_in,buff,1)) != 0){
        ascii_val = *buff;
        code = codes[ascii_val].code;
        for(j = 0; j < strlen(code); j++){
            place_bit(fd_out, code[j]);
        }
    }
    place_bit(fd_out, '2');
    free(buff);
}

/* maintains a buffer of bits that when they add togetber to a byte it 
 * gets written to the file then reset back to a blank unsigned char */
void place_bit(int fd, unsigned char bit){
    static unsigned char out = 0;
    static unsigned int count = 0;
    if(bit == '2' && count != 0){   /* fills last buffer with 0 and outputs */
        for(count = count; count < 8;count ++)
            out <<= 1;
        safe_write(fd, &out, sizeof(out));
        return;
    }
    out <<= 1; /* bit shifts to store next var */
    if(bit == '1') { /* if input bit is set set lsb on output bit */
        out = out | 1; 
    }
    count ++;
    if(count == 8) {    /* once it is the size of a byte outputs to file */
        safe_write(fd, &out, sizeof(out));
        out = 0;
        count = 0;
    }
}

