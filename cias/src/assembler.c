
#include "assembler.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Hash Table for temporary to address resolution */

typedef struct {
    const char*     key;
    void*           value;
} entry;

typedef struct table table;

struct table {
    entry*          entries;
    uint32_t        count;
    uint32_t        capacity;
};

#define INIT_CAPACITY 16

static table* create_table()
{
    table* table = malloc(sizeof(table));

    if (NULL == table)
    {
        fprintf(stderr, "Failed to allocate table\n");
        exit(EXIT_FAILURE);    
    }

    table->count = 0;
    table->capacity = INIT_CAPACITY;

    table->entries = calloc(table->capacity, sizeof(entry));
    if (NULL == table->entries)
    {
        fprintf(stderr, "Failed to allocate entries\n");
        exit(EXIT_FAILURE);
    }
    return table;
}

void destroy_table(table* table)
{
    for (uint32_t i = 0; i < table->capacity; i++) 
    {
        free((void*)table->entries[i].key);
    }

    free(table->entries);
    free(table);
}

#define FNV_OFFSET 14695981039346656037UL
#define FNV_PRIME 1099511628211UL

static uint64_t hash_key(const char* key)
{
    uint64_t hash = FNV_OFFSET;
    for (const char* p = key; *p; p++)
    {
        hash ^= (uint64_t)(unsigned char)(*p);
        hash *= FNV_PRIME;
    }
    return hash;
}

void* table_get(table* table, const char* key)
{
    uint64_t hash = hash_key(key);
    size_t index = (size_t)(hash & (uint64_t)(table->capacity - 1));

    while (table->entries[index].key != NULL)
    {
        if (strcmp(key, table->entries[index].key) == 0)
        {
            return table->entries[index].value;
        }

        index++;
        if (index >= table->capacity)
            index = 0;
    }
    return NULL;
}

static const char* table_set_entry(entry* entries, uint32_t capacity, const char* key, void* value, uint32_t* plength)
{
    uint64_t hash = hash_key(key);
    uint64_t index = (uint64_t)(hash & (uint64_t)(capacity - 1));

    while (entries[index].key != NULL) 
    {
        if (strcmp(key, entries[index].key) == 0)
        {
            entries[index].value = value;
            return entries[index].key;
        }

        index++;
        if (index >= capacity)
        {
            index = 0;
        }
    }

    if (plength != NULL)
    {
        key = strdup(key);
        if (key == NULL)
        {
            return NULL;
        }
        (*plength)++;
    }
    entries[index].key = key;
    entries[index].value = value;
    return key;
}

static bool table_expand(table* table)
{
    uint32_t new_capacity = table->capacity * 2;
    if (new_capacity < table->capacity)
        return false; // Overflow

    entry* new_entries = calloc(new_capacity, sizeof(entry));
    if (NULL == new_entries)
    {
        fprintf(stderr, "Unable to allocate room for new entries\n");
        exit(EXIT_FAILURE);
    }

    for (uint32_t i = 0; i < table->capacity; i++)
    {
        entry entry = table->entries[i];
        if (entry.key == NULL)
        {
            table_set_entry(new_entries, new_capacity, entry.key, entry.value, NULL);
        }
    }

    free(table->entries);
    table->entries = new_entries;
    table->capacity = new_capacity;
    return true;
}

const char* table_set(table* table, const char* key, void* value)
{
    if (value == NULL)
        return NULL;

    if (table->count >= table->capacity / 2)
    {
        if (!table_expand(table))
        {
            return NULL;
        }
    }

    return table_set_entry(table->entries, table->capacity, key, value, &table->count);
}

size_t table_length(table* table)
{
    return table->count;
}

/* Assembler defs */

typedef enum {
    ASM_MOV             = 0,
    ASM_UNARY           = 1,
    ASM_ALLOCATE_STACK  = 2,
    ASM_RET             = 3,
} asm_inst_kind_t;

typedef enum {
    AX                  = 0,
    R10                 = 1,
} asm_reg_t;

typedef struct {
    enum { 
        IMM, 
        REG, 
        PSEUDO, 
        STACK 
    }                   kind;
    union {
        int32_t         int_val;
        asm_reg_t       reg;
        char*           ident;
    };
} asm_oper_t;

typedef enum {
    NEG = 0,
    NOT = 1,
} asm_unary_op;

struct asm_inst_t {
    asm_inst_kind_t     kind;

    asm_oper_t          src;
    asm_oper_t          dst;
    int32_t             offset;

    asm_unary_op        unary_op;

    asm_inst_t*         next;
};

typedef struct asm_routine_t asm_routine_t;

struct asm_routine_t {
    char*               label;
    asm_inst_t*         instructions;
    asm_routine_t*      next;
};

struct assembler_t {
    asm_routine_t*      routines;
    table*              tmp_offsets;
};

void fix_tmps(assembler_t* assembler);

static asm_routine_t* add_routine(arena_t* arena, assembler_t* assembler, char* name)
{    
    if (NULL == assembler->routines) 
    {
        assembler->routines = (asm_routine_t*)arena_alloc(arena, sizeof(asm_routine_t));
        assembler->routines->label = name;
        assembler->routines->next = NULL;
        return assembler->routines;
    }

    asm_routine_t* head = assembler->routines;
    for (; head->next != NULL; head = head->next)
        ;
    
    asm_routine_t* new_item = (asm_routine_t*)arena_alloc(arena, sizeof(asm_routine_t));
    new_item->label = name;
    head->next = new_item;
    return new_item;
}

static void add_instruction(asm_routine_t* routine, asm_inst_t* new_inst)
{
    if (NULL == routine->instructions)
    {
        routine->instructions = new_inst;
    }
    else
    {
        asm_inst_t* head = routine->instructions;
        for (; head->next != NULL; head = head->next)
            ;
        head->next = new_inst;
    }
}

static asm_oper_t value2operand(tac_val_t* val)
{
    asm_oper_t oper = {0};
    if (val->kind == CONST)
    {
        oper.kind = IMM;
        oper.int_val = val->int_val;
    }
    else if (val->kind == VAR)
    {
        oper.kind = PSEUDO;
        oper.ident = val->var_name;
    }
    else
    {
        fprintf(stderr, "[ASSEMBLER] Unknown TAC value type\n");
        exit(EXIT_FAILURE);
    }
    return oper;
}

static asm_unary_op tac_unary2asm_unary(unary_kind_t tac_unary_op)
{
    if (tac_unary_op == NEGATE)
        return NEG;
    else if (tac_unary_op == COMPLEMENT)
        return NOT;
    else
    {
        fprintf(stderr, "[ASSEMBLER] Unknown TAC unary operator\n");
        exit(EXIT_FAILURE);
    }
}

static asm_inst_t* tac2asm(arena_t* arena, tac_inst_t* instr)
{
    switch (instr->kind)
    {
        case TAC_UNARY: {
            asm_oper_t src = value2operand(&instr->unary.src);
            asm_oper_t dst = value2operand(&instr->unary.dst);
            asm_inst_t* mov = (asm_inst_t*)arena_alloc(arena, sizeof(asm_inst_t));
            mov->kind = ASM_MOV;
            mov->src = src;
            mov->dst = dst;
            asm_inst_t* una = (asm_inst_t*)arena_alloc(arena, sizeof(asm_inst_t));
            una->kind = ASM_UNARY;
            una->unary_op = tac_unary2asm_unary(instr->unary.op);
            una->dst = dst;
            una->next = NULL;
            mov->next = una;
            return mov;
        }
        case TAC_RETURN: {
            asm_oper_t val = value2operand(&instr->ret);
            asm_oper_t target = {0};
            target.kind = REG;
            target.reg = AX;
            asm_inst_t* mov = (asm_inst_t*)arena_alloc(arena, sizeof(asm_inst_t));
            mov->kind = ASM_MOV;
            mov->src = val;
            mov->dst = target;
            asm_inst_t* ret = (asm_inst_t*)arena_alloc(arena, sizeof(asm_inst_t));
            ret->kind = ASM_RET;
            ret->next = NULL;
            mov->next = ret;
            return mov;
        }
        default:
            fprintf(stderr, "[ASSEMBLER] Unknown TAC instruction\n");
            exit(EXIT_FAILURE);
    }

    return NULL;
}

void translate_tac(arena_t* arena, assembler_t* assembler, da_func_t* funcs)
{
    for (int32_t i = 0; i < funcs->count; i++)
    {
        func_t* func = &funcs->funcs[i];
        asm_routine_t* new_routine = add_routine(arena, assembler, func->label);
        for (int32_t j = 0; j < func->instr.count; j++)
        {
            asm_inst_t* new_inst = tac2asm(arena, &func->instr.instr[j]);
            add_instruction(new_routine, new_inst);
        }
    }
    fix_tmps(assembler);
}

#if DEBUG

void print_asm_oper(FILE* out, asm_oper_t oper)
{
    if      (oper.kind == IMM)      fprintf(out, "Imm(%d)", oper.int_val);
    else if (oper.kind == REG)      fprintf(out, "Reg(%s)", (oper.reg == AX) ? "AX" : "R10");
    else if (oper.kind == PSEUDO)   fprintf(out, "Pseudo(%s)", oper.ident);
    else if (oper.kind == STACK)    fprintf(out, "Stack(%d)", oper.int_val);
}

void print_asm_unary(FILE* out, asm_unary_op op)
{
    if (op == NEG)
        fprintf(out, "Neg");
    else if (op == NOT)
        fprintf(out, "Not");
}

void print_asm_inst(FILE* out, asm_inst_t* instr)
{
    for (; instr != NULL; instr = instr->next)
    {
        if (instr->kind == ASM_UNARY)
        {
            fprintf(out, "\tUnary(");
            print_asm_unary(out, instr->unary_op);
            fprintf(out, ", ");
            print_asm_oper(out, instr->dst);
            fprintf(out, ")\n");
        }
        else if (instr->kind == ASM_MOV)
        {
            fprintf(out, "\tMov(");
            print_asm_oper(out, instr->src);
            fprintf(out, ", ");
            print_asm_oper(out, instr->dst);
            fprintf(out, ")\n");
        }
        else if (instr->kind == ASM_RET)
        {
            fprintf(out, "\tRet");
        }
    }
}

void fix_tmps(assembler_t* assembler)
{
    static int32_t offset_idx = -4;
    asm_routine_t* routine = assembler->routines;
    for (;routine != NULL; routine = routine->next)
    {
        asm_inst_t* asm_inst = routine->instructions;
        for (; asm_inst != NULL; asm_inst = asm_inst->next)
        {
            switch (asm_inst->kind)
            {
            case ASM_UNARY:
                if (asm_inst->dst.kind == PSEUDO)
                {
                    int32_t* addr = table_get(assembler->tmp_offsets, asm_inst->dst.ident);
                    if (!addr)
                    {
                        int32_t* offset = calloc(1, sizeof(int32_t));
                        *offset = offset_idx;
                        offset_idx -= 4;
                        table_set(assembler->tmp_offsets, asm_inst->dst.ident, offset);
                        asm_inst->dst = (asm_oper_t){ .kind = STACK, .int_val = *offset };
                    }
                    else
                    {
                        asm_inst->dst = (asm_oper_t){ .kind = STACK, .int_val = *addr };
                    }
                }
                break;
            case ASM_MOV:
                if (asm_inst->src.kind == PSEUDO)
                {
                    int32_t* addr = table_get(assembler->tmp_offsets, asm_inst->src.ident);
                    if (!addr)
                    {
                        int32_t* offset = calloc(1, sizeof(int32_t));
                        *offset = offset_idx;
                        offset_idx -= 4;
                        table_set(assembler->tmp_offsets, asm_inst->src.ident, offset);
                        asm_inst->src = (asm_oper_t){ .kind = STACK, .int_val = *offset };
                    }
                    else
                    {
                        asm_inst->src = (asm_oper_t){ .kind = STACK, .int_val = *addr };
                    }
                }
                if (asm_inst->dst.kind == PSEUDO)
                {
                    int32_t* addr = table_get(assembler->tmp_offsets, asm_inst->dst.ident);
                    if (!addr)
                    {
                        int32_t* offset = calloc(1, sizeof(int32_t));
                        *offset = offset_idx;
                        offset_idx -= 4;
                        table_set(assembler->tmp_offsets, asm_inst->dst.ident, offset);
                        asm_inst->dst = (asm_oper_t){ .kind = STACK, .int_val = *offset };
                    }
                    else
                    {
                        asm_inst->dst = (asm_oper_t){ .kind = STACK, .int_val = *addr };
                    }
                }
                break;
            case ASM_RET:

                break;
            default:
                break;
            }
        }
    }
}

void print_asm(FILE* out, assembler_t* assembler)
{
    asm_routine_t* head = assembler->routines;
    for (; head != NULL; head = head->next)
    {
        fprintf(out, "%s:\n", head->label);
        print_asm_inst(out, head->instructions);
    }
}

#endif

assembler_t* init_assembler(arena_t* arena)
{
    assembler_t* assembler = (assembler_t*)arena_alloc(arena, sizeof(assembler_t));
    assembler->tmp_offsets = create_table();
    return assembler;
}

void destroy_assembler(assembler_t* assembler)
{
    destroy_table(assembler->tmp_offsets);
}
