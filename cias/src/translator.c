
#include "translator.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TMP_NAME_SIZE 64

static int32_t tac_id_idx = 0;

typedef enum {
    TAC_RETURN          = 0,
    TAC_UNARY           = 1,
} tac_inst_kind_t;

typedef struct {
    enum { CONST, VAR } kind;
    union {
        int32_t         int_val;
        char*           var_name;  
    };
} tac_val_t;

typedef enum {
    NEGATE          = 0,
    COMPLEMENT      = 1, 
} unary_kind_t;

struct tac_inst_t {
    tac_inst_kind_t     kind;

    union {
        tac_val_t       ret;

        struct {
            unary_kind_t op;
            tac_val_t   src;
            tac_val_t   dst;
        } unary;
    };

};

typedef struct {
    tac_inst_t*         instr;
    int32_t             count;
    int32_t             capacity;
} da_inst_t;

struct translator_t {
    da_inst_t           instr;
};

translator_t* init_translator()
{
    translator_t* translator = (translator_t*)calloc(1, sizeof(translator_t));
    translator->instr.capacity = 0;
    translator->instr.count = 0;
    translator->instr.instr = NULL;
    return translator;
}

static void add_instr(da_inst_t* da, tac_inst_t instr)
{

    if (da->count + 1 >= da->capacity)
    {
        int32_t new_capacity = (da->capacity == 0) ? 8 : da->capacity * 2;
        da->instr = (tac_inst_t*)realloc(da->instr, sizeof(tac_inst_t) * new_capacity);
        if (NULL == da->instr)
        {
            fprintf(stderr, "Unable to allocate memory\n");
            exit(EXIT_FAILURE);
        }
        da->capacity = new_capacity;
    }

    da->instr[da->count++] = instr;
}

static unary_kind_t convert_unop(const char* op)
{
    if      (strcmp(op, "-") == 0) return NEGATE;
    else if (strcmp(op, "~") == 0) return COMPLEMENT;
    else
    {
        fprintf(stderr, "[TRANSLATOR] Unknown unary operator %s\n", op);
        exit(EXIT_FAILURE);
    }
}

static char* make_tmp_name()
{
    char* tmp_name = (char*)malloc(sizeof(char) * TMP_NAME_SIZE);
    sprintf(tmp_name, "tmp.%d", tac_id_idx++);
    return tmp_name;
}

tac_val_t translate_exp(translator_t* translator, ast_exp_t* exp)
{
    switch (exp->kind)
    {
        case NUM_LITERAL: return (tac_val_t){ .kind = CONST , .int_val = exp->as_num.I8 }; // TODO: return value based on type_info
        case UNARY_OP:
            tac_val_t src = translate_exp(translator, exp->as_un.expr);
            char* dst_name = make_tmp_name();
            tac_val_t dst = (tac_val_t){ .kind = VAR, .var_name = dst_name };
            unary_kind_t op = convert_unop(exp->as_un.op);
            tac_inst_t instr = (tac_inst_t){ .kind = TAC_UNARY, .unary = { .op = op, .src = src, .dst = dst }};
            add_instr(&translator->instr, instr);
            return dst;
            break;
        default:
            fprintf(stderr, "[Translator] Unknown expression\n");
            exit(EXIT_FAILURE);
            break;
    }
} 

void translate_stmt(translator_t* translator, ast_stmt_t* stmt)
{
    switch (stmt->kind)
    {
        case RETURN_STMT: {
            tac_inst_t instr = {0};
            instr.kind = TAC_RETURN;
            instr.ret = translate_exp(translator, stmt->as_expr.exp);
            add_instr(&translator->instr, instr);
        }
        break;
        default:
            fprintf(stderr, "[TRANSLATOR] Unknown statement kind\n");
            exit(EXIT_FAILURE);
    }
}

tac_inst_t* translate_ast(translator_t* translator, cu_t* cu, int32_t* size)
{
    stmt_list_t* head = cu->entry->as_callable.body->as_block.stmts;
    for (;head != NULL; head = head->next)
    {
        translate_stmt(translator, head->data);
    }
    *size = translator->instr.count;
    return translator->instr.instr;
}

static void print_tac_val(FILE* out, tac_val_t val)
{
    switch (val.kind)
    {
        case CONST:
            fprintf(out, "Constant(%d)", val.int_val);
            break;
        case VAR:
            fprintf(out, "Var(\"%s\")", val.var_name);
            break;
        default:
            fprintf(out, "[TRANSLATOR] Unknown TAC value\n");
            exit(EXIT_FAILURE);
    }
}

static void print_instr(FILE* out, tac_inst_t* inst)
{
    switch(inst->kind)
    {
        case TAC_RETURN:
            fprintf(out, "Return(");
            print_tac_val(out, inst->ret);
            fprintf(out, ")\n");
            break;
        case TAC_UNARY:
            fprintf(out, "Unary(%s, ", (inst->unary.op == NEGATE) ? "Negate" : "Complement");
            print_tac_val(out, inst->unary.src);
            fprintf(out, ", ");
            print_tac_val(out, inst->unary.dst);
            fprintf(out, ")\n");
            break;
        default:
            fprintf(out, "[TRANSLATOR] Unknown TAC Instruction\n");
            exit(EXIT_FAILURE);
    }
}

void print_tac(FILE* out, tac_inst_t* instr, int32_t num_instr)
{
    for (int32_t i = 0; i < num_instr; i++)
    {
        print_instr(out, &instr[i]);
    }
}