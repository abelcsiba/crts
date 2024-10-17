
#include "compiler.h"
#include "opcode.h"

static int const_index = 0;

opcode_t get_binary_opcode(const char op_c)
{
    if (op_c == '+') return OP_ADD;
    else if (op_c == '-') return OP_SUB;
    else if (op_c == '*') return OP_MUL;
    else return OP_DIV;
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
            code.op = OP_HLT;
    }
    compiler->code[compiler->count++] = code;
}

void compile_ast(compiler_t* compiler, ast_stmt_t* stmt)
{
    compile_expr(compiler, stmt->pl.as_expr.exp);
}