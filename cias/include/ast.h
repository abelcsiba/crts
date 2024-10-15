
#ifndef CIAS_AST_H_
#define CIAS_AST_H_


#include <stdint.h>
#include <stddef.h>

typedef enum {
    NUM_LITERAL,
    STRING_LITERAL,
    CHAR_LITERAL,
    BOOL_LITERAL,
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

ast_node_t* new_node(ast_node_t node);


#endif // CIAS_AST_H