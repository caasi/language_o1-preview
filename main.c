#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lexer.h"
#include "parser.h"
#include "evaluator.h"
#include "symbol_table.h"
#include "core.h"

void print_usage(const char *program_name) {
    printf("Usage: %s [OPTIONS] [FILE]\n", program_name);
    printf("Options:\n");
    printf("  --ast, -a    Print AST instead of evaluating\n");
    printf("  --help, -h   Show this help message\n");
    printf("\nIf no FILE is specified, reads from stdin.\n");
}

int main(int argc, char *argv[])
{
    char *program_text;
    int print_ast = 0;
    char *filename = NULL;
    
    // Parse command line arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--ast") == 0 || strcmp(argv[i], "-a") == 0) {
            print_ast = 1;
        } else if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
            print_usage(argv[0]);
            return EXIT_SUCCESS;
        } else if (argv[i][0] == '-') {
            fprintf(stderr, "Error: Unknown option %s\n", argv[i]);
            print_usage(argv[0]);
            return EXIT_FAILURE;
        } else {
            filename = argv[i];
        }
    }
    
    if (filename) {
        // Read from file
        FILE *file = fopen(filename, "r");
        if (!file) {
            fprintf(stderr, "Error: Cannot open file %s\n", filename);
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
    
    // Skip any type definitions at the beginning
    while (parser.current_token.type == TOKEN_TYPE) {
        // Skip until we find the next expression (let, identifier, etc.)
        while (parser.current_token.type != TOKEN_EOF && 
               parser.current_token.type != TOKEN_KEYWORD_LET) {
            parser.current_token = lexer_get_next_token(&parser.lexer);
        }
    }
    
    // Parse as Core expression instead of ML statements
    CoreExpr *core_expr = parse_core_expression(&parser);

    // Allow leftover tokens (type definitions might leave some)
    // Don't require EOF for programs with type definitions

    if (print_ast) {
        // Print AST instead of evaluating
        core_expr_print(core_expr, 0);
    } else {
        // Evaluate the Core expression (simple evaluator for now)
        double result = core_eval_simple(core_expr);

        // Output the result (same format as original)
        printf("%f\n", result);
    }

    // Clean up
    free(program_text);
    core_expr_free(core_expr);

    return EXIT_SUCCESS;
}
