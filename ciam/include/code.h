

#ifndef CIAM_CODE_H_
#define CIAM_CODE_H_

#include "opcode.h"

#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>

typedef uint8_t opcode_t;
typedef int64_t opnd_t;

typedef struct {
    opcode_t            op;
    opnd_t              opnd1;
} code_t;

typedef struct {
    code_t*             data;
    size_t              count;
    size_t              capacity;
} code_da;

typedef struct {
    uint8_t             type;
    int64_t             value;
} num_const_t;

typedef struct {
    uint32_t            length; // 2^32 num of chars is the upper limit
    char*               chars;
} string_const_t;

typedef struct {
    string_const_t*     strings;
    uint16_t            count;
} const_str_table_t;

typedef struct {
    num_const_t*        nums;
    uint16_t            count;
} const_num_table_t;

typedef struct {
    const_num_table_t   numbers;
    const_str_table_t   strings;
} const_pool_t;

void init_code(code_t* code, size_t length);
void free_code(code_t* code);

void add_to_code_da(code_da* da, code_t val);
void init_code_da(code_da* da);

void print_code(FILE* out, code_t* code, int count);

#endif // CIAM_CODE_H_