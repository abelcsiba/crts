
#include "ast.h"

#include <stdlib.h>
#include <string.h>


ast_node_t* new_node(arena_t* arena, ast_node_t node)
{
    ast_node_t* ptr = (ast_node_t*)arena_alloc(arena, sizeof(ast_node_t));
    if (NULL != ptr) *ptr = node;

    return ptr;
}
