#include <stdio.h>
#include <stdlib.h>
#include "lexer.h"
#include "parser.h"
#include "evaluator.h"

int main()
{
    char line[1024];

    // Read input from stdin
    size_t len = fread(line, 1, sizeof(line) - 1, stdin);
    if (len == 0 && ferror(stdin))
    {
        fprintf(stderr, "Error reading input\n");
        return EXIT_FAILURE;
    }
    line[len] = '\0'; // Null-terminate the string

    // Initialize lexer and parser
    Lexer lexer = lexer_create(line);
    Parser parser = parser_create(lexer);

    // Parse the input and build the AST
    ASTNode *ast = parse_statement_list(&parser);

    // Ensure the entire input was consumed
    if (parser.current_token.type != TOKEN_EOF)
    {
        fprintf(stderr, "Error: Unexpected token after parsing\n");
        return EXIT_FAILURE;
    }

    // Print the AST for debugging
    // print_ast(ast, 0); // Start with an indentation level of 0

    // Initialize the global environment
    Env *global_env = env_create(NULL);

    // Evaluate the AST
    Value *result = evaluate(ast, global_env, 0);

    // Output the result if it's a number
    if (result->type == VAL_NUMBER)
    {
        printf("%lf\n", result->number);
    }
    else
    {
        printf("Result is not a number.\n");
    }

    // Clean up
    free_ast(ast);
    env_destroy(global_env);
    free_value(result);
    // Free the result if needed

    return EXIT_SUCCESS;
}
