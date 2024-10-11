

#ifndef CIAM_CODE_H_
#define CIAM_CODE_H_

#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>

typedef uint8_t opcode_t;
typedef uint64_t opnd_t;

typedef struct {
    opcode_t op;
    opnd_t opnd1;
} code_t;

void init_code(code_t* code, size_t length);
void free_code(code_t* code);

#endif // CIAM_CODE_H_