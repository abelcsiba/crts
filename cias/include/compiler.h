
#ifndef CIAS_COMPILER_H_
#define CIAS_COMPILER_H_

#include "memory.h"
#include "ast.h"


#define NATIVE_LIST     \
    X("print")          \
    

typedef struct {
    module_t*   compiled_m;
    code_da*    code_da;
} compiler_t;

void        compile_ast(compiler_t* compiler, cu_t* stmt);
void        init_module(compiler_t* compiler);

module_t*   transfer_module(compiler_t* compiler);

#endif // CIAS_COMPILER_H_