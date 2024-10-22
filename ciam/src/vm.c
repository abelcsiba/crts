
#include "vm.h"
#include "util.h"

#include <stdbool.h>
#include <stdio.h>
#include <assert.h>


static int thread_counter = 1;

struct ciam_vm_t {
    ciam_thread_t*      threads;
    ciam_thread_t*      last_thread;
    u32                 current_thread_id;
    u32                 num_threads;

    module_t*           module;
};

static void init_main_thread(ciam_vm_t* vm)
{
#define MAIN_THREAD vm->threads
    MAIN_THREAD = (ciam_thread_t*)calloc(1, sizeof(ciam_thread_t));
    MAIN_THREAD->ip = 0x00;
    MAIN_THREAD->sp = 0x00;
    MAIN_THREAD->bp = 0x00;
    MAIN_THREAD->state = THREAD_RUNNING;
    MAIN_THREAD->tid = 0x00;
    MAIN_THREAD->end_addr = 0x00;
    init_stack(&MAIN_THREAD->stack, STACK_MAX_SIZE);
    vm->last_thread = MAIN_THREAD;

#undef MAIN_THREAD
}

// TODO: Incorporate thread handling when groundwork is done
static void __attribute__((unused)) create_thread(ciam_vm_t* vm, u64 entry_addr, u64 end_addr)
{
    ciam_thread_t* curr_thread = vm->last_thread;
    curr_thread->next = (ciam_thread_t*)calloc(1, sizeof(ciam_thread_t));
    curr_thread = curr_thread->next;
    vm->num_threads++;
    curr_thread->ip = entry_addr;
    curr_thread->sp = 0x00;
    curr_thread->bp = 0x00;
    curr_thread->state = THREAD_READY;
    curr_thread->tid = thread_counter++;
    curr_thread->end_addr = end_addr;
    init_stack(&curr_thread->stack, MAX_WORKER_STACK);
    vm->last_thread = curr_thread;
}

// TODO: Incorporate thread handling when groundwork is done
static void __attribute__((unused)) destroy_thread(ciam_vm_t* vm, u64 tid)
{
    ciam_thread_t* prev = vm->threads;
    ciam_thread_t* thread = prev->next;
    do {
        prev = thread;
        thread = thread->next;
    } while (thread != NULL && thread->tid != tid);

    assert(thread != NULL && "Cannot find thread");
    prev->next = thread->next;
    free(thread->stack.slots);
    free(thread);
    vm->num_threads--;
}

ciam_vm_t* ciam_vm_new()
{
    ciam_vm_t* vm = (ciam_vm_t*)calloc(1, sizeof(ciam_vm_t));
    vm->module = NULL;
    vm->current_thread_id = 0;
    vm->num_threads = 1;
    init_main_thread(vm);
    return vm;
}

void ciam_vm_load(ciam_vm_t* vm, module_t* module)
{
    vm->module = module;
}

void ciam_vm_run(ciam_vm_t *vm)
{
#define PC vm->threads[0].ip
#define PUSH(X) do { push_stack(&vm->threads[0].stack, X); } while (false)
#define POP() pop_stack(&vm->threads[0].stack)
#define POP_TOP pop_top_stack(&vm->threads[0].stack)
#define DISPATCH() do { if(PC >= vm->module->code_size) return; goto *jump_table[vm->module->code[PC].op]; } while (false)
#define CODE() vm->module->code[PC]
    
    static void* jump_table[] = { 
    #define X(kind, id, has_operand) &&OP_##kind,
        OPCODE_LIST
    #undef X
    };

    display_init_message(vm);
    code_t code;
    DISPATCH();

    OP_LOAD_CONST:
        code = CODE();
        PUSH(DOUBLE_VAL(code.opnd1));
        printf("LOAD_CONST %ld\n", code.opnd1);
        PC++;
        DISPATCH();
    OP_NOP:
        DISPATCH();
    OP_PUSH:
        code = CODE();
        PUSH(DOUBLE_VAL(code.opnd1));
        printf("OP_PUSH %ld\n", code.opnd1);
        PC++;
        DISPATCH();
    OP_POP_TOP:
        code = CODE();
        //POP();
        printf("OP_POP %ld\n", code.opnd1);
        PC++;
        DISPATCH();
    OP_TOS:
        code = CODE();
        printf("OP_TOS %ld\n", code.opnd1);
        PC++;
        DISPATCH();
    OP_ADD:
        code = CODE();
        printf("OP_ADD %ld\n", code.opnd1);
        PC++;
        DISPATCH();
    OP_SUB:
        code = CODE();
        printf("OP_SUB %ld\n", code.opnd1);
        PC++;
        DISPATCH();
    OP_MUL:
        code = CODE();
        printf("OP_MUL %ld\n", code.opnd1);
        PC++;
        DISPATCH();
    OP_DIV:
        code = CODE();
        printf("OP_DIV %ld\n", code.opnd1);
        PC++;
        DISPATCH();
    OP_HLT:
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

void display_init_message(ciam_vm_t* vm)
{
    printf("\n\n");
    printf("=========== Launching CIAM VM ===============\n");
    printf("|------------------|------------------------|\n");
    printf("| Date/time        | ");
    char buffer[100] = {0};
    print_time(buffer, vm->module->time_stamp);
    printf("%-22s |", buffer);
    printf("\n");
    printf("|------------------|------------------------|\n");
    printf("| Const value size | %-22d |\n", vm->module->pool.numbers.count);
    printf("|------------------|------------------------|\n");
    printf("| File name:       | %-22s |\n", vm->module->file_name);
    printf("|------------------|------------------------|\n");
    printf("\n\n");
}
