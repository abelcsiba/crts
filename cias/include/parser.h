
#ifndef CIAS_PARSER_H_
#define CIAS_PARSER_H_

#include "lexer.h"
#include "ast.h"

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

typedef struct {
    uint64_t curr;
    bool had_error;
    token_list_t *tokens;
} parser_t;

typedef ast_node_t* (*prefix_parser_t)(parser_t*, token_t);
typedef ast_node_t* (*infix_parser_t)(parser_t*, token_t, bool);

void init_parser(parser_t* parser, token_list_t *tokens);
void parse(parser_t* parser);


#endif // CIAS_PARSER_H_