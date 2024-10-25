
#include "vm.h"
#include "opcode.h"
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
  const char* buff = "5 + 4 + 7 * 9.3 + 6 / 155.67 + 14";
  printf("\n\n");
  print_header();
  lex(&lexer, buff);
  print_footer();

  arena_t arena = {0};
  init_arena(&arena, ARENA_DEFAULT_BLOCK_SIZE);

  parser_t parser;
  init_parser(&parser, &lexer.tokens);
  ast_stmt_t* stmt = parse(&arena, &parser);

  if ( NULL != stmt->pl.as_expr.exp ) 
  {
    printf("\n\nCompiled expression: ");
    print_ast_exp(stdout, stmt->pl.as_expr.exp);
  }
  printf("\n");

  compiler_t compiler;
  init_module(&compiler);
  compile_ast(&compiler, stmt);

  module_t* module = transfer_module(&compiler);

  //print_code(stdout, module->code, module->code_size);

  module->file_name = "example.isl";
  struct tm ts = {0};
  module->time_stamp = &ts;

  ciam_vm_t *vm = ciam_vm_new();
  ciam_vm_load(vm, module);
  ciam_vm_run(vm);

  destroy_arena(&arena);
  free(module->code);
  free(module->pool.numbers.nums);
  free(module);
  free(lexer.tokens.data);
  ciam_destroy_vm(vm);
  
  return EXIT_SUCCESS;
}
