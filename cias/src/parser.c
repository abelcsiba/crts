
#include "parser.h"

#include <string.h>

static parse_rule_t parse_table[] = {
    [TOKEN_ASTERISK]        = { .prec = PREC_FACTOR,        .prefix = NULL,     .infix = binary     },
    [TOKEN_AND]             = { .prec = PREC_AND,           .prefix = NULL,     .infix = binary     },
    [TOKEN_BANG]            = { .prec = PREC_NONE,          .prefix = unary,    .infix = NULL       },
    [TOKEN_BANG_EQUAL]      = { .prec = PREC_EQUALITY,      .prefix = NULL,     .infix = binary     },
    [TOKEN_BIT_AND]         = { .prec = PREC_BIT_AND,       .prefix = NULL,     .infix = binary     },
    [TOKEN_BIT_OR]          = { .prec = PREC_BIT_OR,        .prefix = NULL,     .infix = binary     },
    [TOKEN_CHAR]            = { .prec = PREC_NONE,          .prefix = chr_,     .infix = NULL       },
    [TOKEN_COMMA]           = { .prec = PREC_NONE,          .prefix = NULL,     .infix = NULL       },
    [TOKEN_DOT]             = { .prec = PREC_CALL,          .prefix = NULL,     .infix = invoke     },
    [TOKEN_EOF]             = { .prec = PREC_NONE,          .prefix = NULL,     .infix = NULL       },
    [TOKEN_EQUAL]           = { .prec = PREC_NONE,          .prefix = NULL,     .infix = NULL       },
    [TOKEN_EQUAL_EQUAL]     = { .prec = PREC_EQUALITY,      .prefix = NULL,     .infix = binary     },
    [TOKEN_FALSE]           = { .prec = PREC_NONE,          .prefix = boolean,  .infix = NULL       },
    [TOKEN_GREATER]         = { .prec = PREC_COMPARISON,    .prefix = NULL,     .infix = binary     },
    [TOKEN_GREATER_EQUAL]   = { .prec = PREC_COMPARISON,    .prefix = NULL,     .infix = binary     },
    [TOKEN_IDENTIFIER]      = { .prec = PREC_NONE,          .prefix = variable, .infix = NULL       },
    [TOKEN_LEFT_SHIFT]      = { .prec = PREC_SHIFT,         .prefix = NULL,     .infix = binary     },
    [TOKEN_LESS]            = { .prec = PREC_COMPARISON,    .prefix = NULL,     .infix = binary     },
    [TOKEN_LESS_EQUAL]      = { .prec = PREC_COMPARISON,    .prefix = NULL,     .infix = binary     },
    [TOKEN_LBRACE]          = { .prec = PREC_NONE,          .prefix = NULL,     .infix = NULL       },
    [TOKEN_LPAREN]          = { .prec = PREC_NONE,          .prefix = group,    .infix = call       },
    [TOKEN_MINUS]           = { .prec = PREC_TERM,          .prefix = unary,    .infix = binary     },
    [TOKEN_MODULO]          = { .prec = PREC_FACTOR,        .prefix = NULL,     .infix = binary     },
    [TOKEN_NUMBER]          = { .prec = PREC_NONE,          .prefix = number,   .infix = NULL       },
    [TOKEN_NULL]            = { .prec = PREC_NONE,          .prefix = null_,    .infix = NULL       },
    [TOKEN_OR]              = { .prec = PREC_OR,            .prefix = NULL,     .infix = binary     },
    [TOKEN_PLUS]            = { .prec = PREC_TERM,          .prefix = unary,    .infix = binary     },
    [TOKEN_QUESTION]        = { .prec = PREC_TERNARY,       .prefix = NULL,     .infix = ternary    },
    [TOKEN_RBRACE]          = { .prec = PREC_NONE,          .prefix = NULL,     .infix = NULL       },
    [TOKEN_RIGHT_SHIFT]     = { .prec = PREC_SHIFT,         .prefix = NULL,     .infix = binary     },
    [TOKEN_RPAREN]          = { .prec = PREC_NONE,          .prefix = NULL,     .infix = NULL       },
    [TOKEN_SEMI]            = { .prec = PREC_NONE,          .prefix = NULL,     .infix = NULL       },
    [TOKEN_SLASH]           = { .prec = PREC_FACTOR,        .prefix = NULL,     .infix = binary     },
    [TOKEN_STRING]          = { .prec = PREC_NONE,          .prefix = str_,     .infix = NULL       },
    [TOKEN_TILDE]           = { .prec = PREC_NONE,          .prefix = unary,    .infix = NULL       },
    [TOKEN_TRUE]            = { .prec = PREC_NONE,          .prefix = boolean,  .infix = NULL       },
    [TOKEN_XOR]             = { .prec = PREC_BIT_XOR,       .prefix = NULL,     .infix = binary     },
};

static void     consume(parser_t* parser, token_ty_t type, const char* message);
static token_t  previous(parser_t* parser);
static token_t  peek(parser_t* parser);

static ast_stmt_t* parse_block_stmt(arena_t* arena, parser_t* parser);
static ast_stmt_t* parse_exp_stmt(arena_t* arena, parser_t* parser);
static ast_stmt_t* parse_if_stmt(arena_t* arena, parser_t* parser);
static ast_stmt_t* parse_loop_stmt(arena_t* arena, parser_t* parser);
static ast_stmt_t* parse_declaration_stmt(arena_t* arena, parser_t* parser);
static expr_type_t parse_var_type(parser_t* parser);

static void print_ast_exp(FILE* out, ast_exp_t* exp);

static void error_at(parser_t* parser, token_t token, const char* message)
{
    fprintf(stderr, "[line %ld] Error", token.line_no);

    if (token.type == TOKEN_EOF)        fprintf(stderr, " at EOF");
    else if (token.type == TOKEN_ERROR) fprintf(stderr, " with expression '%.*s'", (int)token.length, token.start);
    else                                fprintf(stderr, " at '%.*s'", (int)token.length, token.start);

    fprintf(stderr, ": %s\n", message);
    parser->had_error = true;
}

static void error_exp(parser_t* parser, const char* message)
{
    token_t token = peek(parser);
    const char* code = token.start;
    int index = 0;

    while (code[index] != '\n' && code[index] != EOF) index++;

    fprintf(stderr, "[line %ld] Error: %s\n", token.line_no, message);
    fprintf(stderr, "\n - [Diag] At expression: '%.*s'\n", index, code);
}

static void error(parser_t* parser, const char* message)
{
    error_at(parser, peek(parser),  message);
}

ast_exp_t* variable(arena_t* arena, parser_t* /*parser*/, token_t token)
{
    char* tmp = (char*)arena_alloc(arena, sizeof(char) * token.length + 1);
    sprintf(tmp, "%.*s", (int)token.length, token.start);
    return new_exp(arena, (ast_exp_t){ .kind = VARIABLE, .type_info = UNKNOWN, .as_var = (struct ast_var){ .name = tmp }});
}

ast_exp_t* str_(arena_t* arena, parser_t* /*parser*/, token_t token)
{
    size_t length = (token.length > 2) ? token.length : 1;
    char* tmp = (char*)arena_alloc(arena, sizeof(char) * length);

    if (length > 2) sprintf(tmp, "%.*s", (int)length - 2, &token.start[1]);
    else            sprintf(tmp, "%s", "");

    ast_exp_t* expr = new_exp(arena, (ast_exp_t){ .kind = STRING_LITERAL, .type_info = STRING, .as_str = (struct ast_string){ .STRING = tmp }});
    return expr;
}

ast_exp_t* chr_(arena_t* arena, parser_t* /*parser*/, token_t token)
{
    return new_exp(arena, (ast_exp_t){ .kind = CHAR_LITERAL, .type_info = CHAR, .as_char = (struct ast_char){ .CHAR = token.start[1] }});
}

ast_exp_t* group(arena_t* arena, parser_t* parser, token_t /*token*/)
{
    ast_exp_t* exp = parse_expression(arena, parser, PREC_NONE);
    consume(parser, TOKEN_RPAREN, "Missing ')' symbol");
    return exp;
}

ast_exp_t* number(arena_t* arena, parser_t* /*parser*/, token_t token)
{
#define NUM_VAL(value, type) new_exp(arena, (ast_exp_t){ .kind = NUM_LITERAL, .type_info = type, .as_num = (struct ast_number){ .type = value }})
    char temp[token.length + 1];
    sprintf(temp, "%.*s", (int)token.length, token.start);
    bool is_float = false;

    for (size_t i = 0; i < token.length + 1; i++) 
        if (token.start[i] == '.') 
            is_float = true;

    temp[token.length] = '\0';
    double val;
    sscanf(temp, "%lf", &val);
    ast_exp_t* expr = (is_float) ? NUM_VAL(val, DOUBLE) : NUM_VAL(val, I64);
    return expr;
#undef NUM_VAL
}

ast_exp_t* boolean(arena_t* arena, parser_t* /*parser*/, token_t token)
{
    return new_exp(arena, (ast_exp_t)
            { 
                .kind = BOOL_LITERAL, 
                .type_info = BOOL, 
                .as_bool = (struct ast_bool)
                                { 
                                    .BOOL = (token.type == TOKEN_TRUE) 
                                }
            });
}

ast_exp_t* null_(arena_t* arena, parser_t* /*parser*/, token_t /*token*/)
{
     return new_exp(arena, (ast_exp_t)
            { 
                .kind = NULL_LITERAL, 
                .type_info = UNKNOWN, 
                .as_bool = (struct ast_bool)
                                { 
                                    .BOOL = false 
                                }
            });
}

ast_exp_t* unary(arena_t* arena, parser_t* parser, token_t token)
{
    char* op = (char*)arena_alloc(arena, token.length + 1);
    sprintf(op, "%.*s", (int)token.length, token.start);
    ast_exp_t* expr = parse_expression(arena, parser, parse_table[token.type].prec);
    if (NULL == expr) return NULL;
    return new_exp(arena, (ast_exp_t) 
            { 
              .kind = UNARY_OP, 
              .type_info = UNKNOWN, 
              .as_un = (struct ast_unary)
                            { 
                              .op = op, 
                              .expr = expr
                            }
            });
}

ast_exp_t* call(arena_t* /*arena*/, parser_t* /*parser*/, ast_exp_t* /*left*/, bool /*can_assign*/)
{
    // TODO
    return NULL;
}

ast_exp_t* invoke(arena_t* /*arena*/, parser_t* /*parser*/, ast_exp_t* /*left*/, bool /*can_assign*/)
{
    // TODO
    return NULL;
}

ast_exp_t* binary(arena_t* arena, parser_t* parser, ast_exp_t* left, bool /*can_assign*/)
{
    token_t token = previous(parser);
    char* op = (char*)arena_alloc(arena, token.length + 1);
    sprintf(op, "%.*s", (int)token.length, token.start);
    ast_exp_t* right = parse_expression(arena, parser, parse_table[token.type].prec);
    if (NULL == right) 
    {
        error_exp(parser, "Invalid expression");
        return NULL;
    }

    return new_exp(arena, (ast_exp_t) 
            { 
              .kind = BINARY_OP, 
              .type_info = UNKNOWN, 
              .as_bin = (struct ast_binary)
                            { 
                              .op = op, 
                              .left = left, 
                              .right = right
                            }
            });
}

ast_exp_t* ternary(arena_t* /*arena*/, parser_t* /*parser*/, ast_exp_t* /*left*/, bool /*can_assign*/)
{
    // TODO: ternary implementation here
    return NULL;
}

static token_t advance(parser_t* parser)
{
    return parser->tokens->data[parser->curr++];
}

static token_t previous(parser_t* parser)
{
    return parser->tokens->data[parser->curr - 1];
}

static token_t peek(parser_t* parser)
{
    return parser->tokens->data[parser->curr];
}

static void consume(parser_t* parser, token_ty_t type, const char* message)
{
    if (peek(parser).type == type) 
    {
        advance(parser);
        return;
    }
    error(parser, message);
}

static bool match(parser_t* parser, token_ty_t type)
{
    if (peek(parser).type == type) 
    {
        advance(parser);
        return true;
    }
    return false;
}

void init_parser(parser_t* parser, token_list_t *tokens)
{
    parser->tokens = tokens;
    parser->curr = 0;
}

ast_exp_t* parse_expression(arena_t* arena, parser_t* parser, precedence_t precedence)
{
    token_t token = advance(parser);
    parse_rule_t rule = parse_table[token.type];

    if (NULL == rule.prefix)
    {
        error_at(parser, token, "Unexpected token");
        return NULL;
    }

    prefix_t prefix = rule.prefix;
    ast_exp_t* left = prefix(arena, parser, token);
    if (NULL == left) return NULL;

    while (precedence < parse_table[peek(parser).type].prec)
    {
        token = advance(parser);
        infix_t infix = parse_table[token.type].infix;

        if (NULL == infix) break;

        left = infix(arena, parser, left, true);
        if (NULL == left) {
            return NULL;
        }
    }

    return left;
}

static ast_stmt_t* parse_exp_stmt(arena_t* arena, parser_t* parser)
{
    if (peek(parser).type == TOKEN_SEMI) return NULL;

    ast_stmt_t* stmt = (ast_stmt_t*)arena_alloc(arena, sizeof(ast_stmt_t));
    stmt->as_expr.exp = parse_expression(arena, parser, PREC_NONE);
    if (NULL == stmt->as_expr.exp) return NULL;

    if (!match(parser, TOKEN_SEMI))
    {
        error(parser, "Missing ';' at the end of expression");
        return NULL;
    } 

    return stmt;
}

static ast_stmt_t* parse_if_stmt(arena_t* arena, parser_t* parser)
{
#define defer_error(X) do { error_at(parser, token, X); exit(1); } while (false)
    advance(parser); // eat IF keyword
    ast_stmt_t* stmt = (ast_stmt_t*)arena_alloc(arena, sizeof(ast_stmt_t));
    stmt->kind = IF_STMT;
    stmt->as_if.cond = NULL;
    stmt->as_if.then_b = NULL;
    stmt->as_if.else_b = NULL;

    token_t token = advance(parser);
    if (token.type != TOKEN_LPAREN) defer_error("Missing '(' symbol");

    ast_exp_t* cond = parse_expression(arena, parser, 0);
    if (NULL == cond) defer_error("Invalid if condition expression");

    stmt->as_if.cond = cond;

    token = advance(parser);
    if (token.type != TOKEN_RPAREN) defer_error("Missing ')' symbol");

    ast_stmt_t* then = parse_statement(arena, parser);
    if (NULL == then) defer_error("Invalid statement as 'then' block");

    stmt->as_if.then_b = then;
    
    token = peek(parser);
    if (token.type == TOKEN_ELSE)
    {
        advance(parser);
        ast_stmt_t* else_b = parse_statement(arena, parser);
        if (NULL == else_b) defer_error("Invalid statement as 'else' block");

        stmt->as_if.else_b = else_b; 
    }
    
    return stmt;
#undef defer_error
}

static ast_stmt_t* parse_loop_stmt(arena_t* arena, parser_t* parser)
{
#define defer_error(X) do { error_at(parser, token, X); exit(1); } while (false)
    advance(parser); // eat LOOP keyword
    ast_stmt_t* stmt = (ast_stmt_t*)arena_alloc(arena, sizeof(ast_stmt_t));
    stmt->kind = LOOP_STMT;
    stmt->as_loop.cond = NULL;
    stmt->as_loop.block = NULL;
    token_t token = advance(parser);
    if (TOKEN_LPAREN != token.type) defer_error("Missing '(' symbol");

    ast_exp_t* cond = parse_expression(arena, parser, 0);
    if (NULL == cond) defer_error("Invalid expression as loop condition");
    
    token = advance(parser);
    if (TOKEN_RPAREN != token.type) defer_error("Missing ')' symbol");
    
    ast_stmt_t* block = parse_statement(arena, parser);
    if (NULL == block) defer_error("Invalid statement as loop body");
    
    stmt->as_loop.cond = cond;
    stmt->as_loop.block = block;

    return stmt;
#undef defer_error
}

static expr_type_t parse_var_type(parser_t* parser)
{
#define type_case(X) type = X; break
    advance(parser); // Eat the ':' symbol
    token_ty_t token_type = peek(parser).type;
    expr_type_t type = UNKNOWN;

    switch (token_type)
    {
        case TOKEN_I8:      type_case(I8);
        case TOKEN_I16:     type_case(I16);
        case TOKEN_I32:     type_case(I32);
        case TOKEN_I64:     type_case(I64);
        case TOKEN_FLOAT:   type_case(FLOAT);
        case TOKEN_DOUBLE:  type_case(DOUBLE);
        case TOKEN_CHAR:    type_case(CHAR);
        case TOKEN_BOOL:    type_case(BOOL);
        default:
            error_at(parser, peek(parser), "Unknown type");
            exit(1);
            break;
    }
    advance(parser); // Eat the type token;

    // We shouldn't reach here
    return type;
#undef type_case
}

static ast_stmt_t* parse_declaration_stmt(arena_t* arena, parser_t* parser)
{
#define defer_error(X) do { error_at(parser, token, X); exit(1); } while (false)
    ast_stmt_t* stmt = (ast_stmt_t*)arena_alloc(arena, sizeof(ast_stmt_t));
    stmt->kind = VAR_DECL;
    stmt->as_decl.type = UNKNOWN;
    stmt->as_decl.exp = NULL;
    advance(parser); // Eat the VAR keyword

    token_t token = advance(parser);
    if (TOKEN_IDENTIFIER != token.type) defer_error("Missing variable name");

    stmt->as_decl.name = (char*)arena_alloc(arena, token.length + 1);
    memset(stmt->as_decl.name, '\0', token.length + 1);
    memcpy(stmt->as_decl.name, token.start, token.length);

    if (TOKEN_COLON == peek(parser).type) 
    {
        expr_type_t type = parse_var_type(parser);
        if (UNKNOWN == type) defer_error("Unknown type");
        stmt->as_decl.type = type;
    }

    token = peek(parser);

    if (TOKEN_EQUAL == token.type) 
    {
        advance(parser);
        ast_exp_t* exp = parse_expression(arena, parser, 0);
        if (NULL == exp) defer_error("Invalid expression on var initialization");
        stmt->as_decl.exp = exp;
    }
    
    token = peek(parser);
    if (TOKEN_SEMI != token.type) defer_error("Missing ';'");
    else advance(parser); // Eat the ';' symbol

    return stmt;
#undef defer_error
}

static ast_stmt_t* parse_block_stmt(arena_t* arena, parser_t* parser)
{
#define defer_error(X) do { error_at(parser, token, X); exit(1); } while (false)
    ast_stmt_t* stmt = (ast_stmt_t*)arena_alloc(arena, sizeof(ast_stmt_t));
    stmt->as_block.stmts = (stmt_list_t*)arena_alloc(arena, sizeof(stmt_list_t));
    stmt->as_block.stmts->data = NULL;
    stmt->as_block.stmts->next = NULL;
    stmt->kind = BLOCK_STMT;
    advance(parser); // Eat the '{' symbol
    token_t token = peek(parser); // And check the one after
    while (TOKEN_EOF != token.type && TOKEN_RBRACE != token.type)
    {
        ast_stmt_t* child_stmt;
        child_stmt = parse_statement(arena, parser);

        if (NULL == child_stmt) defer_error("Unknown statement");

        if ( NULL == stmt->as_block.stmts->data )
        {
            stmt->as_block.stmts->data = child_stmt;
        }
        else
        {
            stmt_list_t* entry = (stmt_list_t*)arena_alloc(arena, sizeof(stmt_list_t));
            entry->data = child_stmt;
            entry->next = NULL;
            stmt_list_t* element = stmt->as_block.stmts;
            for (; element->next != NULL; element = element->next)
                ;
            element->next = entry;
        }
        token = peek(parser);
    }

    if (TOKEN_EOF == token.type) defer_error("Unexpected end of file");
    
    if (token.type == TOKEN_RBRACE) advance(parser);

    return stmt;
#undef defer_error
}

ast_stmt_t* parse_statement(arena_t* arena, parser_t* parser)
{
    ast_stmt_t* stmt;
    token_t token = peek(parser);
    switch (token.type)
    {
        case TOKEN_IF:
            stmt = parse_if_stmt(arena, parser);
            break;
        case TOKEN_LOOP:
            stmt = parse_loop_stmt(arena, parser);
            break;
        case TOKEN_VAR:
            stmt = parse_declaration_stmt(arena, parser);
            break;
        case TOKEN_RETURN:
            // TODO: parse return stmt
            break;
        case TOKEN_LBRACE:
            stmt = parse_block_stmt(arena, parser);
            break;
        default:
            stmt = parse_exp_stmt(arena, parser);
            break;
    }
    return stmt;
}

ast_stmt_t* parse(arena_t* arena, parser_t* parser)
{    
    return parse_statement(arena, parser);
}

static void print_ast_exp(FILE* out, ast_exp_t *exp)
{
    switch (exp->kind)
    {
        case NUM_LITERAL:
            if (exp->type_info == I64)
                fprintf(out, "%ld", exp->as_num.I64);
            else if (exp->type_info == DOUBLE)
                fprintf(out, "%.2f", (double)exp->as_num.DOUBLE);
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
        case VARIABLE:
            fprintf(out, "%s", exp->as_var.name);
            break;
        case UNARY_OP:
            fprintf(out, "(%c", *exp->as_un.op);
            print_ast_exp(out, exp->as_un.expr);
            fprintf(out, ")");
            break;
        default:
            fprintf(out, "UNKNOWN");
            break;
    }
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
        case BOOL:    return "bool";
        default:      return "unknown";
    }
    return "unknown";
}

void print_ast_stmt(FILE* out, ast_stmt_t *stmt)
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
    default:
        fprintf(out, "Unknown statement type\n");
        exit(1);
        break;
    }
}