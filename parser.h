#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"

typedef enum
{
    AST_NUMBER,
    AST_STRING,
    AST_BOOL,
    AST_BINOP,
    AST_VARIABLE,
    AST_FUNCTION_DEF,
    AST_FUNCTION_CALL,
    AST_ADT_CONSTRUCTOR,
    AST_ADT_DEFINITION,
    AST_LET_BINDING,
    AST_STATEMENT_LIST,
    AST_IF_EXPR,
} ASTNodeType;

typedef struct Type
{
    char *name; // Type name
    enum
    {
        TYPE_BASIC,
        TYPE_ADT,
        TYPE_FUNCTION,
        TYPE_POLYMORPHIC,
        // ... other type categories as needed
    } kind;
    struct Type **params; // Type parameters for polymorphism (if needed)
    int param_count;
} Type;

typedef struct ASTNode
{
    ASTNodeType type;
    union
    {
        double number;      // For AST_NUMBER
        char *string_value; // For AST_STRING
        int bool_value;     // For AST_BOOL
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
            Type *return_type;
            Type **param_types; // Arrary of types for parameters
        } function_def;         // For AST_FUNCTION_DEF
        struct
        {                               // AST_FUNCTION_CALL
            char *name;                 // Function name
            struct ASTNode **arguments; // Argument expressions
            int arg_count;
        } function_call;
        struct
        {
            char *type_name;            // The ADT type name (e.g. "Maybe")
            char *constructor;          // The constructor name (e.g. "Just")
            struct ASTNode **arguments; // Arguments passed to the constructor
            int arg_count;
        } adt_constructor; // For AST_ADT_CONSTRUCTOR
        struct
        {
            char *type_name;
            struct ASTNode **constructors;
            int constructor_count;
        } adt_definition; // For AST_ADT_DEFINITION
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
        struct
        { // AST_IF_EXPR
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

Type *parse_type(Parser *parser);
Type *parse_atomic_type(Parser *parser);

ASTNode *parse_term(Parser *parser);
ASTNode *parse_string(Parser *parser);
ASTNode *parse_if_expression(Parser *parser);
ASTNode *parse_multiplicative_expression(Parser *parser);
ASTNode *parse_additive_expression(Parser *parser);
ASTNode *parse_expression(Parser *parser);
ASTNode *parse_adt_definition(Parser *parser);
ASTNode *parse_adt_constructor(Parser *parser);
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
