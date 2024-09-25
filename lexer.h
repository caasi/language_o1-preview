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
    TOKEN_EOF,
} TokenType;

typedef struct
{
    TokenType type;
    double value; // Used if type is TOKEN_NUMBER
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

#endif // LEXER_H