#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"

typedef enum
{
    AST_NUMBER,
    AST_BINOP,
    AST_VARIABLE,
    AST_FUNCTION_DEF,
    AST_FUNCTION_CALL,
    AST_LET_BINDING,
    AST_STATEMENT_LIST,
    AST_IF_EXPR,
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
        } binop;    // For AST_BINOP
        char *name; // AST_VARIABLE, variable name
        struct
        {                      // AST_FUNCTION_DEF
            char *name;        // Function name
            char **parameters; // Parameter names
            int param_count;
            struct ASTNode *body; // Function body
        } function_def;
        struct
        {                               // AST_FUNCTION_CALL
            char *name;                 // Function name
            struct ASTNode **arguments; // Argument expressions
            int arg_count;
        } function_call;
        struct
        {                          // AST_LET_BINDING
            char *name;            // Variable name
            struct ASTNode *value; // Value expression
            struct ASTNode *body;  // Expression where the binding is used
        } let_binding;
        struct
        {                                // AST_STATEMENT_LIST
            struct ASTNode **statements; // Array of statements
            int statement_count;         // Number of statements
        } statement_list;
        struct { // AST_IF_EXPR
            struct ASTNode *condition;
            struct ASTNode *then_branch;
            struct ASTNode *else_branch;
        } if_expr;
    };
} ASTNode;

typedef struct
{
    Lexer lexer;
    Token current_token;
} Parser;

Parser parser_create(Lexer lexer);
void parser_eat(Parser *parser, TokenType token_type);

ASTNode *parse_term(Parser *parser);
ASTNode *parse_if_expression(Parser *parser);
ASTNode *parse_multiplicative_expression(Parser *parser);
ASTNode *parse_additive_expression(Parser *parser);
ASTNode *parse_expression(Parser *parser);
ASTNode *parse_comparison(Parser *parser);
ASTNode *parse_factor(Parser *parser);
ASTNode *parse_function_definition(Parser *parser);
ASTNode *parse_function_application(Parser *parser, char *func_name);
ASTNode *parse_let_binding(Parser *parser);
ASTNode *parse_statement_list(Parser *parser);
ASTNode *parse_statement(Parser *parser);

void free_ast(ASTNode *node);

void print_ast(ASTNode *node, int indent);

#endif // PARSER_H
