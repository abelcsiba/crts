
#include "assembler.h"

#include <stdint.h>

typedef enum {
    ASM_MOV             = 0,
    ASM_UNARY           = 1,
    ASM_ALLOCATE_STACK  = 2,
    ASM_RET             = 3,
} asm_inst_kind_t;

typedef enum {
    AX      = 0,
    R10     = 1,
} asm_reg_t;

typedef struct {
    enum { IMM, REG, PSEUDO, STACK } kind;
    union {
        int32_t     int_val;
        asm_reg_t   reg;
        char*       ident;
    };
} asm_oper_t;



typedef struct asm_routine_t asm_routine_t;

struct asm_routine_t {
    char*           label;

    asm_routine_t*  next;
};

struct assembler_t {
    asm_routine_t*  routines;
};