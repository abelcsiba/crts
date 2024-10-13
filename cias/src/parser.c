
#include "parser.h"


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
        printf(" %-17.*s|\n", (int)token.length, token.start);
    } while (token.type != TOKEN_EOF);
}