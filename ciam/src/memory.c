

#include "memory.h"

#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <assert.h>

#define STACK_MAX_SIZE 10 * 1000 * 1000 // TODO: Change hardcoded stack size to dynamic realloc

DEF_DA(value_t)

void init_stack(stack_t* stack)
{
  stack->slots = (slot*)calloc(STACK_MAX_SIZE, sizeof(slot));
  stack->count = 0;
  stack->capacity = STACK_MAX_SIZE;
}

void push_stack(stack_t *stack, value_t val)
{
  assert(stack->count < stack->capacity && "stack overflow");
  stack->slots[stack->count++] = val;
}

value_t pop_stack(stack_t* stack)
{
  assert(stack->count - 1 >= 0 && "stack underflow");
  return stack->slots[--stack->count];
}

void pop_top_stack(stack_t* stack)
{
  assert(stack->count - 1 >= 0 && "stack underflow");
  stack->count--;
}

value_t peek_stack(stack_t* stack, i64 offset)
{
  assert((stack->count - 1 - offset) >= 0 && ( offset > 0 ) && "stack out of bound");
  return stack->slots[stack->count - 1 - offset];
}
