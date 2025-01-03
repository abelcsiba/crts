
#include "allocator.h"

#include <stdio.h>

void* reallocate(void* pointer, size_t /*old_size*/, size_t new_size)
{
    if (new_size == 0)
    {
        free(pointer);
        return NULL;
    }

    void* result = realloc(pointer, new_size);

    if (result == NULL)
    {
        fprintf(stderr, "Unable to allocate memory, aborting...\n");
        exit(EXIT_FAILURE);
    }

    return result;
}