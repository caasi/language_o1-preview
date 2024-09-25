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
    ASTNode *ast = parse_expression(&parser);

    // Ensure the entire input was consumed
    if (parser.current_token.type != TOKEN_EOF)
    {
        fprintf(stderr, "Error: Unexpected token after parsing expression\n");
        return EXIT_FAILURE;
    }

    // Print the AST for debugging
    print_ast(ast, 0); // Start with indent level 0

    // Evaluate the AST
    double result = evaluate(ast);

    // Output the result
    printf("%lf\n", result);

    // Free the AST
    free_ast(ast);

    return EXIT_SUCCESS;
}
