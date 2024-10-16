
#ifndef CIAS_AST_H_
#define CIAS_AST_H_

#include "data.h"

#include <stdint.h>
#include <stddef.h>

typedef enum {
    NUM_LITERAL,
    STRING_LITERAL,
    CHAR_LITERAL,
    BOOL_LITERAL,
    VARIABLE,
    BINARY_OP,
    UNARY_OP
} node_type_t;

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

typedef struct ast_node_t ast_node_t;

struct token_t;

struct ast_node_t {
    node_type_t kind;
    expr_type_t type_info;
    struct token_t *token;

    union data {
        struct ast_number {
            int64_t num;
        } as_num;

        struct ast_string {
            char* cstr;
        } as_str;

        struct ast_char {
            char c;
        } as_char;

        struct ast_var {
            char* name;
        } as_var;

        struct ast_binary {
            ast_node_t* left;
            ast_node_t* right;
            const char* op;
        } as_bin;

        struct ast_unary {
            ast_node_t* expr;
            const char* op;
        } as_un;
    } data;
};

ast_node_t* new_node(arena_t* arena, ast_node_t node);


#endif // CIAS_AST_H