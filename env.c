#include <string.h>
#include "env.h"

Env *env_create(Env *parent)
{
    Env *env = malloc(sizeof(Env));
    env->parent = parent;
    env->names = NULL;
    env->values = NULL;
    env->count = 0;
    return env;
}

void env_define(Env *env, const char *name, Value *value)
{
    // Mark the value as shared
    value->is_shared = 1;

    env->names = realloc(env->names, sizeof(char *) * (env->count + 1));
    env->values = realloc(env->values, sizeof(Value *) * (env->count + 1));
    env->names[env->count] = strdup(name);
    env->values[env->count] = value;
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
        // Free the value if needed (e.g., for functions)
        // For simplicity, we won't free values here
    }
    free(env->names);
    free(env->values);
    free(env);
}

void free_value(Value *value)
{
    if (value == NULL)
        return;

    if (value->is_shared) {
        // Do not free shared values
        return;
    }

    if (value->type == VAL_FUNCTION)
    {
        // Free function-related data if needed
        // For simplicity, we might not need to free anything here
    }

    free(value);
}
