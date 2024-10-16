
#include "vm.h"
#include "opcode.h"
#include "common.h"
#include "reader.h"
#include "code.h"
#include "lexer.h"

#include "parser.h"
#include "data.h"

#include "ast.h"

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

  print_header();
  lexer_t lexer;
  const char* buff = "  3 + 42 . ; ( 8#53 + ident ) var - .. >= 32.1";
  lex(&lexer, buff);
  print_footer();

  parser_t parser;
  init_parser(&parser, &lexer.tokens);
  parse(&parser);

  arena_t arena = {0};
  init_arena(&arena, ARENA_DEFAULT_BLOCK_SIZE);

  ast_node_t* node = new_node(&arena,
            (ast_node_t) 
            { 
              .kind = BINARY_OP, 
              .type_info = UNKNOWN, 
              .data.as_bin = (struct ast_binary)
                            { 
                              .op = "+", 
                              .left = new_node(&arena,(ast_node_t){ .kind = NUM_LITERAL, .type_info = I8, .data.as_num = (struct ast_number){ .num = 23 }}), 
                              .right = new_node(&arena,
            (ast_node_t) 
            { 
              .kind = BINARY_OP, 
              .type_info = UNKNOWN, 
              .data.as_bin = (struct ast_binary)
                            { 
                              .op = "-", 
                              .left = new_node(&arena, (ast_node_t){ .kind = NUM_LITERAL, .type_info = I8, .data.as_num = (struct ast_number){ .num = 23 }}), 
                              .right = new_node(&arena, (ast_node_t){ .kind = NUM_LITERAL, .type_info = I8, .data.as_num = (struct ast_number){ .num = 32 }})
                            }
            }
  )
                            }
            }
  );

  print_ast_node(stdout, node);
  printf("\n");
  
  return EXIT_SUCCESS;
}
