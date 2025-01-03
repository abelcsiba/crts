
#include "parser.h"

#include <string.h>
#include <errno.h>
#include <limits.h>
#include <float.h>

static parse_rule_t parse_table[] = {
    [TOKEN_ASTERISK]        = { .prec = PREC_FACTOR,        .prefix = unary,    .infix = binary     },
    [TOKEN_AND]             = { .prec = PREC_AND,           .prefix = NULL,     .infix = binary     },
    [TOKEN_AS]              = { .prec = PREC_CAST,          .prefix = NULL,     .infix = cast       },
    [TOKEN_BANG]            = { .prec = PREC_NONE,          .prefix = unary,    .infix = NULL       },
    [TOKEN_BANG_EQUAL]      = { .prec = PREC_EQUALITY,      .prefix = NULL,     .infix = binary     },
    [TOKEN_BIT_AND]         = { .prec = PREC_BIT_AND,       .prefix = unary,    .infix = binary     },
    [TOKEN_BIT_OR]          = { .prec = PREC_BIT_OR,        .prefix = NULL,     .infix = binary     },
    [TOKEN_CHAR]            = { .prec = PREC_NONE,          .prefix = chr_,     .infix = NULL       },
    [TOKEN_COMMA]           = { .prec = PREC_NONE,          .prefix = NULL,     .infix = NULL       },
    [TOKEN_DOT]             = { .prec = PREC_CALL,          .prefix = NULL,     .infix = invoke     },
    [TOKEN_EOF]             = { .prec = PREC_NONE,          .prefix = NULL,     .infix = NULL       },
    [TOKEN_EQUAL]           = { .prec = PREC_ASSIGNMENT,    .prefix = NULL,     .infix = assign     },
    [TOKEN_EQUAL_EQUAL]     = { .prec = PREC_EQUALITY,      .prefix = NULL,     .infix = binary     },
    [TOKEN_FALSE]           = { .prec = PREC_NONE,          .prefix = boolean,  .infix = NULL       },
    [TOKEN_GREATER]         = { .prec = PREC_COMPARISON,    .prefix = NULL,     .infix = binary     },
    [TOKEN_GREATER_EQUAL]   = { .prec = PREC_COMPARISON,    .prefix = NULL,     .infix = binary     },
    [TOKEN_IDENTIFIER]      = { .prec = PREC_NONE,          .prefix = variable, .infix = NULL       },
    [TOKEN_LEFT_SHIFT]      = { .prec = PREC_SHIFT,         .prefix = NULL,     .infix = binary     },
    [TOKEN_LESS]            = { .prec = PREC_COMPARISON,    .prefix = NULL,     .infix = binary     },
    [TOKEN_LESS_EQUAL]      = { .prec = PREC_COMPARISON,    .prefix = NULL,     .infix = binary     },
    [TOKEN_LBRACE]          = { .prec = PREC_NONE,          .prefix = NULL,     .infix = NULL       },
    [TOKEN_LPAREN]          = { .prec = PREC_CALL,          .prefix = group,    .infix = call       },
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

static void             consume(parser_t* parser, token_ty_t type, const char* message);
static token_t          previous(parser_t* parser);
static token_t          peek(parser_t* parser);
static token_t          advance(parser_t* parser);

static ast_stmt_t*      parse_block_stmt(arena_t* arena, parser_t* parser, bool is_callable_def);
static ast_stmt_t*      parse_exp_stmt(arena_t* arena, parser_t* parser);
static ast_stmt_t*      parse_if_stmt(arena_t* arena, parser_t* parser);
static ast_stmt_t*      parse_loop_stmt(arena_t* arena, parser_t* parser);
static ast_stmt_t*      parse_declaration_stmt(arena_t* arena, parser_t* parser);
static ast_stmt_t*      parse_return_stmt(arena_t* arena, parser_t* parser);
static ast_stmt_t*      parse_entry_stmt(arena_t* arena, parser_t* parser);
static ast_stmt_t*      parse_pure_stmt(arena_t* arena, parser_t* parser);
static expr_type_t      parse_var_type(parser_t* parser);
static f_arg_list_t*    parse_args(arena_t* arena, parser_t* parser); 

static void error_at(parser_t* parser, token_t token, const char* message)
{
    fprintf(stderr, "[line %lX] Error", token.line_no);

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

static void process_escape_sequences(char *str) {
    char *src = str;
    char *dst = str;  

    while (*src != '\0') 
    {
        if (*src == '\\') 
        {
            src++;
            switch (*src) 
            {
                case 'n':
                    *dst = '\n';
                    break;
                case 't':
                    *dst = '\t';
                    break;
                case 'r':
                    *dst = '\r';
                    break;
                case 'b':
                    *dst = '\b';
                    break;
                case 'f':
                    *dst = '\f';
                    break;
                case 'v':
                    *dst = '\v';
                    break;
                case '\\':
                    *dst = '\\';
                    break;
                case '\"':
                    *dst = '\"';
                    break;
                case '\'':
                    *dst = '\'';
                    break;
                default:
                    dst++;
                    *dst = *src;
                    break;
            }
        } 
        else 
        {
            *dst = *src;
        }
        dst++;
        src++;
    }
    *dst = '\0';
}

ast_exp_t* str_(arena_t* arena, parser_t* /*parser*/, token_t token)
{
    size_t length = (token.length > 2) ? token.length : 1;
    char* tmp = (char*)arena_alloc(arena, sizeof(char) * length);

    if (length > 2) sprintf(tmp, "%.*s", (int)length - 2, &token.start[1]);
    else            sprintf(tmp, "%s", "");

    process_escape_sequences(tmp);

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

typedef struct {
    expr_type_t     type;
    union {
        int8_t      i8;
        int16_t     i16;
        int32_t     i32;
        int64_t     i64;
        float       flt;
        double      dbl;
    };
} num_val_t;

static inline num_val_t parse_int(const char *str) {
    char *endptr;
    errno = 0;

    long long llval = strtoll(str, &endptr, 10);
    if (errno == 0 && *endptr == '\0') {
        if (llval >= INT8_MIN && llval <= INT8_MAX) {
            return (num_val_t){ .type = I8,  .i8  = (int8_t)llval  };
        } else if (llval >= INT16_MIN && llval <= INT16_MAX) {
            return (num_val_t){ .type = I16, .i16 = (int16_t)llval };
        } else if (llval >= INT32_MIN && llval <= INT32_MAX) {
            return (num_val_t){ .type = I32, .i32 = (int32_t)llval };
        } else {
            return (num_val_t){ .type = I64, .i64 = (int64_t)llval };
        }
    }

    // If it doesn't parse as a number
    return (num_val_t){ .type = ERROR, .i64 = (int64_t)0x00 };
}

static inline num_val_t parse_float(const char *str) {
    char *endptr;
    
    errno = 0;
    double dval = strtod(str, &endptr);
    if (errno == 0 && *endptr == '\0') {
        if (dval >= -FLT_MAX && dval <= FLT_MAX) {
            return (num_val_t){ .type = FLOAT, .flt = (float)dval };
        } else {
            return (num_val_t){ .type = DOUBLE, .dbl = (double)dval };
        }
    }

    return (num_val_t){ .type = ERROR, .i64 = (int64_t)0x00 };
}

ast_exp_t* number(arena_t* arena, parser_t* /*parser*/, token_t token)
{
#define NUM_VAL(value, type) new_exp(arena, (ast_exp_t){ .kind = NUM_LITERAL, .type_info = type, .as_num = { .type = value }})
    char temp[token.length + 1];
    sprintf(temp, "%.*s", (int)token.length, token.start);
    bool is_float = false;

    for (size_t i = 0; i < token.length + 1; i++) // TODO: deal with the # prefix for different bases
        if (token.start[i] == '.')
            is_float = true;

    num_val_t num = (is_float) ? parse_float(temp) : parse_int(temp);
    ast_exp_t* expr = NULL;
    switch (num.type) // This switch could be replaced by macro magic
    {
        case I8:
            expr = NUM_VAL(num.i8, I8);
            break;
        case I16:
            expr = NUM_VAL(num.i16, I16);
            break;
        case I32:
            expr = NUM_VAL(num.i32, I32);
            break;
        case I64:
            expr = NUM_VAL(num.i64, I64);
            break;
        case FLOAT:
            expr = NUM_VAL(num.flt, FLOAT);
            break;
        case DOUBLE:
            expr = NUM_VAL(num.dbl, DOUBLE);
            break;
        default:
            // TODO: error logging
            return NULL;
    }

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

ast_exp_t* assign(arena_t* arena, parser_t* parser, ast_exp_t* left, bool /*can_assign*/)
{
    if (VARIABLE != left->kind)
    {
        error_exp(parser, "Invalid assignment. Only variable as left hand value allowed");
        return NULL;
    }
    ast_exp_t* right = parse_expression(arena, parser, parse_table[TOKEN_EQUAL].prec);
    if (NULL == right) 
    {
        error_exp(parser, "Invalid expression");
        return NULL;
    }
    char* op = (char*)arena_alloc(arena, 2);
    op[0] = '=';
    op[1] = '\0';
    return new_exp(arena, (ast_exp_t) 
            { 
              .kind = ASSIGNMENT, 
              .type_info = UNKNOWN, 
              .as_bin = (struct ast_binary)
                            { 
                              .op = op, 
                              .left = left, 
                              .right = right
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
    ast_exp_t* expr = parse_expression(arena, parser, PREC_UNARY);
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

ast_exp_t* call(arena_t* arena, parser_t* parser, ast_exp_t* left, bool /*can_assign*/)
{
    token_t token = previous(parser);
    token = peek(parser);
    arg_list_t* args = (arg_list_t*)arena_alloc(arena, sizeof(arg_list_t));
    arg_list_t* head = args;
    while (TOKEN_EOF != token.type && TOKEN_RPAREN != token.type)
    {
        ast_exp_t* exp = parse_expression(arena, parser, PREC_NONE);
        if (!exp)
        {
            error_exp(parser, "Invalid expression as call argument");
            return NULL;
        }
        head->exp = exp;
        token = peek(parser);
        if (token.type != TOKEN_COMMA && token.type != TOKEN_RPAREN)
        {
            error_exp(parser, "Invalid argument list");
            return NULL;
        }

        if (TOKEN_COMMA == token.type) 
        {
            advance(parser);        // Eat the comma
            token = peek(parser);   // And set to the next token
            head = (arg_list_t*)arena_alloc(arena, sizeof(arg_list_t));
            head->next = args; 
            args = head;
        }
    }

    if (TOKEN_EOF == token.type)
    {
        error_exp(parser, "Missing closing ')' symbol at the end of a call");
        return NULL;
    }
    else advance(parser); // Eat the closing ')' symbol

    char* calle_name = left->as_var.name;
    memset(left, '\0', sizeof(ast_exp_t));
    left->kind = STRING_LITERAL;
    left->as_str.STRING = calle_name;

    return new_exp(arena, (ast_exp_t) 
            { 
              .kind = CALLABLE, 
              .type_info = UNKNOWN, 
              .as_call = (struct ast_callable)
                            { 
                              .args = (head->exp != NULL ) ? head : NULL,
                              .callee_name = left
                            }
            });
}

ast_exp_t* invoke(arena_t* /*arena*/, parser_t* /*parser*/, ast_exp_t* /*left*/, bool /*can_assign*/)
{
    // TODO
    return NULL;
}

ast_exp_t* cast(arena_t* arena, parser_t* parser, ast_exp_t* left, bool /*can_assign*/)
{
    return new_exp(arena, (ast_exp_t) 
            { 
              .kind = CAST_BIN, 
              .type_info = UNKNOWN, 
              .as_cast = (struct ast_cast)
                            { 
                              .exp = left, 
                              .target = parse_var_type(parser)
                            }
            });
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

ast_exp_t* ternary(arena_t* arena, parser_t* parser, ast_exp_t* left, bool /*can_assign*/)
{
    ast_exp_t* then_b = parse_expression(arena, parser, PREC_NONE);
    if (TOKEN_COLON != peek(parser).type)
    {
        error_exp(parser, "Invalid ternary expression");
        return NULL;
    }
    advance(parser);
    ast_exp_t* else_b = parse_expression(arena, parser, PREC_NONE);
    return new_exp(arena, (ast_exp_t) 
            { 
              .kind = TERNARY_OP, 
              .type_info = UNKNOWN, 
              .as_ter = (struct ast_ternary)
                            { 
                              .cond = left,
                              .op1 = then_b,
                              .op2 = else_b
                            }
            });
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
#define defer_error(X) do { error_at(parser, token, X); exit(EXIT_FAILURE); } while (false)
    advance(parser); // eat IF keyword
    ast_stmt_t* stmt = (ast_stmt_t*)arena_alloc(arena, sizeof(ast_stmt_t));
    stmt->kind = IF_STMT;
    stmt->as_if.cond = NULL;
    stmt->as_if.then_b = NULL;
    stmt->as_if.else_b = NULL;

    token_t token = advance(parser);
    if (token.type != TOKEN_LPAREN) defer_error("Missing '(' symbol");

    ast_exp_t* cond = parse_expression(arena, parser, PREC_NONE);
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
#define defer_error(X) do { error_at(parser, token, X); exit(EXIT_FAILURE); } while (false)
    advance(parser); // eat LOOP keyword
    ast_stmt_t* stmt = (ast_stmt_t*)arena_alloc(arena, sizeof(ast_stmt_t));
    stmt->kind = LOOP_STMT;
    stmt->as_loop.cond = NULL;
    stmt->as_loop.block = NULL;
    token_t token = advance(parser);
    if (TOKEN_LPAREN != token.type) defer_error("Missing '(' symbol");

    ast_exp_t* cond = parse_expression(arena, parser, PREC_NONE);
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
        case TOKEN_STRING:  type_case(STRING);
        default:
            error_at(parser, peek(parser), "Unknown type");
            exit(EXIT_FAILURE);
            break;
    }
    advance(parser); // Eat the type token;

    // We shouldn't reach here
    return type;
#undef type_case
}

static ast_stmt_t* parse_declaration_stmt(arena_t* arena, parser_t* parser)
{
#define defer_error(X) do { error_at(parser, token, X); exit(EXIT_FAILURE); } while (false)
    ast_stmt_t* stmt = (ast_stmt_t*)arena_alloc(arena, sizeof(ast_stmt_t));
    stmt->kind = VAR_DECL;
    stmt->as_decl.type = UNKNOWN;
    stmt->as_decl.exp = NULL;
    advance(parser); // Eat the VAR keyword

    token_t token = advance(parser);
    if (TOKEN_IDENTIFIER != token.type) defer_error("Missing variable name");

    stmt->as_decl.name = (char*)arena_alloc(arena, token.length + 1);
    memcpy(stmt->as_decl.name, token.start, token.length);
    stmt->as_decl.name[token.length] = 0;

    if (TOKEN_COLON == peek(parser).type) 
    {
        advance(parser); // Eat the ':' symbol
        expr_type_t type = parse_var_type(parser);
        if (UNKNOWN == type) defer_error("Unknown type");
        stmt->as_decl.type = type;
    }

    token = peek(parser);

    if (TOKEN_EQUAL == token.type) 
    {
        advance(parser);
        ast_exp_t* exp = parse_expression(arena, parser, PREC_NONE);
        if (NULL == exp) defer_error("Invalid expression on var initialization");
        stmt->as_decl.exp = exp;
    }
    
    token = peek(parser);
    if (TOKEN_SEMI != token.type) defer_error("Missing ';'");
    else advance(parser); // Eat the ';' symbol

    return stmt;
#undef defer_error
}

static ast_stmt_t* parse_entry_stmt(arena_t* arena, parser_t* parser)
{
#define defer_error(X) do { error_at(parser, token, X); exit(EXIT_FAILURE); } while (false)
    token_t token = advance(parser);
    ast_stmt_t* stmt = (ast_stmt_t*)arena_alloc(arena, sizeof(ast_stmt_t));
    stmt->kind = ENTRY_STMT;

    ast_stmt_t* body = parse_block_stmt(arena, parser, true);
    if (NULL == body) defer_error("Invalid entry block");

    stmt->as_callable.body = body;

    return stmt;
#undef defer_error
}

static f_arg_list_t* parse_args(arena_t* arena, parser_t* parser)
{
#define defer_error(X) do { error_at(parser, token, X); exit(EXIT_FAILURE); } while (false)
    f_arg_list_t* args = (f_arg_list_t*)arena_alloc(arena, sizeof(f_arg_list_t));
    args->type = UNKNOWN;
    args->next = NULL;

    token_t token = advance(parser);
    while (TOKEN_EOF != token.type && TOKEN_RPAREN != token.type)
    {
        if (TOKEN_IDENTIFIER != token.type) defer_error("Missing arg name");

        char* arg_name = (char*)arena_alloc(arena, token.length + 1);
        memcpy(arg_name, token.start, token.length);
        arg_name[token.length] = '\0';

        token = advance(parser);
        if (TOKEN_COLON != token.type) defer_error("Missing ':' symbol in arg declaration");

        expr_type_t type = parse_var_type(parser);
        if (UNKNOWN == args->type)
        {
            args->type = type;
            args->name = arg_name;
        }
        else
        {
            f_arg_list_t* entry = args;
            for (;entry->next != NULL; entry = entry->next)
                ;
            f_arg_list_t* next = (f_arg_list_t*)arena_alloc(arena, sizeof(f_arg_list_t));
            next->type = type;
            next->name = arg_name;
            next->next = NULL;
            entry->next = next;
        }

        token = peek(parser);
        if (TOKEN_COMMA == token.type)
        {
            advance(parser); // Eat the comma
            token = advance(parser); // And set it to the next token
        }
    }

    if (TOKEN_EOF == token.type) return NULL;

    return args;
#undef defer_error
}

// TODO: this parser function is a mess. Clean it up!
static ast_stmt_t* parse_pure_stmt(arena_t* arena, parser_t* parser)
{
#define defer_error(X) do { error_at(parser, token, X); exit(EXIT_FAILURE); } while (false)
    advance(parser); // Eat the pure keyword
    token_t token = advance(parser);
    ast_stmt_t* stmt = (ast_stmt_t*)arena_alloc(arena, sizeof(ast_stmt_t));
    stmt->kind = PURE_STMT;
    stmt->as_callable.body = NULL;
    stmt->as_callable.ret_type = UNKNOWN;

    if (TOKEN_IDENTIFIER != token.type) defer_error("Missing pure function name");
    stmt->as_callable.name = (char*)arena_alloc(arena, token.length + 1);
    memcpy(stmt->as_callable.name, token.start, token.length);
    stmt->as_callable.name[token.length] = 0;

    token = advance(parser);

    if (TOKEN_LPAREN != token.type) defer_error("Missing '(' symbol");

    if (TOKEN_RPAREN != peek(parser).type) 
    {
        f_arg_list_t* args = parse_args(arena, parser);
        if (NULL == args) defer_error("Invalid argument declaration");
        stmt->as_callable.args = args;
        token = peek(parser);
    }
    else token = advance(parser);

    if (TOKEN_RPAREN != token.type) defer_error("Missing ')' symbol");
    else token = peek(parser);

    if (NULL != stmt->as_callable.args)
    {
        advance(parser);
        token = peek(parser);
    }

    if (TOKEN_RIGHT_ARROW == token.type)
    {
        advance(parser); // Eat the '->' symbol
        expr_type_t type = parse_var_type(parser);
        stmt->as_callable.ret_type = type;
    }
    else stmt->as_callable.ret_type = VOID;

    if (TOKEN_LBRACE != peek(parser).type) defer_error("missing pure body");

    ast_stmt_t* body = parse_block_stmt(arena, parser, true);
    if (NULL == body) defer_error("Invalid pure definition");
    stmt->as_callable.body = body;
    f_arg_list_t* head = stmt->as_callable.args;
    for (;head != NULL; head = head->next)
        stmt->as_callable.arity += 1;

    return stmt;
#undef defer_error
}

static ast_stmt_t* parse_return_stmt(arena_t* arena, parser_t* parser)
{
#define defer_error(X) do { error_at(parser, token, X); exit(EXIT_FAILURE); } while (false)
    ast_stmt_t* stmt = (ast_stmt_t*)arena_alloc(arena, sizeof(ast_stmt_t));
    stmt->kind = RETURN_STMT;

    advance(parser); // Eat return keyword

    token_t token = peek(parser);
    ast_exp_t* exp = parse_expression(arena, parser, PREC_NONE);
    if (NULL == exp) defer_error("Invalid return expression");

    token = advance(parser);
    if (TOKEN_SEMI != token.type) defer_error("Missing ';' symbol");

    stmt->as_expr.exp = exp;

    return stmt;

#undef defer_error
}

static ast_stmt_t* parse_block_stmt(arena_t* arena, parser_t* parser, bool is_callable_def)
{
#define defer_error(X) do { error_at(parser, token, X); exit(EXIT_FAILURE); } while (false)
    ast_stmt_t* stmt = (ast_stmt_t*)arena_alloc(arena, sizeof(ast_stmt_t));
    stmt->as_block.stmts = (stmt_list_t*)arena_alloc(arena, sizeof(stmt_list_t));
    stmt->as_block.stmts->data = NULL;
    stmt->as_block.stmts->next = NULL;
    stmt->as_block.is_callable_def = is_callable_def;
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
            stmt = parse_return_stmt(arena, parser);
            break;
        case TOKEN_LBRACE:
            stmt = parse_block_stmt(arena, parser, false);
            break;
        default:
            stmt = parse_exp_stmt(arena, parser);
            break;
    }
    return stmt;
}

cu_t* parse(arena_t* arena, parser_t* parser)
{
#define defer_error(X) do { error_at(parser, token, X); exit(EXIT_FAILURE); } while (false)
    cu_t* cu = (cu_t*)arena_alloc(arena, sizeof(cu_t));
    cu->type = EXECUTABLE;
    cu->entry = NULL;
    cu->pures = NULL;
    token_t token = peek(parser);

    while (TOKEN_EOF != token.type)
    {
        switch (token.type)
        {
            case TOKEN_ENTRY:
                if (NULL != cu->entry) defer_error("Illegal redefinition of entry");
                ast_stmt_t* entry = parse_entry_stmt(arena, parser);
                cu->entry = entry;
                break;
            case TOKEN_PURE:
                ast_stmt_t* pure = parse_pure_stmt(arena, parser);
                if (NULL == cu->pures) 
                {
                    cu->pures = (stmt_list_t*)arena_alloc(arena, sizeof(stmt_list_t));
                    cu->pures->data = pure;
                }
                else
                {
                    stmt_list_t* element = cu->pures;
                    for (;element->next != NULL; element = element->next)
                        ;
                    stmt_list_t* next = (stmt_list_t*)arena_alloc(arena, sizeof(stmt_list_t));
                    next->data = pure;
                    next->next = NULL;
                    element->next = next;
                }
                break;
            case TOKEN_MODULE:
                advance(parser);
                token_t token = advance(parser);
                if (TOKEN_IDENTIFIER != token.type)
                {
                    fprintf(stderr, "Missing module name\n");
                    exit(EXIT_FAILURE);
                }
                cu->module_name = (char*)arena_alloc(arena, token.length + 1);
                sprintf(cu->module_name, "%.*s", (int)token.length, token.start);
                if (TOKEN_SEMI != peek(parser).type)
                {
                    fprintf(stderr, "Missing ';' symbol\n");
                    exit(EXIT_FAILURE);
                }
                advance(parser);
                break;
            default:

                break;
        }
        token = peek(parser);
    }
    return cu;
#undef defer_error
}
