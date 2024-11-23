
#include "vm.h"
#include "util.h"
#include "macros.h"
#include "helpers.h"
#include "native.h"
#include "allocator.h"

#include <stdbool.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <math.h>

#define EPSILON 1e-12

static int thread_counter = 1;

static inline uint32_t hash_string(const char* key, int length) {
  uint32_t hash = 2166136261u;
  for (int i = 0; i < length; i++) {
    hash ^= (uint8_t)key[i];
    hash *= 16777619;
  }
  return hash;
}

struct ciam_vm_t {
    ciam_thread_t*      threads;
    ciam_thread_t*      last_thread;
    u32                 current_thread_id;
    u32                 num_threads;
    obj_t*              heap;
    cdb_cb_t            cb;

    module_t*           module;
};

void ciam_set_cbg(ciam_vm_t* vm, cdb_cb_t cb)
{
    vm->cb = cb;
}

static inline bool is_zero(double value, double epsilon)
{
    return fabs(value) < epsilon;
}

static inline bool is_turthy(value_t val)
{
    switch (val.type)
    {
        case VAL_BOOL:      return val.as.boolean;
        case VAL_CHAR:      return val.as.chr != 0;
        case VAL_DOUBLE:    return is_zero(val.as.dbl, EPSILON);
        case VAL_FLOAT:     return is_zero(val.as.flt, EPSILON);
        case VAL_I8:        return val.as.i8  != 0;
        case VAL_I16:       return val.as.i16 != 0;
        case VAL_I32:       return val.as.i32 != 0;
        case VAL_I64:       return val.as.i64 != 0;
        case VAL_OBJECT: {
            switch (val.as.obj->obj_type)
            {
                case OBJ_STRING: return true;
                default:         return false; // TODO: Other obj type resolution might be needed
            }
        }
        break;
        default:            return false;
    }
}

#if DEBUG
static inline void dump_stack(ciam_vm_t* vm)
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
#  define X(kind, id, has_operand, label) [id] = label,
    OPCODE_LIST
#  undef X
};
#endif

#define ERROR(Msg)                  do { fprintf(stderr, Msg "\n"); return VM_ERROR; } while (false)
#define PC                          vm->threads[0].ip
#define PUSH(X)                     do { push_stack(&vm->threads[0].stack, X); vm->threads[0].sp++; } while (false)
#define POP()                       (vm->threads[0].sp--, pop_stack(&vm->threads[0].stack))
#define PEEK(idx)                   (vm->threads[0].stack.slots[vm->threads[0].sp - idx])
#define BELOW_BP(idx)               (vm->threads[0].stack.slots[vm->threads[0].bp - idx])
#define POP_TOP                     (vm->threads[0].sp--, pop_top_stack(&vm->threads[0].stack))
#define DISPATCH()                  do { if(PC >= vm->module->code_size) ERROR("Invalid address"); goto *jump_table[vm->module->code[PC].op]; } while (false)
#define CODE()                      vm->module->code[PC]
#if DEBUG
#  define PRINT_DEBUG(op)             do { printf("  0x%02lX | %-14s |\n", PC, op); dump_stack(vm); } while (false)
#  define PRINT_DEBUG_WIDE(op, opnd)  do { printf("  0x%02lX | %-14s | %ld\n", PC, op, opnd); dump_stack(vm); } while (false)
#else
#  define PRINT_DEBUG(op)             do {  } while (false)
#  define PRINT_DEBUG_WIDE(op, opnd)  do {  } while (false)
#endif
#define CURRENT_CODE                op_label[vm->module->code[PC].op]
#define LOAD()                      do {                            \
    num_const_t c = vm->module->pool.numbers.nums[code.opnd1];      \
    switch (c.type)                                                 \
    {                                                               \
        case 0:   /* BOOL */                                        \
            PUSH(BOOL_VAL((bool)c.value));                          \
            break;                                                  \
        case 1:   /* CHAR */                                        \
            PUSH(CHAR_VAL((char)c.value));                          \
            break;                                                  \
        case 2:   /* I8 */                                          \
            PUSH(I8_VAL((int8_t)c.value));                          \
            break;                                                  \
        case 3:   /* I16 */                                         \
            PUSH(I16_VAL((int16_t)c.value));                        \
            break;                                                  \
        case 4:   /* I32 */                                         \
            PUSH(I32_VAL((int32_t)c.value));                        \
            break;                                                  \
        case 5:   /* I64 */                                         \
            PUSH(I64_VAL((int64_t)c.value));                        \
            break;                                                  \
        case 6:   /* FLOAT */                                       \
            PUSH(FLOAT_VAL((float)c.value));                        \
            break;                                                  \
        case 7:   /* DOUBLE */                                      \
            PUSH(DOUBLE_VAL((double)c.value));                      \
            break;                                                  \
        default:                                                    \
            ERROR("Invalid number type");                           \
    }                                                               \
} while (false)                                                     
#define BINARY(var, val, op) do {                                   \
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
        case VAL_FLOAT:                                             \
            val op##= var##_tmp.as.flt;                             \
            break;                                                  \
        case VAL_DOUBLE:                                            \
            val op##= var##_tmp.as.dbl;                             \
            break;                                                  \
        default:                                                    \
            ERROR("Invalid number type");                           \
    }                                                               \
} while (false)
#define ADD2HEAP(heap_elem, obj_ty, stack_store) do {               \
    obj_t* obj = NULL;                                              \
    obj = (obj_t*)heap_elem;                                        \
    obj->obj_type = obj_ty;                                         \
    obj->marked = false;                                            \
    obj->next = NULL;                                               \
    if (NULL != vm->heap)                                           \
        obj->next = vm->heap;                                       \
    vm->heap = obj;                                                 \
    if (stack_store) PUSH(OBJ_VAL(obj));                            \
} while (false)
#define SLOT_FROM_BP(idx) vm->threads[0].stack.slots[vm->threads[0].bp + idx]
#define CALL_MASK   0x00FFFFFFFFFFFFFF

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
    destroy_thread(vm, 0); // TODO: When multi/thread is introduced, this needs to clean them too
    free(vm);
}

void ciam_vm_load(ciam_vm_t* vm, module_t* module)
{
    vm->module = module;
}

ciam_result_t ciam_vm_run(ciam_vm_t *vm)
{   
    static void* jump_table[] = { 
    #define X(kind, id, has_operand, label) &&OP_##kind,
        OPCODE_LIST
    #undef X
    };

    PC = vm->module->start;

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
        PRINT_DEBUG_WIDE(CURRENT_CODE, code.opnd1);
        {
            for (int64_t i = 0; i < code.opnd1; i++)
                POP();
        }
        PC++;
        DISPATCH();
    OP_TOS: // Is this necessary? Prob not.
        code = CODE();
        
        PC++;
        DISPATCH();
    OP_ADD_I8:
        code = CODE();
        {
            int64_t a_val = 0;
            int64_t b_val = 0;
            BINARY(a, a_val, +);
            BINARY(b, b_val, +);
            PUSH(I8_VAL((int8_t)(a_val + b_val)));
        }
        PRINT_DEBUG(CURRENT_CODE);
        PC++;
        DISPATCH();
    OP_ADD_I16:
        code = CODE();
        {
            int64_t a_val = 0;
            int64_t b_val = 0;
            BINARY(a, a_val, +);
            BINARY(b, b_val, +);
            PUSH(I16_VAL((int16_t)(a_val + b_val)));
        }
        PRINT_DEBUG(CURRENT_CODE);
        PC++;
        DISPATCH();
    OP_ADD_I32:
        code = CODE();
        {
            int64_t a_val = 0;
            int64_t b_val = 0;
            BINARY(a, a_val, +);
            BINARY(b, b_val, +);
            PUSH(I32_VAL((int32_t)(a_val + b_val)));
            PRINT_DEBUG(CURRENT_CODE);
        }
        PC++;
        DISPATCH();
    OP_ADD_I64:
        code = CODE();
        {
            int64_t a_val = 0;
            int64_t b_val = 0;
            BINARY(a, a_val, +);
            BINARY(b, b_val, +);
            PUSH(I64_VAL(a_val + b_val));
        }
        PRINT_DEBUG(CURRENT_CODE);
        PC++;
        DISPATCH();
    OP_ADD_F:
        code = CODE();
        {
            double a_val = 0;
            double b_val = 0;
            BINARY(a, a_val, +);
            BINARY(b, b_val, +);
            PUSH(FLOAT_VAL(a_val + b_val));
        }
        PRINT_DEBUG(CURRENT_CODE);
        PC++;
        DISPATCH();
    OP_ADD_D:
        code = CODE();
        {
            double a_val = 0;
            double b_val = 0;
            BINARY(a, a_val, +);
            BINARY(b, b_val, +);
            PUSH(DOUBLE_VAL(a_val + b_val));
        }
        PRINT_DEBUG(CURRENT_CODE);
        PC++;
        DISPATCH();
    OP_SUB_I8:
        code = CODE();
        {
            int64_t a_val = 0;
            int64_t b_val = 0;
            BINARY(a, a_val, +);
            BINARY(b, b_val, +);
            PUSH(I8_VAL((int8_t)(b_val - a_val)));
        }
        PRINT_DEBUG(CURRENT_CODE);
        PC++;
        DISPATCH();
    OP_SUB_I16:
        code = CODE();
        {
            int64_t a_val = 0;
            int64_t b_val = 0;
            BINARY(a, a_val, +);
            BINARY(b, b_val, +);
            PUSH(I16_VAL((int16_t)(b_val - a_val)));
        }
        PRINT_DEBUG(CURRENT_CODE);
        PC++;
        DISPATCH();
    OP_SUB_I32:
        code = CODE();
        {
            int64_t a_val = 0;
            int64_t b_val = 0;
            BINARY(a, a_val, +);
            BINARY(b, b_val, +);
            PUSH(I32_VAL((int32_t)(b_val - a_val)));
        }
        PRINT_DEBUG(CURRENT_CODE);
        PC++;
        DISPATCH();
    OP_SUB_I64:
        code = CODE();
        {
            int64_t a_val = 0;
            int64_t b_val = 0;
            BINARY(a, a_val, +);
            BINARY(b, b_val, +);
            PUSH(I64_VAL(b_val - a_val));
        }
        PRINT_DEBUG(CURRENT_CODE);
        PC++;
        DISPATCH();
    OP_SUB_F:
        code = CODE();
        {
            double a_val = 0;
            double b_val = 0;
            BINARY(a, a_val, +);
            BINARY(b, b_val, +);
            PUSH(FLOAT_VAL(b_val - a_val));
        }
        PRINT_DEBUG(CURRENT_CODE);
        PC++;
        DISPATCH();
    OP_SUB_D:
        code = CODE();
        {
            double a_val = 0;
            double b_val = 0;
            BINARY(a, a_val, +);
            BINARY(b, b_val, +);
            PUSH(DOUBLE_VAL(b_val - a_val));
        }
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
        PUSH(NULL_VAL(0x00));
        PRINT_DEBUG(CURRENT_CODE);
        PC++;
        DISPATCH();
    OP_LOAD_STRING:
        code = CODE();
        {
            string_const_t str = vm->module->pool.strings.strings[code.opnd1];
            obj_string_t* obj_str = ALLOCATE(obj_string_t, 1);
            obj_str->chars = ALLOCATE(char, str.length); 
            obj_str->length = str.length;
            sprintf(obj_str->chars, "%.*s", (int)str.length, str.chars);
            obj_str->hash = hash_string(str.chars, str.length);
            ADD2HEAP(obj_str, OBJ_STRING, true);            
        }
        PRINT_DEBUG(CURRENT_CODE);
        PC++;
        DISPATCH();
    OP_CALL: // This needs a refactoring to support generic call mechanism (and not just for native/core calls)
        code = CODE();
        int8_t call_idx = (int8_t)(((uint64_t)code.opnd1 & (~CALL_MASK)) >> 56); // Will be used to index the cont pool
        int64_t arg_count = code.opnd1 & CALL_MASK;
        if (call_idx >= 2)
        {
            pure_const_t pure = vm->module->pool.pures.pure_defs[call_idx - 2];
            value_t args[arg_count + 1];
            for (int64_t i = arg_count; i >= 0; i--)
                args[i] = POP();
            PUSH(I64_VAL(PC + 1));
            PUSH(I64_VAL(vm->threads[0].bp));
            vm->threads[0].bp = vm->threads[0].sp;
            for (int64_t i = arg_count - 1; i >= 0; i--)
                PUSH(args[i]);
            PC = pure.addr;
            DISPATCH();
        }
        value_t val = POP();
        if (VAL_OBJECT == val.type && OBJ_STRING == val.as.obj->obj_type)
        {
            obj_string_t* obj_str = (obj_string_t*)val.as.obj;
            native_ptr_t nat_ptr = get_native(obj_str->chars);
            if (!nat_ptr) ERROR("Unknown native! Aborting...");

            value_t vals[arg_count];

            int64_t index = 0;
            while (index < arg_count)
                vals[index++] = POP();

            value_t ret = nat_ptr(vm, vals, (size_t)arg_count);
            if (VAL_VOID != ret.type)
            {
                if (VAL_OBJECT == ret.type)
                {
                    ret.as.obj->next = vm->heap;
                    vm->heap = ret.as.obj;
                }
            }
            PUSH(ret);
        }
        PRINT_DEBUG_WIDE(CURRENT_CODE, arg_count);
        PC++;
        DISPATCH();
    OP_STORE_LOCAL:
        code = CODE();
        {
            int64_t rel_idx = code.opnd1;
            SLOT_FROM_BP(rel_idx) = PEEK(1);
        }
        PRINT_DEBUG_WIDE(CURRENT_CODE, code.opnd1);
        PC++;
        DISPATCH();
    OP_LOAD_LOCAL:
        code = CODE();
        {
            int64_t rel_idx = code.opnd1;
            PUSH(SLOT_FROM_BP(rel_idx));
        }
        PRINT_DEBUG_WIDE(CURRENT_CODE, code.opnd1);
        PC++;
        DISPATCH();
    OP_TRAP:
        code = CODE();
        CODE().op = vm->cb(vm, PC);
        DISPATCH();
    OP_HLT:
        PRINT_DEBUG(CURRENT_CODE);
        return VM_SUCCESS;
    OP_NEG:
        code = CODE();
        {
            value_t val = POP();
            PUSH(BOOL_VAL(!is_turthy(val)));
        }
        PRINT_DEBUG(CURRENT_CODE);
        PC++;
        DISPATCH();
    OP_JMP_IF_FALSE:
        code = CODE();
        PRINT_DEBUG_WIDE(CURRENT_CODE, code.opnd1);
        if (!is_turthy(PEEK(1)))
            PC = (uint64_t)code.opnd1;
        else
            PC++;
        DISPATCH();
    OP_JMP_IF_TRUE:
        code = CODE();
        PRINT_DEBUG_WIDE(CURRENT_CODE, code.opnd1);
        if (is_turthy(PEEK(1)))
            PC = (uint64_t)code.opnd1;
        else
            PC++;
        DISPATCH();
    OP_AND:
        code = CODE();
        {
            value_t a = POP();
            value_t b = POP();
            PUSH(BOOL_VAL(is_turthy(a) && is_turthy(b)));
        }
        PRINT_DEBUG(CURRENT_CODE);
        PC++;
        DISPATCH();
    OP_OR:
        code = CODE();
        {
            value_t a = POP();
            value_t b = POP();
            PUSH(BOOL_VAL(is_turthy(a) || is_turthy(b)));
        }
        PRINT_DEBUG(CURRENT_CODE);
        PC++;
        DISPATCH();
    OP_JMP:
        code = CODE();
        PRINT_DEBUG_WIDE(CURRENT_CODE, code.opnd1);
        PC = (uint64_t)code.opnd1;
        DISPATCH();
    OP_EQUALS:
        code = CODE();
        {
            int64_t a_val = 0;
            int64_t b_val = 0;
            BINARY(a, a_val, +);
            BINARY(b, b_val, +);
            PUSH(BOOL_VAL(a_val == b_val));
        }
        PRINT_DEBUG(CURRENT_CODE);
        PC++;
        DISPATCH();
    OP_NEQUALS:
        code = CODE();
        {
            int64_t a_val = 0;
            int64_t b_val = 0;
            BINARY(a, a_val, +);
            BINARY(b, b_val, +);
            PUSH(BOOL_VAL(a_val != b_val));
        }
        PRINT_DEBUG(CURRENT_CODE);
        PC++;
        DISPATCH();
    OP_LESS_THAN:
        code = CODE();
        {
            int64_t a_val = 0;
            int64_t b_val = 0;
            BINARY(a, a_val, +);
            BINARY(b, b_val, +);
            PUSH(BOOL_VAL(b_val < a_val));
        }
        PRINT_DEBUG(CURRENT_CODE);
        PC++;
        DISPATCH();
    OP_GREATER_THAN:
        code = CODE();
        {
            int64_t a_val = 0;
            int64_t b_val = 0;
            BINARY(a, a_val, +);
            BINARY(b, b_val, +);
            PUSH(BOOL_VAL(b_val > a_val));
        }
        PRINT_DEBUG(CURRENT_CODE);
        PC++;
        DISPATCH();
    OP_GT_EQ:
        code = CODE();
        {
            int64_t a_val = 0;
            int64_t b_val = 0;
            BINARY(a, a_val, +);
            BINARY(b, b_val, +);
            PUSH(BOOL_VAL(b_val >= a_val));
        }
        PRINT_DEBUG(CURRENT_CODE);
        PC++;
        DISPATCH();
    OP_LT_EQ:
        code = CODE();
        {
            int64_t a_val = 0;
            int64_t b_val = 0;
            BINARY(a, a_val, +);
            BINARY(b, b_val, +);
            PUSH(BOOL_VAL(b_val <= a_val));
        }
        PRINT_DEBUG(CURRENT_CODE);
        PC++;
        DISPATCH();
    OP_RETURN:
        code = CODE();
        PRINT_DEBUG(CURRENT_CODE);
        {
            value_t ret_val = POP();
            vm->threads[0].stack.count -= (vm->threads[0].sp - vm->threads[0].bp);
            vm->threads[0].sp = vm->threads[0].bp;
            value_t bp = POP();
            vm->threads[0].bp = bp.as.i64;
            value_t ret_addr = POP();
            PC = ret_addr.as.i64;
            PUSH(ret_val); 
        }
        DISPATCH();

    /* We shouldn't reach here, so better abort now. */
    printf("We shouldn't reach this point. Exit with error...\n");
    return VM_ERROR;
}

opcode_t ciam_trap(ciam_vm_t* vm, u64 bp_addr)
{
    opcode_t orig_op = vm->module->code[bp_addr].op;
    vm->module->code[bp_addr].op = TRAP;
    return orig_op;
}

#if DEBUG
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
#endif

#undef CALL_MASK
#undef SLOT_FROM_BP
#undef ADD2HEAP
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
#undef ERROR
