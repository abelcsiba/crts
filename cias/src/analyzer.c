
#include "analyzer.h"
#include "debug.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define     PLUS         '+'
#define     MINUS        '-'
#define     SLASH        '/'
#define     STAR         '*'
#define     MODULO       '%'
#define     BIT_AND      '&'
#define     BIT_OR       '|'
#define     LT           '<'
#define     GT           '>'
#define     XOR          '^'
#define     BANG         '!'
#define     TILDE        '~'

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
        case NUM_LITERAL:
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

void init_global_scope(analyzer_t* analyzer)
{
    analyzer->scope = (scope_t*)malloc(sizeof(scope_t));
    analyzer->scope->parent = NULL;
}

bool var_exists(scope_t* scope, char* name)
{
    for (symtable_t* sym = scope->symtable; sym != NULL; sym = sym->next)
    {
        if (strlen(sym->var_name) == strlen(name) && strcmp(sym->var_name, name) == 0)
            return true;
    }
    return false;
}

void add_to_scope(scope_t* scope, char* name, expr_type_t type)
{
    int index = 0;
    typeinfo_t tinfo = { .type = type, .mem_type = LVALUE, .is_const = false, .is_ptr = false };

    if (scope->symtable == NULL)
    {
        scope->symtable = (symtable_t*)malloc(sizeof(symtable_t));
        scope->symtable->index = index;
        scope->symtable->ty_info = tinfo;
        scope->symtable->var_name = name;
        scope->symtable->next = NULL;
        return;
    }

    symtable_t* sym = scope->symtable;
    for (; sym->next != NULL; sym = sym->next)
    {
        index++;
    }

    sym->next = (symtable_t*)malloc(sizeof(symtable_t));
    sym->next->index = index + 1;
    sym->next->ty_info = tinfo;
    sym->next->var_name = name;
    sym->next->next = NULL;
}

void free_symtable(symtable_t* symtable)
{
    symtable_t* current = symtable;
    symtable_t* next;

    while (current != NULL)
    {
        next = current->next;
        free(current); // WARNING! No not free the var_name as it comes from an arena
        current = next;
    }
}

void enter_scope(analyzer_t* analyzer)
{
    scope_t* scope = (scope_t*)malloc(sizeof(scope_t));
    memset(scope, '\0', sizeof(scope_t));
    scope->parent = analyzer->scope;
    analyzer->scope = scope;
}

void exit_scope(analyzer_t* analyzer)
{
    scope_t* scope = analyzer->scope->parent;
    free_symtable(analyzer->scope->symtable);
    free(analyzer->scope);
    analyzer->scope = scope;
}

bool check_stmt(analyzer_t* analyzer, ast_stmt_t* stmt)
{
    if (stmt == NULL) return false;

    switch (stmt->kind)
    {
        case EXPR_STMT:
            return (resolve_exp_type(analyzer, stmt->as_expr.exp) != ERROR);
        case VAR_DECL:
            expr_type_t type = resolve_exp_type(analyzer, stmt->as_decl.exp);
            if (type == ERROR) return false;
            if (type == stmt->as_decl.type) // TODO: check to possibility if implicit cast here
            {
                if (!var_exists(analyzer->scope, stmt->as_decl.name))
                {
                    add_to_scope(analyzer->scope, stmt->as_decl.name, type);
                    return true;
                }
                else
                {
                    fprintf(stderr, "Redeclaration of variable '%s'\n", stmt->as_decl.name);
                    return false;
                }
            }
            else return false;
            break;
        case IF_STMT:
            expr_type_t cond_type = resolve_exp_type(analyzer, stmt->as_if.cond);
            if (cond_type != BOOL) // TODO: consider implicit cast to bool
                return false;
            bool verdict = check_stmt(analyzer, stmt->as_if.then_b);
            if (stmt->as_if.then_b != NULL)
                verdict = verdict && check_stmt(analyzer, stmt->as_if.then_b);
            return verdict;
        case LOOP_STMT:
            return false; // TODO: implement
        case BLOCK_STMT:
            enter_scope(analyzer);
            stmt_list_t* entry = stmt->as_block.stmts;
            for (;entry != NULL; entry = entry->next)
                if (!check_stmt(analyzer, entry->data))
                    return false;
            exit_scope(analyzer);
            return true;
            break;
        case RETURN_STMT:
            return resolve_exp_type(analyzer, stmt->as_expr.exp) != ERROR;
        case ENTRY_STMT:
            return check_stmt(analyzer, stmt->as_callable.body);
        case PURE_STMT:
            return check_stmt(analyzer, stmt->as_callable.body);
        case PROC_STMT:
            return false; // TODO: implement
        default:

            break;
    }
    return false;
}