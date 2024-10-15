
#include "parser.h"


ast_node_t* group(parser_t* /*parser*/, token_t /*token*/)
{
    return NULL;
}

ast_node_t* unary(parser_t* /*parser*/, token_t /*token*/)
{
    return NULL;
}

ast_node_t* call(parser_t* /*parser*/, ast_node_t* /*left*/, bool /*can_assign*/)
{
    return NULL;
}

ast_node_t* invoke(parser_t* /*parser*/, ast_node_t* /*left*/, bool /*can_assign*/)
{
    return NULL;
}

ast_node_t* binary(parser_t* /*parser*/, ast_node_t* /*left*/, bool /*can_assign*/)
{
    return NULL;
}

static token_t advance(parser_t* parser)
{
    return parser->tokens->data[parser->curr++];
}

static token_t peek(parser_t* parser)
{
    return parser->tokens->data[parser->curr];
}

static void error_at(parser_t* parser, const char* message)
{
    token_t token = peek(parser);
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

static void error(parser_t* parser, const char* message)
{
    error_at(parser, message);
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

void init_parser(parser_t* parser, token_list_t *tokens)
{
    parser->tokens = tokens;
    parser->curr = 0;
}

void parse(parser_t* parser)
{
    token_t token;
    do
    {
        token = advance(parser);
        switch (token.type)
        {
            case TOKEN_MODULE:
                printf("Module declaration\n");
                break;
            case TOKEN_RECORD:
                printf("Record declaration\n");
                break;
            case TOKEN_ENTRY:
                printf("Entry declaration\n");
                break;
            case TOKEN_PURE:
                printf("Pure declaration\n");
                break;
            case TOKEN_ENTITY:
                printf("Entity declaration\n");
                break;
            case TOKEN_VAR:
                printf("Var declaration\n");
                break;
            default:
                printf("Invalid statement\n");
        }
        printf(" %-17.*s|\n", (int)token.length, token.start);
    } while (token.type != TOKEN_EOF);
}

void print_ast_node(FILE* out, ast_node_t *node)
{
    switch (node->kind)
    {
        case NUM_LITERAL:
            fprintf(out, "%ld", node->data.as_num.num);
            break;
        case BINARY_OP:
            fprintf(out, "(");
            print_ast_node(out, node->data.as_bin.left);
            fprintf(out, " %c ", *node->data.as_bin.op);
            print_ast_node(out, node->data.as_bin.right);
            fprintf(out, ")");
            break;
        default:
            fprintf(out, "UNKNOWN");
            break;
    }
}