
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