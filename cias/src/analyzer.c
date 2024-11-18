
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
#define     EQUAL        '='

static inline int max(int a, int b)
{
    return (a > b) ? a : b;
}

static inline bool is_num_type(expr_type_t type)
{
    return ( (type >= I8) && (type < STRING) );
}

static inline bool is_integral_type(expr_type_t type)
{
    return ( (type >= I8) && (type < FLOAT) );
}

static inline expr_type_t arith(expr_type_t left, expr_type_t right)
{
    if ( DOUBLE == left || DOUBLE == right ) return DOUBLE;
    if ( FLOAT  == left || FLOAT  == right ) return FLOAT;
    
    return max(left, right) < I32 ? I32 : max(left, right);
}

static inline expr_type_t modulo(expr_type_t left, expr_type_t right)
{
    if (!is_integral_type(left) || !is_integral_type(right))
        return ERROR;
    else
        return (max(left, right) < I32 ? I32 : max(left, right));
}

static expr_type_t resolve_bin_type(analyzer_t* /*analyzer*/, const char* op, expr_type_t lht, expr_type_t rht)
{
    if (op[0] == PLUS  || 
        op[0] == MINUS || 
        op[0] == STAR  || 
        op[0] == SLASH )         return arith(lht, rht);
    else if ( op[0] == MODULO  ) return modulo(lht, rht);
    else if ( op[0] == BIT_AND ) return (strlen(op) == 2 && op[1] == BIT_AND) ? BOOL : I64; // TODO: check for op[1] for &&
    else if ( op[0] == BIT_OR  ) return (strlen(op) == 2 && op[1] == BIT_OR) ? BOOL : I64; // TODO: check for op[1] for ||
    else if ( op[0] == LT      ) return BOOL; // TODO: proper check here, mate
    else if ( op[0] == GT      ) return BOOL; // TODO: proper check here, mate
    else if ( op[0] == XOR     ) {}
    else if ( op[0] == EQUAL   ) return ((rht <= lht) ? lht : ERROR);
    else 
    {
        fprintf(stderr, "Unknown operator '%s'\n", op);
        return ERROR;
    } // TODO: Unknown operator, burn brotha!

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

void init_global_scope(analyzer_t* analyzer)
{
    analyzer->scope = (scope_t*)calloc(1, sizeof(scope_t));
    analyzer->scope->symtable = NULL;
    analyzer->scope->parent = NULL;
}

expr_type_t var_exists(scope_t* scope, char* name)
{
    for (symtable_t* sym = scope->symtable; sym != NULL; sym = sym->next)
    {
        if (strlen(sym->var_name) == strlen(name) && strcmp(sym->var_name, name) == 0)
            return sym->ty_info.type;
    }

    if (scope->parent != NULL)
        return var_exists(scope->parent, name);

    return ERROR;
}

expr_type_t resolve_exp_type(analyzer_t* analyzer, ast_exp_t* exp)
{
#define ERROR_CHECK(X) do { if (X == ERROR) { fprintf(stderr, "Invalid type at "); print_ast_exp(stderr, exp); fprintf(stderr, "\n"); exit(EXIT_FAILURE); } } while (false)
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
            expr_type_t var_ty  = var_exists(analyzer->scope, exp->as_var.name);
            if (ERROR == var_ty)
            {
                fprintf(stderr, "Undeclared variable usage\n");
                exit(EXIT_FAILURE);
            }
            return var_ty;
        case BINARY_OP: {
            expr_type_t lht     = resolve_exp_type(analyzer, exp->as_bin.left);
            expr_type_t rht     = resolve_exp_type(analyzer, exp->as_bin.right);
            expr_type_t et_bin  = resolve_bin_type(analyzer, exp->as_bin.op, lht, rht);
            ERROR_CHECK(et_bin);
            exp->target_type = et_bin;
            return et_bin;
        }
        case ASSIGNMENT: {
            expr_type_t lht     = resolve_exp_type(analyzer, exp->as_bin.left);
            expr_type_t rht     = resolve_exp_type(analyzer, exp->as_bin.right);
            expr_type_t et_asn  = resolve_bin_type(analyzer, exp->as_bin.op, lht, rht);
            ERROR_CHECK(et_asn);
            exp->target_type = et_asn;
            return et_asn;
        }
        case UNARY_OP:
            expr_type_t et_un   = resolve_exp_type(analyzer, exp->as_un.expr);
            expr_type_t rt      = resolve_un_type(analyzer, exp->as_un.op, et_un);
            ERROR_CHECK(rt);
            return rt;
        case CALLABLE:
            expr_type_t res     = UNKNOWN;
            arg_list_t *args    = exp->as_call.args;
            while (args != NULL)
            {
                res = resolve_exp_type(analyzer, args->exp);
                if (ERROR == res) return res;
                args = args->next;
            }
            return BOOL; // TODO: Change this to the exact return type
        case CAST_BIN:
            resolve_exp_type(analyzer, exp->as_cast.exp);
            return exp->as_cast.target; // TODO: check if cast is even possible
        default:
            fprintf(stderr, "Unknown expression kind\n");
            exit(EXIT_FAILURE);
            break;
    }
#undef ERROR_CHECK
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
        free(current); // WARNING! Do not free the var_name as it comes from an arena
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
            if (type <= stmt->as_decl.type) 
            {
                if (ERROR == var_exists(analyzer->scope, stmt->as_decl.name)) // TODO: Consider shadowing
                {
                    add_to_scope(analyzer->scope, stmt->as_decl.name, stmt->as_decl.type);
                    return true;
                }
                else
                {
                    fprintf(stderr, "Redeclaration of variable '%s'\n", stmt->as_decl.name);
                    return false;
                }
            }
            else 
            {
                fprintf(stderr, "Invalid type conversion on declaration\n");
                return false; // TODO: check the possibility if implicit cast with truncating value
            }
            break;
        case IF_STMT: {
            expr_type_t cond_type = resolve_exp_type(analyzer, stmt->as_if.cond);
            if (cond_type == ERROR) // TODO: consider implicit cast to bool
                return false;
            bool verdict = check_stmt(analyzer, stmt->as_if.then_b);
            if (stmt->as_if.then_b != NULL)
                verdict = verdict && check_stmt(analyzer, stmt->as_if.then_b);
            return verdict;
        }
        case LOOP_STMT: {
            expr_type_t cond_type = resolve_exp_type(analyzer, stmt->as_loop.cond);
            if (cond_type == ERROR) // TODO: consider implicit cast to bool
                return false;
            return check_stmt(analyzer, stmt->as_loop.block);
        }
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
            enter_scope(analyzer);
            f_arg_list_t* arg = stmt->as_callable.args;
            for (;arg != NULL; arg = arg->next)
                add_to_scope(analyzer->scope, arg->name, arg->type);
            bool res = check_stmt(analyzer, stmt->as_callable.body);
            exit_scope(analyzer);
            return res;
        case PROC_STMT:
            return false; // TODO: implement
        default:
            fprintf(stderr, "Unrecognized statement\n");
            break;
    }
    return false;
}