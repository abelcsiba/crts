
#include "vm.h"
#include "opcode.h"
#include "common.h"
#include "reader.h"
#include "code.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

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
  printf("Exiting...\n");
  return EXIT_SUCCESS;
}
