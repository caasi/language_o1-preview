#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "evaluator.h"
#include "env.h"
#include "symbol_table.h"

#define MAX_RECURSION_DEPTH 1000

Value *evaluate(ASTNode *node, Env *env, SymbolTable *sym_table, int depth)
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
        Value *left = evaluate(node->binop.left, env, sym_table, depth + 1);
        Value *right = evaluate(node->binop.right, env, sym_table, depth + 1);

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
            Value *arg_val = evaluate(node->function_call.arguments[i], env, sym_table, depth + 1);
            // Bind parameter to argument
            env_define(func_env, func_def->function_def.parameters[i], arg_val, 1);
        }

        // Evaluate function body in the new environment
        Value *result = evaluate(func_def->function_def.body, func_env, sym_table, depth + 1);

        // Clean up
        env_destroy(func_env);
        return result;
    }
    case AST_ADT_CONSTRUCTOR_DEF:
    {
        return NULL; // ADT constructor definitions do not produce a runtime value
    }
    case AST_ADT_DEFINITION:
    {
        // Register each constructor in the symbol table
        for (int i = 0; i < node->adt_definition.constructor_count; i++)
        {
            ASTNode *constructor_def = node->adt_definition.constructors[i];
            symbol_table_add(sym_table, constructor_def->adt_constructor_def.constructor, node->adt_definition.type_name);
        }
        return NULL; // ADT definitions do not produce a runtime value
    }
    case AST_ADT_CONSTRUCTOR_CALL:
    {
        // Lookup the constructor to get its ADT type
        const char *adt_type = symbol_table_lookup(sym_table, node->adt_constructor_call.constructor);
        if (adt_type == NULL)
        {
            fprintf(stderr, "Error: Undefined constructor '%s'\n", node->adt_constructor_call.constructor);
            exit(EXIT_FAILURE);
        }

        // Evaluate constructor arguments
        Value **evaluated_args = NULL;
        if (node->adt_constructor_call.arg_count > 0)
        {
            evaluated_args = malloc(sizeof(Value *) * node->adt_constructor_call.arg_count);
            if (!evaluated_args)
            {
                fprintf(stderr, "Error: Memory allocation failed for constructor arguments\n");
                exit(EXIT_FAILURE);
            }
            for (int i = 0; i < node->adt_constructor_call.arg_count; i++)
            {
                evaluated_args[i] = evaluate(node->adt_constructor_call.arguments[i], env, sym_table, depth + 1);
            }
        }

        // Create the ADT value
        Value *adt_instance = malloc(sizeof(Value));
        if (!adt_instance)
        {
            fprintf(stderr, "Error: Memory allocation failed for ADT instance\n");
            exit(EXIT_FAILURE);
        }
        adt_instance->type = VAL_ADT;
        adt_instance->adt.type_name = strdup(adt_type);
        adt_instance->adt.constructor = strdup(node->adt_constructor_call.constructor);
        adt_instance->adt.field_count = node->adt_constructor_call.arg_count;
        adt_instance->adt.fields = evaluated_args; // NULL if no arguments

        return adt_instance;
    }
    case AST_LET_BINDING:
    {
        // Create a new environment
        Env *let_env = env_create(env);

        // Evaluate the value
        Value *value = evaluate(node->let_binding.value, env, sym_table, depth + 1);

        // Bind the variable
        env_define(let_env, node->let_binding.name, value, 1);

        // Evaluate the body in the new environment
        Value *result = evaluate(node->let_binding.body, let_env, sym_table, depth + 1);

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
            result = evaluate(stmt, env, sym_table, depth + 1);
            // The environment should not be modified here
        }
        return result;
    }
    case AST_IF_EXPR:
    {
        Value *condition = evaluate(node->if_expr.condition, env, sym_table, depth + 1);

        if (condition->type != VAL_NUMBER)
        {
            fprintf(stderr, "Error: Condition must be a number\n");
            exit(EXIT_FAILURE);
        }

        Value *result;
        if (condition->number != 0)
        {
            // Evaluate then branch
            result = evaluate(node->if_expr.then_branch, env, sym_table, depth + 1);
        }
        else
        {
            // Evaluate else branch
            result = evaluate(node->if_expr.else_branch, env, sym_table, depth + 1);
        }

        // Free condition if necessary
        if (!condition->is_shared)
            free_value(condition);

        return result;
    }
    case AST_CASE_EXPR:
    {
        Value *matched_value = evaluate(node->case_expr.expression, env, sym_table, depth + 1);

        if (matched_value->type != VAL_ADT)
        {
            fprintf(stderr, "Error: Case expression requires an ADT value\n");
            exit(EXIT_FAILURE);
        }

        for (int i = 0; i < node->case_expr.pattern_count; i++)
        {
            Pattern *pattern = node->case_expr.patterns[i];
            if (strcmp(pattern->constructor, matched_value->adt.constructor) == 0)
            {
                Env *new_env = env_create(env);

                // **Bind Variable Only If It Exists**
                if (pattern->variable != NULL)
                {
                    // Assuming single field constructors
                    if (matched_value->adt.field_count == 1)
                    {
                        env_define(new_env, pattern->variable, matched_value->adt.fields[0], 0);
                    }
                    else
                    {
                        fprintf(stderr, "Error: Pattern matching currently supports single field constructors\n");
                        exit(EXIT_FAILURE);
                    }
                }

                Value *result = evaluate(pattern->result_expr, new_env, sym_table, depth + 1);
                env_destroy(new_env);
                return result;
            }
        }

        fprintf(stderr, "Error: No matching pattern found in case expression\n");
        exit(EXIT_FAILURE);
    }
    default:
    {
        const char *got = ast_node_type_to_string(node->type);
        fprintf(stderr, "Error: Evaluating unknown AST node type '%s'\n", got);
        exit(EXIT_FAILURE);
    }
    }
}
