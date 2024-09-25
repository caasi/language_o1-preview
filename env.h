#ifndef ENV_H
#define ENV_H
#include "parser.h"

typedef enum
{
    VAL_NUMBER,
    VAL_FUNCTION
} ValueType;

typedef struct Env Env;

typedef struct
{
    ValueType type;
    int is_shared; // 1 if the value is shared and should not be freed
    union
    {
        double number; // For VAL_NUMBER
        struct
        {                      // For VAL_FUNCTION
            ASTNode *func_def; // Function definition AST node
            Env *env;          // Environment where the function was defined
        } function;
    };
} Value;

struct Env
{
    struct Env *parent; // Pointer to parent environment (for scoping)
    char **names;       // Array of variable names
    Value **values;     // Array of variable values (we'll define Value type)
    int count;          // Number of variables
};

Env *env_create(Env *parent);
void env_define(Env *env, const char *name, Value *value);
Value *env_lookup(Env *env, const char *name);
void env_destroy(Env *env);

void free_value(Value *value);

#endif // ENV_H