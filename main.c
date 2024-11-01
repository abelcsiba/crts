
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
  printf("================ Launching CIAM Compiler ==================\n");
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

int main(int argc, char** argv)
{
  char *src;
  if (argc < 2)
  {
    fprintf(stderr, "Missing input file, using default '%s'", "./test/expr.isl");
    src = "./test/expr3.isl";
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
  //const char* buff = "5 + 4 + 7 * 9.3 + 6 / 155.67 + 14";
  printf("\n\n");
  print_header();
  lex(&lexer, buff);
  print_footer();

  arena_t arena = {0};
  init_arena(&arena, ARENA_DEFAULT_BLOCK_SIZE);

  parser_t parser;
  init_parser(&parser, &lexer.tokens);
  cu_t* cu = parse(&arena, &parser);

  if (NULL == cu) goto closure;

  printf("\n\nCompiled statements:\n");
  print_cu(stdout, cu);
  // if (stmt->kind ==  EXPR_STMT) 
  // {
  //   printf("\n\nCompiled expression: ");
  //   print_ast_exp(stdout, stmt->as_expr.exp);
  // }
  printf("\n");

  compiler_t compiler;
  init_module(&compiler);
  compile_ast(&compiler, cu->entry);

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
