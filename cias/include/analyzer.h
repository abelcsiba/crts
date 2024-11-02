
#ifndef CIAS_ANALYZER_H_
#define CIAS_ANALYZER_H_

#include "ast.h"

#include <stdbool.h>

typedef struct {
    cu_t*   cu;

} analyzer_t;

expr_type_t resolve_exp_type(analyzer_t* analyzer, ast_exp_t* exp);


#endif // CIAS_ANALYZER_H_