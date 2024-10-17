
#include "parser.h"

#include <string.h>

static parse_rule_t parse_table[] = {
    [TOKEN_LPAREN]          = { .prec = PREC_NONE,          .prefix = group,    .infix = call },
    [TOKEN_RPAREN]          = { .prec = PREC_NONE,          .prefix = NULL,     .infix = NULL },
    [TOKEN_LBRACE]          = { .prec = PREC_NONE,          .prefix = NULL,     .infix = NULL },
    [TOKEN_RBRACE]          = { .prec = PREC_NONE,          .prefix = NULL,     .infix = NULL },
    [TOKEN_COMMA]           = { .prec = PREC_NONE,          .prefix = NULL,     .infix = NULL },
    [TOKEN_DOT]             = { .prec = PREC_CALL,          .prefix = NULL,     .infix = invoke },
    [TOKEN_MINUS]           = { .prec = PREC_TERM,          .prefix = unary,    .infix = binary },
    [TOKEN_PLUS]            = { .prec = PREC_TERM,          .prefix = unary,     .infix = binary },
    [TOKEN_SEMI]            = { .prec = PREC_NONE,          .prefix = NULL,     .infix = NULL },
    [TOKEN_SLASH]           = { .prec = PREC_FACTOR,        .prefix = NULL,     .infix = binary },
    [TOKEN_ASTERISK]        = { .prec = PREC_FACTOR,        .prefix = NULL,     .infix = binary },
    [TOKEN_BANG]            = { .prec = PREC_NONE,          .prefix = unary,    .infix = NULL },
    [TOKEN_BANG_EQUAL]      = { .prec = PREC_EQUALITY,      .prefix = NULL,     .infix = binary },
    [TOKEN_NUMBER]          = { .prec = PREC_NONE,          .prefix = number,   .infix = NULL },
    [TOKEN_EOF]             = { .prec = PREC_NONE,          .prefix = NULL,     .infix = NULL },
    [TOKEN_STRING]          = { .prec = PREC_NONE,          .prefix = str_,     .infix = NULL },
    [TOKEN_CHAR]            = { .prec = PREC_NONE,          .prefix = chr_,     .infix = NULL },
    [TOKEN_IDENTIFIER]      = { .prec = PREC_NONE,          .prefix = variable, .infix = NULL },
};

static void consume(parser_t* parser, token_ty_t type, const char* message);
static token_t previous(parser_t* parser);
static token_t peek(parser_t* parser);

static void error_at(parser_t* parser, token_t token, const char* message)
{
    fprintf(stderr, "[line %ld] Error", token.line_no);

    if (token.type == TOKEN_EOF)
    {
        fprintf(stderr, " at end");
    }
    else if (token.type == TOKEN_ERROR)
    {

    }
    else 
    {
        fprintf(stderr, " at '%.*s'", (int)token.length, token.start);
    }
    fprintf(stderr, ": %s\n", message);
    parser->had_error = true;
}

static void error_exp(parser_t* parser, const char* message)
{
    token_t token = peek(parser);
    const char* code = token.start;
    int index = 0;
    while (code[index] != '\n' && code[index] != EOF)
    {
        index++;
    }
    fprintf(stderr, "[line %ld] Error: %s\n", token.line_no, message);
    fprintf(stderr, "\n - [Diag] At expression: '%.*s'\n", index, code);
}

static void error(parser_t* parser, const char* message)
{
    error_at(parser, peek(parser),  message);
}

ast_exp_t* variable(parser_t* parser, token_t token)
{
    char* tmp = (char*)arena_alloc(parser->arena, sizeof(char) * token.length + 1);
    sprintf(tmp, "%.*s", (int)token.length, token.start);
    return new_exp(parser->arena, (ast_exp_t){ .kind = VARIABLE, .type_info = UNKNOWN, .data.as_var = (struct ast_var){ .name = tmp }});
}

ast_exp_t* str_(parser_t* parser, token_t token)
{
    size_t length = (token.length > 2) ? token.length : 1;
    char* tmp = (char*)arena_alloc(parser->arena, sizeof(char) * length);
    if (length > 2) sprintf(tmp, "%.*s", (int)length - 2, &token.start[1]);
    else sprintf(tmp, "%s", "");

    ast_exp_t* expr = new_exp(parser->arena, (ast_exp_t){ .kind = STRING_LITERAL, .type_info = STRING, .data.as_str = (struct ast_string){ .cstr = tmp }});
    return expr;
}

ast_exp_t* chr_(parser_t* parser, token_t token)
{
    return new_exp(parser->arena, (ast_exp_t){ .kind = CHAR_LITERAL, .type_info = CHAR, .data.as_char = (struct ast_char){ .c = token.start[1] }});
}

ast_exp_t* group(parser_t* parser, token_t /*token*/)
{
    ast_exp_t* exp = parse_expression(parser, PREC_NONE);
    consume(parser, TOKEN_RPAREN, "Missing ')' symbol");
    return exp;
}

ast_exp_t* number(parser_t* parser, token_t token)
{
    // This is killing me. There has to be an easier way.
    char* tmp = (char*)malloc(sizeof(char) * token.length + 1);
    sprintf(tmp, "%.*s", (int)token.length, token.start);
    ast_exp_t* expr = new_exp(parser->arena, (ast_exp_t){ .kind = NUM_LITERAL, .type_info = I8, .data.as_num = (struct ast_number){ .num = strtof(tmp, NULL) }});
    free(tmp);
    return expr;
}

ast_exp_t* unary(parser_t* parser, token_t token)
{
    char* op = (char*)arena_alloc(parser->arena, token.length + 1);
    sprintf(op, "%.*s", (int)token.length, token.start);
    ast_exp_t* expr = parse_expression(parser, parse_table[token.type].prec);
    if (NULL == expr) return NULL;
    return new_exp(parser->arena, (ast_exp_t) 
            { 
              .kind = UNARY_OP, 
              .type_info = UNKNOWN, 
              .data.as_un = (struct ast_unary)
                            { 
                              .op = op, 
                              .expr = expr
                            }
            });
}

ast_exp_t* call(parser_t* /*parser*/, ast_exp_t* /*left*/, bool /*can_assign*/)
{
    return NULL;
}

ast_exp_t* invoke(parser_t* /*parser*/, ast_exp_t* /*left*/, bool /*can_assign*/)
{
    return NULL;
}

ast_exp_t* binary(parser_t* parser, ast_exp_t* left, bool /*can_assign*/)
{
    token_t token = previous(parser);
    char* op = (char*)arena_alloc(parser->arena, token.length + 1);
    sprintf(op, "%.*s", (int)token.length, token.start);
    ast_exp_t* right = parse_expression(parser, parse_table[token.type].prec);
    if (NULL == right) 
    {
        error_exp(parser, "Invalid expression");
        return NULL;
    }

    return new_exp(parser->arena, (ast_exp_t) 
            { 
              .kind = BINARY_OP, 
              .type_info = UNKNOWN, 
              .data.as_bin = (struct ast_binary)
                            { 
                              .op = op, 
                              .left = left, 
                              .right = right
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

void init_parser(parser_t* parser, arena_t* arena, token_list_t *tokens)
{
    parser->arena = arena;
    parser->tokens = tokens;
    parser->curr = 0;
}

ast_exp_t* parse_expression(parser_t* parser, precedence_t precedence)
{
    token_t token = advance(parser);
    parse_rule_t rule = parse_table[token.type];

    if (NULL == rule.prefix)
    {
        error_at(parser, token, "Unexpected token");
        return NULL;
    }

    prefix_t prefix = rule.prefix;
    ast_exp_t* left = prefix(parser, token);
    if (NULL == left) return NULL;

    while (precedence < parse_table[peek(parser).type].prec)
    {
        token = advance(parser);
        infix_t infix = parse_table[token.type].infix;

        if (NULL == infix) break;

        left = infix(parser, left, true);
        if (NULL == left) {
            return NULL;
        }
    }

    return left;
}

ast_stmt_t* parse(parser_t* parser)
{
    //token_t token;
    //ast_stmt_t* exp = NULL;
    //do
    //{
        //token = advance(parser);

        ast_stmt_t* stmt = (ast_stmt_t*)calloc(1, sizeof(ast_stmt_t));
        stmt->pl.as_expr.exp = parse_expression(parser, 0);
        //if (NULL == exp) break;
        //token = peek(parser);
        // switch (token.type)
        // {
        //     case TOKEN_MODULE:
        //         printf("Module declaration\n");
        //         break;
        //     case TOKEN_RECORD:
        //         printf("Record declaration\n");
        //         break;
        //     case TOKEN_ENTRY:
        //         printf("Entry declaration\n");
        //         break;
        //     case TOKEN_PURE:
        //         printf("Pure declaration\n");
        //         break;
        //     case TOKEN_ENTITY:
        //         printf("Entity declaration\n");
        //         break;
        //     case TOKEN_VAR:
        //         printf("Var declaration\n");
        //         break;
        //     default:
        //         printf("Invalid statement\n");
        // }
        // printf(" %-17.*s|\n", (int)token.length, token.start);
    //} while (token.type != TOKEN_EOF);

    return stmt;
}

void print_ast_exp(FILE* out, ast_exp_t *exp)
{
    switch (exp->kind)
    {
        case NUM_LITERAL:
            fprintf(out, "%ld", exp->data.as_num.num);
            break;
        case STRING_LITERAL:
            fprintf(out, "\"%s\"", exp->data.as_str.cstr);
            break;
        case CHAR_LITERAL:
            fprintf(out, "'%c'", exp->data.as_char.c);
            break;
        case BINARY_OP:
            fprintf(out, "(");
            print_ast_exp(out, exp->data.as_bin.left);
            fprintf(out, " %c ", *exp->data.as_bin.op);
            print_ast_exp(out, exp->data.as_bin.right);
            fprintf(out, ")");
            break;
        case VARIABLE:
            fprintf(out, "%s", exp->data.as_var.name);
            break;
        case UNARY_OP:
            fprintf(out, "(%c", *exp->data.as_un.op);
            print_ast_exp(out, exp->data.as_un.expr);
            fprintf(out, ")");
            break;
        default:
            fprintf(out, "UNKNOWN");
            break;
    }
}