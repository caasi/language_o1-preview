#include <stdio.h>
#include <stdlib.h>
#include "lexer.h"
#include "parser.h"
#include "evaluator.h"
#include "symbol_table.h"

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
    ASTNode *ast = parse_statement_list(&parser);

    // Ensure the entire input was consumed
    if (parser.current_token.type != TOKEN_EOF)
    {
        fprintf(stderr, "Error: Unexpected token after parsing\n");
        return EXIT_FAILURE;
    }

    // Initialize the symbol table
    SymbolTable *sym_table = symbol_table_create();

    // Initialize the global environment
    Env *global_env = env_create(NULL);

    // Evaluate the AST
    Value *result = evaluate(ast, global_env, sym_table, 0);

    // Output the result
    if (result != NULL)
    {
        print_value(result, 0);
        free_value(result);
    }
    else
    {
        printf("No result.\n");
    }

    // Clean up
    free(program_text);
    free_ast(ast);
    symbol_table_free(sym_table);
    env_destroy(global_env);

    return EXIT_SUCCESS;
}
