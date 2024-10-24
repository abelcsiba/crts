
#include "compiler.h"
#include "opcode.h"

#include <string.h>

static int const_index = 0;


opcode_t get_binary_opcode(const char op_c)
{
    if (op_c == '+') return ADD;
    else if (op_c == '-') return SUB;
    else if (op_c == '*') return MUL;
    else return DIV;
}

static num_const_t emit_num_const(ast_exp_t* exp)
{
    num_const_t num = {0};
    num.type = exp->type_info;
    if (num.type == DOUBLE) memcpy(&num.value, &exp->as_num.DOUBLE, sizeof(double));
    else if (num.type == I8) memcpy(&num.value, &exp->as_num.I8, sizeof(int8_t));      
    else if (num.type == I16) memcpy(&num.value, &exp->as_num.I16, sizeof(int16_t));
    else if (num.type == I32) memcpy(&num.value, &exp->as_num.I32, sizeof(int32_t));
    else if (num.type == I64) memcpy(&num.value, &exp->as_num.I64, sizeof(int64_t));

    return num;
}

static void compile_expr(compiler_t* compiler, ast_exp_t* exp)
{
    code_t code;
    switch (exp->kind)
    {
        case NUM_LITERAL:
            code.op = LOAD_CONST;
            code.opnd1 = (opnd_t)const_index;
            compiler->compiled_m->pool.numbers.nums[const_index++] = emit_num_const(exp);
            break;
        case BINARY_OP:
            compile_expr(compiler, exp->as_bin.left);
            compile_expr(compiler, exp->as_bin.right);
            code.op = get_binary_opcode(*exp->as_bin.op);
            break;
        default:
            code.op = HLT;
    }
    add_to_code_da(compiler->code_da, code);
}

void compile_ast(compiler_t* compiler, ast_stmt_t* stmt)
{
    compile_expr(compiler, stmt->pl.as_expr.exp);
    add_to_code_da(compiler->code_da, (code_t){ .op = HLT, .opnd1 = 0x00 });
    compiler->compiled_m->code_size = compiler->code_da->count;
    compiler->compiled_m->code = compiler->code_da->data;
    compiler->compiled_m->pool.numbers.count = const_index;
    free(compiler->code_da);
}

void init_module(compiler_t* compiler)
{
    compiler->compiled_m = (module_t*)calloc(1, sizeof(module_t));
    compiler->compiled_m->pool.numbers.nums = (num_const_t*)calloc(5, sizeof(num_const_t));
    compiler->code_da = (code_da*)calloc(1, sizeof(code_da));
    init_code_da(compiler->code_da);
}

module_t* transfer_module(compiler_t* compiler)
{
    module_t* tmp = compiler->compiled_m;
    compiler->compiled_m = NULL;
    return tmp;
}