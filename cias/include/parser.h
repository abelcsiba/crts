
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
} parse_table_t;

ast_node_t* group(parser_t* parser, token_t token);
ast_node_t* unary(parser_t* parser, token_t token);
ast_node_t* call(parser_t* parser, ast_node_t* left, bool can_assign);
ast_node_t* invoke(parser_t* parser, ast_node_t* left, bool can_assign);
ast_node_t* binary(parser_t* parser, ast_node_t* left, bool can_assign);

static parse_table_t parse_table[] = {
    [TOKEN_LPAREN]          = { .prec = PREC_NONE,          .prefix = group,    .infix = call },
    [TOKEN_RPAREN]          = { .prec = PREC_NONE,          .prefix = NULL,     .infix = NULL },
    [TOKEN_LBRACE]          = { .prec = PREC_NONE,          .prefix = NULL,     .infix = NULL },
    [TOKEN_RBRACE]          = { .prec = PREC_NONE,          .prefix = NULL,     .infix = NULL },
    [TOKEN_COMMA]           = { .prec = PREC_NONE,          .prefix = NULL,     .infix = NULL },
    [TOKEN_DOT]             = { .prec = PREC_CALL,          .prefix = NULL,     .infix = invoke },
    [TOKEN_MINUS]           = { .prec = PREC_TERM,          .prefix = unary,    .infix = binary },
    [TOKEN_PLUS]            = { .prec = PREC_TERM,          .prefix = NULL,     .infix = binary },
    [TOKEN_SEMI]            = { .prec = PREC_NONE,          .prefix = NULL,     .infix = NULL },
    [TOKEN_SLASH]           = { .prec = PREC_FACTOR,        .prefix = NULL,     .infix = binary },
    [TOKEN_ASTERISK]        = { .prec = PREC_FACTOR,        .prefix = NULL,     .infix = binary },
    [TOKEN_BANG]            = { .prec = PREC_NONE,          .prefix = unary,    .infix = NULL },
    [TOKEN_BANG_EQUAL]      = { .prec = PREC_EQUALITY,      .prefix = NULL,     .infix = binary },
};

void init_parser(parser_t* parser, token_list_t *tokens);
void parse(parser_t* parser);

void print_ast_node(FILE* out, ast_node_t* node);


#endif // CIAS_PARSER_H_