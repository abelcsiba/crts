
#include "io.h"
#include <stdio.h>

value_t max(struct ciam_vm_t* /*vm*/, value_t* values, size_t argc)
{
    if (argc != 2)
    {
        fprintf(stderr, "Invalid argument count to 'max'. Expected '2', got '%d'\n", (int)argc);
        exit(EXIT_FAILURE);
    }
    value_t a = values[0];
    value_t b = values[1];
    if (a.type != b.type)
    {
        fprintf(stderr, "Type conversion is not allowed yet\n");
        exit(EXIT_FAILURE);
    }
    return (a.as.i16 > b.as.i16) ? a : b;
}