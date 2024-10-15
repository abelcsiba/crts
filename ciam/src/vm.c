
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
#define DISPATCH() do { if(vm->pc >= vm->memory.module->code_size) return; goto *jump_table[vm->memory.module->code[PC].op]; } while (false)
#define CODE() vm->memory.module->code[PC]
    
    static void* jump_table[] = { 
            &&op_load_const,
            &&op_nop, 
            &&op_push,
            &&op_pop_top, 
            &&op_tos,
            &&op_add,
            &&op_sub,
            &&op_mul,
            &&op_div,
            &&op_hlt
    };

    display_init_message(vm);
    code_t code;
    DISPATCH();

    op_load_const:
        code = CODE();
        PUSH(DOUBLE_VAL(code.opnd1));
        printf("LOAD_CONST %f\n", AS_DOUBLE(POP()));
        vm->pc++;
        DISPATCH();
    op_nop:
        DISPATCH();
    op_push:
        code = CODE();
        PUSH(DOUBLE_VAL(code.opnd1));
        printf("OP_PUSH %f\n", AS_DOUBLE(POP()));
        vm->pc++;
        DISPATCH();
    op_pop_top:
        code = CODE();
        POP();
        printf("OP_POP %ld\n", code.opnd1);
        vm->pc++;
        DISPATCH();
    op_tos:
        code = CODE();
        printf("OP_TOS %ld\n", code.opnd1);
        vm->pc++;
        DISPATCH();
    op_add:
        code = CODE();
        printf("OP_ADD %ld\n", code.opnd1);
        vm->pc++;
        DISPATCH();
    op_sub:
        code = CODE();
        printf("OP_SUB %ld\n", code.opnd1);
        vm->pc++;
        DISPATCH();
    op_mul:
        code = CODE();
        printf("OP_MUL %ld\n", code.opnd1);
        vm->pc++;
        DISPATCH();
    op_div:
        code = CODE();
        printf("OP_DIV %ld\n", code.opnd1);
        vm->pc++;
        DISPATCH();
    op_hlt:
        return;

    /* We shouldn't reach here, so better abort now. */
    printf("We shouldn't reach this point. Aborting...\n");
    exit(1);

#undef CODE
#undef DISPATCH
#undef POP_TOP
#undef POP
#undef PUSH
#undef PC
}

void display_init_message(vm_t* vm)
{
    printf("\n\n");
    printf("=========== Launching CIAM VM ===============\n");
    printf("|------------------|------------------------|\n");
    printf("| Date/time        | ");
    char buffer[100] = {0};
    print_time(buffer, vm->memory.module->time_stamp);
    printf("%-22s |", buffer);
    printf("\n");
    printf("|------------------|------------------------|\n");
    printf("| Const value size | %-22ld |\n", vm->memory.module->constants.count);
    printf("|------------------|------------------------|\n");
    printf("| File name:       | %-22s |\n", vm->memory.module->file_name);
    printf("|------------------|------------------------|\n");
    printf("\n\n");
}
