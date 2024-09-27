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
        const char *expected = token_type_to_string(token_type);
        const char *got = token_type_to_string(parser->current_token.type);
        fprintf(stderr, "Error: Expected token type '%s' but got '%s'\n", expected, got);
        exit(EXIT_FAILURE);
    }
}

Type *parse_atomic_type(Parser *parser)
{
    Type *type = malloc(sizeof(Type));
    if (!type)
    {
        fprintf(stderr, "Error: Memory allocation failed for type\n");
        exit(EXIT_FAILURE);
    }

    // Initialzie type parameters to NULL
    type->params = NULL;
    type->param_count = 0;

    // Handle basic types
    if (parser->current_token.type == TOKEN_TYPE_NUMBER)
    {
        type->name = strdup("Number");
        type->kind = TYPE_BASIC;
        parser_eat(parser, TOKEN_TYPE_NUMBER);
    }
    else if (parser->current_token.type == TOKEN_TYPE_STRING)
    {
        type->name = strdup("String");
        type->kind = TYPE_BASIC;
        parser_eat(parser, TOKEN_TYPE_STRING);
    }
    else if (parser->current_token.type == TOKEN_TYPE_BOOL)
    {
        type->name = strdup("Bool");
        type->kind = TYPE_BASIC;
        parser_eat(parser, TOKEN_TYPE_BOOL);
    }
    else if (parser->current_token.type == TOKEN_IDENTIFIER)
    {
        type->name = strdup(parser->current_token.text);
        type->kind = TYPE_ADT;
        parser_eat(parser, TOKEN_IDENTIFIER);

        // Check for type parameters (e.g., Maybe Number)
        if (parser->current_token.type == TOKEN_IDENTIFIER ||
            parser->current_token.type == TOKEN_TYPE_NUMBER ||
            parser->current_token.type == TOKEN_TYPE_STRING ||
            parser->current_token.type == TOKEN_LPAREN) // Extend as needed
        {
            // Parse type parameters recursively
            type->params = malloc(sizeof(Type *) * 1); // Initial allocation for one parameter
            if (!type->params)
            {
                fprintf(stderr, "Error: Memory allocation failed for type parameters\n");
                exit(EXIT_FAILURE);
            }
            type->params[0] = parse_type(parser); // Recursive call for each type parameter
            type->param_count = 1;

            // Extend to handle multyple type parameters if necessary
            // For example, in future: Maybe a b, etc.
        }
    }
    else if (parser->current_token.type == TOKEN_LPAREN)
    {
        parser_eat(parser, TOKEN_LPAREN); // Consume '('
        type = parse_type(parser);        // Parse the enclosed type
        parser_eat(parser, TOKEN_RPAREN); // Consume ')'
    }
    else
    {
        char *got = token_type_to_string(parser->current_token.type);
        fprintf(stderr, "Error: Unexpected token '%s' while parsing type\n", got);
        exit(EXIT_FAILURE);
    }

    return type;
}

Type *parse_type(Parser *parser)
{
    // Parse the left-hand side (atomic type)
    Type *left = parse_atomic_type(parser);

    // Handle function types (e.g., Number -> Number)
    while (parser->current_token.type == TOKEN_ARROW)
    {
        parser_eat(parser, TOKEN_ARROW);

        // Parse the right-hand side (function return type)
        Type *right = parse_type(parser); // Recursive call for right associativity

        // Create a new Type for the function
        Type *func_type = malloc(sizeof(Type));
        if (!func_type)
        {
            fprintf(stderr, "Error: Memory allocation failed for function type\n");
            exit(EXIT_FAILURE);
        }

        func_type->name = strdup("Function");
        func_type->kind = TYPE_FUNCTION;

        // Allocate and assign type parameters: [InputType, OutputType]
        func_type->params = malloc(sizeof(Type *) * 2);
        if (!func_type->params)
        {
            fprintf(stderr, "Error: Memory allocation failed for function type parameters\n");
            exit(EXIT_FAILURE);
        }
        func_type->params[0] = left;
        func_type->params[1] = right;
        func_type->param_count = 2;

        left = func_type; // The new type becomes the left for any further '->'
    }

    return left;
}

ASTNode *parse_string(Parser *parser)
{
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
    else if (token.type == TOKEN_STRING)
    {
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
        char *got = token_type_to_string(token.type);
        fprintf(stderr, "Error: Unexpected token '%s' in factor\n", got);
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

ASTNode *parse_if_expression(Parser *parser)
{
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

ASTNode *parse_multiplicative_expression(Parser *parser)
{
    ASTNode *node = parse_factor(parser);

    while (parser->current_token.type == TOKEN_MUL ||
           parser->current_token.type == TOKEN_DIV)
    {

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

ASTNode *parse_additive_expression(Parser *parser)
{
    ASTNode *node = parse_multiplicative_expression(parser);

    while (parser->current_token.type == TOKEN_PLUS ||
           parser->current_token.type == TOKEN_MINUS)
    {

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

ASTNode *parse_comparison(Parser *parser)
{
    ASTNode *node = parse_additive_expression(parser);

    while (parser->current_token.type == TOKEN_EQUAL_EQUAL ||
           parser->current_token.type == TOKEN_NOT_EQUAL ||
           parser->current_token.type == TOKEN_LESS ||
           parser->current_token.type == TOKEN_LESS_EQUAL ||
           parser->current_token.type == TOKEN_GREATER ||
           parser->current_token.type == TOKEN_GREATER_EQUAL)
    {

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

    if (parser->current_token.type == TOKEN_KEYWORD_LET)
    {
        node = parse_let_binding(parser);
    }
    else if (parser->current_token.type == TOKEN_KEYWORD_IF)
    {
        node = parse_if_expression(parser);
    }
    else if (parser->current_token.type == TOKEN_IDENTIFIER)
    {
        // Check if the identifier is a type name
        if (parser->current_token.text[0] >= 'A' && parser->current_token.text[0] <= 'Z')
        {
            node = parse_adt_constructor_call(parser);
        }
        else
        {
            node = parse_term(parser);
        }
    }
    else
    {
        node = parse_comparison(parser);
    }

    // Handle additional expressions (e.g., '+ x') after the initial expression
    while (parser->current_token.type == TOKEN_PLUS ||
           parser->current_token.type == TOKEN_MINUS)
    {
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

ASTNode *parse_adt_definition(Parser *parser)
{
    parser_eat(parser, TOKEN_TYPE);

    // Parse the type name
    if (parser->current_token.type != TOKEN_IDENTIFIER)
    {
        fprintf(stderr, "Error: Expected type name after 'type'\n");
        exit(EXIT_FAILURE);
    }
    char *type_name = strdup(parser->current_token.text);
    parser_eat(parser, TOKEN_IDENTIFIER);

    // Expect '='
    parser_eat(parser, TOKEN_EQUAL);

    // Parse constructors separated by '|'
    ASTNode **constructors = NULL;
    int constructor_count = 0;
    do
    {
        // Parse costructor name
        if (parser->current_token.type != TOKEN_IDENTIFIER)
        {
            fprintf(stderr, "Error: Expected constructor name\n");
            exit(EXIT_FAILURE);
        }
        char *constructor_name = strdup(parser->current_token.text);
        parser_eat(parser, TOKEN_IDENTIFIER);

        // Parse constructor fields (if any)
        ASTNode **fields = NULL;
        int field_count = 0;

        // Parse fields until a '|' separator or ';' is encountered
        while (
            parser->current_token.type == TOKEN_TYPE_NUMBER ||
            parser->current_token.type == TOKEN_TYPE_STRING ||
            parser->current_token.type == TOKEN_TYPE_BOOL ||
            parser->current_token.type == TOKEN_IDENTIFIER ||
            parser->current_token.type == TOKEN_LPAREN)
        {
            // Parse field type
            Type *field_type = parse_type(parser);

            fields = realloc(fields, sizeof(ASTNode *) * (field_count + 1));
            fields[field_count++] = field_type;
        }

        // Create a constructor node
        ASTNode *constructor = malloc(sizeof(ASTNode));
        constructor->type = AST_ADT_CONSTRUCTOR_DEF;
        constructor->adt_constructor_def.type_name = strdup(type_name);
        constructor->adt_constructor_def.constructor = strdup(constructor_name);
        constructor->adt_constructor_def.arguments = fields;
        constructor->adt_constructor_def.arg_count = field_count;

        // Note: You may need to extend AST_ADT_CONSTRUCTOR to include field types if necessary
        constructors = realloc(constructors, sizeof(ASTNode *) * (constructor_count + 1));
        constructors[constructor_count++] = constructor;

        // Check for '|' separator
        if (parser->current_token.type == TOKEN_PIPE)
        {
            parser_eat(parser, TOKEN_PIPE);
        }
        else
        {
            break;
        }
    } while (1);

    // Create the ADT definition node
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = AST_ADT_DEFINITION;
    node->adt_definition.type_name = type_name;
    node->adt_definition.constructors = constructors;
    node->adt_definition.constructor_count = constructor_count;

    return node;
}

ASTNode *parse_adt_constructor_call(Parser *parser)
{
    // For this example, constructors are treated similarly to function calls
    // Parse constructor arguments if any
    char *constructor_name = strdup(parser->current_token.text);
    parser_eat(parser, TOKEN_IDENTIFIER);

    ASTNode **arguments = NULL;
    int arg_count = 0;

    // Parse arguments until a delimiter is encountered
    while (parser->current_token.type != TOKEN_SEMICOLON &&
           parser->current_token.type != TOKEN_PIPE &&
           parser->current_token.type != TOKEN_RPAREN &&
           parser->current_token.type != TOKEN_COMMA &&
           parser->current_token.type != TOKEN_KEYWORD_IN &&
           parser->current_token.type != TOKEN_KEYWORD_END &&
           parser->current_token.type != TOKEN_EOF)
    {
        ASTNode *arg = parse_expression(parser);
        arguments = realloc(arguments, sizeof(ASTNode *) * (arg_count + 1));
        arguments[arg_count++] = arg;
    }

    // Create the ADT constructor call node
    ASTNode *constructor_call = malloc(sizeof(ASTNode));
    if (!constructor_call)
    {
        fprintf(stderr, "Error: Memory allocation failed for ADT constructor call node\n");
        exit(EXIT_FAILURE);
    }
    constructor_call->type = AST_ADT_CONSTRUCTOR_CALL;
    constructor_call->adt_constructor_def.type_name = NULL; // To be filled during evaluation
    constructor_call->adt_constructor_def.constructor = constructor_name;
    constructor_call->adt_constructor_def.arguments = arguments;
    constructor_call->adt_constructor_def.arg_count = arg_count;

    return constructor_call;
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
    case AST_ADT_CONSTRUCTOR_DEF:
        free(node->adt_constructor_def.type_name);
        free(node->adt_constructor_def.constructor);
        for (int i = 0; i < node->adt_constructor_def.arg_count; i++)
        {
            free_ast(node->adt_constructor_def.arguments[i]);
        }
        free(node->adt_constructor_def.arguments);
        break;
    case AST_ADT_DEFINITION:
        free(node->adt_definition.type_name);
        for (int i = 0; i < node->adt_definition.constructor_count; i++)
        {
            free_ast(node->adt_definition.constructors[i]);
        }
        free(node->adt_definition.constructors);
        break;
    case AST_ADT_CONSTRUCTOR_CALL:
        free(node->adt_constructor_call.type_name);
        free(node->adt_constructor_call.constructor);
        for (int i = 0; i < node->adt_constructor_call.arg_count; i++)
        {
            free_ast(node->adt_constructor_call.arguments[i]);
        }
        free(node->adt_constructor_call.arguments);
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
    {
        const char *got = ast_node_type_to_string(node->type);
        fprintf(stderr, "Error: Unknown AST node type '%s (%d)' in free_ast\n", got, node->type);
        exit(EXIT_FAILURE);
    }
    }

    free(node);
}

const char *ast_node_type_to_string(ASTNodeType type)
{
    switch (type)
    {
    case AST_NUMBER:
        return "Number";
    case AST_STRING:
        return "String";
    case AST_BOOL:
        return "Bool";
    case AST_BINOP:
        return "Binary Operation";
    case AST_VARIABLE:
        return "Variable";
    case AST_FUNCTION_DEF:
        return "Function Definition";
    case AST_FUNCTION_CALL:
        return "Function Call";
    case AST_ADT_CONSTRUCTOR_DEF:
        return "ADT Constructor Definition";
    case AST_ADT_DEFINITION:
        return "ADT Definition";
    case AST_ADT_CONSTRUCTOR_CALL:
        return "ADT Constructor Call";
    case AST_LET_BINDING:
        return "Let Binding";
    case AST_STATEMENT_LIST:
        return "Statement List";
    case AST_IF_EXPR:
        return "If Expression";
    default:
        return "Unknown";
    }
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

    case AST_ADT_CONSTRUCTOR_DEF:
    {
        printf("ADT Constructor Definition: %s\n", node->adt_constructor_def.constructor);
        // Print arguments
        for (int i = 0; i < node->adt_constructor_def.arg_count; i++)
        {
            for (int j = 0; j < indent + 1; j++)
            {
                printf("  ");
            }
            printf("Field %d:\n", i + 1);
            print_ast(node->adt_constructor_def.arguments[i], indent + 2);
        }
        break;
    }

    case AST_ADT_DEFINITION:
    {
        printf("ADT Definition: %s\n", node->adt_definition.type_name);
        // Print constructors
        for (int i = 0; i < node->adt_definition.constructor_count; i++)
        {
            print_ast(node->adt_definition.constructors[i], indent + 1);
        }
        break;
    }

    case AST_ADT_CONSTRUCTOR_CALL:
    {
        printf("ADT Constructor Call: %s\n", node->adt_constructor_call.constructor);
        // Print arguments
        for (int i = 0; i < node->adt_constructor_call.arg_count; i++)
        {
            for (int j = 0; j < indent + 1; j++)
            {
                printf("  ");
            }
            printf("Argument %d:\n", i + 1);
            print_ast(node->adt_constructor_call.arguments[i], indent + 2);
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

    case AST_IF_EXPR:
    {
        printf("If Expression:\n");
        for (int i = 0; i < indent + 1; i++)
            printf("  ");
        printf("Condition:\n");
        print_ast(node->if_expr.condition, indent + 2);
        for (int i = 0; i < indent + 1; i++)
            printf("  ");
        printf("Then Branch:\n");
        print_ast(node->if_expr.then_branch, indent + 2);
        for (int i = 0; i < indent + 1; i++)
            printf("  ");
        printf("Else Branch:\n");
        print_ast(node->if_expr.else_branch, indent + 2);
        break;
    }

    default:
    {
        const char *got = ast_node_type_to_string(node->type);
        printf("Unknown node type: %s\n", got);
        break;
    }
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
    if (!statements)
    {
        fprintf(stderr, "Error: Memory allocation failed for statement list\n");
        exit(EXIT_FAILURE);
    }

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
            char *got = token_type_to_string(parser->current_token.type);
            fprintf(stderr, "Error: Expected ';' or end of input after statement, but got token type '%s'\n", got);
            print_ast(stmt, 0);
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
    else if (parser->current_token.type == TOKEN_TYPE)
    {
        return parse_adt_definition(parser);
    }
    else
    {
        // Expression
        return parse_expression(parser);
    }
}