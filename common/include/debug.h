
#ifndef COMMON_DEBUG_H_
#define COMMON_DEBUG_H_

#include "ast.h"
#include "lexer.h"
#include "macros.h"

#include <stdio.h>
#include <stdlib.h>


void print_header();
void print_footer();

void print_tokens(token_list_t* tokens);

void print_cu(FILE* out, cu_t* cu);
void print_ast_exp(FILE* out, ast_exp_t *exp);

#endif // COMMON_DEBUG_H_