# Malloc Library

November 2020

Alexandra Demski

**https://github.com/alexandra-demski/Malloc-Library**

## Generalities

The main goal of the project is to implement my own dynamic allocation functions `malloc`, `free`, `calloc`, `realloc`which will use the systems calls in order to manipulate the program break limit.

```c
int brk(void *addr);
void *sbrk(intptr_t increment);
```

## The project

### Functionalities

The project will at the very least include the usual functions and a debug tool.

```c
void* malloc(size_t size);
void* realloc(void *ptr, size_t size);
void* calloc(size_t nmemb, size_t size);
void  free(void *ptr);
void  print_stats();
```

Each allocated block will incorporate a header with administrative informations necessary for an efficient `malloc` and a memory block used to actually store informations.

The allocation algorithm will, at first, search for a free and big enough memory block. If it succeeds, it will return the address of this zone and label the block as used. If it fails, it will create a new block using `sbrk`.

With this minimalist implementation, deallocate memory ; the `free` function will only mark a block as unused.

The debug tool `print_stats` will display useful information such as the number of allocated block, used and free ones, their size and the quantity of wasted memory space.

### Structure

The project is divided in several files. The main code is stored in `memory.c` with a header `memory.h` used to include its functions. The various `test.c` files are used to confirm the effectiveness of the library. The last one called `Makefile` simply compiles all the code for use : it can be called with the `make` command which will produce executable files.

## The code

### Block

A memory block contains useful information which will be stored in a header.

```c
struct block {
	size_t size;
	size_t used_size;
	struct block *next;
	struct block *previous;
	int free;
};
#define META_SIZE sizeof(struct block)
```

It works as a linked list, each block will point to the next and the previous one. The first block in the list will be called first and because malloc wasn't called yet, it is initialized as `NULL`.

```c
void *first = NULL;
```

### Malloc

The first step, before allocating any block, is to align the size of it, which will enhance the speed and veracity of the program. In the `malloc` function we will call `align` which will return a multiple of 4 based on the given size.

```c
size_t align(size_t size){
	size_t n = size;
	if(!multiple4(n)){
		return ((n/4)+1)*4;
	}
	return n;
}
```

The `multiple4` function simply check if a given number is already a multiple and therefor, doesn't need to be aligned.

```c
int multiple4(size_t n){
	int x = (int)n;
	while(x > 0){
		x = x-4;
	}
	if(x == 0){
		return 1;
	}
	return 0;
}
```

Obviously, after aligning the size, we check if the given number is greater or equal to zero in order to prevent any abnormal behavior.

After that, I decided to separated two different income : either it's the first time we use `malloc` and therefor, don't need to search for a free block and directly allocate a new one, or we already allocated some, so we will need to check them.

```c
void *_malloc (size_t size) {
	struct block *b;
	size_t s;
	s = align(size);

	if(s <= 0)
		return NULL;
	if(!first){
		b = _new_block(NULL,s);
		if(!b)
			return NULL;
		first = b;
	} else {
		struct block *last = first;
		b = _find_block(&last, s);
		if(!b){
			b = _new_block(last, s);
			if(!b)
				return NULL;
		} else {
			b->free = 0;
			b->used_size = s;
		}
	}
	return(b+1);
}
```

The `_new_block` function push the program break with the system call `sbrk` and return the newly created block.

```c
struct block *_new_block(struct block* last, size_t size){
	struct block *b;
	b = sbrk(0);
	void *ptr = sbrk(size + META_SIZE);
	if(ptr == (void*)-1)
		return NULL;
	if(last){
		last->next = b;
		b->previous = last;
	}
	b->size = size;
	b->next = NULL;
	b->free = 0;
	b->used_size = size;
	return b;
}
```

The `_find_block` function search through the linked list for the most optimal block. If it doesn't find any, it returns `NULL`.

```c
struct block *_find_block(struct block **last, size_t size){
	struct block *current = first;
	struct block *optimal = NULL;
	while(current){
		if(current->free && current->size >= size){
			if(optimal){
				if(optimal->size > current->size){
					optimal = current;
				}
			} else {
				optimal = current;
			}
		}
		*last = current;
		current = current->next;
	}
}
```

### Free

As we can't give back to the machine an already allocated zone with `sbrk`, the `free` function will only mark a given block as unused. It is important to check the pointer before using it, as it can generate errors.

```c
void _free(void *ptr) {
	if(!ptr)
		return;
	struct block *b;
	b = _block_ptr(ptr);
	b->free = 1;
	b->used_size = 0;
}
```

The `_block_ptr` function returns the address of the block without its header.

```c
struct block *_block_ptr(void *ptr){
	return (struct block*)ptr - 1;
}
```

### Calloc

The `calloc` function will use our `malloc` to allocate a new block and then will set all his bytes to 0 with the functions `memset`.

```c
void *_calloc(size_t nmemb, size_t size){
	size_t s = nmemb * size;
	void *ptr = malloc(s);
	memset(ptr, 0, size);
	return ptr;
}
```

### Realloc

Just as `calloc` our `realloc` function will simply call `malloc` and `free` while copying the data from one block to another with `memcpy`. It's important to check the pointer before any tasks as it can raise an error.

```c
void *_realloc(void *ptr, size_t size){
	if(!ptr)
		return malloc(size);
	struct block *b_ptr = _block_ptr(ptr);
	if(b_ptr->size >= size)
		return ptr;
	void *new_ptr;
	new_ptr = malloc(size);
	if(!new_ptr)
		return NULL;
	memcpy(new_ptr, ptr, b_ptr->used_size);
	free(ptr);
	return new_ptr;
}
```

## Improvements

In order to improve the library, some additional features were needed. Indeed, the library suffers from fragmentation so in order to correct that, I implemented two improvements.

### Split at allocation

Before a free block is given by the function `_find_block`, if its size is big enough, it will be split into to different ones. In order to do so, we need to add new element in the linked list.

```c
if(optimal && (optimal->size >= (size + META_SIZE + 4))){ 
	struct block *next, *new;
	next = optimal->next;
	new = _block_ptr(optimal) + size;
	new->next = next;
	optimal->next = new;
	new->previous = optimal;
	if(next != NULL){
		next->previous = new;
	}
	new->size = optimal->size - size - META_SIZE;
	optimal->size = size;
	new->free = 1;
	new->used_size = 0;
}
```

### Unit before freeing

The complementary addition will unit free neighboring blocks before setting one to unused. It will work similarly to the previous function but instead of adding a new link, we will erased and incorporate one.

```c
struct block *n, *p;
n = b->next;
p = b->previous;
if(n && n->free){
	b->next = n->next;
	b->size = b->size + n->size + META_SIZE;
}
if(p && p->free){
	p->next = b->next;
	p->size = p->size + b->size + META_SIZE;
}
```
