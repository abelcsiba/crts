
#ifndef CIAS_TRANSLATOR_H_
#define CIAS_TRANSLATOR_H_

#include "ast.h"

typedef struct tac_inst_t tac_inst_t;
typedef struct translator_t translator_t;
typedef struct da_func_t da_func_t;

// TODO: This should use an arena
da_func_t* translate_ast(translator_t* translator, cu_t* cu, int32_t* size);
translator_t* init_translator();

#if DEBUG
void print_tac(FILE* out, da_func_t* func_defs);
#endif

#endif // CIAS_TRANSLATOR_H_