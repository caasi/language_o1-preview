#include <stdio.h>
#include <stdlib.h>
#include "lexer.h"
#include "parser.h"
#include "evaluator.h"
#include "symbol_table.h"
#include "core.h"

int main(int argc, char *argv[])
{
    char *program_text;
    
    if (argc > 1) {
        // Read from file
        FILE *file = fopen(argv[1], "r");
        if (!file) {
            fprintf(stderr, "Error: Cannot open file %s\n", argv[1]);
            return EXIT_FAILURE;
        }
        
        fseek(file, 0, SEEK_END);
        long length = ftell(file);
        fseek(file, 0, SEEK_SET);
        program_text = malloc(length + 1);
        if (!program_text) {
            fprintf(stderr, "Error: Memory allocation failed for program text\n");
            fclose(file);
            return EXIT_FAILURE;
        }
        fread(program_text, 1, length, file);
        program_text[length] = '\0';
        fclose(file);
    } else {
        // Read from stdin
        fseek(stdin, 0, SEEK_END);
        long length = ftell(stdin);
        fseek(stdin, 0, SEEK_SET);
        program_text = malloc(length + 1);
        if (!program_text) {
            fprintf(stderr, "Error: Memory allocation failed for program text\n");
            return EXIT_FAILURE;
        }
        fread(program_text, 1, length, stdin);
        program_text[length] = '\0';
    }

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
