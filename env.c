#include <string.h>
#include <stdio.h>
#include "print.h"
#include "env.h"

const char *value_type_to_string(ValueType type)
{
    switch (type)
    {
    case VAL_NUMBER:
        return "Number";
    case VAL_STRING:
        return "String";
    case VAL_FUNCTION:
        return "Function";
    case VAL_ADT:
        return "ADT";
    default:
        return "Unknown";
    }
}

void print_value(Value *val, int indent, int newline)
{
    if (val == NULL)
    {
        print_indentation(indent);
        printf("null");
        if (newline)
            printf("\n");
        return;
    }

    switch (val->type)
    {
    case VAL_NUMBER:
        print_indentation(indent);
        printf("%lf", val->number);
        if (newline)
            printf("\n");
        break;
    case VAL_STRING:
        print_indentation(indent);
        printf("\"%s\"", val->string_value);
        if (newline)
            printf("\n");
        break;
    case VAL_FUNCTION:
        print_indentation(indent);
        printf("Function");
        if (newline)
            printf("\n");
        break;
    case VAL_ADT:
        print_indentation(indent);
        printf("%s", val->adt.constructor);
        if (val->adt.field_count > 0)
        {
            printf(" (\n");
            for (int i = 0; i < val->adt.field_count; i++)
            {
                print_value(val->adt.fields[i], indent + 1, 0);
                if (i < val->adt.field_count - 1)
                    printf(",\n");
                else
                    printf("\n");
            }
            print_indentation(indent);
            printf(")");
        }
        if (newline)
            printf("\n");
        break;
    // ... handle other value types as needed ...
    default:
        printf("Unknown value type '%s'\n", value_type_to_string(val->type));
        break;
    }
}

Env *env_create(Env *parent)
{
    Env *env = malloc(sizeof(Env));
    env->parent = parent;
    env->names = NULL;
    env->values = NULL;
    env->is_owned = NULL;
    env->count = 0;
    return env;
}

void env_define(Env *env, const char *name, Value *value, int is_owned)
{
    // Mark the value as shared
    value->is_shared = 1;

    env->names = realloc(env->names, sizeof(char *) * (env->count + 1));
    env->values = realloc(env->values, sizeof(Value *) * (env->count + 1));
    env->is_owned = realloc(env->is_owned, sizeof(int) * (env->count + 1));
    env->names[env->count] = strdup(name);
    env->values[env->count] = value;
    env->is_owned[env->count] = is_owned;
    env->count++;
}

Value *env_lookup(Env *env, const char *name)
{
    for (int i = 0; i < env->count; i++)
    {
        if (strcmp(env->names[i], name) == 0)
        {
            return env->values[i];
        }
    }
    if (env->parent != NULL)
    {
        return env_lookup(env->parent, name);
    }
    return NULL;
}

void env_destroy(Env *env)
{
    for (int i = 0; i < env->count; i++)
    {
        free(env->names[i]);
        if (env->is_owned[i])
        {
            free_value(env->values[i]);
        }
    }
    free(env->names);
    free(env->values);
    free(env->is_owned);
    free(env);
}

void free_value(Value *value)
{
    if (value == NULL)
        return;

    if (value->is_shared)
    {
        // Do not free shared values
        return;
    }

    if (value->type == VAL_FUNCTION)
    {
        // Free function-related data if needed
        // For simplicity, we might not need to free anything here
    }
    else if (value->type == VAL_STRING)
    {
        free(value->string_value);
    }

    free(value);
}
