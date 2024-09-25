#include <stdio.h>
#include <stdlib.h>
#include "evaluator.h"
#include "env.h"

Value *evaluate(ASTNode *node, Env *env)
{
    if (node->type == AST_NUMBER)
    {
        Value *val = malloc(sizeof(Value));
        val->type = VAL_NUMBER;
        val->number = node->number;
        val->is_shared = 0; // Not shared, shoould be freed by evaluator
        return val;
    }
    else if (node->type == AST_BINOP)
    {
        Value *left = evaluate(node->binop.left, env);
        Value *right = evaluate(node->binop.right, env);

        if (left->type != VAL_NUMBER || right->type != VAL_NUMBER)
        {
            fprintf(stderr, "Error: Binary operation on non-number\n");
            exit(EXIT_FAILURE);
        }

        double result;
        switch (node->binop.op)
        {
        case TOKEN_PLUS:
            result = left->number + right->number;
            break;
        case TOKEN_MINUS:
            result = left->number - right->number;
            break;
        case TOKEN_MUL:
            result = left->number * right->number;
            break;
        case TOKEN_DIV:
            if (right->number == 0)
            {
                fprintf(stderr, "Error: Division by zero\n");
                exit(EXIT_FAILURE);
            }
            result = left->number / right->number;
            break;
        default:
            fprintf(stderr, "Error: Unknown operator '%d'\n", node->binop.op);
            exit(EXIT_FAILURE);
        }

        if (!left->is_shared) {
            free_value(left);
        }
        if (!right->is_shared) {
            free_value(right);
        }

        Value *val = malloc(sizeof(Value));
        val->type = VAL_NUMBER;
        val->number = result;
        val->is_shared = 0; // Not shared
        return val;
    }
    else if (node->type == AST_VARIABLE)
    {
        // Look up the variable in the environment
        Value *val = env_lookup(env, node->name);
        if (val == NULL)
        {
            fprintf(stderr, "Error: Undefined variable '%s'\n", node->name);
            exit(EXIT_FAILURE);
        }
        return val;
    }
    else if (node->type == AST_FUNCTION_DEF)
    {
        // Create a function value
        Value *val = malloc(sizeof(Value));
        val->type = VAL_FUNCTION;
        val->function.func_def = node;
        val->function.env = env; // Capture the environment (closure)
        // Store the function in the environment
        env_define(env, node->function_def.name, val);
        return val;
    }
    else if (node->type == AST_FUNCTION_CALL)
    {
        // Evaluate the function call
        Value *func_val = env_lookup(env, node->function_call.name);
        if (func_val == NULL || func_val->type != VAL_FUNCTION)
        {
            fprintf(stderr, "Error: Undefined function '%s'\n", node->function_call.name);
            exit(EXIT_FAILURE);
        }

        // Create a new environment for the function call
        Env *func_env = env_create(func_val->function.env);

        // Bind arguments to parameters
        ASTNode *func_def = func_val->function.func_def;
        if (func_def->function_def.param_count != node->function_call.arg_count)
        {
            fprintf(stderr, "Error: Function '%s' expected %d arguments but got %d\n",
                    node->function_call.name, func_def->function_def.param_count, node->function_call.arg_count);
            exit(EXIT_FAILURE);
        }

        for (int i = 0; i < func_def->function_def.param_count; i++)
        {
            // Evaluate argument
            Value *arg_val = evaluate(node->function_call.arguments[i], env);
            // Bind parameter to argument
            env_define(func_env, func_def->function_def.parameters[i], arg_val);
        }

        // Evaluate function body in the new environment
        Value *result = evaluate(func_def->function_def.body, func_env);

        // Clean up
        env_destroy(func_env);
        return result;
    }
    else if (node->type == AST_LET_BINDING)
    {
        // Create a new environment
        Env *let_env = env_create(env);

        // Evaluate the value
        Value *value = evaluate(node->let_binding.value, env);

        // Bind the variable
        env_define(let_env, node->let_binding.name, value);

        // Evaluate the body in the new environment
        Value *result = evaluate(node->let_binding.body, let_env);

        // Clean up
        env_destroy(let_env);
        return result;
    }
    else if (node->type == AST_STATEMENT_LIST)
    {
        Value *result = NULL;
        for (int i = 0; i < node->statement_list.statement_count; i++)
        {
            ASTNode *stmt = node->statement_list.statements[i];
            // Evaluate each statement
            Value *stmt_result = evaluate(stmt, env);
            // Free the previous result if needed
            if (result != NULL)
            {
                free(result);
                result = NULL;
            }
            result = stmt_result;
            // Note: The result of the last statement is returned
        }
        return result;
    }
    else
    {
        fprintf(stderr, "Error: Unknown AST node type '%d'\n", node->type);
        exit(EXIT_FAILURE);
    }
}
