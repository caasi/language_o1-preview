#include <stdio.h>
#include <stdlib.h>
#include "parser.h"

Parser parser_create(Lexer lexer)
{
    Parser parser;
    parser.lexer = lexer;
    parser.current_token = lexer_get_next_token(&parser.lexer);
    return parser;
}

void parser_eat(Parser *parser, TokenType token_type)
{
    if (parser->current_token.type == token_type)
    {
        parser->current_token = lexer_get_next_token(&parser->lexer);
    }
    else
    {
        fprintf(stderr, "Error: Expected token type %d but got %d\n", token_type, parser->current_token.type);
        exit(EXIT_FAILURE);
    }
}

ASTNode *parse_factor(Parser *parser)
{
    Token token = parser->current_token;

    if (token.type == TOKEN_NUMBER)
    {
        parser_eat(parser, TOKEN_NUMBER);

        ASTNode *node = malloc(sizeof(ASTNode));
        node->type = AST_NUMBER;
        node->number = token.value;
        return node;
    }
    else if (token.type == TOKEN_LPAREN)
    {
        parser_eat(parser, TOKEN_LPAREN);
        ASTNode *node = parse_expression(parser);
        parser_eat(parser, TOKEN_RPAREN);
        return node;
    }
    else
    {
        fprintf(stderr, "Error: Unexpected token '%d' in factor\n", token.type);
        exit(EXIT_FAILURE);
    }
}

ASTNode *parse_term(Parser *parser)
{
    ASTNode *node = parse_factor(parser);

    while (parser->current_token.type == TOKEN_MUL || parser->current_token.type == TOKEN_DIV)
    {
        Token token = parser->current_token;
        if (token.type == TOKEN_MUL)
        {
            parser_eat(parser, TOKEN_MUL);
        }
        else if (token.type == TOKEN_DIV)
        {
            parser_eat(parser, TOKEN_DIV);
        }

        ASTNode *new_node = malloc(sizeof(ASTNode));
        new_node->type = AST_BINOP;
        new_node->binop.left = node;
        new_node->binop.op = token.type;
        new_node->binop.right = parse_factor(parser);

        node = new_node;
    }

    return node;
}

ASTNode *parse_expression(Parser *parser)
{
    ASTNode *node = parse_term(parser);

    while (parser->current_token.type == TOKEN_PLUS || parser->current_token.type == TOKEN_MINUS)
    {
        Token token = parser->current_token;
        if (token.type == TOKEN_PLUS)
        {
            parser_eat(parser, TOKEN_PLUS);
        }
        else if (token.type == TOKEN_MINUS)
        {
            parser_eat(parser, TOKEN_MINUS);
        }

        ASTNode *new_node = malloc(sizeof(ASTNode));
        new_node->type = AST_BINOP;
        new_node->binop.left = node;
        new_node->binop.op = token.type;
        new_node->binop.right = parse_term(parser);

        node = new_node;
    }

    return node;
}

void free_ast(ASTNode *node)
{
    if (node == NULL)
        return;

    if (node->type == AST_BINOP)
    {
        free_ast(node->binop.left);
        free_ast(node->binop.right);
    }

    free(node);
}

void print_ast(ASTNode *node, int indent)
{
    if (node == NULL)
        return;

    // Print indentation spaces
    for (int i = 0; i < indent; i++)
    {
        printf("  "); // Two spaces per indent level
    }

    // Check the node type
    if (node->type == AST_NUMBER)
    {
        // Print the number node
        printf("Number: %lf\n", node->number);
    }
    else if (node->type == AST_BINOP)
    {
        // Print the operator
        const char *op_str;
        switch (node->binop.op)
        {
        case TOKEN_PLUS:
            op_str = "+";
            break;
        case TOKEN_MINUS:
            op_str = "-";
            break;
        case TOKEN_MUL:
            op_str = "*";
            break;
        case TOKEN_DIV:
            op_str = "/";
            break;
        default:
            op_str = "?";
            break;
        }
        printf("Operator: %s\n", op_str);

        // Recursively print left and right subtrees
        // Increase indent level for child nodes
        print_ast(node->binop.left, indent + 1);
        print_ast(node->binop.right, indent + 1);
    }
    else
    {
        // Handle any other node types if present
        printf("Unknown node type\n");
    }
}
