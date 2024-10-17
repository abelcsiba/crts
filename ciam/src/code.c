
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

void print_code(FILE* out, code_t* code, int count)
{
    for (int i = 0; i < count; i++)
    {
        switch (code[i].op)
        {
            case LOAD_CONST:
                fprintf(out, "LOAD_CONST %ld\n", code[i].opnd1);
                break;
            case OP_ADD:
                fprintf(out, "OP_ADD\n");
                break;
            case OP_SUB:
                fprintf(out, "OP_SUB\n");
                break;
            case OP_MUL:
                fprintf(out, "OP_MUL\n");
                break;
            case OP_DIV:
                fprintf(out, "OP_DIV\n");
                break;
        }
    }
}