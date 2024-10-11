

#ifndef CIAS_LEXER_H_
#define CIAS_LEXER_H_

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

typedef uint64_t pos_t;

#define TOKEN_LIST \
    X(TOKEN_LPAREN, "(", "TOKEN_LPAREN") \
    X(TOKEN_RPAREN, ")", "TOKEN_RPAREN") \
    X(TOKEN_LBRACE, "{", "TOKEN_LBRACE") \
    X(TOKEN_RBRACE, "}", "TOKEN_RBRACE") \
    X(TOKEN_LBRACKET, "[", "TOKEN_LBRACKET") \
    X(TOKEN_RBRACKET, "]", "TOKEN_RBRACKET") \
    X(TOKEN_COMMA, ",", "TOKEN_COMMA") \
    X(TOKEN_DOT, ".", "TOKEN_DOT") \
    X(TOKEN_MINUS, "-", "TOKEN_MINUS") \
    X(TOKEN_PLUS, "+", "TOKEN_PLUS") \
    X(TOKEN_SEMI, ";", "TOKEN_SEMI") \
    X(TOKEN_SLASH, "/", "TOKEN_SLASH") \
    X(TOKEN_ASTERISK, "*", "TOKEN_ASTERISK") \
    X(TOKEN_BANG, "!", "TOKEN_BANG") \
    X(TOKEN_BANG_EQUAL, "!=", "TOKEN_BANG_EQUAL") \
    X(TOKEN_EQUAL, "=", "TOKEN_EQUAL") \
    X(TOKEN_EQUAL_EQUAL, "==", "TOKEN_EQUAL_EQUAL") \
    X(TOKEN_GREATER, ">", "TOKEN_GREATER") \
    X(TOKEN_GREATER_EQUAL, ">=", "TOKEN_GREATER_EQUAL") \
    X(TOKEN_LESS, "<", "TOKEN_LESS") \
    X(TOKEN_LESS_EQUAL, "<=", "TOKEN_LESS_EQUAL") \
    X(TOKEN_IDENTIFIER, "identifier", "TOKEN_IDENTIFIER") \
    X(TOKEN_NUMBER, "number", "TOKEN_NUMBER") \
    X(TOKEN_STRING, "str", "TOKEN_STRING") \
    X(TOKEN_CHAR, "char", "TOKEN_CHAR") \
    X(TOKEN_VAR, "var", "TOKEN_VAR") \
    X(TOKEN_RETURN, "return", "TOKEN_RETURN") \
    X(TOKEN_EOF, "eof", "TOKEN_EOF") \
    X(TOKEN_ERROR, "error", "TOKEN_ERROR") \


typedef enum {
#define X(type, val, label) type,
    TOKEN_LIST
#undef X
} token_ty_t;

typedef struct {
    token_ty_t type;
    const char* start;
    size_t length;
    pos_t line_no;
} token_t;

typedef struct {
    const char* start;
    const char* curr;
    size_t line_no;
} lexer_t;

void init_lexer(lexer_t* lexer, const char* source);
token_t lex_token(lexer_t* lexer);
void lex(lexer_t* lexer, const char* source);

#endif // CIAS_LEXER_H_