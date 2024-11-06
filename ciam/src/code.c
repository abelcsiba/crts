
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
            case ADD_I8:
                fprintf(out, "ADD_I8\n");
                break;
            case ADD_I16:
                fprintf(out, "ADD_I16\n");
                break;
            case ADD_I32:
                fprintf(out, "ADD_I32\n");
                break;
            case ADD_I64:
                fprintf(out, "ADD_I64\n");
                break;
            case ADD_F:
                fprintf(out, "ADD_F\n");
                break;
            case ADD_D:
                fprintf(out, "ADD_D\n");
                break;
            case SUB_I8:
                fprintf(out, "SUB_I8\n");
                break;
            case SUB_I16:
                fprintf(out, "SUB_I16\n");
                break;
            case SUB_I32:
                fprintf(out, "SUB_I32\n");
                break;
            case SUB_I64:
                fprintf(out, "SUB_I64\n");
                break;
            case SUB_F:
                fprintf(out, "SUB_F\n");
                break;
            case SUB_D:
                fprintf(out, "SUB_D\n");
                break;
            case MUL_I8:
                fprintf(out, "MUL_I8\n");
                break;
            case MUL_I16:
                fprintf(out, "MUL_I16\n");
                break;
            case MUL_I32:
                fprintf(out, "MUL_I32\n");
                break;
            case MUL_I64:
                fprintf(out, "MUL_I64\n");
                break;
            case MUL_F:
                fprintf(out, "MUL_F\n");
                break;
            case MUL_D:
                fprintf(out, "MUL_D\n");
                break;
            case DIV_I8:
                fprintf(out, "DIV_I8\n");
                break;
            case DIV_I16:
                fprintf(out, "DIV_I16\n");
                break;
            case DIV_I32:
                fprintf(out, "DIV_I32\n");
                break;
            case DIV_I64:
                fprintf(out, "DIV_I64\n");
                break;
            case DIV_F:
                fprintf(out, "DIV_F\n");
                break;
            case DIV_D:
                fprintf(out, "DIV_D\n");
                break;
            case LOAD_STRING:
                fprintf(out, "LOAD_STRING\n");
                break;
        }
    }
}