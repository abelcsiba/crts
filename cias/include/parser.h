
#ifndef CIAS_PARSER_H_
#define CIAS_PARSER_H_

#include "lexer.h"
#include "ast.h"
#include "data.h"

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

typedef struct {
    uint64_t        curr;
    bool            had_error;
    token_list_t*   tokens;
} parser_t;

typedef ast_exp_t* (*prefix_t)(arena_t*, parser_t*, token_t);
typedef ast_exp_t* (*infix_t)(arena_t*, parser_t*, ast_exp_t*, bool);

typedef enum {
    PREC_NONE = 0,          // 
    PREC_ASSIGNMENT,        // =
    PREC_TERNARY,           // ?:
    PREC_OR,                // ||
    PREC_AND,               // &&
    PREC_BIT_OR,            // |
    PREC_BIT_XOR,           // ^
    PREC_BIT_AND,           // &
    PREC_EQUALITY,          // == !=
    PREC_COMPARISON,        // < > <= >=
    PREC_SHIFT,             // << >>
    PREC_TERM,              // + -
    PREC_FACTOR,            // * /
    PREC_UNARY,             // - ! & *
    PREC_CALL,              // . ()
    PREC_PRIMARY
} precedence_t;

typedef struct {
    precedence_t    prec;
    prefix_t        prefix;
    infix_t         infix;
} parse_rule_t;

ast_exp_t*      group(arena_t* arena, parser_t* parser, token_t token);
ast_exp_t*      unary(arena_t* arena, parser_t* parser, token_t token);
ast_exp_t*      number(arena_t* arena, parser_t* parser, token_t token);
ast_exp_t*      call(arena_t* arena, parser_t* parser, ast_exp_t* left, bool can_assign);
ast_exp_t*      invoke(arena_t* arena, parser_t* parser, ast_exp_t* left, bool can_assign);
ast_exp_t*      binary(arena_t* arena, parser_t* parser, ast_exp_t* left, bool can_assign);
ast_exp_t*      ternary(arena_t* arena, parser_t* parser, ast_exp_t* left, bool can_assign);
ast_exp_t*      str_(arena_t* arena, parser_t* parser, token_t token);
ast_exp_t*      chr_(arena_t* arena, parser_t* parser, token_t token);
ast_exp_t*      variable(arena_t* arena, parser_t* parser, token_t token);
ast_exp_t*      boolean(arena_t* arena, parser_t* /*parser*/, token_t token);
ast_exp_t*      null_(arena_t* arena, parser_t* /*parser*/, token_t /*token*/);

void            init_parser(parser_t* parser, token_list_t *tokens);
ast_exp_t*      parse_expression(arena_t* arena, parser_t* parser, precedence_t precedence);
ast_stmt_t*     parse_statement(arena_t* arena, parser_t* parser);
cu_t*           parse(arena_t* arena, parser_t* parser);

#endif // CIAS_PARSER_H_