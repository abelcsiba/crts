

#ifndef CIAM_OPCODE_H_
#define CIAM_OPCODE_H_

#define OPCODE_LIST                             \
    X(LOAD_CONST,   0, 1,   "LOAD_CONST"    )   \
    X(NOP,          1, 0,   "OP_NOP"        )   \
    X(PUSH,         2, 0,   "OP_PUSH"       )   \
    X(POP_TOP,      3, 0,   "OP_OPO_TOP"    )   \
    X(TOS,          4, 0,   "OP_TOS"        )   \
    X(ADD,          5, 0,   "OP_ADD"        )   \
    X(SUB,          6, 0,   "OP_SUB"        )   \
    X(MUL,          7, 0,   "OP_MUL"        )   \
    X(DIV,          8, 0,   "OP_DIV"        )   \
    X(HLT,          9, 0,   "OP_HLT"        )   \
    X(LOAD_IMM,    10, 1,   "OP_LOAD_IMM"   )   \
    X(LOAD_NULL,   11, 0,   "OP_LOAD_NULL"  )   \

typedef enum {
#define X(kind, id, has_operand, label) kind = id,
    OPCODE_LIST
#undef X
} opcode_const_t;


#endif // CIAM_OPCODE_H_