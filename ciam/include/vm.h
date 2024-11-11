
#ifndef CIAM_VM_H_
#define CIAM_VM_H_

#include "memory.h"
#include "code.h"
#include "macros.h"

#include <stdint.h>

typedef uint64_t            pc_t;
typedef uint32_t            u32;

typedef struct ciam_vm_t    ciam_vm_t;

typedef opcode_t (*cdb_cb_t)(ciam_vm_t*);

typedef enum {
    VM_SUCCESS  = 0,
    VM_ERROR    = 1
} ciam_result_t;

// VM Management
ciam_vm_t*      ciam_vm_new();
void            ciam_vm_load(ciam_vm_t* vm, module_t* module);
ciam_result_t   ciam_vm_run(ciam_vm_t *vm);
void            ciam_destroy_vm(ciam_vm_t* vm);
opcode_t        ciam_trap(ciam_vm_t* vm, u64 bp_addr);
void            ciam_set_cbg(ciam_vm_t* vm, cdb_cb_t cb);

// Misc
void            display_init_message(ciam_vm_t* vm);

#endif // CIAM_VM_H_