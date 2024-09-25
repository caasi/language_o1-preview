#ifndef LEXER_H
#define LEXER_H

#include <stdlib.h>

typedef enum
{
    TOKEN_NUMBER,
    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_MUL,
    TOKEN_DIV,
    TOKEN_LPAREN,
    TOKEN_RPAREN,
    TOKEN_IDENTIFIER,
    TOKEN_KEYWORD_FUN,
    TOKEN_KEYWORD_LET,
    TOKEN_KEYWORD_IN,
    TOKEN_KEYWORD_END,
    TOKEN_ARROW,     // '->'
    TOKEN_COMMA,     // ','
    TOKEN_SEMICOLON, // ';'
    TOKEN_EQUAL,     // '='
    TOKEN_EOF,
} TokenType;

typedef struct
{
    TokenType type;
    double value; // Used if type is TOKEN_NUMBER
    char *text;   // Used if type is TOKEN_IDENTIFIER or keyword
} Token;

typedef struct
{
    const char *text;
    size_t pos;
    char current_char;
} Lexer;

Lexer lexer_create(const char *text);
void lexer_advance(Lexer *lexer);
void lexer_skip_whitespace(Lexer *lexer);
Token lexer_get_number(Lexer *lexer);
Token lexer_get_next_token(Lexer *lexer);
Token lexer_get_identifier(Lexer *lexer);

#endif // LEXER_H