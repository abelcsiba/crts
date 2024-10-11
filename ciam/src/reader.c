
#include "reader.h"

#include <stdio.h>
#include <assert.h>


DECL_DA(code_t)
DEF_DA(code_t)

static i64 parse_operand(const char* buffer)
{
    i64 result = *((i64*)buffer);
    return result;
}

code_t* parse_code(const char* buffer, size_t length)
{
    code_t_array_t array;
    array.count = 0;
    array.capacity = 0;
    array.data = NULL;
    size_t index = 0;
    while (index < length)
    {
        code_t c;
        c.op = (byte)buffer[index++];
        assert(length >= index + 8 && "invalid bytecode format");
        c.opnd1 = parse_operand(&buffer[index]);
        index += 8;
        add_code_t(&array, c);
    }
    
    return array.data;
}

code_t* open_bc_source_file(const char* path)
{
    FILE* file = fopen(path, "rb");
    if (NULL == file) return NULL;

    fseek(file, 0, SEEK_END);

    size_t length = ftell(file);

    fseek(file, 0, SEEK_SET);

    char* buff = (char*)calloc(length + 1, sizeof(char));
    fread(buff, sizeof(char), length, file);

    if (ferror(file)) return NULL;

    return parse_code(buff, length);
}