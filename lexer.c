#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "lexer.h"

const char *token_type_to_string(TokenType type)
{
    switch (type)
    {
    case TOKEN_NUMBER:
        return "Number";
    case TOKEN_STRING:
        return "String";
    case TOKEN_PLUS:
        return "Plus";
    case TOKEN_MINUS:
        return "Minus";
    case TOKEN_MUL:
        return "Multiply";
    case TOKEN_DIV:
        return "Divide";
    case TOKEN_LPAREN:
        return "Left Parenthesis";
    case TOKEN_RPAREN:
        return "Right Parenthesis";
    case TOKEN_IDENTIFIER:
        return "Identifier";
    case TOKEN_TYPE:
        return "Type";
    case TOKEN_PIPE:
        return "Pipe";
    case TOKEN_LBRACE:
        return "Left Brace";
    case TOKEN_RBRACE:
        return "Right Brace";
    case TOKEN_TYPE_NUMBER:
        return "Number Type";
    case TOKEN_TYPE_STRING:
        return "String Type";
    case TOKEN_KEYWORD_FUN:
        return "Function Keyword";
    case TOKEN_KEYWORD_LET:
        return "Let Keyword";
    case TOKEN_KEYWORD_IN:
        return "In Keyword";
    case TOKEN_KEYWORD_END:
        return "End Keyword";
    case TOKEN_KEYWORD_IF:
        return "If Keyword";
    case TOKEN_KEYWORD_THEN:
        return "Then Keyword";
    case TOKEN_KEYWORD_ELSE:
        return "Else Keyword";
    case TOKEN_EQUAL_EQUAL:
        return "Equal Equal";
    case TOKEN_NOT_EQUAL:
        return "Not Equal";
    case TOKEN_LESS:
        return "Less Than";
    case TOKEN_LESS_EQUAL:
        return "Less Than or Equal";
    case TOKEN_GREATER:
        return "Greater Than";
    case TOKEN_GREATER_EQUAL:
        return "Greater Than or Equal";
    case TOKEN_ARROW:
        return "Arrow";
    case TOKEN_COMMA:
        return "Comma";
    case TOKEN_SEMICOLON:
        return "Semicolon";
    case TOKEN_EQUAL:
        return "Equal";
    case TOKEN_EOF:
        return "End of File";
    default:
        return "Unknown token type";
    }
}

Lexer lexer_create(const char *text)
{
    Lexer lexer;
    lexer.text = text;
    lexer.pos = 0;
    lexer.current_char = lexer.text[lexer.pos];
    return lexer;
}

char lexer_peek(Lexer *lexer)
{
    if (lexer->pos + 1 < strlen(lexer->text))
    {
        return lexer->text[lexer->pos + 1];
    }
    else
    {
        return '\0';
    }
}

void lexer_advance(Lexer *lexer)
{
    lexer->pos++;
    if (lexer->pos < strlen(lexer->text))
    {
        lexer->current_char = lexer->text[lexer->pos];
    }
    else
    {
        lexer->current_char = '\0';
    }
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

Token lexer_get_string(Lexer *lexer)
{
    lexer_advance(lexer); // Skip the opening quote

    char buffer[1024]; // Adjust size as needed
    int length = 0;

    while (lexer->current_char != '"' && lexer->current_char != '\0')
    {
        buffer[length++] = lexer->current_char;
        lexer_advance(lexer);
    }

    if (lexer->current_char == '\0')
    {
        fprintf(stderr, "Error: Unterminated string literal\n");
        exit(EXIT_FAILURE);
    }

    buffer[length] = '\0';
    lexer_advance(lexer); // Skip the closing quote

    // Return the string token
    return (Token){TOKEN_STRING, 0, strdup(buffer)};
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
        if (lexer->current_char == '"')
        {
            return lexer_get_string(lexer);
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

        // Handle '|'
        if (lexer->current_char == '|')
        {
            lexer_advance(lexer);
            return (Token){TOKEN_PIPE, 0, NULL};
        }

        // Handle '{' and '}'
        if (lexer->current_char == '{')
        {
            lexer_advance(lexer);
            return (Token){TOKEN_LBRACE, 0, NULL};
        }
        if (lexer->current_char == '}')
        {
            lexer_advance(lexer);
            return (Token){TOKEN_RBRACE, 0, NULL};
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
            if (lexer_peek(lexer) == '=')
            {
                lexer_advance(lexer); // Skip '='
                lexer_advance(lexer); // Skip second '='
                return (Token){TOKEN_EQUAL_EQUAL, 0, NULL};
            }
            else
            {
                lexer_advance(lexer);
                return (Token){TOKEN_EQUAL, 0, NULL};
            }
        }

        if (lexer->current_char == '!')
        {
            if (lexer_peek(lexer) == '=')
            {
                lexer_advance(lexer); // Skip '!'
                lexer_advance(lexer); // Skip '='
                return (Token){TOKEN_NOT_EQUAL, 0, NULL};
            }
            else
            {
                fprintf(stderr, "Error: Unexpected character '!'\n");
                exit(EXIT_FAILURE);
            }
        }

        if (lexer->current_char == '<')
        {
            if (lexer_peek(lexer) == '=')
            {
                lexer_advance(lexer); // Skip '<'
                lexer_advance(lexer); // Skip '='
                return (Token){TOKEN_LESS_EQUAL, 0, NULL};
            }
            else
            {
                lexer_advance(lexer);
                return (Token){TOKEN_LESS, 0, NULL};
            }
        }

        if (lexer->current_char == '>')
        {
            if (lexer_peek(lexer) == '=')
            {
                lexer_advance(lexer); // Skip '>'
                lexer_advance(lexer); // Skip '='
                return (Token){TOKEN_GREATER_EQUAL, 0, NULL};
            }
            else
            {
                lexer_advance(lexer);
                return (Token){TOKEN_GREATER, 0, NULL};
            }
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
    if (strcmp(buffer, "type") == 0)
    {
        return (Token){TOKEN_TYPE, 0, NULL};
    }
    if (strcmp(buffer, "Number") == 0)
    {
        return (Token){TOKEN_TYPE_NUMBER, 0, NULL};
    }
    else if (strcmp(buffer, "String") == 0)
    {
        return (Token){TOKEN_TYPE_STRING, 0, NULL};
    }
    else if (strcmp(buffer, "fun") == 0)
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
    else if (strcmp(buffer, "if") == 0)
    {
        return (Token){TOKEN_KEYWORD_IF, 0, NULL};
    }
    else if (strcmp(buffer, "then") == 0)
    {
        return (Token){TOKEN_KEYWORD_THEN, 0, NULL};
    }
    else if (strcmp(buffer, "else") == 0)
    {
        return (Token){TOKEN_KEYWORD_ELSE, 0, NULL};
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
