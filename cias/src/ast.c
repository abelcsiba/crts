
#include "ast.h"

#include <stdlib.h>
#include <string.h>


ast_node_t* new_node(ast_node_t node)
{
    ast_node_t* ptr = (ast_node_t*)malloc(sizeof(ast_node_t));
    if (NULL != ptr) *ptr = node;

    return ptr;
}
