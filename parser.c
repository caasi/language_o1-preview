#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "print.h"
#include "parser.h"
#include "core.h"

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
        const char *got = token_type_to_string(parser->current_token.type);
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

void free_type(Type *type)
{
    if (type == NULL)
        return;

    free(type->name);

    // Free type parameters recursively
    if (type->params)
    {
        for (int i = 0; i < type->param_count; i++)
        {
            free_type(type->params[i]);
        }
        free(type->params);
    }

    free(type);
}

void print_type(Type *type, int indent)
{
    if (type == NULL)
        return;

    // Print indentation spaces
    for (int i = 0; i < indent; i++)
    {
        printf("  "); // Two spaces per indent level
    }

    // Print the type name
    printf("%s", type->name);

    // Print type parameters if any
    if (type->param_count > 0)
    {
        printf(" [");
        for (int i = 0; i < type->param_count; i++)
        {
            print_type(type->params[i], 0); // Recursive call with zero indent
            if (i < type->param_count - 1)
            {
                printf(", ");
            }
        }
        printf("]");
    }

    // Print the kind of type
    switch (type->kind)
    {
    case TYPE_BASIC:
        printf(" (Basic)\n");
        break;
    case TYPE_ADT:
        printf(" (ADT)\n");
        break;
    case TYPE_FUNCTION:
        printf(" (Function)\n");
        break;
    default:
        printf(" (Unknown Type)\n");
        break;
    }
}

void free_pattern(Pattern *pattern)
{
    if (pattern == NULL)
        return;

    free(pattern->constructor);
    free(pattern->variable);
    free_ast(pattern->result_expr);
    free(pattern);
}

void print_pattern(Pattern *pattern, int indent)
{
    if (pattern == NULL)
        return;

    // Print indentation spaces
    print_indentation(indent);

    // Print the pattern constructor
    printf("%s", pattern->constructor);

    // Print the variable if any
    if (pattern->variable)
    {
        printf(" %s", pattern->variable);
    }

    // Print the result expression if any
    if (pattern->result_expr)
    {
        printf(" => ");
        print_ast(pattern->result_expr, 0); // Recursive call with zero indent
    }

    printf("\n");
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
        const char *got = token_type_to_string(token.type);
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

    if (parser->current_token.type == TOKEN_KEYWORD_CASE)
    {
        node = parse_case_expression(parser);
    }
    else if (parser->current_token.type == TOKEN_KEYWORD_LET)
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
        Type **fields = NULL;
        int field_count = 0;

        // Parse fields until a '|' separator or ';' is encountered
        while (
            parser->current_token.type == TOKEN_TYPE_NUMBER ||
            parser->current_token.type == TOKEN_TYPE_STRING ||
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
    constructor_call->adt_constructor_call.type_name = NULL; // To be filled during evaluation
    constructor_call->adt_constructor_call.constructor = constructor_name;
    constructor_call->adt_constructor_call.arguments = arguments;
    constructor_call->adt_constructor_call.arg_count = arg_count;

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
            free_type(node->adt_constructor_def.arguments[i]);
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
    case AST_CASE_EXPR:
        free_ast(node->case_expr.expression);
        for (int i = 0; i < node->case_expr.pattern_count; i++)
        {
            free_pattern(node->case_expr.patterns[i]);
        }
        free(node->case_expr.patterns);
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
    case AST_CASE_EXPR:
        return "Case Expression";
    default:
        return "Unknown";
    }
}

void print_ast(ASTNode *node, int indent)
{
    if (node == NULL)
        return;

    // Print indentation spaces
    print_indentation(indent);

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
            print_indentation(indent + 1);
            print_ast(node->function_def.param_types[i], indent + 1);
        }
        // Print function body
        print_indentation(indent + 1);
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
            print_indentation(indent + 1);
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
            print_indentation(indent + 1);
            printf("Field %d:\n", i + 1);
            print_type(node->adt_constructor_def.arguments[i], indent + 2);
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
            print_indentation(indent + 1);
            printf("Argument %d:\n", i + 1);
            print_ast(node->adt_constructor_call.arguments[i], indent + 2);
        }
        break;
    }

    case AST_LET_BINDING:
    {
        printf("Let Binding: %s\n", node->let_binding.name);
        // Print value expression
        print_indentation(indent + 1);
        printf("Value:\n");
        print_ast(node->let_binding.value, indent + 2);
        // Print body expression
        print_indentation(indent + 1);
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
        print_indentation(indent + 1);
        printf("Condition:\n");
        print_ast(node->if_expr.condition, indent + 2);
        print_indentation(indent + 1);
        printf("Then Branch:\n");
        print_ast(node->if_expr.then_branch, indent + 2);
        print_indentation(indent + 1);
        printf("Else Branch:\n");
        print_ast(node->if_expr.else_branch, indent + 2);
        break;
    }

    case AST_CASE_EXPR:
    {
        printf("Case Expression:\n");
        print_indentation(indent + 1);
        printf("Expression:\n");
        print_ast(node->case_expr.expression, indent + 2);
        // Print patterns
        for (int i = 0; i < node->case_expr.pattern_count; i++)
        {
            print_indentation(indent + 1);
            printf("Pattern %d:\n", i + 1);
            print_pattern(node->case_expr.patterns[i], indent + 2);
        }
        break;
    }

    default:
    {
        const char *got = ast_node_type_to_string(node->type);
        printf("Unknown node type: %s (%d)\n", got, node->type);
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
            const char *got = token_type_to_string(parser->current_token.type);
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

ASTNode *parse_case_expression(Parser *parser)
{
    ASTNode *case_node = malloc(sizeof(ASTNode));
    if (!case_node)
    {
        fprintf(stderr, "Error: Memory allocation failed for case expression\n");
        exit(EXIT_FAILURE);
    }
    case_node->type = AST_CASE_EXPR;

    parser_eat(parser, TOKEN_KEYWORD_CASE); // Consume 'case'

    case_node->case_expr.expression = parse_expression(parser); // Parse the expression to match

    parser_eat(parser, TOKEN_KEYWORD_OF); // Consume 'of'

    // Initialize patterns
    case_node->case_expr.patterns = NULL;
    case_node->case_expr.pattern_count = 0;

    // Parse patterns
    while (parser->current_token.type != TOKEN_EOF &&
           parser->current_token.type != TOKEN_KEYWORD_END &&
           parser->current_token.type != TOKEN_KEYWORD_ELSE) // Adjust based on your syntax
    {
        // Allocate memory for a new pattern
        case_node->case_expr.patterns =
            realloc(case_node->case_expr.patterns, sizeof(Pattern *) * (case_node->case_expr.pattern_count + 1));
        if (!case_node->case_expr.patterns)
        {
            fprintf(stderr, "Error: Memory allocation failed for patterns\n");
            exit(EXIT_FAILURE);
        }

        // Allocate memory for the new Pattern struct
        Pattern *new_pattern = malloc(sizeof(Pattern));
        if (!new_pattern)
        {
            fprintf(stderr, "Error: Memory allocation failed for a pattern\n");
            exit(EXIT_FAILURE);
        }

        // Initialize the new Pattern
        new_pattern->constructor = NULL;
        new_pattern->variable = NULL;
        new_pattern->result_expr = NULL;

        // Assign to the patterns array
        case_node->case_expr.patterns[case_node->case_expr.pattern_count] = new_pattern;
        case_node->case_expr.pattern_count++;

        // Parse Constructor
        if (parser->current_token.type != TOKEN_IDENTIFIER)
        {
            fprintf(stderr, "Error: Expected constructor in case pattern\n");
            exit(EXIT_FAILURE);
        }
        new_pattern->constructor = strdup(parser->current_token.text);
        parser_eat(parser, TOKEN_IDENTIFIER);

        // Parse Variable (Optional)
        if (parser->current_token.type == TOKEN_IDENTIFIER)
        {
            new_pattern->variable = strdup(parser->current_token.text);
            parser_eat(parser, TOKEN_IDENTIFIER);
        }
        else
        {
            new_pattern->variable = NULL; // No variable to bind
        }

        // Parse '=>'
        if (parser->current_token.type != TOKEN_FAT_ARROW)
        {
            fprintf(stderr, "Error: Expected '=>' in case pattern\n");
            exit(EXIT_FAILURE);
        }
        parser_eat(parser, TOKEN_FAT_ARROW);

        // Parse Result Expression
        new_pattern->result_expr = parse_expression(parser);

        // Parse '|', if present
        if (parser->current_token.type == TOKEN_PIPE)
        {
            parser_eat(parser, TOKEN_PIPE);
        }
        else
        {
            break;
        }
    }

    return case_node;
}

// ============================================================================
// Core Expression Parsing (Phase 2)
// ============================================================================

// Parse Core expressions: handles let, lambda, case, application
CoreExpr *parse_core_expression(Parser *parser) {
    // Check for let expression
    if (parser->current_token.type == TOKEN_KEYWORD_LET) {
        return parse_core_let(parser);
    }
    
    // Check for lambda expression: \x. body
    if (parser->current_token.type == TOKEN_BACKSLASH) {
        return parse_core_lambda(parser);
    }
    
    // Check for case expression
    if (parser->current_token.type == TOKEN_KEYWORD_CASE) {
        return parse_core_case(parser);
    }
    
    // Otherwise, parse application
    return parse_core_application(parser);
}

// Parse atomic Core expressions: variables, literals, parenthesized expressions
CoreExpr *parse_core_atom(Parser *parser) {
    if (parser->current_token.type == TOKEN_NUMBER) {
        double val = parser->current_token.value;
        parser_eat(parser, TOKEN_NUMBER);
        return core_double(val);
    }
    
    if (parser->current_token.type == TOKEN_STRING) {
        char *str = strdup(parser->current_token.text);
        parser_eat(parser, TOKEN_STRING);
        return core_string(str);
    }
    
    if (parser->current_token.type == TOKEN_IDENTIFIER) {
        char *name = strdup(parser->current_token.text);
        parser_eat(parser, TOKEN_IDENTIFIER);
        return core_var(name);
    }
    
    // Handle parenthesized expressions
    if (parser->current_token.type == TOKEN_LPAREN) {
        parser_eat(parser, TOKEN_LPAREN);
        
        // Check if this is an operator in parentheses like (+), (-), (*), (/)
        if (parser->current_token.type == TOKEN_PLUS ||
            parser->current_token.type == TOKEN_MINUS ||
            parser->current_token.type == TOKEN_MUL ||
            parser->current_token.type == TOKEN_DIV ||
            parser->current_token.type == TOKEN_EQUAL_EQUAL ||
            parser->current_token.type == TOKEN_NOT_EQUAL ||
            parser->current_token.type == TOKEN_LESS ||
            parser->current_token.type == TOKEN_LESS_EQUAL ||
            parser->current_token.type == TOKEN_GREATER ||
            parser->current_token.type == TOKEN_GREATER_EQUAL) {
            
            // Convert operator token to variable name
            char *op_name = NULL;
            switch (parser->current_token.type) {
                case TOKEN_PLUS: op_name = strdup("+"); break;
                case TOKEN_MINUS: op_name = strdup("-"); break;
                case TOKEN_MUL: op_name = strdup("*"); break;
                case TOKEN_DIV: op_name = strdup("/"); break;
                case TOKEN_EQUAL_EQUAL: op_name = strdup("=="); break;
                case TOKEN_NOT_EQUAL: op_name = strdup("!="); break;
                case TOKEN_LESS: op_name = strdup("<"); break;
                case TOKEN_LESS_EQUAL: op_name = strdup("<="); break;
                case TOKEN_GREATER: op_name = strdup(">"); break;
                case TOKEN_GREATER_EQUAL: op_name = strdup(">="); break;
                default: break;
            }
            
            parser_eat(parser, parser->current_token.type);
            parser_eat(parser, TOKEN_RPAREN);
            return core_var(op_name);
        } else {
            // Regular parenthesized expression
            CoreExpr *expr = parse_core_expression(parser);
            parser_eat(parser, TOKEN_RPAREN);
            return expr;
        }
    }
    
    fprintf(stderr, "Error: Unexpected token in Core expression: %s\n", 
            token_type_to_string(parser->current_token.type));
    exit(EXIT_FAILURE);
}

// Parse Core application: f x y (left-associative)
CoreExpr *parse_core_application(Parser *parser) {
    CoreExpr *expr = parse_core_atom(parser);
    
    // Keep applying as long as we have atoms
    while (parser->current_token.type == TOKEN_NUMBER ||
           parser->current_token.type == TOKEN_STRING ||
           parser->current_token.type == TOKEN_IDENTIFIER ||
           parser->current_token.type == TOKEN_LPAREN) {
        CoreExpr *arg = parse_core_atom(parser);
        expr = core_expr_create_app(expr, arg);
    }
    
    return expr;
}

// Parse Core lambda: \x. body
CoreExpr *parse_core_lambda(Parser *parser) {
    parser_eat(parser, TOKEN_BACKSLASH);
    
    if (parser->current_token.type != TOKEN_IDENTIFIER) {
        fprintf(stderr, "Error: Expected parameter name in lambda\n");
        exit(EXIT_FAILURE);
    }
    
    char *param_name = strdup(parser->current_token.text);
    parser_eat(parser, TOKEN_IDENTIFIER);
    parser_eat(parser, TOKEN_DOT);
    
    CoreExpr *body = parse_core_expression(parser);
    return core_lambda(param_name, body);
}

// Parse Core let: let x = value in body
CoreExpr *parse_core_let(Parser *parser) {
    parser_eat(parser, TOKEN_KEYWORD_LET);
    
    if (parser->current_token.type != TOKEN_IDENTIFIER) {
        fprintf(stderr, "Error: Expected variable name in let\n");
        exit(EXIT_FAILURE);
    }
    
    char *var_name = strdup(parser->current_token.text);
    parser_eat(parser, TOKEN_IDENTIFIER);
    parser_eat(parser, TOKEN_EQUAL);
    
    CoreExpr *value = parse_core_expression(parser);
    
    parser_eat(parser, TOKEN_KEYWORD_IN);
    CoreExpr *body = parse_core_expression(parser);
    
    // Check if this should be a recursive let by looking for the variable name in the value expression
    if (core_expr_contains_var(value, var_name)) {
        return core_letrec_simple(var_name, value, body);
    } else {
        return core_let_simple(var_name, value, body);
    }
}

// Parse Core case: case expr of pattern -> result; pattern -> result
CoreExpr *parse_core_case(Parser *parser) {
    parser_eat(parser, TOKEN_KEYWORD_CASE);
    
    CoreExpr *expr = parse_core_expression(parser);
    parser_eat(parser, TOKEN_KEYWORD_OF);
    
    // For now, implement a simple case parser
    // In a full implementation, we'd handle multiple patterns
    CoreAlt **alts = (CoreAlt **)malloc(2 * sizeof(CoreAlt *));
    int alt_count = 0;
    
    // Parse first alternative
    if (parser->current_token.type == TOKEN_IDENTIFIER) {
        char *constructor = strdup(parser->current_token.text);
        parser_eat(parser, TOKEN_IDENTIFIER);
        parser_eat(parser, TOKEN_ARROW);
        CoreExpr *result = parse_core_expression(parser);
        
        alts[alt_count++] = core_alt_create_con(constructor, NULL, 0, result);
        
        // Check for semicolon and second alternative
        if (parser->current_token.type == TOKEN_SEMICOLON) {
            parser_eat(parser, TOKEN_SEMICOLON);
            
            if (parser->current_token.type == TOKEN_IDENTIFIER) {
                char *constructor2 = strdup(parser->current_token.text);
                parser_eat(parser, TOKEN_IDENTIFIER);
                parser_eat(parser, TOKEN_ARROW);
                CoreExpr *result2 = parse_core_expression(parser);
                
                alts[alt_count++] = core_alt_create_con(constructor2, NULL, 0, result2);
            }
        }
    }
    
    return core_expr_create_case(expr, NULL, NULL, alts, alt_count);
}