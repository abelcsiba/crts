
#ifndef CIAS_COMPILER_H_
#define CIAS_COMPILER_H_

#include "memory.h"
#include "ast.h"

#define UINT16_COUNT (UINT16_MAX + 1)

typedef struct {
    const char* name;
    int16_t     depth;
} local_t;

typedef struct {
    module_t*   compiled_m;
    code_da*    code_da;

    local_t     locals[UINT16_COUNT];
    int64_t     local_count;
    int16_t     scope_depth;
} compiler_t;

void        compile_ast(compiler_t* compiler, cu_t* stmt);
void        init_module(compiler_t* compiler);

module_t*   transfer_module(compiler_t* compiler);

#endif // CIAS_COMPILER_H_