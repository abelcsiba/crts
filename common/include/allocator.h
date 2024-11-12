
#ifndef COMMON_ALLOCATOR_H_
#define COMMON_ALLOCATOR_H_

#include <stdlib.h>

// Stolen from https://github.com/munificent/craftinginterpreters
#define ALLOCATE(type, size)                            (type*)reallocate(NULL, 0, sizeof(type) * (size))
#define FREE(type, pointer)                             reallocate(pointer, sizeof(type), 0)
#define GROW_CAPACITY(capacity)                         ((capacity) < 8 ? 8 : (capacity) * 2)
#define GROW_ARRAY(type, pointer, old_size, new_size)   (type*)reallocate(pointer, sizeof(type) * (old_size), sizeof(type) * (new_size))
#define FREE_ARRAY(type, pointer, old_size)             reallocate(pointer, sizeof(type) * (old_size), 0)

void* reallocate(void* pointer, size_t old_size, size_t new_size);

#endif // COMMON_ALLOCATOR_H_