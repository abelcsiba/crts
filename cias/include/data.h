
#ifndef CIAS_DATA_H_
#define CIAS_DATA_H_

#include <stdio.h>

#define ARENA_DEFAULT_BLOCK_SIZE 1024 * 1024

struct block_t {
    size_t count;
    size_t capacity;
    void* data;
    struct block_t* next;
};

typedef struct block_t block_t;

typedef struct {
    size_t block_size;
    block_t* head;
    block_t* end;
} arena_t;

block_t* create_block(size_t capacity);
void destroy_block(block_t* block);

void init_arena(arena_t* arena, size_t size);
arena_t* create_arena(size_t block_size);
void reset_arena(arena_t* arena);
void destroy_arena(arena_t* arena);

void* arena_alloc(arena_t* arena, size_t size);

#endif // CIAS_DATA_H_