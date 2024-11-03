
#ifndef CIAS_ANALYZER_H_
#define CIAS_ANALYZER_H_

#include "ast.h"

#include <stdbool.h>

typedef struct {
    expr_type_t     type;
    enum {
        LVALUE,
        RVALUE
    }               mem_t;
    bool            is_const;
    bool            is_ptr;
} typeinfo_t;

typedef struct symtable_t {
    char*           var_name;
    typeinfo_t      ty_info;
    symtable_t*     next;
} symtable_t;

typedef struct {
    cu_t*           cu;
    symtable_t*     symtable;
} analyzer_t;

expr_type_t resolve_exp_type(analyzer_t* analyzer, ast_exp_t* exp);


#endif // CIAS_ANALYZER_H_