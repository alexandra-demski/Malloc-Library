#ifndef _MEMORY_H
#define _MEMORY_H
#include <stddef.h>

int multiple4(size_t n);
size_t align(size_t size);
struct block *_block_ptr(void *ptr);
struct block *_find_block(struct block **last, size_t size);
struct block *_new_block(struct block* last, size_t size);
void *malloc(size_t size);
void free(void *ptr);
void *realloc(void *ptr, size_t size);
void *calloc(size_t nmemb, size_t size);
void print_stats();
int get_nb_total();
int get_nb_use();
int get_nb_free();
size_t get_size_total();
size_t get_size_use();
size_t get_size_free();
size_t get_size_trash();

#endif
