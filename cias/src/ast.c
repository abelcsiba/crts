
#include "ast.h"

#include <stdlib.h>
#include <string.h>


ast_node_t* new_node(ast_node_t node)
{
    ast_node_t* ptr = (ast_node_t*)malloc(sizeof(ast_node_t));
    if (NULL != ptr) *ptr = node;

    return ptr;
}

// char* print_ast_node(ast_node_t* node)
// {
//     switch (node->kind)
//     {
//         case NUM_LITERAL:
//             return "NUM";
//         case STRING_LITERAL:
//             return "STRING";
//         case CHAR_LITERAL:
//             return "CHAR";
//         case BOOL_LITERAL:
//             return "BOOL";
//         case BINARY_OP:
//             char* left = print_ast_node(node->data.as_bin.left);
//             char* right = print_ast_node(node->data.as_bin.right);
//             char* result = (char*)malloc(strlen(left) + strlen(right) + 2);
//             memset(result, '\0', strlen(left) + strlen(right) + 2);
//             strncpy(result, left, strlen(left));
//             strncpy(result + strlen(left), "+", 1);
//             strncpy(result + strlen(left) + 1, right, strlen(right));
//             return result;
//         case UNARY_OP:
//             return "UNARY";
//         default:
//             return "UNKNOWN";
//     }
// }