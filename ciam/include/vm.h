
#ifndef CIAM_VM_H_
#define CIAM_VM_H_

#include "memory.h"
#include "code.h"

#include <stdint.h>

typedef uint64_t pc_t;

typedef struct {
    memory_t memory;
    pc_t pc;
} vm_t;

void init_vm(vm_t* vm, module_t* module);

void run(vm_t *vm);

void display_init_message(vm_t* vm);

#endif // CIAM_VM_H_