#ifndef LEXER_H
#define LEXER_H

#include <stdlib.h>

typedef enum
{
    TOKEN_NUMBER,
    TOKEN_STRING,
    TOKEN_PLUS,   // '+'
    TOKEN_MINUS,  // '-'
    TOKEN_MUL,    // '*'
    TOKEN_DIV,    // '/'
    TOKEN_LPAREN, // '('
    TOKEN_RPAREN, // ')'
    TOKEN_IDENTIFIER,
    TOKEN_TYPE,          // 'type' keyword
    TOKEN_PIPE,          // '|'
    TOKEN_LBRACE,        // '{'
    TOKEN_RBRACE,        // '}'
    TOKEN_TYPE_NUMBER,   // 'Number'
    TOKEN_TYPE_STRING,   // 'String'
    TOKEN_KEYWORD_FUN,   // 'fun'
    TOKEN_KEYWORD_LET,   // 'let'
    TOKEN_KEYWORD_IN,    // 'in'
    TOKEN_KEYWORD_END,   // 'end'
    TOKEN_KEYWORD_IF,    // 'if'
    TOKEN_KEYWORD_THEN,  // 'then'
    TOKEN_KEYWORD_ELSE,  // 'else'
    TOKEN_KEYWORD_CASE,  // 'case'
    TOKEN_KEYWORD_OF,    // 'of'
    TOKEN_EQUAL_EQUAL,   // '=='
    TOKEN_NOT_EQUAL,     // '!='
    TOKEN_LESS,          // '<'
    TOKEN_LESS_EQUAL,    // '<='
    TOKEN_GREATER,       // '>'
    TOKEN_GREATER_EQUAL, // '>='
    TOKEN_ARROW,         // '->'
    TOKEN_FAT_ARROW,     // '=>'
    TOKEN_BACKSLASH,     // '\' (lambda)
    TOKEN_DOT,           // '.'
    TOKEN_COMMA,         // ','
    TOKEN_SEMICOLON,     // ';'
    TOKEN_EQUAL,         // '='
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

const char *token_type_to_string(TokenType type);

Lexer lexer_create(const char *text);
char lexer_peek(Lexer *lexer);
void lexer_advance(Lexer *lexer);
void lexer_skip_whitespace(Lexer *lexer);
void lexer_skip_single_line_comment(Lexer *lexer);
void lexer_skip_multi_line_comment(Lexer *lexer);
Token lexer_get_number(Lexer *lexer);
Token lexer_get_next_token(Lexer *lexer);
Token lexer_get_identifier(Lexer *lexer);

#endif // LEXER_H