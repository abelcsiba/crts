
#ifndef CIAM_MEMORY_H_
#define CIAM_MEMORY_H_

#include "value.h"
#include "code.h"

#include <time.h>
#include <stdint.h>

typedef value_t               slot;
typedef uint64_t              u64;
typedef int64_t               i64;

#define MAX_THREAD_COUNT      8
#define STACK_MAX_SIZE        10 * 1000 * 1000 // TODO: Change hardcoded stack size to dynamic realloc
#define MAX_WORKER_STACK      1000

typedef struct {
  slot*                       slots;
  i64                         count;
  i64                         capacity;
} stack_t;

typedef struct {
  char*                       file_name;
  struct tm*                  time_stamp;

  code_t*                     code;
  u64                         code_size;
  const_pool_t                pool;
} module_t;

typedef enum {
    THREAD_RUNNING,
    THREAD_BLOCKED,
    THREAD_READY,
    THREAD_TERMINATED
} thread_state_t;

typedef struct ciam_thread_t {
    uint64_t                  tid;
    thread_state_t            state;
    stack_t                   stack;
    u64                       end_addr;

    // Registers
    u64                       sp;
    u64                       bp;
    u64                       ip;

    struct ciam_thread_t*     next;
} ciam_thread_t;

void        init_stack(stack_t* stack, size_t max_size);
void        push_stack(stack_t* stack, value_t val);
value_t     pop_stack(stack_t* stack);
void        pop_top_stack(stack_t* stack);
value_t     peek_stack(stack_t* stack, i64 offset);

#endif // CIAM_MEMORY_H_
