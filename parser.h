#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"

typedef enum
{
    AST_NUMBER,
    AST_STRING,
    AST_BINOP,
    AST_VARIABLE,
    AST_FUNCTION_DEF,
    AST_FUNCTION_CALL,
    AST_ADT_CONSTRUCTOR_DEF,
    AST_ADT_DEFINITION,
    AST_ADT_CONSTRUCTOR_CALL,
    AST_LET_BINDING,
    AST_STATEMENT_LIST,
    AST_IF_EXPR,
    AST_CASE_EXPR,
} ASTNodeType;

typedef struct Type
{
    char *name; // Type name
    enum
    {
        TYPE_BASIC,
        TYPE_ADT,
        TYPE_FUNCTION,
        TYPE_POLYMORPHIC,
        // ... other type categories as needed
    } kind;
    struct Type **params; // Type parameters for polymorphism (if needed)
    int param_count;
} Type;

typedef struct Pattern
{
    char *constructor;           // Constructor name
    char *variable;              // Variable to bind
    struct ASTNode *result_expr; // Result expression
} Pattern;                       // Array of patterns

// ============================================================================
// GHC Core AST Structures (Phase 1 - alongside existing AST)
// ============================================================================

typedef enum
{
    CORE_VAR,       // Variables and data constructors  
    CORE_LIT,       // Primitive literals
    CORE_APP,       // Function/constructor application
    CORE_LAM,       // Lambda abstraction
    CORE_LET,       // Let bindings (rec/non-rec)
    CORE_CASE,      // Case expressions
    CORE_CAST,      // Type coercions (simplified)
    CORE_TICK,      // Source annotations (optional)
    CORE_TYPE,      // Type expressions
    CORE_COERCION   // Type equality (simplified)
} CoreExprType;

typedef struct CoreType
{
    enum {
        CORE_TYPE_VAR,      // Type variables (α, β)
        CORE_TYPE_CON,      // Type constructors (Int, Bool, →)
        CORE_TYPE_APP,      // Type application
        CORE_TYPE_FORALL    // Universal quantification (∀α.τ)
    } kind;
    union {
        char *var_name;     // For type variables
        char *con_name;     // For type constructors
        struct {
            struct CoreType *fun;
            struct CoreType *arg;
        } app;              // For type application
        struct {
            char *var;
            struct CoreType *body;
        } forall;           // For ∀α.τ
    };
} CoreType;

typedef struct CoreVar
{
    char *name;             // Variable name
    CoreType *type;         // Variable type (optional for now)
    enum {
        VAR_LOCAL,          // Local variable
        VAR_GLOBAL,         // Global variable
        VAR_DATA_CON        // Data constructor
    } var_kind;
} CoreVar;

typedef struct CoreLit
{
    enum {
        LIT_INT,            // Integer literals
        LIT_DOUBLE,         // Double literals  
        LIT_STRING,         // String literals
        LIT_CHAR            // Character literals
    } lit_kind;
    union {
        int int_val;
        double double_val;
        char *string_val;
        char char_val;
    };
} CoreLit;

typedef struct CoreBind
{
    CoreVar *var;           // Bound variable
    struct CoreExpr *expr;  // Bound expression
} CoreBind;

typedef struct CoreAlt
{
    enum {
        ALT_CON,            // Constructor pattern
        ALT_LIT,            // Literal pattern
        ALT_DEFAULT         // Default pattern (_)
    } alt_kind;
    union {
        struct {
            char *constructor;      // Constructor name
            CoreVar **vars;         // Bound variables
            int var_count;
        } con;
        CoreLit *lit;              // Literal pattern
    };
    struct CoreExpr *expr;         // Alternative expression
} CoreAlt;

typedef struct CoreExpr
{
    CoreExprType expr_type;
    union {
        CoreVar *var;              // CORE_VAR
        CoreLit *lit;              // CORE_LIT
        struct {                   // CORE_APP
            struct CoreExpr *fun;
            struct CoreExpr *arg;
        } app;
        struct {                   // CORE_LAM
            CoreVar *var;
            struct CoreExpr *body;
        } lam;
        struct {                   // CORE_LET
            CoreBind **binds;      // Array of bindings
            int bind_count;
            struct CoreExpr *body;
            int is_recursive;      // 1 for letrec, 0 for let
        } let;
        struct {                   // CORE_CASE
            struct CoreExpr *expr;
            CoreVar *var;          // Case binder (optional)
            CoreType *type;        // Result type
            CoreAlt **alts;        // Alternatives
            int alt_count;
        } case_expr;
        struct {                   // CORE_CAST
            struct CoreExpr *expr;
            CoreType *from_type;
            CoreType *to_type;
        } cast;
        struct {                   // CORE_TICK
            char *tick_info;       // Tick annotation
            struct CoreExpr *expr;
        } tick;
        CoreType *type;            // CORE_TYPE
        struct {                   // CORE_COERCION
            CoreType *from_type;
            CoreType *to_type;
        } coercion;
    };
} CoreExpr;

// Core AST Functions
CoreExpr *core_expr_create_var(CoreVar *var);
CoreExpr *core_expr_create_lit(CoreLit *lit);
CoreExpr *core_expr_create_app(CoreExpr *fun, CoreExpr *arg);
CoreExpr *core_expr_create_lam(CoreVar *var, CoreExpr *body);
CoreExpr *core_expr_create_let(CoreBind **binds, int bind_count, CoreExpr *body, int is_recursive);
CoreExpr *core_expr_create_case(CoreExpr *expr, CoreVar *var, CoreType *type, CoreAlt **alts, int alt_count);

CoreVar *core_var_create(char *name, CoreType *type, int var_kind);
CoreLit *core_lit_create_int(int val);
CoreLit *core_lit_create_double(double val);
CoreLit *core_lit_create_string(char *val);
CoreBind *core_bind_create(CoreVar *var, CoreExpr *expr);
CoreAlt *core_alt_create_con(char *constructor, CoreVar **vars, int var_count, CoreExpr *expr);
CoreAlt *core_alt_create_default(CoreExpr *expr);

void core_expr_free(CoreExpr *expr);
void core_type_free(CoreType *type);
void core_var_free(CoreVar *var);
void core_lit_free(CoreLit *lit);
void core_bind_free(CoreBind *bind);
void core_alt_free(CoreAlt *alt);

void core_expr_print(CoreExpr *expr, int indent);
const char *core_expr_type_to_string(CoreExprType type);

typedef struct ASTNode
{
    ASTNodeType type;
    union
    {
        double number;      // For AST_NUMBER
        char *string_value; // For AST_STRING
        struct
        {
            struct ASTNode *left;
            TokenType op; // Operator token type
            struct ASTNode *right;
        } binop;    // For AST_BINOP
        char *name; // AST_VARIABLE, variable name
        struct
        {                      // AST_FUNCTION_DEF
            char *name;        // Function name
            char **parameters; // Parameter names
            int param_count;
            struct ASTNode *body; // Function body
            Type *return_type;
            struct ASTNode **param_types; // Arrary of types for parameters
        } function_def;                   // For AST_FUNCTION_DEF
        struct
        {                               // AST_FUNCTION_CALL
            char *name;                 // Function name
            struct ASTNode **arguments; // Argument expressions
            int arg_count;
        } function_call; // For AST_FUNCTION_CALL
        struct
        {
            char *type_name;         // The ADT type name (e.g. "Maybe")
            char *constructor;       // The constructor name (e.g. "Just")
            struct Type **arguments; // Arguments passed to the constructor
            int arg_count;
        } adt_constructor_def; // For AST_ADT_CONSTRUCTOR_DEF
        struct
        {
            char *type_name;
            struct ASTNode **constructors;
            int constructor_count;
        } adt_definition; // For AST_ADT_DEFINITION
        struct
        {
            char *type_name;            // The ADT type name (e.g. "Maybe")
            char *constructor;          // The constructor name (e.g. "Just")
            struct ASTNode **arguments; // Arguments passed to the constructor
            int arg_count;
        } adt_constructor_call; // For AST_ADT_CONSTRUCTOR_CALL
        struct
        {                          // AST_LET_BINDING
            char *name;            // Variable name
            struct ASTNode *value; // Value expression
            struct ASTNode *body;  // Expression where the binding is used
        } let_binding;
        struct
        {                                // AST_STATEMENT_LIST
            struct ASTNode **statements; // Array of statements
            int statement_count;         // Number of statements
        } statement_list;
        struct
        { // AST_IF_EXPR
            struct ASTNode *condition;
            struct ASTNode *then_branch;
            struct ASTNode *else_branch;
        } if_expr;
        struct
        {
            struct ASTNode *expression; // Expression to match
            Pattern **patterns;         // Array of patterns
            int pattern_count;
        } case_expr;
    };
} ASTNode;

typedef struct
{
    Lexer lexer;
    Token current_token;
} Parser;

Parser parser_create(Lexer lexer);
void parser_eat(Parser *parser, TokenType token_type);

Type *parse_type(Parser *parser);
Type *parse_atomic_type(Parser *parser);
void free_type(Type *type);
void print_type(Type *type, int indent);

void free_pattern(Pattern *pattern);
void print_pattern(Pattern *pattern, int indent);

ASTNode *parse_term(Parser *parser);
ASTNode *parse_string(Parser *parser);
ASTNode *parse_if_expression(Parser *parser);
ASTNode *parse_multiplicative_expression(Parser *parser);
ASTNode *parse_additive_expression(Parser *parser);
ASTNode *parse_expression(Parser *parser);
ASTNode *parse_adt_definition(Parser *parser);
ASTNode *parse_adt_constructor_call(Parser *parser);
ASTNode *parse_comparison(Parser *parser);
ASTNode *parse_factor(Parser *parser);
ASTNode *parse_function_definition(Parser *parser);
ASTNode *parse_function_application(Parser *parser, char *func_name);
ASTNode *parse_let_binding(Parser *parser);
ASTNode *parse_statement_list(Parser *parser);
ASTNode *parse_statement(Parser *parser);
ASTNode *parse_case_expression(Parser *parser);

void free_ast(ASTNode *node);
const char *ast_node_type_to_string(ASTNodeType type);
void print_ast(ASTNode *node, int indent);

// Core parsing functions (Phase 2)
CoreExpr *parse_core_expression(Parser *parser);
CoreExpr *parse_core_atom(Parser *parser);
CoreExpr *parse_core_application(Parser *parser);
CoreExpr *parse_core_lambda(Parser *parser);
CoreExpr *parse_core_let(Parser *parser);
CoreExpr *parse_core_case(Parser *parser);

#endif // PARSER_H
