
#include "code.h"

void init_code(code_t* code, size_t length)
{
    for (size_t i = 0; i < length; i++)
    {
        code[i].op = 0x00;
        code[i].opnd1 = 0x00;
    }
}

void free_code(code_t* code)
{
    free(code);
}