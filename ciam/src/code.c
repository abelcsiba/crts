
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

void add_to_code_da(code_da* da, code_t val)
{
    if (da->count == da->capacity)
    {
        size_t new_cap = (0 == da->capacity ? 8 : da->capacity * 2);
        da->data = (code_t*)realloc(da->data, sizeof(code_t) * new_cap);
        da->capacity = new_cap;
    }
    da->data[da->count++] = val;
}

void init_code_da(code_da* da)
{
    da->count = da->capacity = 0;
    da->data = NULL;
}

void print_code(FILE* out, code_t* code, int count)
{
    for (int i = 0; i < count; i++)
    {
        switch (code[i].op)
        {
            case LOAD_CONST:
                fprintf(out, "LOAD_CONST %ld\n", code[i].opnd1);
                break;
            case ADD:
                fprintf(out, "ADD\n");
                break;
            case SUB:
                fprintf(out, "SUB\n");
                break;
            case MUL:
                fprintf(out, "MUL\n");
                break;
            case DIV:
                fprintf(out, "DIV\n");
                break;
        }
    }
}