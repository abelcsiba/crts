
#ifndef CIAS_COMPILER_H_
#define CIAS_COMPILER_H_

#include "code.h"
#include "ast.h"

typedef struct {
    code_t code[256];
    size_t count;
} compiler_t;

void compile_ast(compiler_t* compiler, ast_stmt_t* stmt);

#endif // CIAS_COMPILER_H_