
#include "vm.h"
#include "util.h"
#include "macros.h"
#include "helpers.h"

#include <stdbool.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

static int thread_counter = 1;

struct ciam_vm_t {
    ciam_thread_t*      threads;
    ciam_thread_t*      last_thread;
    u32                 current_thread_id;
    u32                 num_threads;
    obj_t*              heap;

    module_t*           module;
};

#if DEBUG
static void dump_stack(ciam_vm_t* vm)
{
    int64_t index = vm->threads[0].stack.count - 1;
    printf("        _________________  \n");
    printf("       |   STACK BEGIN   |  \n");
    while (index >= 0)
    {
        printf("  0x%02lX |     ", index);
        print_value(vm->threads[0].stack.slots[index]);
        printf("\n");
        index--;
    }
    printf("       |XXXXXXXXXXXXXXXXX|  \n");
    printf("\n");
}
#endif

#if DEBUG
static char* op_label[] = {
#define X(kind, id, has_operand, label) [id] = label,
    OPCODE_LIST
#undef X
};
#endif

#define PC                          vm->threads[0].ip
#define PUSH(X)                     do { push_stack(&vm->threads[0].stack, X); vm->threads[0].sp++; } while (false)
#define POP()                       pop_stack(&vm->threads[0].stack)
#define DEC_SP(X)                   do { vm->threads[0].sp -= X; } while (false)
#define POP_TOP                     pop_top_stack(&vm->threads[0].stack)
#define DISPATCH()                  do { if(PC >= vm->module->code_size) return; goto *jump_table[vm->module->code[PC].op]; } while (false)
#define CODE()                      vm->module->code[PC]
#if DEBUG
#define PRINT_DEBUG(op)             do { printf("  0x%02lX | %-14s |\n", PC, op); dump_stack(vm); } while(false)
#define PRINT_DEBUG_WIDE(op, opnd)  do { printf("  0x%02lX | %-14s | %ld\n", PC, op, opnd); dump_stack(vm); } while (false)
#else
# define PRINT_DEBUG(op)             do {  } while(false)
# define PRINT_DEBUG_WIDE(op, opnd)  do {  } while (false)
#endif
#define CURRENT_CODE                op_label[vm->module->code[PC].op]
#define LOAD()                      do {                            \
    num_const_t c = vm->module->pool.numbers.nums[code.opnd1];      \
    switch (c.type)                                                 \
    {                                                               \
        case 0:                                                     \
            PUSH(I8_VAL((int8_t)c.value));                          \
            break;                                                  \
        case 1:                                                     \
            PUSH(I16_VAL((int16_t)c.value));                        \
            break;                                                  \
        case 2:                                                     \
            PUSH(I32_VAL((int32_t)c.value));                        \
            break;                                                  \
        case 3:                                                     \
            PUSH(I64_VAL((int64_t)c.value));                        \
            break;                                                  \
        case 4:                                                     \
            PUSH(FLOAT_VAL((float)c.value));                        \
            break;                                                  \
        case 5:                                                     \
            PUSH(DOUBLE_VAL((double)c.value));                      \
            break;                                                  \
        case 6:                                                     \
            PUSH(BOOL_VAL((bool)c.value));                          \
            break;                                                  \
        case 7:                                                     \
            PUSH(CHAR_VAL((char)c.value));                          \
            break;                                                  \
        default:                                                    \
            fprintf(stderr, "Invalid num type\n");                  \
            exit(1);                                                \
    }                                                               \
} while (false)                                                     
#define BINARY_I(var, val, op) do {                                 \
    value_t var##_tmp = POP();                                      \
    switch (var##_tmp.type)                                         \
    {                                                               \
        case VAL_I8:                                                \
            val op##= var##_tmp.as.i8;                              \
            break;                                                  \
        case VAL_I16:                                               \
            val op##= var##_tmp.as.i16;                             \
            break;                                                  \
        case VAL_I32:                                               \
            val op##= var##_tmp.as.i32;                             \
            break;                                                  \
        case VAL_I64:                                               \
            val op##= var##_tmp.as.i64;                             \
            break;                                                  \
        default:                                                    \
            fprintf(stderr, "Invalid num type\n");                  \
            exit(1);                                                \
    }                                                               \
} while (false)                                                     \


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
    if (vm->threads->next == NULL)
    {
        free(vm->threads->stack.slots);
        free(vm->threads);
        return;
    }

    ciam_thread_t* prev = vm->threads;
    ciam_thread_t* thread = prev->next;
    while (thread != NULL && thread->tid != tid) {
        prev = thread;
        thread = thread->next;
    }

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

void ciam_destroy_vm(ciam_vm_t* vm)
{
    destroy_thread(vm, 0);
    free(vm);
}

void ciam_vm_load(ciam_vm_t* vm, module_t* module)
{
    vm->module = module;
}

void ciam_vm_run(ciam_vm_t *vm)
{   
    static void* jump_table[] = { 
    #define X(kind, id, has_operand, label) &&OP_##kind,
        OPCODE_LIST
    #undef X
    };

    if (DEBUG)
        display_init_message(vm);
    
    code_t code;
    DISPATCH();

    OP_LOAD_CONST:
        code = CODE();
        LOAD();
        PRINT_DEBUG_WIDE(CURRENT_CODE, code.opnd1);
        PC++;
        DISPATCH();
    OP_NOP:
        PC++;
        DISPATCH();
    OP_PUSH: // Is this necessary? If so, I would prob need typed instructions.
        code = CODE();
        PRINT_DEBUG(CURRENT_CODE);
        PUSH(DOUBLE_VAL(code.opnd1));
        PC++;
        DISPATCH();
    OP_POP_TOP:
        code = CODE();
        PRINT_DEBUG(CURRENT_CODE);
        POP();
        DEC_SP(1);
        PC++;
        DISPATCH();
    OP_TOS: // Is this necessary? Prob not.
        code = CODE();
        PRINT_DEBUG(CURRENT_CODE);
        PC++;
        DISPATCH();
    OP_ADD_I8:
        code = CODE();
        {
            int64_t a_val = 0;
            int64_t b_val = 0;
            BINARY_I(a, a_val, +);
            BINARY_I(b, b_val, +);
            PUSH(I8_VAL((int8_t)(a_val + b_val)));
            DEC_SP(1);
        }
        PRINT_DEBUG(CURRENT_CODE);
        PC++;
        DISPATCH();
    OP_ADD_I16:
        code = CODE();
        {
            int64_t a_val = 0;
            int64_t b_val = 0;
            BINARY_I(a, a_val, +);
            BINARY_I(b, b_val, +);
            PUSH(I16_VAL((int16_t)(a_val + b_val)));
            DEC_SP(1);
        }
        PRINT_DEBUG(CURRENT_CODE);
        PC++;
        DISPATCH();
    OP_ADD_I32:
        code = CODE();
        {
            int64_t a_val = 0;
            int64_t b_val = 0;
            BINARY_I(a, a_val, +);
            BINARY_I(b, b_val, +);
            PUSH(I32_VAL((int32_t)(a_val + b_val)));
            DEC_SP(1);
            PRINT_DEBUG(CURRENT_CODE);
        }
        PC++;
        DISPATCH();
    OP_ADD_I64:
        code = CODE();
        {
            int64_t a_val = 0;
            int64_t b_val = 0;
            BINARY_I(a, a_val, +);
            BINARY_I(b, b_val, +);
            PUSH(I64_VAL(a_val + b_val));
            DEC_SP(1);
        }
        PRINT_DEBUG(CURRENT_CODE);
        PC++;
        DISPATCH();
    OP_ADD_F:
        code = CODE();
        PRINT_DEBUG(CURRENT_CODE);
        PC++;
        DISPATCH();
    OP_ADD_D:
        code = CODE();
        PRINT_DEBUG(CURRENT_CODE);
        PC++;
        DISPATCH();
    OP_SUB_I8:
        code = CODE();
        {
            int64_t a_val = 0;
            int64_t b_val = 0;
            BINARY_I(a, a_val, +);
            BINARY_I(b, b_val, +);
            PUSH(I8_VAL((int8_t)(b_val - a_val)));
            DEC_SP(1);
        }
        PRINT_DEBUG(CURRENT_CODE);
        PC++;
        DISPATCH();
    OP_SUB_I16:
        code = CODE();
        {
            int64_t a_val = 0;
            int64_t b_val = 0;
            BINARY_I(a, a_val, +);
            BINARY_I(b, b_val, +);
            PUSH(I16_VAL((int16_t)(b_val - a_val)));
            DEC_SP(1);
        }
        PRINT_DEBUG(CURRENT_CODE);
        PC++;
        DISPATCH();
    OP_SUB_I32:
        code = CODE();
        {
            int64_t a_val = 0;
            int64_t b_val = 0;
            BINARY_I(a, a_val, +);
            BINARY_I(b, b_val, +);
            PUSH(I32_VAL((int32_t)(b_val - a_val)));
            DEC_SP(1);
        }
        PRINT_DEBUG(CURRENT_CODE);
        PC++;
        DISPATCH();
    OP_SUB_I64:
        code = CODE();
        {
            int64_t a_val = 0;
            int64_t b_val = 0;
            BINARY_I(a, a_val, +);
            BINARY_I(b, b_val, +);
            PUSH(I64_VAL(b_val - a_val));
            DEC_SP(1);
        }
        PRINT_DEBUG(CURRENT_CODE);
        PC++;
        DISPATCH();
    OP_SUB_F:
        code = CODE();
        PRINT_DEBUG(CURRENT_CODE);
        PC++;
        DISPATCH();
    OP_SUB_D:
        code = CODE();
        PRINT_DEBUG(CURRENT_CODE);
        PC++;
        DISPATCH();
    OP_MUL_I8:
        code = CODE();
        PRINT_DEBUG(CURRENT_CODE);
        PC++;
        DISPATCH();
    OP_MUL_I16:
        code = CODE();
        PRINT_DEBUG(CURRENT_CODE);
        PC++;
        DISPATCH();
    OP_MUL_I32:
        code = CODE();
        PRINT_DEBUG(CURRENT_CODE);
        PC++;
        DISPATCH();
    OP_MUL_I64:
        code = CODE();
        PRINT_DEBUG(CURRENT_CODE);
        PC++;
        DISPATCH();
    OP_MUL_F:
        code = CODE();
        PRINT_DEBUG(CURRENT_CODE);
        PC++;
        DISPATCH();
    OP_MUL_D:
        code = CODE();
        PRINT_DEBUG(CURRENT_CODE);
        PC++;
        DISPATCH();
    OP_DIV_I8:
        code = CODE();
        PRINT_DEBUG(CURRENT_CODE);
        PC++;
        DISPATCH();
    OP_DIV_I16:
        code = CODE();
        PRINT_DEBUG(CURRENT_CODE);
        PC++;
        DISPATCH();
    OP_DIV_I32:
        code = CODE();
        PRINT_DEBUG(CURRENT_CODE);
        PC++;
        DISPATCH();
    OP_DIV_I64:
        code = CODE();
        PRINT_DEBUG(CURRENT_CODE);
        PC++;
        DISPATCH();
    OP_DIV_F:
        code = CODE();
        PRINT_DEBUG(CURRENT_CODE);
        PC++;
        DISPATCH();
    OP_DIV_D:
        code = CODE();
        PRINT_DEBUG(CURRENT_CODE);
        PC++;
        DISPATCH();
    OP_LOAD_IMM:
        code = CODE();
        PRINT_DEBUG_WIDE(CURRENT_CODE, code.opnd1);
        PC++;
        DISPATCH();
    OP_LOAD_NULL:
        code = CODE();
        PRINT_DEBUG(CURRENT_CODE);
        PC++;
        DISPATCH();
    OP_PRINT: // TODO: This is quite clumsy right now. Need to figure out a better way to print multiple values.
        code = CODE();
        {
            int64_t index = code.opnd1 - 1;
            while (index >= 0)
            {
                value_t val = POP();
                if (index != (int64_t)(code.opnd1 - 1)) printf(" ");
                print_value(val);
                DEC_SP(1);
                index--;
            }
            printf("\n");
        }
        PRINT_DEBUG(CURRENT_CODE);
        PC++;
        DISPATCH();
    OP_LOAD_STRING:
        code = CODE();
        string_const_t str = vm->module->pool.strings.strings[code.opnd1];
        obj_string_t* obj_str = (obj_string_t*)malloc(sizeof(obj_string_t));
        obj_str->chars = (char*)malloc(sizeof(char) * str.length);
        memcpy(obj_str->chars, str.chars, str.length);
        obj_str->chars[str.length] = '\0';
        obj_t* obj = (obj_t*)obj_str;
        obj->obj_type = OBJ_STRING;
        obj->marked = false;
        obj->next = NULL;
        PUSH(OBJ_VAL(obj));
        PRINT_DEBUG(CURRENT_CODE);
        PC++;
        DISPATCH();
    OP_HLT:
        PRINT_DEBUG(CURRENT_CODE);
        return;

    /* We shouldn't reach here, so better abort now. */
    printf("We shouldn't reach this point. Aborting...\n");
    exit(EXIT_FAILURE);
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
    char name[200];
    memset(name, '\0', 200);
    sprintf(name, "%d.%d.%d-%d", CIAM_VER_MAJOR, CIAM_VER_MINOR, CIAM_VER_PATCH, CIAM_VER_REV);
    printf("| CIAM Version     | %-22.20s |\n", name);
    printf("|------------------|------------------------|\n");
    printf("| Const value size | %-22d |\n", vm->module->pool.numbers.count);
    printf("|------------------|------------------------|\n");
    printf("| File name:       | %-22.20s |\n", vm->module->file_name);
    printf("|------------------|------------------------|\n");
    printf("| Code size:       | %-22ld |\n", vm->module->code_size);
    printf("|------------------|------------------------|\n");
    printf("\n\n");
    printf("  Addr | CIAM Instruct. | Operand\n");
    printf("---------------------------------\n");
}

#undef BINARY_I
#undef LOAD
#undef CURRENT_CODE
#undef PRINT_DEBUG_WIDE
#undef PRINT_DEBUG
#undef CODE
#undef DISPATCH
#undef POP_TOP
#undef DEC_SP
#undef POP
#undef PUSH
#undef PC
