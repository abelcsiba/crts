
#ifndef CIAS_ANALYZER_H_
#define CIAS_ANALYZER_H_

#include "ast.h"
#include "data.h"

#include <stdbool.h>

typedef struct {
    expr_type_t         type;
    enum {
        LVALUE,
        RVALUE
    }                   mem_type;
    bool                is_const;
    bool                is_ptr;
} typeinfo_t;

typedef struct symtable_t {
    char*               var_name;
    typeinfo_t          ty_info;
    int                 index;
    struct symtable_t*  next;
} symtable_t;

typedef struct scope_t {
    struct scope_t*     parent;
    struct symtable_t*  symtable;
} scope_t;

typedef struct {
    cu_t*               cu;
    scope_t*            scope;
} analyzer_t;

expr_type_t resolve_exp_type(analyzer_t* analyzer, ast_exp_t* exp);
bool        check_stmt(analyzer_t* analyzer, ast_stmt_t* stmt);
void        init_global_scope(analyzer_t* analyzer);


#endif // CIAS_ANALYZER_H_