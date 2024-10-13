
#include "parser.h"

void init_parser(parser_t* parser)
{
    parser->curr = 0;
}

void parse(parser_t* parser, token_list_t *tokens)
{
    token_t token;
    do
    {
        token = tokens->data[parser->curr++];
        printf(" %-17.*s|\n", (int)token.length, token.start);
    } while (token.type != TOKEN_EOF);
}