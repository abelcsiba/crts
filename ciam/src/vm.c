
#include "vm.h"
#include "util.h"

#include <stdbool.h>
#include <stdio.h>


void init_vm(vm_t* vm, module_t* module)
{
    vm->memory.module = module;
    init_value_t_array_t(&vm->memory.module->constants);
    vm->pc = 0x00;
    init_stack(&vm->memory.stack);    
}

void run(vm_t *vm)
{
#define PC vm->pc
#define PUSH(X) do { push_stack(&vm->memory.stack, X); } while (false)
#define POP() pop_stack(&vm->memory.stack)
#define POP_TOP pop_top_stack(&vm->memory.stack)

    static void* jump_table[] = { 
            &&op_load_const,
            &&op_nop, 
            &&op_push,
            &&op_pop_top, 
            &&op_tos,
            &&op_add,
            &&op_sub,
            &&op_mul,
            &&op_div 
    };

    display_init_message(vm);

    while (PC < vm->memory.module->code_size)
    {
        code_t code = vm->memory.module->code[PC++];
        opcode_t op = code.op;
        goto *jump_table[op];

    op_load_const:
        PUSH(DOUBLE_VAL(code.opnd1));
        printf("LOAD_CONST %f\n", AS_DOUBLE(POP()));
        continue;
    op_nop:
        continue;

    op_push:
        PUSH(DOUBLE_VAL(code.opnd1));
        printf("OP_PUSH %f\n", AS_DOUBLE(POP()));
        continue;
    op_pop_top:
        POP();
        printf("OP_POP %ld\n", code.opnd1);
        continue;
    op_tos:
        printf("OP_TOS %ld\n", code.opnd1);
        continue;
    op_add:
        printf("OP_ADD %ld\n", code.opnd1);
        continue;
    op_sub:
        printf("OP_SUB %ld\n", code.opnd1);
        continue;
    op_mul:
        printf("OP_MUL %ld\n", code.opnd1);
        continue;
    op_div:
        printf("OP_DIV %ld\n", code.opnd1);
        continue;

    /* We encountered an unknown opcode, so we abort here. */
        printf("We shouldn't reach this point. Aborting...\n");
        exit(1);
    }

#undef POP_TOP
#undef POP
#undef PUSH
#undef PC
}

void display_init_message(vm_t* vm)
{
    printf("=========== Launching CIAM VM ===========\n");
    printf("|------------------|\n");
    printf("| Date/time        | ");
    print_time(vm->memory.module->time_stamp);
    printf("\n");
    printf("|------------------|\n");
    printf("| Const value size | %ld\n", vm->memory.module->constants.count);
    printf("|------------------|\n");
    printf("| File name:       | %s\n", vm->memory.module->file_name);
    printf("|------------------|\n");
}
