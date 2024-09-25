#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"

typedef enum
{
    AST_NUMBER,
    AST_BINOP
} ASTNodeType;

typedef struct ASTNode
{
    ASTNodeType type;
    union
    {
        double number; // For AST_NUMBER
        struct
        {
            struct ASTNode *left;
            TokenType op; // Operator token type
            struct ASTNode *right;
        } binop; // For AST_BINOP
    };
} ASTNode;

typedef struct
{
    Lexer lexer;
    Token current_token;
} Parser;

Parser parser_create(Lexer lexer);
void parser_eat(Parser *parser, TokenType token_type);

ASTNode *parse_expression(Parser *parser);
ASTNode *parse_term(Parser *parser);
ASTNode *parse_factor(Parser *parser);

void free_ast(ASTNode *node);

void print_ast(ASTNode *node, int indent);

#endif // PARSER_H
