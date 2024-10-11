
#ifndef CIAM_MEMORY_H_
#define CIAM_MEMORY_H_

#include "common.h"
#include "value.h"
#include "code.h"

#include <time.h>

typedef value_t slot;

typedef struct {
  slot* slots;
  i64 count;
  i64 capacity;
} stack_t;

DECL_DA(value_t)

typedef value_t_array_t const_da_t;

typedef struct {
  char* file_name;
  struct tm* time_stamp;

  code_t* code;
  u64 code_size;
  const_da_t constants;
} module_t;

typedef struct {
  stack_t stack;
  module_t* module;
} memory_t;

void init_stack(stack_t* stack);
void push_stack(stack_t* stack, value_t val);
value_t pop_stack(stack_t* stack);
void pop_top_stack(stack_t* stack);
value_t peek_stack(stack_t* stack, i64 offset);

#endif // CIAM_MEMORY_H_
