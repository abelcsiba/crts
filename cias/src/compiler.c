
#include "compiler.h"
#include "opcode.h"
#include "allocator.h"

#include <string.h>

static int const_num_index = 0;
static int const_str_index = 0;

static void compile_expr(compiler_t* compiler, ast_exp_t* exp);

static void enter_scope(compiler_t* compiler)
{
    compiler->scope_depth++;
}

static void exit_scope(compiler_t* compiler)
{
    compiler->scope_depth--;

    code_t code = {0};
    int64_t elem_count = 0;
    while (compiler->local_count > 0 && compiler->locals[compiler->local_count - 1].depth > compiler->scope_depth) // TODO: change pop to take operands and do this in one step
    {
        compiler->local_count--;
        elem_count++;
    }
    code.op = POP_TOP;
    code.opnd1 = elem_count;
    add_to_code_da(compiler->code_da, code);
}

static void add_local(compiler_t* compiler, char* const name)
{
    if (compiler->local_count == UINT16_COUNT)
    {
        fprintf(stderr, "Too many local variables defined\n");
        exit(EXIT_FAILURE);
    }
    local_t* local = &compiler->locals[compiler->local_count++];
    local->name = name;
    local->depth = compiler->scope_depth;
}

static bool id_eq(const char* const a, const char* const b)
{
    if (strlen(a) != strlen(b))
        return false;

    return (memcmp(a, b, strlen(a)) == 0);
}

static void decl_var(compiler_t* compiler, char* const name)
{
    if (compiler->scope_depth == 0) return;

    for (int i = compiler->local_count - 1; i >=0 ; i--)
    {
        local_t* local = &compiler->locals[i];
        if (local->depth != -1 && local->depth < compiler->scope_depth)
        {
            break;
        }

        if (id_eq(name, local->name))
        {
            fprintf(stderr, "Variable already declared in this scope\n");
            exit(EXIT_FAILURE);
        }
    }

    add_local(compiler, name);
}

static int64_t resolve_local(compiler_t* compiler, char* const name)
{
    for (int64_t i = compiler->local_count - 1; i >=0; i--)
    {
        local_t* local = &compiler->locals[i];
        if (id_eq(name, local->name))
            return i;
    }

    return -1;
}

static int8_t resolve_pure_idx(compiler_t* compiler, const char* name) // TODO: Check for arity, type etc.
{
    for (uint64_t i = 0; i < compiler->compiled_m->pool.pures.count; i++)
    {
        if (strcmp(compiler->compiled_m->pool.pures.pure_defs[i].name, name) == 0)
            return (int8_t)i;
    }
    return -1;
}

static opcode_t get_unary_opcode(const char op_c)
{
    if ( op_c == '!' ) return NEG;
    else
    {
        fprintf(stderr, "Unary operator not yet supported\n");
        exit(EXIT_FAILURE);
    }
    return HLT; // TODO: Remove this when all unary ops are supported
}

static opcode_t get_binary_opcode(const char op_c, ast_exp_t* exp)
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
            fprintf(stderr, "We shouldn't reach here %d", exp->target_type); \
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

static inline int num_const_exist(compiler_t* compiler,ast_exp_t* exp)
{
    for (int i = 0; i < const_num_index; i++)
    {
        if (compiler->compiled_m->pool.numbers.nums[i].type == exp->type_info && memcmp(&compiler->compiled_m->pool.numbers.nums[i].value, &exp->as_num, sizeof(uint64_t)) == 0)
        {
            fprintf(stderr, "Match: %d %s\n", i, (memcmp(&compiler->compiled_m->pool.numbers.nums[i].value, &exp->as_num, sizeof(uint64_t)) == 0) ? "true" : "false");
            return i;
        }
    }
    return -1;
}

static num_const_t emit_num_const(ast_exp_t* exp)
{
    num_const_t num = {0};
    num.type = exp->type_info;

    if      ( num.type == DOUBLE )    memcpy(&num.value, &exp->as_num.DOUBLE, sizeof(double)    );
    else if ( num.type == FLOAT  )    memcpy(&num.value, &exp->as_num.FLOAT,  sizeof(float)     );
    else if ( num.type == I8     )    memcpy(&num.value, &exp->as_num.I8,     sizeof(int8_t)    );      
    else if ( num.type == I16    )    memcpy(&num.value, &exp->as_num.I16,    sizeof(int16_t)   );
    else if ( num.type == I32    )    memcpy(&num.value, &exp->as_num.I32,    sizeof(int32_t)   );
    else if ( num.type == I64    )    memcpy(&num.value, &exp->as_num.I64,    sizeof(int64_t)   );
    else if ( num.type == CHAR   )    memcpy(&num.value, &exp->as_char.CHAR,  sizeof(char)      );
    else if ( num.type == BOOL   )    memcpy(&num.value, &exp->as_bool.BOOL,  sizeof(bool)      );

    return num;
}

static string_const_t emit_string_exp(ast_exp_t* exp)
{
    string_const_t str = {0};
    str.length = strlen(exp->as_str.STRING);
    str.chars = exp->as_str.STRING; // This comes from an arena in case of vars
    return str;
}

static opcode_t compile_logic_op(compiler_t* compiler, ast_exp_t* exp)
{
    char op0 = exp->as_bin.op[0];
    char op1 = exp->as_bin.op[1];
    if ( op0 == '&' && op1 == '&' )
    {
        compile_expr(compiler, exp->as_bin.left);
        add_to_code_da(compiler->code_da, (code_t){ .op = JMP_IF_FALSE, .opnd1 = 0x00 });
        code_t* end_ptr = &compiler->code_da->data[compiler->code_da->count - 1];
        compile_expr(compiler, exp->as_bin.right);
        add_to_code_da(compiler->code_da, (code_t){ .op = AND, .opnd1 = 0x00 });
        end_ptr->opnd1 = compiler->code_da->count;
        return AND;
    }
    else if ( op0 == '|' && op1 == '|')
    {
        compile_expr(compiler, exp->as_bin.left);
        add_to_code_da(compiler->code_da, (code_t){ .op = JMP_IF_TRUE, .opnd1 = 0x00 });
        code_t* end_ptr = &compiler->code_da->data[compiler->code_da->count - 1];
        compile_expr(compiler, exp->as_bin.right);
        add_to_code_da(compiler->code_da, (code_t){ .op = OR, .opnd1 = 0x00 });
        end_ptr->opnd1 = compiler->code_da->count;
        return AND;
    }
    else if (op0 == '=' && op1 == '=')
    {
        compile_expr(compiler, exp->as_bin.left);
        compile_expr(compiler, exp->as_bin.right);
        add_to_code_da(compiler->code_da, (code_t){ .op = EQUALS, .opnd1 = 0x00 });
        return EQUALS;
    }
    else if ( op0 == '!' && op1 == '=')
    {
        compile_expr(compiler, exp->as_bin.left);
        compile_expr(compiler, exp->as_bin.right);
        add_to_code_da(compiler->code_da, (code_t){ .op = NEQUALS, .opnd1 = 0x00 });
        return NEQUALS;
    }
    else if ( op0 == '>' && op1 == '=')
    {
        compile_expr(compiler, exp->as_bin.left);
        compile_expr(compiler, exp->as_bin.right);
        add_to_code_da(compiler->code_da, (code_t){ .op = GT_EQ, .opnd1 = 0x00 });
        return GT_EQ;
    }
    else if ( op0 == '<' && op1 == '=')
    {
        compile_expr(compiler, exp->as_bin.left);
        compile_expr(compiler, exp->as_bin.right);
        add_to_code_da(compiler->code_da, (code_t){ .op = LT_EQ, .opnd1 = 0x00 });
        return GT_EQ;
    }
    else
    {
        fprintf(stderr, "Unrecognized logic operator %s\n", exp->as_bin.op);
        exit(EXIT_FAILURE);
    }

    // We shouldn't reach here, better HLT
    return HLT;
}

static void compile_expr(compiler_t* compiler, ast_exp_t* exp)
{
    code_t code = {0};
    bool skip = false;
    switch (exp->kind)
    {
        case NUM_LITERAL:
            code.op = LOAD_CONST;
            code.opnd1 = (opnd_t)const_num_index;
            compiler->compiled_m->pool.numbers.nums[const_num_index++] = emit_num_const(exp);
            break;
        case BOOL_LITERAL:
            code.op = LOAD_CONST;
            code.opnd1 = (opnd_t)const_num_index;
            compiler->compiled_m->pool.numbers.nums[const_num_index++] = emit_num_const(exp);
            break;
        case CHAR_LITERAL:
            code.op = LOAD_CONST;
            code.opnd1 = (opnd_t)const_num_index;
            compiler->compiled_m->pool.numbers.nums[const_num_index++] = emit_num_const(exp);
            break;
        case VARIABLE: {
            code.op = LOAD_LOCAL;
            int64_t local_idx = resolve_local(compiler, exp->as_var.name);
            // if (local_idx == -1) // TODO: Remove this test when the analyzer declaration check is done for funcs as well
            // {
            //     fprintf(stderr, "Cannot compile undeclared var '%s'\n", exp->as_var.name);
            //     exit(EXIT_FAILURE);
            // }
            code.opnd1 = local_idx;
            break;
        }
        case ASSIGNMENT: {
            compile_expr(compiler, exp->as_bin.right);
            code.op = STORE_LOCAL;
            int64_t local_idx = resolve_local(compiler, exp->as_bin.left->as_var.name);
            // if (local_idx == -1) // TODO: Remove this test when the analyzer declaration check is done for funcs as well
            // {
            //     fprintf(stderr, "Cannot compile undeclared var '%s'\n", exp->as_var.name);
            //     exit(EXIT_FAILURE);
            // }
            code.opnd1 = local_idx;
            break;
        }
        case NULL_LITERAL:
            code.op = LOAD_NULL;
            break;
        case STRING_LITERAL:
            code.op = LOAD_STRING;
            code.opnd1 = (opnd_t)const_str_index;
            compiler->compiled_m->pool.strings.strings[const_str_index++] = emit_string_exp(exp);
            break;
        case BINARY_OP: // TODO: Differentiate between groups of binops e.g.: arith, logic, bitwise, assign, invoke...
            if (strlen(exp->as_bin.op) == 2)
            {
                compile_logic_op(compiler, exp);
                skip = true;
            }
            else
            {
                compile_expr(compiler, exp->as_bin.left);
                compile_expr(compiler, exp->as_bin.right);
                if ( exp->as_bin.op[0] != '<' && exp->as_bin.op[0] != '>')
                    code.op = get_binary_opcode(*exp->as_bin.op, exp);
                else
                    code.op = (exp->as_bin.op[0] == '>') ? GREATER_THAN : LESS_THAN;
            }
            break;
        case UNARY_OP:
            compile_expr(compiler, exp->as_un.expr);
            code.op = get_unary_opcode(*exp->as_un.op);
            break;
        case CALLABLE: // TODO: differentiate between call types like natve, core, bc, foreign etc.
            if (strcmp(exp->as_call.callee_name->as_str.STRING, "print") == 0 ||
                strcmp(exp->as_call.callee_name->as_str.STRING, "read")  == 0)
            {
                arg_list_t* head = exp->as_call.args;
                int64_t arg_num = 0;
                while (head != NULL)
                {
                    compile_expr(compiler, head->exp);
                    head = head->next;
                    arg_num++;
                }
                compile_expr(compiler, exp->as_call.callee_name);
                code.op = CALL;
                code.opnd1 = arg_num | ((uint64_t)0x01 << 56);
            }
            else
            {
                arg_list_t* head = exp->as_call.args;
                int64_t arg_num = 0;
                while (head != NULL)
                {
                    compile_expr(compiler, head->exp);
                    head = head->next;
                    arg_num++;
                }
                compile_expr(compiler, exp->as_call.callee_name);
                code.op = CALL;
                int8_t idx = resolve_pure_idx(compiler, exp->as_call.callee_name->as_str.STRING);
                code.opnd1 = arg_num | ((uint64_t)(idx + 0b10) << 56);
            }
            break;
        case CAST_BIN:
            compile_expr(compiler, exp->as_cast.exp);
            skip = true;
            break;
        case TERNARY_OP:
            compile_expr(compiler, exp->as_ter.cond);
            add_to_code_da(compiler->code_da, (code_t){ .op = JMP_IF_FALSE, .opnd1 = 0x00 });
            code_t* else_ptr = &compiler->code_da->data[compiler->code_da->count - 1];
            add_to_code_da(compiler->code_da, (code_t){ .op = POP_TOP, .opnd1 = 0x01 });
            compile_expr(compiler, exp->as_ter.op1);
            add_to_code_da(compiler->code_da, (code_t){ .op = JMP, .opnd1 = 0x00 });
            else_ptr->opnd1 = compiler->code_da->count; // Backpatch first address
            code_t* end_ptr = &compiler->code_da->data[compiler->code_da->count - 1];
            add_to_code_da(compiler->code_da, (code_t){ .op = POP_TOP, .opnd1 = 0x01 });
            compile_expr(compiler, exp->as_ter.op2);
            end_ptr->opnd1 = compiler->code_da->count;
            skip = true;
            break;
        default:
            fprintf(stderr, "Unrecognized expression\n");
            exit(EXIT_FAILURE);
            break;
    }
    if (!skip)
        add_to_code_da(compiler->code_da, code);
}

static void define_pure(compiler_t* compiler, char* name, int arity) // TODO: Check for duplicates!
{
    if (NULL == compiler->compiled_m->pool.pures.pure_defs)
    {
        int new_capacity = GROW_CAPACITY(compiler->compiled_m->pool.pures.capacity);
        compiler->compiled_m->pool.pures.pure_defs = ALLOCATE(pure_const_t, new_capacity);
    }
    if (compiler->compiled_m->pool.pures.count + 1 >= compiler->compiled_m->pool.pures.capacity )
    {
        int new_capacity = GROW_CAPACITY(compiler->compiled_m->pool.pures.capacity);
        compiler->compiled_m->pool.pures.pure_defs = GROW_ARRAY(pure_const_t, compiler->compiled_m->pool.pures.pure_defs, compiler->compiled_m->pool.pures.capacity, new_capacity);
    }
    char* pure_name = (char*)calloc(strlen(name) + 1, sizeof(char));
    memcpy(pure_name, name, strlen(name));
    compiler->compiled_m->pool.pures.pure_defs[compiler->compiled_m->pool.pures.count++] = (pure_const_t){ .addr = compiler->code_da->count, .arity = arity, .name = pure_name };
}

static void compile_stmt(compiler_t* compiler, ast_stmt_t* stmt)
{
    switch (stmt->kind)
    {
        case EXPR_STMT:
            compile_expr(compiler, stmt->as_expr.exp);
            add_to_code_da(compiler->code_da, (code_t){ .op = POP_TOP, .opnd1 = 0x01 }); // What if this expression does not produce a value, e.g. a native call?
            break;
        case VAR_DECL:
            decl_var(compiler, stmt->as_decl.name);
            compile_expr(compiler, stmt->as_decl.exp);
            break;
        case IF_STMT:
            compile_expr(compiler, stmt->as_if.cond);
            add_to_code_da(compiler->code_da, (code_t){ .op = JMP_IF_FALSE, .opnd1 = 0x00 });
            code_t* else_ptr = &compiler->code_da->data[compiler->code_da->count - 1];
            add_to_code_da(compiler->code_da, (code_t){ .op = POP_TOP, .opnd1 = 0x01 }); // Get rid of the if condition expression result
            compile_stmt(compiler, stmt->as_if.then_b);
            if (NULL == stmt->as_if.else_b)
            {
                
                else_ptr->opnd1 = compiler->code_da->count - 1;
            }
            else
            {
                add_to_code_da(compiler->code_da, (code_t){ .op = JMP, .opnd1 = 0x00 }); // Get rid of the if condition expression result
                else_ptr->opnd1 = compiler->code_da->count;
                code_t* end_ptr = &compiler->code_da->data[compiler->code_da->count - 1];
                add_to_code_da(compiler->code_da, (code_t){ .op = POP_TOP, .opnd1 = 0x01 });
                compile_stmt(compiler, stmt->as_if.else_b);
                end_ptr->opnd1 = compiler->code_da->count;
                
            }
            break;
        case LOOP_STMT: {
            uint64_t start_addr = compiler->code_da->count;
            compile_expr(compiler, stmt->as_loop.cond);
            add_to_code_da(compiler->code_da, (code_t){ .op = JMP_IF_FALSE, .opnd1 = 0x00 });
            code_t* jmp_target = &compiler->code_da->data[compiler->code_da->count - 1];
            add_to_code_da(compiler->code_da, (code_t){ .op = POP_TOP, .opnd1 = 0x01 });
            compile_stmt(compiler, stmt->as_loop.block);
            add_to_code_da(compiler->code_da, (code_t){ .op = JMP, .opnd1 = (int64_t)start_addr });
            add_to_code_da(compiler->code_da, (code_t){ .op = POP_TOP, .opnd1 = 0x01 });
            jmp_target->opnd1 = compiler->code_da->count - 1;
        }
        break;
        case BLOCK_STMT:
            stmt_list_t* element = stmt->as_block.stmts;
            enter_scope(compiler);
            for (;element != NULL; element = element->next)
                compile_stmt(compiler, element->data);
            exit_scope(compiler);
            break;
        case RETURN_STMT: // TODO: expression result should be saved so that after the return, it is on stack top
            compile_expr(compiler, stmt->as_expr.exp);
            add_to_code_da(compiler->code_da, (code_t){ .op = RETURN, .opnd1 = 0x00 });
            break;
        case ENTRY_STMT:
            compile_stmt(compiler, stmt->as_callable.body);
            break;
        case PURE_STMT:
            define_pure(compiler, stmt->as_callable.name, stmt->as_callable.arity);
            enter_scope(compiler);
            f_arg_list_t* arg = stmt->as_callable.args;
            for (;arg != NULL; arg = arg->next)
                decl_var(compiler, arg->name);
            //add_to_code_da(compiler->code_da, (code_t){ .op = LOAD_PARAMS, .opnd1 = stmt->as_callable.arity });
            compile_stmt(compiler, stmt->as_callable.body);
            exit_scope(compiler);
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
    if (NULL != cu->pures)
    {
        stmt_list_t* element = cu->pures;
        for (;element != NULL; element = element->next)
            compile_stmt(compiler, element->data);
    }

    if (NULL != cu->entry)
    {
        compiler->compiled_m->start = compiler->code_da->count;
        compile_stmt(compiler,  cu->entry->as_callable.body);
    }
    
    add_to_code_da(compiler->code_da, (code_t){ .op = HLT, .opnd1 = 0x00 });
    compiler->compiled_m->code_size = compiler->code_da->count;
    compiler->compiled_m->code = compiler->code_da->data;
    compiler->compiled_m->pool.numbers.count = const_num_index;
    compiler->compiled_m->pool.strings.count = const_str_index;
    free(compiler->code_da);
}

void init_module(compiler_t* compiler)
{
    compiler->compiled_m = (module_t*)calloc(1, sizeof(module_t));
    compiler->compiled_m->pool.numbers.nums = (num_const_t*)calloc(30, sizeof(num_const_t)); // TODO: hardcoded. fix it!
    compiler->compiled_m->pool.strings.strings = (string_const_t*)calloc(30, sizeof(string_const_t)); // TODO: hardcoded. fix it!
    compiler->local_count = 0;
    compiler->scope_depth = 0;
    compiler->code_da = (code_da*)calloc(1, sizeof(code_da));
    init_code_da(compiler->code_da);
}

module_t* transfer_module(compiler_t* compiler)
{
    module_t* tmp = compiler->compiled_m;
    compiler->compiled_m = NULL;
    return tmp;
}