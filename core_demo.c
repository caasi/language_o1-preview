#include <stdio.h>
#include <stdlib.h>
#include "parser.h"
#include "core.h"

int main() {
    printf("=== GHC Core AST Demo ===\n\n");
    
    // Demonstrate Core AST creation
    printf("1. Creating Core expressions:\n");
    
    // Build: λx. x + 1
    CoreExpr *x_var = core_var("x");
    CoreExpr *one = core_int(1);
    CoreExpr *plus_var = core_var("+");
    CoreExpr *add_expr = core_app2(plus_var, x_var, one);
    CoreExpr *lambda_expr = core_lambda("x", add_expr);
    
    printf("λx. (+) x 1:\n");
    core_expr_print(lambda_expr, 1);
    printf("\n");
    
    // Build: let f = λx. x + 1 in f 5
    CoreExpr *five = core_int(5);
    CoreExpr *f_var = core_var("f");
    CoreExpr *app_f_5 = core_expr_create_app(f_var, five);
    CoreExpr *let_expr = core_let_simple("f", lambda_expr, app_f_5);
    
    printf("let f = λx. (+) x 1 in f 5:\n");
    core_expr_print(let_expr, 1);
    printf("\n");
    
    // Demonstrate AST to Core conversion
    printf("2. Converting existing AST to Core:\n");
    
    // Create a simple AST: 2 + 3
    ASTNode *two = (ASTNode *)malloc(sizeof(ASTNode));
    two->type = AST_NUMBER;
    two->number = 2.0;
    
    ASTNode *three = (ASTNode *)malloc(sizeof(ASTNode));
    three->type = AST_NUMBER;
    three->number = 3.0;
    
    ASTNode *add_ast = (ASTNode *)malloc(sizeof(ASTNode));
    add_ast->type = AST_BINOP;
    add_ast->binop.left = two;
    add_ast->binop.op = TOKEN_PLUS;
    add_ast->binop.right = three;
    
    printf("Original AST: 2 + 3\n");
    print_ast(add_ast, 1);
    
    // Convert to Core
    CoreExpr *core_add = ast_to_core(add_ast);
    printf("Converted to Core: (+) 2.0 3.0\n");
    core_expr_print(core_add, 1);
    printf("\n");
    
    // Demonstrate Core utilities
    printf("3. Core expression analysis:\n");
    printf("Is lambda a value? %s\n", core_expr_is_value(lambda_expr) ? "Yes" : "No");
    printf("Is application a value? %s\n", core_expr_is_value(app_f_5) ? "Yes" : "No");
    printf("Lambda count in λx. x + 1: %d\n", core_expr_count_lambdas(lambda_expr));
    
    // Clean up
    core_expr_free(let_expr);  // This will free the nested lambda_expr too
    core_expr_free(core_add);
    free_ast(add_ast);  // This will free the nested nodes too
    
    printf("\n=== Demo Complete ===\n");
    return 0;
}