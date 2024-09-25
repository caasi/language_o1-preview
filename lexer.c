#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
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
            return (Token){TOKEN_PLUS, 0, NULL};
        }
        if (lexer->current_char == '-')
        {
            lexer_advance(lexer);
            return (Token){TOKEN_MINUS, 0, NULL};
        }
        if (lexer->current_char == '*')
        {
            lexer_advance(lexer);
            return (Token){TOKEN_MUL, 0, NULL};
        }
        if (lexer->current_char == '/')
        {
            lexer_advance(lexer);
            return (Token){TOKEN_DIV, 0, NULL};
        }
        if (lexer->current_char == '(')
        {
            lexer_advance(lexer);
            return (Token){TOKEN_LPAREN, 0, NULL};
        }
        if (lexer->current_char == ')')
        {
            lexer_advance(lexer);
            return (Token){TOKEN_RPAREN, 0, NULL};
        }

        // Idenitifier or keywords
        if (isalpha(lexer->current_char) || lexer->current_char == '_')
        {
            return lexer_get_identifier(lexer);
        }

        // Arrow '->'
        if (lexer->current_char == '-')
        {
            if (lexer->text[lexer->pos + 1] == '>')
            {
                lexer_advance(lexer); // Skip '-'
                lexer_advance(lexer); // Skip '>'
                return (Token){TOKEN_ARROW, 0, NULL};
            }
            else
            {
                // Handle minus operator
                lexer_advance(lexer);
                return (Token){TOKEN_MINUS, 0, NULL};
            }
        }

        // Comma ','
        if (lexer->current_char == ',')
        {
            lexer_advance(lexer);
            return (Token){TOKEN_COMMA, 0, NULL};
        }

        // Semicolon ';'
        if (lexer->current_char == ';')
        {
            lexer_advance(lexer);
            return (Token){TOKEN_SEMICOLON, 0, NULL};
        }

        // Equal '='
        if (lexer->current_char == '=')
        {
            lexer_advance(lexer);
            return (Token){TOKEN_EQUAL, 0, NULL};
        }

        fprintf(stderr, "Error: Unknown character '%c'\n", lexer->current_char);
        exit(EXIT_FAILURE);
    }

    return (Token){TOKEN_EOF, 0, NULL};
}

int is_identifier_char(char c)
{
    return isalnum(c) || c == '_';
}

Token lexer_get_identifier(Lexer *lexer)
{
    char buffer[64];
    int i = 0;

    while (lexer->current_char != '\0' && is_identifier_char(lexer->current_char))
    {
        buffer[i++] = lexer->current_char;
        lexer_advance(lexer);
    }
    buffer[i] = '\0';

    // Check for keywords
    if (strcmp(buffer, "fun") == 0)
    {
        return (Token){TOKEN_KEYWORD_FUN, 0, NULL};
    }
    else if (strcmp(buffer, "let") == 0)
    {
        return (Token){TOKEN_KEYWORD_LET, 0, NULL};
    }
    else if (strcmp(buffer, "in") == 0)
    {
        return (Token){TOKEN_KEYWORD_IN, 0, NULL};
    }
    else if (strcmp(buffer, "end") == 0)
    {
        return (Token){TOKEN_KEYWORD_END, 0, NULL};
    }
    else
    {
        // It's an identifier
        Token token;
        token.type = TOKEN_IDENTIFIER;
        token.value = 0;
        token.text = strdup(buffer); // Store the identifier name
        return token;
    }
}
