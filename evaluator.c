#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "evaluator.h"
#include "env.h"

#define MAX_RECURSION_DEPTH 1000

Value *evaluate(ASTNode *node, Env *env, int depth)
{
    if (depth > MAX_RECURSION_DEPTH)
    {
        fprintf(stderr, "Error: Stack overflow due to infinite recursion\n");
        exit(EXIT_FAILURE);
    }

    switch (node->type)
    {
    case AST_NUMBER:
    {
        Value *val = malloc(sizeof(Value));
        val->type = VAL_NUMBER;
        val->number = node->number;
        val->is_shared = 0; // Not shared, shoould be freed by evaluator
        return val;
    }
    case AST_STRING:
    {
        Value *val = malloc(sizeof(Value));
        val->type = VAL_STRING;
        val->string_value = node->string_value;
        val->is_shared = 1; // Shared, should not be freed by evaluator
        return val;
    }
    case AST_BINOP:
    {
        Value *left = evaluate(node->binop.left, env, depth + 1);
        Value *right = evaluate(node->binop.right, env, depth + 1);

        double result;
        int bool_result; // For comparisons

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
        case TOKEN_EQUAL_EQUAL:
            if (left->type == VAL_NUMBER && right->type == VAL_NUMBER)
            {
                bool_result = (left->number == right->number);
            }
            else if (left->type == VAL_STRING && right->type == VAL_STRING)
            {
                bool_result = (strcmp(left->string_value, right->string_value) == 0);
            }
            else
            {
                fprintf(stderr, "Error: Cannot compare different types with '=='\n");
                exit(EXIT_FAILURE);
            }
            result = bool_result ? 1.0 : 0.0;
            break;
        case TOKEN_NOT_EQUAL:
            if (left->type == VAL_NUMBER && right->type == VAL_NUMBER)
            {
                bool_result = (left->number != right->number);
            }
            else if (left->type == VAL_STRING && right->type == VAL_STRING)
            {
                bool_result = (strcmp(left->string_value, right->string_value) != 0);
            }
            else
            {
                fprintf(stderr, "Error: Cannot compare different types with '!='\n");
                exit(EXIT_FAILURE);
            }
            result = bool_result ? 1.0 : 0.0;
            break;
        case TOKEN_LESS:
            bool_result = (left->number < right->number);
            result = bool_result ? 1.0 : 0.0;
            break;
        case TOKEN_LESS_EQUAL:
            bool_result = (left->number <= right->number);
            result = bool_result ? 1.0 : 0.0;
            break;
        case TOKEN_GREATER:
            bool_result = (left->number > right->number);
            result = bool_result ? 1.0 : 0.0;
            break;
        case TOKEN_GREATER_EQUAL:
            bool_result = (left->number >= right->number);
            result = bool_result ? 1.0 : 0.0;
            break;
        default:
            fprintf(stderr, "Error: Unknown operator '%d'\n", node->binop.op);
            exit(EXIT_FAILURE);
        }

        if (!left->is_shared)
        {
            free_value(left);
        }
        if (!right->is_shared)
        {
            free_value(right);
        }

        Value *val = malloc(sizeof(Value));
        val->type = VAL_NUMBER;
        val->number = result;
        val->is_shared = 0; // Not shared
        return val;
    }
    case AST_VARIABLE:
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
    case AST_FUNCTION_DEF:
    {
        // Create a function value
        Value *val = malloc(sizeof(Value));
        val->type = VAL_FUNCTION;
        val->function.func_def = node;
        val->function.env = env; // Capture the environment (closure)
        // Store the function in the environment
        env_define(env, node->function_def.name, val, 1);
        return val;
    }
    case AST_FUNCTION_CALL:
    {
        // Evaluate the function call
        Value *func_val = env_lookup(env, node->function_call.name);
        if (func_val == NULL)
        {
            fprintf(stderr, "Error: Undefined function '%s'\n", node->function_call.name);
            exit(EXIT_FAILURE);
        }
        if (func_val->type != VAL_FUNCTION)
        {
            fprintf(stderr, "Error: '%s' is not a function\n", node->function_call.name);
            exit(EXIT_FAILURE);
        }

        // Create a new environment for the function call
        Env *func_env = env_create(func_val->function.env);

        // Bind arguments to parameters
        ASTNode *func_def = func_val->function.func_def;
        if (func_def->function_def.param_count != node->function_call.arg_count)
        {
            fprintf(stderr, "Error: Function '%s' expects %d arguments but got %d\n",
                    node->function_call.name, func_def->function_def.param_count, node->function_call.arg_count);
            exit(EXIT_FAILURE);
        }

        for (int i = 0; i < func_def->function_def.param_count; i++)
        {
            // Evaluate argument
            Value *arg_val = evaluate(node->function_call.arguments[i], env, depth + 1);
            // Bind parameter to argument
            env_define(func_env, func_def->function_def.parameters[i], arg_val, 1);
        }

        // Evaluate function body in the new environment
        Value *result = evaluate(func_def->function_def.body, func_env, depth + 1);

        // Clean up
        env_destroy(func_env);
        return result;
    }
    case AST_LET_BINDING:
    {
        // Create a new environment
        Env *let_env = env_create(env);

        // Evaluate the value
        Value *value = evaluate(node->let_binding.value, env, depth + 1);

        // Bind the variable
        env_define(let_env, node->let_binding.name, value, 1);

        // Evaluate the body in the new environment
        Value *result = evaluate(node->let_binding.body, let_env, depth + 1);

        // Clean up
        env_destroy(let_env);
        return result;
    }
    case AST_STATEMENT_LIST:
    {
        Value *result = NULL;
        for (int i = 0; i < node->statement_list.statement_count; i++)
        {
            ASTNode *stmt = node->statement_list.statements[i];
            // Evaluate each statement
            result = evaluate(stmt, env, depth + 1);
            // The environment should not be modified here
        }
        return result;
    }
    case AST_IF_EXPR:
    {
        Value *condition = evaluate(node->if_expr.condition, env, depth + 1);

        if (condition->type != VAL_NUMBER)
        {
            fprintf(stderr, "Error: Condition must be a number\n");
            exit(EXIT_FAILURE);
        }

        Value *result;
        if (condition->number != 0)
        {
            // Evaluate then branch
            result = evaluate(node->if_expr.then_branch, env, depth + 1);
        }
        else
        {
            // Evaluate else branch
            result = evaluate(node->if_expr.else_branch, env, depth + 1);
        }

        // Free condition if necessary
        if (!condition->is_shared)
            free_value(condition);

        return result;
    }
    default:
    {
        const char *got = ast_node_type_to_string(node->type);
        fprintf(stderr, "Error: Evaluating unknown AST node type '%s'\n", got);
        exit(EXIT_FAILURE);
    }
    }
}
