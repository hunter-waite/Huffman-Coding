#include "huff_help.h"
#include <stdint.h>

#define NUM_BITS 8

h_node *read_header(int fd_in);
void decode_file(int fd_in, int fd_out, h_node *n, int tchars);
