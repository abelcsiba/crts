
#ifndef CIAS_AST_H_
#define CIAS_AST_H_

#include "data.h"

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

typedef enum {
    NULL_LITERAL,
    NUM_LITERAL,
    STRING_LITERAL,
    CHAR_LITERAL,
    BOOL_LITERAL,
    VARIABLE,
    BINARY_OP,
    UNARY_OP
} expr_kind_t;

typedef enum {
    EXPR_STMT,
    VAR_DECL,
    IF_STMT,
    FOR_STMT,
    BLOCK_STMT
} stmt_kind_t;

typedef enum {
    I8,
    I16,
    I32,
    I64,
    FLOAT,
    DOUBLE,
    BOOL,
    CHAR,
    STRING,
    UNKNOWN
} expr_type_t;

typedef struct ast_exp_t        ast_exp_t;
typedef struct ast_stmt_t       ast_stmt_t;

struct token_t;

struct ast_exp_t {
    expr_kind_t                 kind;
    expr_type_t                 type_info;
    struct token_t*             token;

    union {
        struct ast_number {
            int8_t      I8;
            int16_t     I16;
            int32_t     I32;
            int64_t     I64;
            double      DOUBLE;
        } as_num;

        struct ast_string {
            char*       STRING;
        } as_str;

        struct ast_char {
            char        CHAR;
        } as_char;

        struct ast_bool {
            bool        BOOL;
        } as_bool;

        struct ast_var {
            char*       name;
        } as_var;

        struct ast_binary {
            ast_exp_t*  left;
            ast_exp_t*  right;
            const char* op;
        } as_bin;

        struct ast_unary {
            ast_exp_t*  expr;
            const char* op;
        } as_un;
    };
};

struct ast_stmt_t {
    stmt_kind_t         kind;

    union pl {
        struct {
            ast_exp_t*  exp;
        } as_expr;
    } pl;
};

ast_exp_t* new_exp(arena_t* arena, ast_exp_t exp);


#endif // CIAS_AST_H