
#include "compiler.h"
#include "opcode.h"

#include <string.h>

static int const_index = 0;


opcode_t get_binary_opcode(const char op_c, ast_exp_t* exp)
{
#define BINARY(oper) do {                               \
    switch (exp->target_type)                           \
    {                                                   \
        case I8:     return oper##_I8;                  \
        case I16:    return oper##_I16;                 \
        case I32:    return oper##_I32;                 \
        case I64:    return oper##_I64;                 \
        case FLOAT:  return oper##_F;                   \
        case DOUBLE: return oper##_D;                   \
        default:                                        \
            fprintf(stderr, "We shouldn't reach here"); \
            exit(EXIT_FAILURE);                         \
            break;                                      \
    }                                                   \
} while (false)                                         \

    if      (op_c == '+') BINARY(ADD);
    else if (op_c == '-') BINARY(SUB);
    else if (op_c == '*') BINARY(MUL);
    else                  BINARY(DIV);

#undef BINARY
}

static num_const_t emit_num_const(ast_exp_t* exp)
{
    num_const_t num = {0};
    num.type = exp->type_info;
    if      (num.type == DOUBLE)    memcpy(&num.value, &exp->as_num.DOUBLE, sizeof(double));
    else if (num.type == I8)        memcpy(&num.value, &exp->as_num.I8,     sizeof(int8_t));      
    else if (num.type == I16)       memcpy(&num.value, &exp->as_num.I16,    sizeof(int16_t));
    else if (num.type == I32)       memcpy(&num.value, &exp->as_num.I32,    sizeof(int32_t));
    else if (num.type == I64)       memcpy(&num.value, &exp->as_num.I64,    sizeof(int64_t));

    return num;
}

static void compile_expr(compiler_t* compiler, ast_exp_t* exp)
{
    code_t code = {0};
    switch (exp->kind)
    {
        case NUM_LITERAL:
            code.op = LOAD_CONST;
            code.opnd1 = (opnd_t)const_index;
            compiler->compiled_m->pool.numbers.nums[const_index++] = emit_num_const(exp);
            break;
        case BOOL_LITERAL:
            code.op = LOAD_IMM;
            code.opnd1 = (opnd_t)((exp->as_bool.BOOL) ? 1 : 0);
            break;
        case NULL_LITERAL:
            code.op = LOAD_NULL;
            break;
        case BINARY_OP:
            compile_expr(compiler, exp->as_bin.left);
            compile_expr(compiler, exp->as_bin.right);
            code.op = get_binary_opcode(*exp->as_bin.op, exp);
            break;
        case CALLABLE:
            if (strcmp(exp->as_call.var_name->as_var.name, "print") == 0)
            {
                compile_expr(compiler, exp->as_call.args->exp);
                code.op = PRINT;
            }
            break;
        default:
            printf("Unrecognized expression\n");
            exit(EXIT_FAILURE);
            break;
    }
    add_to_code_da(compiler->code_da, code);
}

static void compile_stmt(compiler_t* compiler, ast_stmt_t* stmt)
{
    switch (stmt->kind)
    {
        case EXPR_STMT:
            compile_expr(compiler, stmt->as_expr.exp);
            break;
        case VAR_DECL:
            compile_expr(compiler, stmt->as_decl.exp);
            break;
        case IF_STMT:
            // TODO: IF compile
            break;
        case LOOP_STMT:
            // TODO: loop compile
            break;
        case BLOCK_STMT:
            stmt_list_t* element = stmt->as_block.stmts;
            for (;element != NULL; element = element->next)
                compile_stmt(compiler, element->data);
            break;
        case RETURN_STMT:
            compile_expr(compiler, stmt->as_expr.exp);
            break;
        case ENTRY_STMT:
            compile_stmt(compiler, stmt->as_callable.body);
            break;
        case PURE_STMT:
            compile_stmt(compiler, stmt->as_callable.body);
            break;
        case PROC_STMT:
            // TODO: PROC compile
            break;
        default:

            break;
    }
}

void compile_ast(compiler_t* compiler, cu_t* cu)
{
    if (NULL != cu->pures->data)
    {
        stmt_list_t* element = cu->pures;
        for (;element != NULL; element = element->next)
            compile_stmt(compiler, element->data);
    }

    if (NULL != cu->entry)
        compile_stmt(compiler,  cu->entry->as_callable.body);
    
    add_to_code_da(compiler->code_da, (code_t){ .op = HLT, .opnd1 = 0x00 });
    compiler->compiled_m->code_size = compiler->code_da->count;
    compiler->compiled_m->code = compiler->code_da->data;
    compiler->compiled_m->pool.numbers.count = const_index;
    free(compiler->code_da);
}

void init_module(compiler_t* compiler)
{
    compiler->compiled_m = (module_t*)calloc(1, sizeof(module_t));
    compiler->compiled_m->pool.numbers.nums = (num_const_t*)calloc(15, sizeof(num_const_t));
    compiler->code_da = (code_da*)calloc(1, sizeof(code_da));
    init_code_da(compiler->code_da);
}

module_t* transfer_module(compiler_t* compiler)
{
    module_t* tmp = compiler->compiled_m;
    compiler->compiled_m = NULL;
    return tmp;
}