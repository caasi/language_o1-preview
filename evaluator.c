#include <stdio.h>
#include <stdlib.h>
#include "evaluator.h"

double evaluate(ASTNode *node)
{
    if (node->type == AST_NUMBER)
    {
        return node->number;
    }
    else if (node->type == AST_BINOP)
    {
        double left = evaluate(node->binop.left);
        double right = evaluate(node->binop.right);

        switch (node->binop.op)
        {
        case TOKEN_PLUS:
            return left + right;
        case TOKEN_MINUS:
            return left - right;
        case TOKEN_MUL:
            return left * right;
        case TOKEN_DIV:
            if (right == 0)
            {
                fprintf(stderr, "Error: Division by zero\n");
                exit(EXIT_FAILURE);
            }
            return left / right;
        default:
            fprintf(stderr, "Error: Unknown operator '%d'\n", node->binop.op);
            exit(EXIT_FAILURE);
        }
    }
    else
    {
        fprintf(stderr, "Error: Unknown AST node type '%d'\n", node->type);
        exit(EXIT_FAILURE);
    }
}
