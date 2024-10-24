
#ifndef CIAM_VM_H_
#define CIAM_VM_H_

#include "memory.h"
#include "code.h"

#include <stdint.h>

typedef uint64_t            pc_t;
typedef uint32_t            u32;

typedef struct ciam_vm_t    ciam_vm_t;

// VM Management
ciam_vm_t*      ciam_vm_new();
void            ciam_vm_load(ciam_vm_t* vm, module_t* module);
void            ciam_vm_run(ciam_vm_t *vm);
void            ciam_destroy_vm(ciam_vm_t* vm);

// Misc
void            display_init_message(ciam_vm_t* vm);

#endif // CIAM_VM_H_