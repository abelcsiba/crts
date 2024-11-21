
#ifndef CIAS_TRANSLATOR_H_
#define CIAS_TRANSLATOR_H_

#include "ast.h"

typedef struct tac_inst_t tac_inst_t;
typedef struct translator_t translator_t;

// TODO: This should use an arena
tac_inst_t* translate_ast(translator_t* translator, cu_t* cu, int32_t* size);
translator_t* init_translator();

void print_tac(FILE* out, tac_inst_t* instr, int32_t num_instr);

#endif // CIAS_TRANSLATOR_H_