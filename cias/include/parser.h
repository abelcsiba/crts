
#ifndef CIAS_PARSER_H_
#define CIAS_PARSER_H_

#include "lexer.h"
#include "ast.h"
#include "data.h"

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

typedef struct {
    uint64_t curr;
    bool had_error;
    token_list_t *tokens;
    arena_t *arena;
} parser_t;

typedef ast_node_t* (*prefix_t)(parser_t*, token_t);
typedef ast_node_t* (*infix_t)(parser_t*, ast_node_t*, bool);

typedef enum {
    PREC_NONE,              // 
    PREC_ASSIGNMENT,        // =
    PREC_OR,                // ||
    PREC_AND,               // &&
    PREC_EQUALITY,          // == !=
    PREC_COMPARISON,        // < > <= >=
    PREC_TERM,              // + -
    PREC_FACTOR,            // * /
    PREC_UNARY,             // - !
    PREC_CALL,              // . ()
    PREC_PRIMARY
} precedence_t;

typedef struct {
    precedence_t prec;
    prefix_t prefix;
    infix_t infix;
} parse_rule_t;

ast_node_t* group(parser_t* parser, token_t token);
ast_node_t* unary(parser_t* parser, token_t token);
ast_node_t* number(parser_t* parser, token_t token);
ast_node_t* call(parser_t* parser, ast_node_t* left, bool can_assign);
ast_node_t* invoke(parser_t* parser, ast_node_t* left, bool can_assign);
ast_node_t* binary(parser_t* parser, ast_node_t* left, bool can_assign);
ast_node_t* str_(parser_t* parser, token_t token);
ast_node_t* chr_(parser_t* parser, token_t token);
ast_node_t* variable(parser_t* parser, token_t token);

void init_parser(parser_t* parser, arena_t *arena, token_list_t *tokens);
ast_node_t* parse_expression(parser_t* parser, precedence_t precedence);
ast_node_t* parse(parser_t* parser);

void print_ast_node(FILE* out, ast_node_t* node);


#endif // CIAS_PARSER_H_