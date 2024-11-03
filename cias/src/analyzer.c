
#include "analyzer.h"
#include "debug.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static enum {
    PLUS        = '+',
    MINUS       = '-',
    SLASH       = '/',
    STAR        = '*',
    MODULO      = '%',
    BIT_AND     = '&',
    BIT_OR      = '|',
    LT          = '<',
    GT          = '>',
    XOR         = '^',
    BANG        = '!',
    TILDE       = '~'
};


int max(int a, int b)
{
    return (a > b) ? a : b;
}

bool is_num_type(expr_type_t type)
{
    return ( (type >= I8) && (type < BOOL) );
}

bool is_integral_type(expr_type_t type)
{
    return ( (type >= I8) && (type < FLOAT) );
}

expr_type_t resolve_bin_type(analyzer_t* /*analyzer*/, const char* op, expr_type_t lht, expr_type_t rht)
{
    size_t len = strlen(op);

    if (len == 1)
    {
        if (op[0] == PLUS || op[0] == MINUS || op[0] == STAR)
        {
            if (is_num_type(lht) && is_num_type(rht)) return max(lht, rht);
            return ERROR;
        }
        if (op[0] == SLASH)
        {
            if (is_num_type(lht) && is_num_type(rht)) return max(lht, rht);
            return ERROR;
        }
    }
    else if (len == 2)
    {

    }
    else
    {
        return ERROR;
    }
    // We shouldn't reach here
    return ERROR;
}

expr_type_t resolve_un_type(analyzer_t* /*analyzer*/, const char* op, expr_type_t exp)
{
    if ( op[0] == MINUS || op[0] == PLUS )
    {
        if (is_num_type(exp)) return exp;
        return ERROR;
    }
    else if ( op[0] == BANG)
    {
        return BOOL;
    }
    else if ( op[0] == TILDE)
    {
        if (is_integral_type(exp)) return exp;
        return ERROR;
    }
    else
    {
        fprintf(stderr, "Unknown unary operator\n");
        return ERROR;
    }
}

expr_type_t resolve_exp_type(analyzer_t* analyzer, ast_exp_t* exp)
{
#define ERROR_CHECK(X) do { if (X == ERROR) { fprintf(stderr, "Invalid type at "); print_ast_exp(stderr, exp); exit(EXIT_FAILURE); } } while (false)
    switch (exp->kind)
    {
        case NULL_LITERAL:
            return exp->type_info;
        case STRING_LITERAL:
            return STRING;
            break;
        case CHAR_LITERAL:
            return CHAR;
        case BOOL_LITERAL:
            return BOOL;
        case VARIABLE:
            // TODO: lookup variable
            return I8;
        case BINARY_OP:
            expr_type_t lht     = resolve_exp_type(analyzer, exp->as_bin.left);
            expr_type_t rht     = resolve_exp_type(analyzer, exp->as_bin.right);
            expr_type_t et_bin  = resolve_bin_type(analyzer, exp->as_bin.op, lht, rht);
            ERROR_CHECK(et_bin);
            return et_bin;
        case UNARY_OP:
            expr_type_t et_un   = resolve_exp_type(analyzer, exp->as_un.expr);
            expr_type_t rt      = resolve_un_type(analyzer, exp->as_un.op, et_un);
            ERROR_CHECK(rt);
            return rt;
        default:
            fprintf(stderr, "Unknown expression kind\n");
            exit(EXIT_FAILURE);
            break;
    }
#undef ERROR_CHECK
}