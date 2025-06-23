#include <stdio.h>
#include <stdlib.h>
#include "lexer.h"
#include "parser.h"
#include "evaluator.h"
#include "symbol_table.h"
#include "core.h"

int main()
{
    // Read the input program (for simplicity, read form stdin)
    fseek(stdin, 0, SEEK_END);
    long length = ftell(stdin);
    fseek(stdin, 0, SEEK_SET);
    char *program_text = malloc(length + 1);
    if (!program_text)
    {
        fprintf(stderr, "Error: Memory allocation failed for program text\n");
        return EXIT_FAILURE;
    }
    fread(program_text, 1, length, stdin);
    program_text[length] = '\0';

    // Initialize lexer and parser
    Lexer lexer = lexer_create(program_text);
    Parser parser = parser_create(lexer);
    
    // Parse as Core expression instead of ML statements
    CoreExpr *core_expr = parse_core_expression(&parser);

    // Ensure the entire input was consumed
    if (parser.current_token.type != TOKEN_EOF)
    {
        fprintf(stderr, "Error: Unexpected token after parsing\n");
        return EXIT_FAILURE;
    }

    // Evaluate the Core expression (simple evaluator for now)
    double result = core_eval_simple(core_expr);

    // Output the result (same format as original)
    printf("%f\n", result);

    // Clean up
    free(program_text);
    core_expr_free(core_expr);

    return EXIT_SUCCESS;
}
