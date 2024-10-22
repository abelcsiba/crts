
#include "compiler.h"
#include "opcode.h"

static int const_index = 0;


opcode_t get_binary_opcode(const char op_c)
{
    if (op_c == '+') return ADD;
    else if (op_c == '-') return SUB;
    else if (op_c == '*') return MUL;
    else return DIV;
}

static void compile_expr(compiler_t* compiler, ast_exp_t* exp)
{
    code_t code;
    switch (exp->kind)
    {
        case NUM_LITERAL:
            code.op = LOAD_CONST;
            code.opnd1 = (opnd_t)const_index++;
            break;
        case BINARY_OP:
            compile_expr(compiler, exp->data.as_bin.left);
            compile_expr(compiler, exp->data.as_bin.right);
            code.op = get_binary_opcode(*exp->data.as_bin.op);
            break;
        default:
            code.op = HLT;
    }
    add_to_code_da(compiler->code_da, code);
}

void compile_ast(compiler_t* compiler, ast_stmt_t* stmt)
{
    compile_expr(compiler, stmt->pl.as_expr.exp);
    compiler->compiled_m->code_size = compiler->code_da->count + 1;
    compiler->compiled_m->code = (code_t*)calloc(compiler->compiled_m->code_size, sizeof(code_t));
    for (size_t i = 0; i < compiler->compiled_m->code_size - 1; i++) compiler->compiled_m->code[i] = compiler->code_da->data[i];

    compiler->compiled_m->code[compiler->compiled_m->code_size] = (code_t){ .op = HLT, .opnd1 = 0x00 };
}

void init_module(compiler_t* compiler)
{
    compiler->compiled_m = (module_t*)calloc(1, sizeof(module_t));
    compiler->code_da = (code_da*)calloc(1, sizeof(code_da));
    init_code_da(compiler->code_da);
}