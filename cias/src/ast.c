
#include "ast.h"

#include <stdlib.h>
#include <string.h>


ast_exp_t* new_exp(arena_t* arena, ast_exp_t exp)
{
    ast_exp_t* ptr = (ast_exp_t*)arena_alloc(arena, sizeof(ast_exp_t));
    if (NULL != ptr) 
    {
        memset(ptr, 0, sizeof(ast_exp_t));
        *ptr = exp;
    }

    return ptr;
}
