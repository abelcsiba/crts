
#ifndef CIAS_PARSER_H_
#define CIAS_PARSER_H_

#include "lexer.h"
#include <stdint.h>
#include <stdio.h>

typedef struct {
    uint64_t curr; 
} parser_t;

void init_parser(parser_t* parser);
void parse(parser_t* parser, token_list_t *tokens);


#endif // CIAS_PARSER_H_