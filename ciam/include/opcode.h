

#ifndef CIAM_OPCODE_H_
#define CIAM_OPCODE_H_


#define OPCODE_LIST                             \
    X(LOAD_CONST,       0, 1,   "LOAD_CONST"    )   \
    X(NOP,              1, 0,   "OP_NOP"        )   \
    X(PUSH,             2, 0,   "OP_PUSH"       )   \
    X(POP_TOP,          3, 0,   "OP_POP_TOP"    )   \
    X(TOS,              4, 0,   "OP_TOS"        )   \
    X(ADD_I8,           5, 0,   "OP_ADD_I8"     )   \
    X(ADD_I16,          6, 0,   "OP_ADD_I16"    )   \
    X(ADD_I32,          7, 0,   "OP_ADD_I32"    )   \
    X(ADD_I64,          8, 0,   "OP_ADD_I64"    )   \
    X(ADD_F,            9, 0,   "OP_ADD_F"      )   \
    X(ADD_D,           10, 0,   "OP_ADD_D"      )   \
    X(SUB_I8,          11, 0,   "OP_SUB_I8"     )   \
    X(SUB_I16,         12, 0,   "OP_SUB_I16"    )   \
    X(SUB_I32,         13, 0,   "OP_SUB_I32"    )   \
    X(SUB_I64,         14, 0,   "OP_SUB_I64"    )   \
    X(SUB_F,           15, 0,   "OP_SUB_F"      )   \
    X(SUB_D,           16, 0,   "OP_SUB_D"      )   \
    X(MUL_I8,          17, 0,   "OP_MUL_I8"     )   \
    X(MUL_I16,         18, 0,   "OP_MUL_I16"    )   \
    X(MUL_I32,         19, 0,   "OP_MUL_I32"    )   \
    X(MUL_I64,         20, 0,   "OP_MUL_I64"    )   \
    X(MUL_F,           21, 0,   "OP_MUL_F"      )   \
    X(MUL_D,           22, 0,   "OP_MUL_D"      )   \
    X(DIV_I8,          23, 0,   "OP_DIV_I8"     )   \
    X(DIV_I16,         24, 0,   "OP_DIV_I16"    )   \
    X(DIV_I32,         25, 0,   "OP_DIV_I32"    )   \
    X(DIV_I64,         26, 0,   "OP_DIV_I64"    )   \
    X(DIV_F,           27, 0,   "OP_DIV_F"      )   \
    X(DIV_D,           28, 0,   "OP_DIV_D"      )   \
    X(HLT,             29, 0,   "OP_HLT"        )   \
    X(LOAD_IMM,        30, 1,   "OP_LOAD_IMM"   )   \
    X(LOAD_NULL,       31, 0,   "OP_LOAD_NULL"  )   \
    X(LOAD_STRING,     32, 0,   "LOAD_STRING"   )   \
    X(CALL,            33, 1,   "CALL"          )   \
    X(STORE_LOCAL,     34, 1,   "STORE_LOCAL"   )   \
    X(LOAD_LOCAL,      35, 1,   "LOAD_LOCAL"    )   \
    X(TRAP,            36, 0,   "TRAP"          )   \
    X(NEG,             37, 0,   "NEG"           )   \
    X(JMP_IF_FALSE,    38, 1,   "JMP_IF_FALSE"  )   \
    X(JMP,             39, 1,   "JMP"           )   \


typedef enum {
#define X(kind, id, has_operand, label) kind = id,
    OPCODE_LIST
#undef X
} opcode_const_t;

#endif // CIAM_OPCODE_H_