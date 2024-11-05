
#include "vm.h"
#include "opcode.h"
#include "code.h"
#include "lexer.h"
#include "compiler.h"
#include "analyzer.h"

#include "parser.h"
#include "data.h"

#include "ast.h"
#include "codec.h"
#include "debug.h"

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <float.h>
#include <math.h>


char default_source[] = "./test/expr3.isl";

int main(int argc, char** argv)
{
  char *src;
  if (argc < 2)
  {
    fprintf(stderr, "Missing input file, using default '%s'\n", default_source);
    src = default_source;
  }
  else src = argv[1];
  
  FILE* fp = fopen(src, "rb");
  if (NULL == fp)
  {
    fprintf(stderr, "Failed to open file '%s'", src);
    return EXIT_FAILURE;
  }

  fseek(fp, 0, SEEK_END);
  size_t length = ftell(fp);
  fseek(fp, 0, SEEK_SET);
  char* buff = (char*)calloc(length + 1, sizeof(char));
  fread(buff, sizeof(char), length, fp);

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

/*
  analyzer_t analyzer = {0};
  init_global_scope(&analyzer);

  check_stmt(&analyzer, cu->entry);

  free(analyzer.scope); // This should be dealt with later. Right now, free shit manually.
*/
  compiler_t compiler;
  init_module(&compiler);
  compile_ast(&compiler, cu);

  module_t* module = transfer_module(&compiler);

  //print_code(stdout, module->code, module->code_size);

  module->file_name = src;
  time_t current_time;
  current_time = time(NULL);
  struct tm *tm_local = localtime(&current_time);
  module->time_stamp = tm_local;

  ciam_vm_t *vm = ciam_vm_new();
  ciam_vm_load(vm, module);
  ciam_vm_run(vm);

  ciam_destroy_vm(vm);
  free(module->code);
  free(module->pool.numbers.nums);
  free(module);

closure:
  destroy_arena(&arena);
  free(lexer.tokens.data);
  
  fclose(fp);
  free(buff);
  
  return EXIT_SUCCESS;
}
