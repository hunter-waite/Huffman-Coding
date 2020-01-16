#include "huff_help.h"
#include <stdint.h>

int write_header(int fd, h_node *freq_list);
void write_body(int fd_in, int fd_out, c_table *codes);
void place_bit(int fd, unsigned char bit);
