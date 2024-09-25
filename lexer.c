#include <stdio.h>
#include <ctype.h>
#include "lexer.h"

Lexer lexer_create(const char *text)
{
    Lexer lexer;
    lexer.text = text;
    lexer.pos = 0;
    lexer.current_char = lexer.text[lexer.pos];
    return lexer;
}

void lexer_advance(Lexer *lexer)
{
    lexer->pos++;
    lexer->current_char = lexer->text[lexer->pos];
}

void lexer_skip_whitespace(Lexer *lexer)
{
    while (lexer->current_char != '\0' && isspace(lexer->current_char))
    {
        lexer_advance(lexer);
    }
}

Token lexer_get_number(Lexer *lexer)
{
    char buffer[64];
    int i = 0;

    while (lexer->current_char != '\0' && (isdigit(lexer->current_char) || lexer->current_char == '.'))
    {
        buffer[i++] = lexer->current_char;
        lexer_advance(lexer);
    }
    buffer[i] = '\0';

    Token token;
    token.type = TOKEN_NUMBER;
    token.value = atof(buffer);
    return token;
}

Token lexer_get_next_token(Lexer *lexer)
{
    while (lexer->current_char != '\0')
    {
        if (isspace(lexer->current_char))
        {
            lexer_skip_whitespace(lexer);
            continue;
        }
        if (isdigit(lexer->current_char) || lexer->current_char == '.')
        {
            return lexer_get_number(lexer);
        }
        if (lexer->current_char == '+')
        {
            lexer_advance(lexer);
            return (Token){TOKEN_PLUS, 0};
        }
        if (lexer->current_char == '-')
        {
            lexer_advance(lexer);
            return (Token){TOKEN_MINUS, 0};
        }
        if (lexer->current_char == '*')
        {
            lexer_advance(lexer);
            return (Token){TOKEN_MUL, 0};
        }
        if (lexer->current_char == '/')
        {
            lexer_advance(lexer);
            return (Token){TOKEN_DIV, 0};
        }
        if (lexer->current_char == '(')
        {
            lexer_advance(lexer);
            return (Token){TOKEN_LPAREN, 0};
        }
        if (lexer->current_char == ')')
        {
            lexer_advance(lexer);
            return (Token){TOKEN_RPAREN, 0};
        }

        fprintf(stderr, "Error: Unknown character '%c'\n", lexer->current_char);
        exit(EXIT_FAILURE);
    }

    return (Token){TOKEN_EOF, 0};
}
