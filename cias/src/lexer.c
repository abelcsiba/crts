

#include "lexer.h"

#include <string.h>
#include <stdio.h>

static const char* token_labels[] = {
#define X(type, val, label) label,
    TOKEN_LIST
#undef X
};


static const char* token_ty2label(token_ty_t type)
{
    return token_labels[type];
}

void init_lexer(lexer_t* lexer, const char* source)
{
    lexer->start = source;
    lexer->curr = source;
    lexer->line_no = 1;
    init_token_da(&lexer->tokens);
}

static bool is_at_end(lexer_t* lexer)
{
    return *lexer->curr == '\0';
}

static bool is_digit(char c) // TODO: remove this
{
    return c >= '0' && c <= '9';
}

static bool is_alpha(char c)
{
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c == '_');
}

static char advance(lexer_t* lexer)
{
    lexer->curr++;
    return lexer->curr[-1];
}

static char peek(lexer_t* lexer)
{
    return *lexer->curr;
}

static char peek_next(lexer_t* lexer)
{
    return lexer->curr[1];
}

static void skip_whitesace(lexer_t* lexer)
{
    for(;;)
    {
        char c = peek(lexer);
        if ( ( c == ' ' ) || ( c == '\r' ) || ( c == '\t' ) ) advance(lexer);
        else if ( c == '\n' ) { lexer->line_no++; advance(lexer); }
        else if ( ( c == '/' ) && (peek_next(lexer) == '/' )) while (peek(lexer) != '\n' && is_at_end(lexer)) advance(lexer);
        else return;
    }
}

static token_t make_token(lexer_t* lexer, token_ty_t type)
{
    token_t token;
    token.type = type;
    token.start = lexer->start;
    token.length = (size_t)(lexer->curr - lexer->start);
    token.line_no = lexer->line_no;
    return token;
}

static token_t error_token(lexer_t* lexer, const char* message)
{
    token_t token;
    token.type = TOKEN_ERROR;
    token.start = message;
    token.length = (size_t)strlen(message);
    token.line_no = lexer->line_no;
    return token;
}

static token_t make_string(lexer_t* lexer, const char delimiter)
{
    while (peek(lexer) != delimiter && !is_at_end(lexer))
    {
        if (peek(lexer) == '\n') lexer->line_no++;
        advance(lexer);
    }

    if (is_at_end(lexer)) return error_token(lexer, "Unterminated string.");

    advance(lexer);
    return make_token(lexer, (delimiter == '"') ? TOKEN_STRING : TOKEN_CHAR);
}

static token_t make_number(lexer_t* lexer)
{
    while (is_digit(peek(lexer))) advance(lexer);

    if ((peek(lexer) == '.' || peek(lexer) == '#') && is_digit(peek_next(lexer)))
    {
        advance(lexer); // eat the dot/pound
        while (is_digit(peek(lexer))) advance(lexer);
    }

    return make_token(lexer, TOKEN_NUMBER);
}

static token_ty_t check_keyword(lexer_t* lexer, int start, int length, const char* rest, token_ty_t type)
{
    if ( ( lexer->curr - lexer->start == start + length ) && 
         ( memcmp(lexer->start + start, rest, length) == 0 ) )
    {
        return type;
    }

    return TOKEN_IDENTIFIER;
}

static token_ty_t identifier_type(lexer_t* lexer)
{
    switch(lexer->start[0]) // TODO: add the rest of the keywords
    {
        case 'e': 
            if (lexer->curr - lexer->start > 3)
            {
                switch (lexer->start[1])
                {
                    case 'l': return check_keyword(lexer, 2, 2, "se", TOKEN_ELSE);
                    case 'n': return (check_keyword(lexer, 2, 4, "tity", TOKEN_ENTITY) == TOKEN_ENTITY) ? TOKEN_ENTITY : check_keyword(lexer, 2, 3, "try", TOKEN_ENTRY);
                }
            }
            break;
        case 'f': 
            if (lexer->curr - lexer->start > 1)
            {
                switch (lexer->start[1])
                {
                    case 'o': return check_keyword(lexer, 2, 1, "r", TOKEN_FOR);
                    case 'a': return check_keyword(lexer, 2, 3, "lse", TOKEN_FALSE);
                }
            }
            break;
        case 'i': 
            if (lexer->curr - lexer->start > 1)
            {
                switch (lexer->start[1])
                {
                    case 'f': return check_keyword(lexer, 1, 1, "f", TOKEN_IF);
                    case 'm': return check_keyword(lexer, 1, 5, "mport", TOKEN_IF);
                }
            }
            break;
        case 'm': return check_keyword(lexer, 1, 5, "odule", TOKEN_MODULE);
        case 'n': return check_keyword(lexer, 1, 3, "ull", TOKEN_NULL);
        case 'p': 
            if (lexer->curr - lexer->start > 1)
            {
                switch (lexer->start[1])
                {
                    case 'u': return check_keyword(lexer, 2, 3, "ure", TOKEN_PURE);
                    case 'r': return check_keyword(lexer, 2, 3, "roc", TOKEN_PROC);
                }
            }
            break;
        case 'r': return check_keyword(lexer, 1, 5, "eturn", TOKEN_RETURN);
        case 't': return check_keyword(lexer, 1, 3, "rue", TOKEN_TRUE);
        case 'v': return check_keyword(lexer, 1, 2, "ar", TOKEN_VAR);
    }
    return TOKEN_IDENTIFIER;
}

static token_t make_identifier(lexer_t* lexer)
{
    while (is_alpha(peek(lexer)) || is_digit(peek(lexer))) advance(lexer);
    return make_token(lexer, identifier_type(lexer));
}

static bool match(lexer_t* lexer, char expected)
{
    if (is_at_end(lexer)) return false;
    if (*lexer->curr != expected) return false;

    lexer->curr++;

    return true;
}

token_t lex_token(lexer_t* lexer)
{
    skip_whitesace(lexer);
    lexer->start = lexer->curr;

    if (is_at_end(lexer)) return make_token(lexer, TOKEN_EOF);

    char c = advance(lexer);

    if ( c == '(' )         return make_token(lexer, TOKEN_LPAREN);
    else if ( c == ')' )    return make_token(lexer, TOKEN_RPAREN);
    else if ( c == '{' )    return make_token(lexer, TOKEN_LBRACE);
    else if ( c == '}' )    return make_token(lexer, TOKEN_RBRACE);
    else if ( c == '[' )    return make_token(lexer, TOKEN_LBRACKET);
    else if ( c == ']' )    return make_token(lexer, TOKEN_RBRACKET);
    else if ( c == ';' )    return make_token(lexer, TOKEN_SEMI);
    else if ( c == ':' )    return make_token(lexer, TOKEN_COLON);
    else if ( c == '?' )    return make_token(lexer, TOKEN_QUESTION);
    else if ( c == ',' )    return make_token(lexer, TOKEN_COMMA);
    else if ( c == '.' )    return make_token(lexer, TOKEN_DOT);
    else if ( c == '+' )    return make_token(lexer, TOKEN_PLUS);
    else if ( c == '-' )    return make_token(lexer, (match(lexer, '>') ? TOKEN_RIGHT_ARROW : TOKEN_MINUS));
    else if ( c == '/' )    return make_token(lexer, TOKEN_SLASH);
    else if ( c == '*' )    return make_token(lexer, TOKEN_ASTERISK);
    else if ( c == '~' )    return make_token(lexer, TOKEN_TILDE);
    else if ( c == '^' )    return make_token(lexer, TOKEN_XOR);
    else if ( c == '&' )    return make_token(lexer, (match(lexer, '&') ? TOKEN_AND : TOKEN_BIT_AND));
    else if ( c == '|' )    return make_token(lexer, (match(lexer, '|') ? TOKEN_OR : TOKEN_BIT_OR));
    else if ( c == '!' )    return make_token(lexer, (match(lexer, '=') ? TOKEN_BANG_EQUAL : TOKEN_BANG));
    else if ( c == '=' )    return make_token(lexer, (match(lexer, '=') ? TOKEN_EQUAL_EQUAL : TOKEN_EQUAL));
    else if ( c == '<' )    return make_token(lexer, (match(lexer, '=') ? TOKEN_LESS_EQUAL : (match(lexer, '<') ? TOKEN_LEFT_SHIFT : TOKEN_LESS)));
    else if ( c == '>' )    return make_token(lexer, (match(lexer, '=') ? TOKEN_GREATER_EQUAL : (match(lexer, '>') ? TOKEN_RIGHT_SHIFT : TOKEN_GREATER)));
    else if ( c == '"' )    return make_string(lexer, '"');
    else if ( c == '\'' )   return make_string(lexer, '\''); 
    else if ( is_digit(c) ) return make_number(lexer);
    else if ( is_alpha(c) ) return make_identifier(lexer);
    

    return error_token(lexer, "Unexpected character.");
}

Errno lex(lexer_t* lexer, const char* source)
{
    init_lexer(lexer, source);
    pos_t line = -1;
    while (true)
    {
        token_t token = lex_token(lexer);
        if (token.line_no != line)
        {
            printf("| 0x%04ld |", token.line_no);
            line = token.line_no;
        }
        else
        {
            printf("|        |");
        }

        printf(" 0x%02d | %-17.20s |", token.type, token_ty2label(token.type));
        printf(" %-20.*s|\n", (int)token.length, token.start);

        add_to_token_da(&lexer->tokens, token);

        if (token.type == TOKEN_ERROR) break;
        if (token.type == TOKEN_EOF) return 0;
    }

    return -1;
}

size_t token_length(token_t *token)
{
    return token->length;
}

void add_to_token_da(token_da* da, token_t val)
{
    if (da->count == da->capacity)
    {
        size_t new_cap = (0 == da->capacity ? 8 : da->capacity * 2);
        da->data = (token_t*)realloc(da->data, sizeof(token_t) * new_cap);
        da->capacity = new_cap;
    }
    da->data[da->count++] = val;
}

void init_token_da(token_da* da)
{
    da->count = da->capacity = 0;
    da->data = NULL;
}