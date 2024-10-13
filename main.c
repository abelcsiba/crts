
#include "vm.h"
#include "opcode.h"
#include "common.h"
#include "reader.h"
#include "code.h"
#include "lexer.h"

#include "parser.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void print_header()
{
  printf("================ Launching CIAM Assembler =================\n");
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

  print_header();
  lexer_t lexer;
  const char* buff = "  3 + 42 . ; ( 53 + ident ) var - .. >= 32.1";
  lex(&lexer, buff);
  print_footer();

  parser_t parser;
  init_parser(&parser);
  parse(&parser, &lexer.tokens);

  return EXIT_SUCCESS;
}
