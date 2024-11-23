
#ifndef CIAS_ASSEMBLER_H_
#define CIAS_ASSEMBLER_H_

#include "data.h"
#include "translator.h"

typedef struct asm_inst_t asm_inst_t;
typedef struct assembler_t assembler_t;

void translate_tac(arena_t* arena, assembler_t* assembler, da_func_t* funcs);
assembler_t* init_assembler(arena_t* arena);
void destroy_assembler(assembler_t* assembler);
void print_asm(FILE* out, assembler_t* assembler);
void translate_tac(arena_t* arena, assembler_t* assembler, da_func_t* funcs);

#endif // CIAS_ASSEMBLER_H_