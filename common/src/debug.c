
#include "debug.h"

static const char* token_labels[] = {
#define X(type, val, label) label,
    TOKEN_LIST
#undef X
};


static const char* token_ty2label(token_ty_t type)
{
    return token_labels[type];
}

void print_header()
{
  printf("================ Launching CIAM Compiler ==================\n");
  printf("|                                                         |\n");
  printf("|                      LEXER OUTPUT                       |\n");
  printf("|                                                         |\n");
  printf("|---------------------------------------------------------|\n");
  printf("| LineNo |  ID  | TokenName            | Token Lit. Value |\n");
  printf("|---------------------------------------------------------|\n");
}

void print_tokens(token_list_t* tokens)
{
    pos_t line = -1;
    for (size_t i = 0; i < tokens->count; i++)
    {
        token_t token = tokens->data[i];
        if (token.line_no != line)
        {
            printf("| 0x%04lX |", token.line_no);
            line = token.line_no;
        }
        else
        {
            printf("|        |");
        }

        printf(" 0x%02d | %-20.20s |", token.type, token_ty2label(token.type));
        printf(" %-17.*s|\n", (int)token.length, token.start);
    }
}

void print_footer()
{
  printf("|---------------------------------------------------------|\n");
}

static char* type2string(expr_type_t type)
{
    switch (type)
    {
        case I8:      return "i8";
        case I16:     return "i16";
        case I32:     return "i32";
        case I64:     return "i64";
        case FLOAT:   return "float";
        case DOUBLE:  return "double";
        case CHAR:    return "char";
        case STRING:  return "string";
        case BOOL:    return "bool";
        case VOID:    return "void";
        default:      return "unknown";
    }
    return "unknown";
}

void print_ast_exp(FILE* out, ast_exp_t *exp)
{
    switch (exp->kind)
    {
        case NUM_LITERAL:
            if (exp->type_info == I8)
                fprintf(out, "%d", exp->as_num.I8);
            else if (exp->type_info == I16) 
                fprintf(out, "%d", exp->as_num.I16);
            else if (exp->type_info == I32) 
                fprintf(out, "%d", exp->as_num.I32);
            else if (exp->type_info == I64) 
                fprintf(out, "%ld", exp->as_num.I64);
            else if (exp->type_info == FLOAT)
                fprintf(out, "%f", exp->as_num.FLOAT);
            else if (exp->type_info == DOUBLE)
                fprintf(out, "%.2lf", (double)exp->as_num.DOUBLE);
            break;
        case STRING_LITERAL:
            fprintf(out, "\"%s\"", exp->as_str.STRING);
            break;
        case CHAR_LITERAL:
            fprintf(out, "'%c'", exp->as_char.CHAR);
            break;
        case BOOL_LITERAL:
            fprintf(out, "%s", (exp->as_bool.BOOL) ? "true" : "false");
            break;
        case NULL_LITERAL:
            fprintf(out, "null");
            break;
        case BINARY_OP:
            fprintf(out, "(");
            print_ast_exp(out, exp->as_bin.left);
            fprintf(out, " %s ", exp->as_bin.op);
            print_ast_exp(out, exp->as_bin.right);
            fprintf(out, ")");
            break;
        case CAST_BIN:
            fprintf(out, "(");
            print_ast_exp(out, exp->as_cast.exp);
            fprintf(out, " as ");
            fprintf(out, "%s", type2string(exp->as_cast.target));
            fprintf(out, ")");
            break;
        case CALLABLE:
            print_ast_exp(out, exp->as_call.callee_name);
            fprintf(out, "(");
            arg_list_t* head = exp->as_call.args;
            fprintf(out, " ");
            while (head != NULL)
            {
                print_ast_exp(out, head->exp);
                fprintf(out, " ");
                head = head->next;
            }
            fprintf(out, ")");
            break;
        case VARIABLE:
            fprintf(out, "%s", exp->as_var.name);
            break;
        case UNARY_OP:
            fprintf(out, "(%c", *exp->as_un.op);
            print_ast_exp(out, exp->as_un.expr);
            fprintf(out, ")");
            break;
        case ASSIGNMENT:
            print_ast_exp(out, exp->as_bin.left);
            fprintf(out, " = ");
            print_ast_exp(out, exp->as_bin.right);
            break;
        default:
            fprintf(out, "UNKNOWN");
            break;
    }
}

static void print_ast_stmt(FILE* out, ast_stmt_t *stmt)
{
    switch (stmt->kind)
    {
    case EXPR_STMT:
        fprintf(out, "EXP:");
        print_ast_exp(out, stmt->as_expr.exp);
        fprintf(out, "\n");
        break;
    case BLOCK_STMT:
        fprintf(out, "{\n");
        for (stmt_list_t* entry = stmt->as_block.stmts; entry != NULL; entry = entry->next)
            print_ast_stmt(out, entry->data);
        fprintf(out, "}\n");
        break;
    case LOOP_STMT:
        fprintf(out, "LOOP ");
        print_ast_exp(out, stmt->as_loop.cond);
        fprintf(out, "\n[\n");
        print_ast_stmt(out, stmt->as_loop.block);
        fprintf(out, "]\n");
        break;
    case VAR_DECL:
        fprintf(out, "VAR ");
        fprintf(out, "%s", stmt->as_decl.name);
        if (UNKNOWN != stmt->as_decl.type) fprintf(out, " : %s ", type2string(stmt->as_decl.type));
        if (NULL != stmt->as_decl.exp) 
        {
            fprintf(out, " = ");
            print_ast_exp(out, stmt->as_decl.exp);
        }
        fprintf(out, "\n");
        break;
    case IF_STMT:
        fprintf(out, "IF ");
        print_ast_exp(out, stmt->as_if.cond);
        fprintf(out, "\n[\n");
        print_ast_stmt(out, stmt->as_if.then_b);
        fprintf(out, "]\n");
        if (NULL != stmt->as_if.else_b)
        {
            fprintf(out, "ELSE\n");
            fprintf(out, "[\n");
            print_ast_stmt(out, stmt->as_if.else_b);
            fprintf(out, "]\n");
        }
        break;
    case RETURN_STMT:
        fprintf(out, "RETURN ");
        print_ast_exp(out, stmt->as_expr.exp);
        fprintf(out, "\n");
        break;
    case ENTRY_STMT:
        fprintf(out, "ENTRY \n");
        print_ast_stmt(out, stmt->as_callable.body);
        break;
    case PURE_STMT:
        fprintf(out, "PURE %s (", stmt->as_callable.name);
        if (NULL != stmt->as_callable.args)
        {
            f_arg_list_t* entry = stmt->as_callable.args;
            for (;entry != NULL; entry = entry->next)
                fprintf(out, " %s", type2string(entry->type));
            fprintf(out, " ");
        }
        fprintf(out, ") -> %s\n", type2string(stmt->as_callable.ret_type));
        print_ast_stmt(out, stmt->as_callable.body);
        break;
    default:
        fprintf(out, "Unknown statement type\n");
        exit(EXIT_FAILURE);
        break;
    }
}

void print_cu(FILE* out, cu_t* cu)
{
    printf("\n\nCompiled statements:\n");
    fprintf(out, "\n");
    if (NULL != cu->pures)
    {
        stmt_list_t* element = cu->pures;
        for (;element != NULL; element = element->next)
        {
            print_ast_stmt(out, element->data);
            fprintf(out, "\n");
        }
    }
    fprintf(out, "\n");
    if (NULL != cu->entry)
    {
        print_ast_stmt(out, cu->entry);
    }
    printf("\n");
}
