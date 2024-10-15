
#include "data.h"

#include <stdlib.h>

block_t* create_block(size_t capacity)
{
    block_t* block = (block_t*)malloc(sizeof(block_t));
    if (NULL == block)
    {
        fprintf(stderr, "failed init block for the arena\n");
        exit(EXIT_FAILURE);
    }

    block->capacity = capacity;
    block->count = 0;
    block->data = malloc(capacity);
    if (NULL == block->data)
    {
        fprintf(stderr, "failed to init block data\n");
        exit(EXIT_FAILURE);
    }
    block->next = NULL;
    return block;
}

void destroy_block(block_t* block)
{
    if (NULL != block)
    {
        if (NULL != block->data) free(block->data);
        free(block);
    }
}

void init_arena(arena_t* arena, size_t block_size)
{
    arena->block_size = block_size;
    arena->head = create_block(block_size);
    arena->end = arena->head;
}

void* arena_alloc(arena_t* arena, size_t size)
{
    if (size > arena->block_size)
    {
        fprintf(stderr, "memory need exceeds max block size\n");
        exit(EXIT_FAILURE);
    }

    block_t *block = arena->head;

    while (NULL != block && block->count + size > block->capacity) block = block->next;

    if (NULL == block) 
    {
        block = create_block(ARENA_DEFAULT_BLOCK_SIZE);
        arena->end->next = block;
        arena->end = block;
    }

    void* ptr = block->data + block->count;
    block->count += size;
    return ptr;
}

void reset_arena(arena_t* arena)
{
    block_t* block = arena->head;

    while (NULL != block)
    {
        block->count = 0;
        block = block->next;
    }
    arena->end = arena->head;
}

void destroy_arena(arena_t* arena)
{
    block_t* block = arena->head;
    while (NULL != block)
    {
        block_t* next = block->next;
        destroy_block(block);
        block = next;
    }
    free(arena);
}