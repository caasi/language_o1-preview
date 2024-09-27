#ifndef ENV_H
#define ENV_H
#include "parser.h"

typedef enum
{
    VAL_NUMBER,
    VAL_STRING,
    VAL_BOOL,
    VAL_FUNCTION,
    VAL_ADT
} ValueType;

typedef struct Env Env;

typedef struct
{
    ValueType type;
    int is_shared; // 1 if the value is shared and should not be freed
    union
    {
        double number;      // For VAL_NUMBER
        char *string_value; // For VAL_STRING
        int bool_value;     // For VAL_BOOL
        struct
        {                      // For VAL_FUNCTION
            ASTNode *func_def; // Function definition AST node
            Env *env;          // Environment where the function was defined
        } function;
        struct
        {
            char *type_name;       // Name of the ADT
            char *constructor;     // Constructor used
            struct Value **fields; // Fields carried by the constructor
            int field_count;
        } adt;
    };
} Value;

struct Env
{
    char **names;       // Array of variable names
    Value **values;     // Array of variable values (we'll define Value type)
    int *is_owned;      // Array of ownership flags
    int count;          // Number of variables
    struct Env *parent; // Pointer to parent environment (for scoping)
};

const char *value_type_to_string(ValueType type);
void print_value(Value *value);

Env *env_create(Env *parent);
void env_define(Env *env, const char *name, Value *value, int is_owned);
Value *env_lookup(Env *env, const char *name);
void env_destroy(Env *env);

void free_value(Value *value);

#endif // ENV_H