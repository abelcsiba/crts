
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

  char* tmp = (char*)calloc(sizeof(ciam_header_t) + 2 + (9 * 5) + 81, sizeof(char));

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

  encode(tmp, compiler.code, compiler.count, &pool);

  const_pool_t pool2;
  decode(tmp, &pool2);
  uint16_t code_length = ((ciam_header_t*)tmp)->code_size;
  fwrite(tmp, sizeof(ciam_header_t) + 2 + (9 * pool.numbers.count) + code_length, 1, fp);
  free(tmp);
  //free(pool.numbers.nums);

  // const char *horizontal = "─";
  // const char *vertical = "│";
  // const char *top_left = "┌";
  // const char *top_right = "┐";
  // const char *bottom_left = "└";
  // const char *bottom_right = "┘";

  fclose(fp);
  
  return EXIT_SUCCESS;
}
