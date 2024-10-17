
#include "vm.h"
#include "opcode.h"
#include "common.h"
#include "reader.h"
#include "code.h"
#include "lexer.h"
#include "compiler.h"

#include "parser.h"
#include "data.h"

#include "ast.h"
#include "common/codec.h"

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
  char* file_name = "./test/test.raw";
  code_t *code = open_bc_source_file(file_name);
  
  vm_t vm;
  module_t module;
  time_t raw_time;
  time(&raw_time);
  module.code = code;
  module.code_size = (u64)3;
  module.file_name = file_name;
  module.time_stamp = localtime(&raw_time);
  init_vm(&vm, &module);
  run(&vm);
  printf("Exiting VM...\n\n\n");
  free(vm.memory.stack.slots);

  print_header();
  lexer_t lexer;
  // const char* buff = "-3 + 5 * 4 + 7 + \"this is a string\" + 'X' * (53 + 'a') * sum / alef";
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
  compile_ast(&compiler, stmt);

  print_code(stdout, compiler.code, compiler.count);

  destroy_arena(&arena);
  free(code);
  free(parser.tokens->data);

  FILE *fp = fopen("./test/bytecode.ciam", "wb");

  if (NULL == fp)
  {
    fprintf(stderr, "Unable to open file\n");
    return EXIT_FAILURE;
  }

  char* tmp = (char*)malloc(sizeof(ciam_header_t));

  encode(tmp, NULL);

  fwrite(tmp, sizeof(ciam_header_t), 1, fp);
  free(tmp);

  fclose(fp);
  
  return EXIT_SUCCESS;
}
