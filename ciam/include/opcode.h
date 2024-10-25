

#ifndef CIAM_OPCODE_H_
#define CIAM_OPCODE_H_

#define OPCODE_LIST       \
    X(LOAD_CONST,   0, 1) \
    X(NOP,          1, 0) \
    X(PUSH,         2, 0) \
    X(POP_TOP,      3, 0) \
    X(TOS,          4, 0) \
    X(ADD,          5, 0) \
    X(SUB,          6, 0) \
    X(MUL,          7, 0) \
    X(DIV,          8, 0) \
    X(HLT,          9, 0) \
    X(LOAD_IMM,    10, 1) \
    X(LOAD_NULL,   11, 0) \

typedef enum {
#define X(kind, id, has_operand) kind = id,
    OPCODE_LIST
#undef X
} opcode_const_t;


#endif // CIAM_OPCODE_H_