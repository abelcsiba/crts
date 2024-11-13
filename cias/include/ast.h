
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
    ASSIGNMENT,
    UNARY_OP,
    CALLABLE
} expr_kind_t;

typedef enum {
    EXPR_STMT,
    VAR_DECL,
    IF_STMT,
    LOOP_STMT,
    BLOCK_STMT,
    RETURN_STMT,
    ENTRY_STMT,
    PURE_STMT,
    PROC_STMT
} stmt_kind_t;

typedef enum {
    BOOL,
    CHAR,
    I8,
    I16,
    I32,
    I64,
    FLOAT,
    DOUBLE,
    STRING,
    VOID,
    UNKNOWN,
    ERROR
} expr_type_t;

typedef struct ast_exp_t    ast_exp_t;
typedef struct ast_stmt_t   ast_stmt_t;

struct token_t;

typedef struct arg_list_t {
    ast_exp_t*              exp;
    struct arg_list_t*      next;
} arg_list_t;

struct ast_exp_t {
    expr_kind_t             kind;
    expr_type_t             type_info;
    struct token_t*         token;
    expr_type_t             target_type;

    union {
        struct ast_number {
            int8_t          I8;
            int16_t         I16;
            int32_t         I32;
            int64_t         I64;
            double          DOUBLE;
            float           FLOAT;
        } as_num;

        struct ast_string {
            char*           STRING;
        } as_str;

        struct ast_char {
            char            CHAR;
        } as_char;

        struct ast_bool {
            bool            BOOL;
        } as_bool;

        struct ast_var {
            char*           name;
        } as_var;

        struct ast_binary {
            ast_exp_t*      left;
            ast_exp_t*      right;
            const char*     op;
        } as_bin;

        struct ast_ternary {
            ast_exp_t*      cond;
            ast_exp_t*      op1;
            ast_exp_t*      op2;
        } as_ter;

        struct ast_unary {
            ast_exp_t*      expr;
            const char*     op;
        } as_un;

        struct ast_callable {
            arg_list_t*     args;
            ast_exp_t*      callee_name;
            expr_type_t     ret_type;
        } as_call;
    };
};

typedef struct stmt_list_t stmt_list_t;

struct stmt_list_t {
    ast_stmt_t*             data;
    stmt_list_t*            next;
};

typedef struct f_arg_list_t f_arg_list_t;

struct f_arg_list_t {
    expr_type_t             type;
    char*                   name;
    f_arg_list_t*           next;
};

typedef enum {
    LIBRARY,
    EXECUTABLE
} cu_type_t;

typedef struct {
    cu_type_t               type;
    stmt_list_t*            pures;
    ast_stmt_t*             entry;
} cu_t;

struct ast_stmt_t {
    stmt_kind_t             kind;

    union {
        struct {
            expr_type_t     ret_type;
            size_t          arity;
            f_arg_list_t*   args;
            ast_stmt_t*     body;
            char*           name;
        } as_callable;

        struct {
            ast_exp_t*      exp;
        } as_expr;

        struct {
            char*           name;
            expr_type_t     type;
            ast_exp_t*      exp;
            int             index;
        } as_decl;

        struct {
            ast_exp_t*      cond;
            ast_stmt_t*     block;
        } as_loop;

        struct {
            ast_exp_t*      cond;
            ast_stmt_t*     then_b;
            ast_stmt_t*     else_b;
        } as_if;

        struct {
            stmt_list_t*    stmts;
        } as_block;
    };
};

ast_exp_t* new_exp(arena_t* arena, ast_exp_t exp);

#endif // CIAS_AST_H
