#ifndef CORE_H
#define CORE_H

#include "parser.h"

// Core AST creation functions are already declared in parser.h
// This header provides additional Core-specific utilities

// ============================================================================
// Core Expression Builder Utilities
// ============================================================================

// Convenience functions for building common Core expressions

// Build a simple variable reference
CoreExpr *core_var(char *name);

// Build primitive literals
CoreExpr *core_int(int val);
CoreExpr *core_double(double val);
CoreExpr *core_string(char *val);

// Build function application (curried)
CoreExpr *core_app2(CoreExpr *fun, CoreExpr *arg1, CoreExpr *arg2);
CoreExpr *core_app3(CoreExpr *fun, CoreExpr *arg1, CoreExpr *arg2, CoreExpr *arg3);

// Build lambda abstractions
CoreExpr *core_lambda(char *var_name, CoreExpr *body);
CoreExpr *core_lambda2(char *var1, char *var2, CoreExpr *body);

// Build let bindings
CoreExpr *core_let_simple(char *var_name, CoreExpr *value, CoreExpr *body);
CoreExpr *core_letrec_simple(char *var_name, CoreExpr *value, CoreExpr *body);

// Build case expressions
CoreExpr *core_case_simple(CoreExpr *expr, CoreAlt **alts, int alt_count);

// ============================================================================
// Core Expression Analysis
// ============================================================================

// Check if expression is a value (in weak head normal form)
int core_expr_is_value(CoreExpr *expr);

// Get the name of a variable (if expr is CORE_VAR)
char *core_expr_get_var_name(CoreExpr *expr);

// Count the number of lambda abstractions at the top level
int core_expr_count_lambdas(CoreExpr *expr);

// ============================================================================
// Core Expression Transformation
// ============================================================================

// Convert existing AST to Core (for migration)
CoreExpr *ast_to_core(ASTNode *ast);

// Simplify Core expressions (basic optimizations)
CoreExpr *core_expr_simplify(CoreExpr *expr);

#endif // CORE_H