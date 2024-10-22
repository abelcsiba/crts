

#ifndef CIAM_CODE_H_
#define CIAM_CODE_H_

#include "opcode.h"

#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>

typedef uint8_t opcode_t;
typedef uint64_t opnd_t;

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
    uint64_t            value;
} num_const_t;

typedef struct {
    num_const_t*        nums;
    uint16_t            count;
} const_num_table_t;

typedef struct {
    const_num_table_t   numbers;
} const_pool_t;

void init_code(code_t* code, size_t length);
void free_code(code_t* code);

void add_to_code_da(code_da* da, code_t val);
void init_code_da(code_da* da);

void print_code(FILE* out, code_t* code, int count);

#endif // CIAM_CODE_H_