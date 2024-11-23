
#ifndef CIAS_TRANSLATOR_H_
#define CIAS_TRANSLATOR_H_

#include "ast.h"
#include "macros.h"

typedef struct tac_inst_t tac_inst_t;
typedef struct func_t func_t;
typedef struct translator_t translator_t;
typedef struct da_func_t da_func_t;

typedef enum {
    TAC_RETURN          = 0,
    TAC_UNARY           = 1,
} tac_inst_kind_t;

typedef struct {
    enum { CONST, VAR } kind;
    union {
        int32_t         int_val;
        char*           var_name;  
    };
} tac_val_t;

typedef enum {
    NEGATE          = 0,
    COMPLEMENT      = 1, 
} unary_kind_t;

struct tac_inst_t {
    tac_inst_kind_t     kind;

    union {
        tac_val_t       ret;

        struct {
            unary_kind_t op;
            tac_val_t   src;
            tac_val_t   dst;
        } unary;
    };
};

typedef struct {
    tac_inst_t*         instr;
    int32_t             count;
    int32_t             capacity;
} da_inst_t;

struct func_t {
    char*               label;
    da_inst_t           instr;
};

struct da_func_t {
    func_t*             funcs;
    int32_t             count;
    int32_t             capacity;
};

// TODO: This should use an arena
da_func_t* translate_ast(translator_t* translator, cu_t* cu, int32_t* size);
translator_t* init_translator();

#if DEBUG
void print_tac(FILE* out, da_func_t* func_defs);
#endif

#endif // CIAS_TRANSLATOR_H_