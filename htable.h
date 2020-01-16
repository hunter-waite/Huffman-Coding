#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>

#define NUM_ELEMENTS 256 /* the size of the ascii table, so the max number
                               of h_node possible */
#define CHAR_READ 50    /* the amount of characters to be read by read(2) */

typedef struct h_node h_node;
struct h_node{
    unsigned char c;
    int freq;
    h_node *l_child;
    h_node *r_child;
    h_node *next;
};

typedef struct c_table c_table;
struct c_table{
    unsigned char c;
    char *code;
};

h_node *make_tree(h_node *l);
h_node *make_super(h_node *l, h_node *r);
h_node *insert(h_node *l, h_node *n);
void extract_codes(h_node *n);
void extract_codes_helper(h_node *n, c_table *codes,char *curr_code);
c_table *filter_codes(c_table *codes);
h_node *get_lists(int f);
h_node *sort_list(h_node *l);
int cmp_freq_then_name(const void *p1, const void *p2);
void print_list(h_node *l);
