
#include "vm.h"
#include "opcode.h"
#include "reader.h"
#include "code.h"
#include "lexer.h"
#include "compiler.h"

#include "parser.h"
#include "data.h"

#include "ast.h"
#include "codec.h"

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <float.h>
#include <math.h>

void print_header()
{
  printf("================ Launching CIAM Assembler =================\n");
  printf("|                                                         |\n");
  printf("|                      LEXER OUTPUT                       |\n");
  printf("|                                                         |\n");
  printf("|---------------------------------------------------------|\n");
  printf("| LineNo |  ID  | TokenName            | Token Lit. Value |\n");
  printf("|---------------------------------------------------------|\n");
}

void print_footer()
{
  printf("|---------------------------------------------------------|\n");
}

int main(void)
{
  lexer_t lexer;
  const char* buff = "5 + 4 + 7 * 9 + 6";
  lex(&lexer, buff);
  print_footer();

  arena_t arena = {0};
  init_arena(&arena, ARENA_DEFAULT_BLOCK_SIZE);

  parser_t parser;
  init_parser(&parser, &arena, &lexer.tokens);
  ast_stmt_t* stmt = parse(&parser);

  if ( NULL != stmt->pl.as_expr.exp ) print_ast_exp(stdout, stmt->pl.as_expr.exp);
  printf("\n");

  compiler_t compiler;
  init_module(&compiler);
  compile_ast(&compiler, stmt);

  print_code(stdout, compiler.compiled_m->code, compiler.compiled_m->code_size);

  /*char tmp[2048];

  const_pool_t pool;
  pool.numbers.nums = (num_const_t*)malloc(sizeof(num_const_t) * 5);
  pool.numbers.count = 5;
  pool.numbers.nums[0].type = VAL_I8;
  pool.numbers.nums[0].value = 0xBB;

  pool.numbers.nums[1].type = VAL_I16;
  pool.numbers.nums[1].value = 0xCC;

  pool.numbers.nums[2].type = VAL_I32;
  pool.numbers.nums[2].value = 0xDD;

  pool.numbers.nums[3].type = VAL_I64;
  pool.numbers.nums[3].value = 0xEE;

  pool.numbers.nums[4].type = VAL_I64;
  pool.numbers.nums[4].value = 0xFF;

  encode(tmp, compiler.compiled_m);
  
  module_t module = {0};
  module.time_stamp = (struct tm*)malloc(sizeof(struct tm));
  module.file_name = "example.isl";
  ciam_vm_t *vm = ciam_vm_new();
  decode(tmp, &module);
  ciam_vm_load(vm, &module);
  ciam_vm_run(vm);*/
  
  return EXIT_SUCCESS;
}
