

#ifndef CIAS_LEXER_H_
#define CIAS_LEXER_H_

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

typedef uint64_t    pos_t;
typedef int         Errno;

#define TOKEN_LIST                                              \
    X(TOKEN_AND,            "&&",       "TOKEN_AND")            \
    X(TOKEN_ASTERISK,       "*",        "TOKEN_ASTERISK")       \
    X(TOKEN_BANG,           "!",        "TOKEN_BANG")           \
    X(TOKEN_BANG_EQUAL,     "!=",       "TOKEN_BANG_EQUAL")     \
    X(TOKEN_BIT_AND,        "&",        "TOKEN_BIT_AND")        \
    X(TOKEN_BIT_OR,         "|",        "TOKEN_BIT_OR")         \
    X(TOKEN_BOOL,           "bool",     "TOKEN_BOOL")           \
    X(TOKEN_CHAR,           "char",     "TOKEN_CHAR")           \
    X(TOKEN_COLON,          ":",        "TOKEN_COLON")          \
    X(TOKEN_COMMA,          ",",        "TOKEN_COMMA")          \
    X(TOKEN_DOUBLE,         "double",   "TOKEN_DOUBLE")         \
    X(TOKEN_DOT,            ".",        "TOKEN_DOT")            \
    X(TOKEN_EQUAL,          "=",        "TOKEN_EQUAL")          \
    X(TOKEN_EQUAL_EQUAL,    "==",       "TOKEN_EQUAL_EQUAL")    \
    X(TOKEN_GREATER,        ">",        "TOKEN_GREATER")        \
    X(TOKEN_GREATER_EQUAL,  ">=",       "TOKEN_GREATER_EQUAL")  \
    X(TOKEN_ELSE,           "else",     "TOKEN_ELSE")           \
    X(TOKEN_ENTITY,         "entity",   "TOKEN_ENTITY")         \
    X(TOKEN_ENTRY,          "entry",    "TOKEN_ENTRY")          \
    X(TOKEN_EOF,            "eof",      "TOKEN_EOF")            \
    X(TOKEN_ERROR,          "error",    "TOKEN_ERROR")          \
    X(TOKEN_FALSE,          "false",    "TOKEN_FALSE")          \
    X(TOKEN_FOR,            "for",      "TOKEN_FOR")            \
    X(TOKEN_FLOAT,          "float",    "TOKEN_FLOAT")          \
    X(TOKEN_IDENTIFIER,     "ident",    "TOKEN_IDENTIFIER")     \
    X(TOKEN_I8,             "i8",       "TOKEN_I8")             \
    X(TOKEN_I16,            "i16",      "TOKEN_I16")            \
    X(TOKEN_I32,            "i32",      "TOKEN_I32")            \
    X(TOKEN_I64,            "i64",      "TOKEN_I64")            \
    X(TOKEN_IF,             "if",       "TOKEN_IF")             \
    X(TOKEN_LBRACE,         "{",        "TOKEN_LBRACE")         \
    X(TOKEN_LBRACKET,       "[",        "TOKEN_LBRACKET")       \
    X(TOKEN_LEFT_SHIFT,     "<<",       "TOKEN_LEFT_SHIFT")     \
    X(TOKEN_LESS,           "<",        "TOKEN_LESS")           \
    X(TOKEN_LESS_EQUAL,     "<=",       "TOKEN_LESS_EQUAL")     \
    X(TOKEN_LOOP,           "loop",     "TOKEN_LOOP")           \
    X(TOKEN_LPAREN,         "(",        "TOKEN_LPAREN")         \
    X(TOKEN_IMPORT,         "import",   "TOKEN_IMPORT")         \
    X(TOKEN_MINUS,          "-",        "TOKEN_MINUS")          \
    X(TOKEN_MODULE,         "module",   "TOKEN_MODULE")         \
    X(TOKEN_MODULO,         "%",        "TOKEN_MODULO")         \
    X(TOKEN_NUMBER,         "number",   "TOKEN_NUMBER")         \
    X(TOKEN_NULL,           "null",     "TOKEN_NULL")           \
    X(TOKEN_OR,             "||",       "TOKEN_OR")             \
    X(TOKEN_PLUS,           "+",        "TOKEN_PLUS")           \
    X(TOKEN_PURE,           "pure",     "TOKEN_PURE")           \
    X(TOKEN_PROC,           "proc",     "TOKEN_PROC")           \
    X(TOKEN_QUESTION,       "?",        "TOKEN_QUESTION")       \
    X(TOKEN_RBRACE,         "}",        "TOKEN_RBRACE")         \
    X(TOKEN_RBRACKET,       "]",        "TOKEN_RBRACKET")       \
    X(TOKEN_RECORD,         "record",   "TOKEN_RECORD")         \
    X(TOKEN_RETURN,         "return",   "TOKEN_RETURN")         \
    X(TOKEN_RIGHT_ARROW,    "->",       "TOKEN_RIGHT_ARROW")    \
    X(TOKEN_RIGHT_SHIFT,    ">>",       "TOKEN_RIGHT_SHIFT")    \
    X(TOKEN_RPAREN,         ")",        "TOKEN_RPAREN")         \
    X(TOKEN_SEMI,           ";",        "TOKEN_SEMI")           \
    X(TOKEN_SLASH,          "/",        "TOKEN_SLASH")          \
    X(TOKEN_STRING,         "str",      "TOKEN_STRING")         \
    X(TOKEN_TILDE,          "~",        "TOKEN_TILDE")          \
    X(TOKEN_TRUE,           "true",     "TOKEN_TRUE")           \
    X(TOKEN_VAR,            "var",      "TOKEN_VAR")            \
    X(TOKEN_VOID,           "void",     "TOKEN_VOID")           \
    X(TOKEN_XOR,            "^",        "TOKEN_XOR")            \
    

typedef enum {
#define X(type, val, label) type,
    TOKEN_LIST
#undef X
} token_ty_t;

typedef struct {
    token_ty_t          type;
    const char*         start;
    size_t              length;
    pos_t               line_no;
} token_t;

typedef struct {
    token_t*            data;
    size_t              count;
    size_t              capacity;
} token_da;

typedef token_da        token_list_t;

typedef struct {
    const char*         start;
    const char*         curr;
    size_t              line_no;
    token_list_t        tokens;
} lexer_t;

void        init_lexer(lexer_t* lexer, const char* source);
token_t     lex_token(lexer_t* lexer);
Errno       lex(lexer_t* lexer, const char* source);

void add_to_token_da(token_da* da, token_t val);
void init_token_da(token_da* da);

#endif // CIAS_LEXER_H_