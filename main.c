
#include "vm.h"
#include "opcode.h"
#include "code.h"
#include "lexer.h"
#include "compiler.h"
#include "analyzer.h"

#include "parser.h"
#include "data.h"
#include "translator.h"

#include "ast.h"
#include "codec.h"
#include "debug.h"
#include "driver.h"

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <float.h>


int main(int argc, char** argv)
{
  driver_t driver;
  init_driver(&driver);

  char* buff = parse_args(&driver, argc, argv);

  lexer_t lexer;
#if DEBUG
  printf("\n\n");
  print_header();
#endif

  lex(&lexer, buff);

#if DEBUG
  print_tokens(&lexer.tokens);
  print_footer();
#endif

  arena_t arena = {0};
  init_arena(&arena, ARENA_DEFAULT_BLOCK_SIZE);

  parser_t parser;
  init_parser(&parser, &lexer.tokens);
  cu_t* cu = parse(&arena, &parser);

  if (NULL == cu) goto closure;

#if DEBUG
  print_cu(stdout, cu);
#endif

  analyzer_t analyzer = {0};
  init_global_scope(&analyzer);

  stmt_list_t* head = cu->pures;
  for (;head != NULL; head = head->next)
  {
    if (!check_stmt(&analyzer, head->data))
    {
      fprintf(stderr, "Semantic error in pures\n");
      exit(EXIT_FAILURE);
    }
  }

  if (!check_stmt(&analyzer, cu->entry))
  {
    fprintf(stderr, "Semantic error in entry\n");
    exit(EXIT_FAILURE);
  }

  free(analyzer.scope); // This should be dealt with later. Right now, free shit manually.

  translator_t* translator = init_translator();
  int32_t num_intsr = 0;
  tac_inst_t* instr = translate_ast(translator, cu, &num_intsr);
  print_tac(stdout, instr, num_intsr);

  compiler_t compiler;
  init_module(&compiler);
  compile_ast(&compiler, cu);

  module_t* module = transfer_module(&compiler);

#if DEBUG
  print_code(stdout, module->code, module->code_size);
#endif

  module->file_name = driver.input;
  time_t current_time;
  current_time = time(NULL);
  struct tm *tm_local = localtime(&current_time);
  module->time_stamp = tm_local;

  ciam_vm_t *vm = ciam_vm_new();
  ciam_vm_load(vm, module);
  ciam_result_t rc = ciam_vm_run(vm);

#if DEBUG
  fprintf(stdout, "VM Execution result: %s\n", (rc == 0) ? "SUCCESS" : "FAILURE");
#endif

  ciam_destroy_vm(vm);
  free(module->code);
  free(module->pool.numbers.nums);
  free(module);

closure:
  destroy_arena(&arena);
  free(lexer.tokens.data);
  
  return rc;
}
