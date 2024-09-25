#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

ASTNode *parse_string(Parser *parser) {
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = AST_STRING;
    node->string_value = strdup(parser->current_token.text);
    parser_eat(parser, TOKEN_STRING);
    return node;
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
    else if (token.type == TOKEN_STRING) {
        return parse_string(parser);
    }
    else if (token.type == TOKEN_LPAREN)
    {
        parser_eat(parser, TOKEN_LPAREN);
        ASTNode *node = parse_expression(parser);
        parser_eat(parser, TOKEN_RPAREN);
        return node;
    }
    else if (token.type == TOKEN_IDENTIFIER)
    {
        char *name = strdup(token.text);
        parser_eat(parser, TOKEN_IDENTIFIER);

        if (parser->current_token.type == TOKEN_LPAREN)
        {
            // Function call
            return parse_function_application(parser, name);
        }
        else
        {
            // Variable reference
            ASTNode *node = malloc(sizeof(ASTNode));
            node->type = AST_VARIABLE;
            node->name = name;
            return node;
        }
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

ASTNode *parse_if_expression(Parser *parser) {
    parser_eat(parser, TOKEN_KEYWORD_IF);

    // Parse condition
    ASTNode *condition = parse_comparison(parser);

    // Expect 'then'
    parser_eat(parser, TOKEN_KEYWORD_THEN);

    // Parse 'then' branch
    ASTNode *then_branch = parse_expression(parser);

    // Expect 'else'
    parser_eat(parser, TOKEN_KEYWORD_ELSE);

    // Parse 'else' branch
    ASTNode *else_branch = parse_expression(parser);

    // Create if expression node
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = AST_IF_EXPR;
    node->if_expr.condition = condition;
    node->if_expr.then_branch = then_branch;
    node->if_expr.else_branch = else_branch;

    return node;
}

ASTNode *parse_multiplicative_expression(Parser *parser) {
    ASTNode *node = parse_factor(parser);

    while (parser->current_token.type == TOKEN_MUL ||
           parser->current_token.type == TOKEN_DIV) {

        TokenType op = parser->current_token.type;
        parser_eat(parser, op);

        ASTNode *right = parse_factor(parser);

        // Create a binary operation node
        ASTNode *new_node = malloc(sizeof(ASTNode));
        new_node->type = AST_BINOP;
        new_node->binop.left = node;
        new_node->binop.op = op;
        new_node->binop.right = right;

        node = new_node;
    }

    return node;
}

ASTNode *parse_additive_expression(Parser *parser) {
    ASTNode *node = parse_multiplicative_expression(parser);

    while (parser->current_token.type == TOKEN_PLUS ||
           parser->current_token.type == TOKEN_MINUS) {

        TokenType op = parser->current_token.type;
        parser_eat(parser, op);

        ASTNode *right = parse_multiplicative_expression(parser);

        // Create a binary operation node
        ASTNode *new_node = malloc(sizeof(ASTNode));
        new_node->type = AST_BINOP;
        new_node->binop.left = node;
        new_node->binop.op = op;
        new_node->binop.right = right;

        node = new_node;
    }

    return node;
}

ASTNode *parse_comparison(Parser *parser) {
    ASTNode *node = parse_additive_expression(parser);

    while (parser->current_token.type == TOKEN_EQUAL_EQUAL ||
           parser->current_token.type == TOKEN_NOT_EQUAL ||
           parser->current_token.type == TOKEN_LESS ||
           parser->current_token.type == TOKEN_LESS_EQUAL ||
           parser->current_token.type == TOKEN_GREATER ||
           parser->current_token.type == TOKEN_GREATER_EQUAL) {

        TokenType op = parser->current_token.type;
        parser_eat(parser, op);

        ASTNode *right = parse_additive_expression(parser);

        // Create a binary operation node
        ASTNode *new_node = malloc(sizeof(ASTNode));
        new_node->type = AST_BINOP;
        new_node->binop.left = node;
        new_node->binop.op = op;
        new_node->binop.right = right;

        node = new_node;
    }

    return node;
}

ASTNode *parse_expression(Parser *parser)
{
    ASTNode *node;

    if (parser->current_token.type == TOKEN_KEYWORD_LET) {
        node = parse_let_binding(parser);
    } else if (parser->current_token.type == TOKEN_KEYWORD_IF) {
        node = parse_if_expression(parser);
    } else {
        node = parse_comparison(parser);
    }

    // Handle additional expressions (e.g., '+ x') after the initial expression
    while (parser->current_token.type == TOKEN_PLUS ||
           parser->current_token.type == TOKEN_MINUS) {
        TokenType op = parser->current_token.type;
        parser_eat(parser, op);

        ASTNode *right = parse_expression(parser);

        ASTNode *new_node = malloc(sizeof(ASTNode));
        new_node->type = AST_BINOP;
        new_node->binop.left = node;
        new_node->binop.op = op;
        new_node->binop.right = right;

        node = new_node;
    }

    return node;
}

void free_ast(ASTNode *node)
{
    if (node == NULL)
        return;

    switch (node->type)
    {
    case AST_NUMBER:
        // Nothing to free
        break;
    case AST_STRING:
        free(node->string_value);
        break;
    case AST_BINOP:
        free_ast(node->binop.left);
        free_ast(node->binop.right);
        break;
    case AST_VARIABLE:
        free(node->name);
        break;
    case AST_FUNCTION_DEF:
        free(node->function_def.name);
        for (int i = 0; i < node->function_def.param_count; i++)
        {
            free(node->function_def.parameters[i]);
        }
        free(node->function_def.parameters);
        free_ast(node->function_def.body);
        break;
    case AST_FUNCTION_CALL:
        free(node->function_call.name);
        for (int i = 0; i < node->function_call.arg_count; i++)
        {
            free_ast(node->function_call.arguments[i]);
        }
        free(node->function_call.arguments);
        break;
    case AST_LET_BINDING:
        free(node->let_binding.name);
        free_ast(node->let_binding.value);
        free_ast(node->let_binding.body);
        break;
    case AST_STATEMENT_LIST:
        for (int i = 0; i < node->statement_list.statement_count; i++)
        {
            free_ast(node->statement_list.statements[i]);
        }
        free(node->statement_list.statements);
        break;
    case AST_IF_EXPR:
        free_ast(node->if_expr.condition);
        free_ast(node->if_expr.then_branch);
        free_ast(node->if_expr.else_branch);
        break;
    default:
        fprintf(stderr, "Error: Unknown AST node type '%d' in free_ast\n", node->type);
        exit(EXIT_FAILURE);
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

    switch (node->type)
    {
    case AST_NUMBER:
        printf("Number: %lf\n", node->number);
        break;

    case AST_BINOP:
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
        print_ast(node->binop.left, indent + 1);
        print_ast(node->binop.right, indent + 1);
        break;
    }

    case AST_VARIABLE:
        printf("Variable: %s\n", node->name);
        break;

    case AST_FUNCTION_DEF:
    {
        printf("Function Definition: %s\n", node->function_def.name);
        // Print parameters
        for (int i = 0; i < node->function_def.param_count; i++)
        {
            for (int j = 0; j < indent + 1; j++)
            {
                printf("  ");
            }
            printf("Parameter: %s\n", node->function_def.parameters[i]);
        }
        // Print function body
        for (int i = 0; i < indent + 1; i++)
        {
            printf("  ");
        }
        printf("Body:\n");
        print_ast(node->function_def.body, indent + 2);
        break;
    }

    case AST_FUNCTION_CALL:
    {
        printf("Function Call: %s\n", node->function_call.name);
        // Print arguments
        for (int i = 0; i < node->function_call.arg_count; i++)
        {
            for (int j = 0; j < indent + 1; j++)
            {
                printf("  ");
            }
            printf("Argument %d:\n", i + 1);
            print_ast(node->function_call.arguments[i], indent + 2);
        }
        break;
    }

    case AST_LET_BINDING:
    {
        printf("Let Binding: %s\n", node->let_binding.name);
        // Print value expression
        for (int i = 0; i < indent + 1; i++)
        {
            printf("  ");
        }
        printf("Value:\n");
        print_ast(node->let_binding.value, indent + 2);
        // Print body expression
        for (int i = 0; i < indent + 1; i++)
        {
            printf("  ");
        }
        printf("Body:\n");
        print_ast(node->let_binding.body, indent + 2);
        break;
    }

    case AST_STATEMENT_LIST:
    {
        printf("Statement List:\n");
        for (int i = 0; i < node->statement_list.statement_count; i++)
        {
            print_ast(node->statement_list.statements[i], indent + 1);
        }
        break;
    }

    case AST_IF_EXPR: {
        printf("If Expression:\n");
        for (int i = 0; i < indent + 1; i++) printf("  ");
        printf("Condition:\n");
        print_ast(node->if_expr.condition, indent + 2);
        for (int i = 0; i < indent + 1; i++) printf("  ");
        printf("Then Branch:\n");
        print_ast(node->if_expr.then_branch, indent + 2);
        for (int i = 0; i < indent + 1; i++) printf("  ");
        printf("Else Branch:\n");
        print_ast(node->if_expr.else_branch, indent + 2);
        break;
    }

    default:
        printf("Unknown node type: %d\n", node->type);
        break;
    }
}

ASTNode *parse_function_definition(Parser *parser)
{
    parser_eat(parser, TOKEN_KEYWORD_FUN);

    // Expect an identifier (function name)
    if (parser->current_token.type != TOKEN_IDENTIFIER)
    {
        fprintf(stderr, "Error: Expected function name after 'fun'\n");
        exit(EXIT_FAILURE);
    }
    char *func_name = strdup(parser->current_token.text);
    parser_eat(parser, TOKEN_IDENTIFIER);

    // Parse parameters
    char **parameters = NULL;
    int param_count = 0;

    // Check for parameters
    if (parser->current_token.type == TOKEN_IDENTIFIER)
    {
        // Collect parameters
        parameters = malloc(sizeof(char *) * 10); // Support up to 10 parameters for simplicity
        while (parser->current_token.type == TOKEN_IDENTIFIER)
        {
            parameters[param_count++] = strdup(parser->current_token.text);
            parser_eat(parser, TOKEN_IDENTIFIER);
            if (parser->current_token.type == TOKEN_COMMA)
            {
                parser_eat(parser, TOKEN_COMMA);
            }
            else
            {
                break;
            }
        }
    }

    // Expect '='
    if (parser->current_token.type != TOKEN_EQUAL)
    {
        fprintf(stderr, "Error: Expected '=' after function parameters\n");
        exit(EXIT_FAILURE);
    }
    parser_eat(parser, TOKEN_EQUAL);

    // Parse function body expression
    ASTNode *body = parse_expression(parser);

    // Create function definition node
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = AST_FUNCTION_DEF;
    node->function_def.name = func_name;
    node->function_def.parameters = parameters;
    node->function_def.param_count = param_count;
    node->function_def.body = body;

    return node;
}

ASTNode *parse_function_application(Parser *parser, char *func_name)
{
    parser_eat(parser, TOKEN_LPAREN);

    ASTNode **arguments = NULL;
    int arg_count = 0;

    // Parse argument list
    if (parser->current_token.type != TOKEN_RPAREN)
    {
        arguments = malloc(sizeof(ASTNode *) * 10); // Support up to 10 arguments
        do
        {
            ASTNode *arg = parse_expression(parser);
            arguments[arg_count++] = arg;
            if (parser->current_token.type == TOKEN_COMMA)
            {
                parser_eat(parser, TOKEN_COMMA);
            }
            else
            {
                break;
            }
        } while (parser->current_token.type != TOKEN_RPAREN);
    }

    parser_eat(parser, TOKEN_RPAREN);

    // Create function call node
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = AST_FUNCTION_CALL;
    node->function_call.name = func_name;
    node->function_call.arguments = arguments;
    node->function_call.arg_count = arg_count;

    return node;
}

ASTNode *parse_let_binding(Parser *parser)
{
    parser_eat(parser, TOKEN_KEYWORD_LET);

    // Expect an identifier
    if (parser->current_token.type != TOKEN_IDENTIFIER)
    {
        fprintf(stderr, "Error: Expected variable name after 'let'\n");
        exit(EXIT_FAILURE);
    }
    char *var_name = strdup(parser->current_token.text);
    parser_eat(parser, TOKEN_IDENTIFIER);

    // Expect '='
    parser_eat(parser, TOKEN_EQUAL);

    // Parse value expression
    ASTNode *value = parse_expression(parser);

    // Expect 'in'
    parser_eat(parser, TOKEN_KEYWORD_IN);

    // Parse body expression
    ASTNode *body = parse_expression(parser);

    // Expect 'end'
    parser_eat(parser, TOKEN_KEYWORD_END);

    // Create let binding node
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = AST_LET_BINDING;
    node->let_binding.name = var_name;
    node->let_binding.value = value;
    node->let_binding.body = body;

    return node;
}

ASTNode *parse_statement_list(Parser *parser)
{
    ASTNode **statements = NULL;
    int statement_count = 0;

    // Allocate initial space for statements
    int capacity = 10; // Adjust as needed
    statements = malloc(sizeof(ASTNode *) * capacity);

    // Parse statements separated by semicolons
    while (parser->current_token.type != TOKEN_EOF)
    {
        ASTNode *stmt = parse_statement(parser);

        // Add the statement to the list
        if (statement_count >= capacity)
        {
            capacity *= 2;
            statements = realloc(statements, sizeof(ASTNode *) * capacity);
        }
        statements[statement_count++] = stmt;

        // Expect a semicolon after each statement
        if (parser->current_token.type == TOKEN_SEMICOLON)
        {
            parser_eat(parser, TOKEN_SEMICOLON);
        }
        else if (parser->current_token.type != TOKEN_EOF)
        {
            fprintf(stderr, "Error: Expected ';' or end of input after statement\n");
            exit(EXIT_FAILURE);
        }
    }

    // Create the statement list node
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = AST_STATEMENT_LIST;
    node->statement_list.statements = statements;
    node->statement_list.statement_count = statement_count;

    return node;
}

ASTNode *parse_statement(Parser *parser)
{
    if (parser->current_token.type == TOKEN_KEYWORD_FUN)
    {
        // Function definition
        return parse_function_definition(parser);
    }
    else if (parser->current_token.type == TOKEN_KEYWORD_LET)
    {
        // Let binding
        return parse_let_binding(parser);
    }
    else
    {
        // Expression
        return parse_expression(parser);
    }
}