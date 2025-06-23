#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"
#include "core.h"

// ============================================================================
// Core Expression Creation Functions
// ============================================================================

CoreExpr *core_expr_create_var(CoreVar *var) {
    CoreExpr *expr = (CoreExpr *)malloc(sizeof(CoreExpr));
    expr->expr_type = CORE_VAR;
    expr->var = var;
    return expr;
}

CoreExpr *core_expr_create_lit(CoreLit *lit) {
    CoreExpr *expr = (CoreExpr *)malloc(sizeof(CoreExpr));
    expr->expr_type = CORE_LIT;
    expr->lit = lit;
    return expr;
}

CoreExpr *core_expr_create_app(CoreExpr *fun, CoreExpr *arg) {
    CoreExpr *expr = (CoreExpr *)malloc(sizeof(CoreExpr));
    expr->expr_type = CORE_APP;
    expr->app.fun = fun;
    expr->app.arg = arg;
    return expr;
}

CoreExpr *core_expr_create_lam(CoreVar *var, CoreExpr *body) {
    CoreExpr *expr = (CoreExpr *)malloc(sizeof(CoreExpr));
    expr->expr_type = CORE_LAM;
    expr->lam.var = var;
    expr->lam.body = body;
    return expr;
}

CoreExpr *core_expr_create_let(CoreBind **binds, int bind_count, CoreExpr *body, int is_recursive) {
    CoreExpr *expr = (CoreExpr *)malloc(sizeof(CoreExpr));
    expr->expr_type = CORE_LET;
    expr->let.binds = binds;
    expr->let.bind_count = bind_count;
    expr->let.body = body;
    expr->let.is_recursive = is_recursive;
    return expr;
}

CoreExpr *core_expr_create_case(CoreExpr *expr_val, CoreVar *var, CoreType *type, CoreAlt **alts, int alt_count) {
    CoreExpr *expr = (CoreExpr *)malloc(sizeof(CoreExpr));
    expr->expr_type = CORE_CASE;
    expr->case_expr.expr = expr_val;
    expr->case_expr.var = var;
    expr->case_expr.type = type;
    expr->case_expr.alts = alts;
    expr->case_expr.alt_count = alt_count;
    return expr;
}

// ============================================================================
// Core Helper Structure Creation Functions
// ============================================================================

CoreVar *core_var_create(char *name, CoreType *type, int var_kind) {
    CoreVar *var = (CoreVar *)malloc(sizeof(CoreVar));
    var->name = strdup(name);
    var->type = type;
    var->var_kind = var_kind;
    return var;
}

CoreLit *core_lit_create_int(int val) {
    CoreLit *lit = (CoreLit *)malloc(sizeof(CoreLit));
    lit->lit_kind = LIT_INT;
    lit->int_val = val;
    return lit;
}

CoreLit *core_lit_create_double(double val) {
    CoreLit *lit = (CoreLit *)malloc(sizeof(CoreLit));
    lit->lit_kind = LIT_DOUBLE;
    lit->double_val = val;
    return lit;
}

CoreLit *core_lit_create_string(char *val) {
    CoreLit *lit = (CoreLit *)malloc(sizeof(CoreLit));
    lit->lit_kind = LIT_STRING;
    lit->string_val = strdup(val);
    return lit;
}

CoreBind *core_bind_create(CoreVar *var, CoreExpr *expr) {
    CoreBind *bind = (CoreBind *)malloc(sizeof(CoreBind));
    bind->var = var;
    bind->expr = expr;
    return bind;
}

CoreAlt *core_alt_create_con(char *constructor, CoreVar **vars, int var_count, CoreExpr *expr) {
    CoreAlt *alt = (CoreAlt *)malloc(sizeof(CoreAlt));
    alt->alt_kind = ALT_CON;
    alt->con.constructor = strdup(constructor);
    alt->con.vars = vars;
    alt->con.var_count = var_count;
    alt->expr = expr;
    return alt;
}

CoreAlt *core_alt_create_default(CoreExpr *expr) {
    CoreAlt *alt = (CoreAlt *)malloc(sizeof(CoreAlt));
    alt->alt_kind = ALT_DEFAULT;
    alt->expr = expr;
    return alt;
}

// ============================================================================
// Core Memory Management Functions
// ============================================================================

void core_expr_free(CoreExpr *expr) {
    if (!expr) return;
    
    switch (expr->expr_type) {
        case CORE_VAR:
            core_var_free(expr->var);
            break;
        case CORE_LIT:
            core_lit_free(expr->lit);
            break;
        case CORE_APP:
            core_expr_free(expr->app.fun);
            core_expr_free(expr->app.arg);
            break;
        case CORE_LAM:
            core_var_free(expr->lam.var);
            core_expr_free(expr->lam.body);
            break;
        case CORE_LET:
            for (int i = 0; i < expr->let.bind_count; i++) {
                core_bind_free(expr->let.binds[i]);
            }
            free(expr->let.binds);
            core_expr_free(expr->let.body);
            break;
        case CORE_CASE:
            core_expr_free(expr->case_expr.expr);
            core_var_free(expr->case_expr.var);
            core_type_free(expr->case_expr.type);
            for (int i = 0; i < expr->case_expr.alt_count; i++) {
                core_alt_free(expr->case_expr.alts[i]);
            }
            free(expr->case_expr.alts);
            break;
        case CORE_CAST:
            core_expr_free(expr->cast.expr);
            core_type_free(expr->cast.from_type);
            core_type_free(expr->cast.to_type);
            break;
        case CORE_TICK:
            free(expr->tick.tick_info);
            core_expr_free(expr->tick.expr);
            break;
        case CORE_TYPE:
            core_type_free(expr->type);
            break;
        case CORE_COERCION:
            core_type_free(expr->coercion.from_type);
            core_type_free(expr->coercion.to_type);
            break;
    }
    free(expr);
}

void core_type_free(CoreType *type) {
    if (!type) return;
    
    switch (type->kind) {
        case CORE_TYPE_VAR:
            free(type->var_name);
            break;
        case CORE_TYPE_CON:
            free(type->con_name);
            break;
        case CORE_TYPE_APP:
            core_type_free(type->app.fun);
            core_type_free(type->app.arg);
            break;
        case CORE_TYPE_FORALL:
            free(type->forall.var);
            core_type_free(type->forall.body);
            break;
    }
    free(type);
}

void core_var_free(CoreVar *var) {
    if (!var) return;
    free(var->name);
    core_type_free(var->type);
    free(var);
}

void core_lit_free(CoreLit *lit) {
    if (!lit) return;
    if (lit->lit_kind == LIT_STRING) {
        free(lit->string_val);
    }
    free(lit);
}

void core_bind_free(CoreBind *bind) {
    if (!bind) return;
    core_var_free(bind->var);
    core_expr_free(bind->expr);
    free(bind);
}

void core_alt_free(CoreAlt *alt) {
    if (!alt) return;
    
    switch (alt->alt_kind) {
        case ALT_CON:
            free(alt->con.constructor);
            for (int i = 0; i < alt->con.var_count; i++) {
                core_var_free(alt->con.vars[i]);
            }
            free(alt->con.vars);
            break;
        case ALT_LIT:
            core_lit_free(alt->lit);
            break;
        case ALT_DEFAULT:
            // Nothing extra to free
            break;
    }
    core_expr_free(alt->expr);
    free(alt);
}

// ============================================================================
// Core Pretty Printing Functions
// ============================================================================

const char *core_expr_type_to_string(CoreExprType type) {
    switch (type) {
        case CORE_VAR: return "CORE_VAR";
        case CORE_LIT: return "CORE_LIT";
        case CORE_APP: return "CORE_APP";
        case CORE_LAM: return "CORE_LAM";
        case CORE_LET: return "CORE_LET";
        case CORE_CASE: return "CORE_CASE";
        case CORE_CAST: return "CORE_CAST";
        case CORE_TICK: return "CORE_TICK";
        case CORE_TYPE: return "CORE_TYPE";
        case CORE_COERCION: return "CORE_COERCION";
        default: return "UNKNOWN_CORE_EXPR";
    }
}

static void print_indent(int indent) {
    for (int i = 0; i < indent; i++) {
        printf("  ");
    }
}

void core_expr_print(CoreExpr *expr, int indent) {
    if (!expr) {
        print_indent(indent);
        printf("NULL\n");
        return;
    }
    
    print_indent(indent);
    printf("%s:\n", core_expr_type_to_string(expr->expr_type));
    
    switch (expr->expr_type) {
        case CORE_VAR:
            print_indent(indent + 1);
            printf("name: %s\n", expr->var->name);
            break;
        case CORE_LIT:
            print_indent(indent + 1);
            switch (expr->lit->lit_kind) {
                case LIT_INT:
                    printf("int: %d\n", expr->lit->int_val);
                    break;
                case LIT_DOUBLE:
                    printf("double: %f\n", expr->lit->double_val);
                    break;
                case LIT_STRING:
                    printf("string: \"%s\"\n", expr->lit->string_val);
                    break;
                case LIT_CHAR:
                    printf("char: '%c'\n", expr->lit->char_val);
                    break;
            }
            break;
        case CORE_APP:
            print_indent(indent + 1);
            printf("fun:\n");
            core_expr_print(expr->app.fun, indent + 2);
            print_indent(indent + 1);
            printf("arg:\n");
            core_expr_print(expr->app.arg, indent + 2);
            break;
        case CORE_LAM:
            print_indent(indent + 1);
            printf("var: %s\n", expr->lam.var->name);
            print_indent(indent + 1);
            printf("body:\n");
            core_expr_print(expr->lam.body, indent + 2);
            break;
        case CORE_LET:
            print_indent(indent + 1);
            printf("recursive: %s\n", expr->let.is_recursive ? "true" : "false");
            print_indent(indent + 1);
            printf("bindings (%d):\n", expr->let.bind_count);
            for (int i = 0; i < expr->let.bind_count; i++) {
                print_indent(indent + 2);
                printf("%s =\n", expr->let.binds[i]->var->name);
                core_expr_print(expr->let.binds[i]->expr, indent + 3);
            }
            print_indent(indent + 1);
            printf("body:\n");
            core_expr_print(expr->let.body, indent + 2);
            break;
        case CORE_CASE:
            print_indent(indent + 1);
            printf("expr:\n");
            core_expr_print(expr->case_expr.expr, indent + 2);
            print_indent(indent + 1);
            printf("alternatives (%d):\n", expr->case_expr.alt_count);
            for (int i = 0; i < expr->case_expr.alt_count; i++) {
                print_indent(indent + 2);
                CoreAlt *alt = expr->case_expr.alts[i];
                switch (alt->alt_kind) {
                    case ALT_CON:
                        printf("%s ->", alt->con.constructor);
                        break;
                    case ALT_DEFAULT:
                        printf("_ ->");
                        break;
                    case ALT_LIT:
                        printf("literal ->");
                        break;
                }
                printf("\n");
                core_expr_print(alt->expr, indent + 3);
            }
            break;
        default:
            print_indent(indent + 1);
            printf("(not implemented for printing)\n");
            break;
    }
}

// ============================================================================
// Core Expression Builder Utilities
// ============================================================================

CoreExpr *core_var(char *name) {
    CoreVar *var = core_var_create(name, NULL, VAR_LOCAL);
    return core_expr_create_var(var);
}

CoreExpr *core_int(int val) {
    CoreLit *lit = core_lit_create_int(val);
    return core_expr_create_lit(lit);
}

CoreExpr *core_double(double val) {
    CoreLit *lit = core_lit_create_double(val);
    return core_expr_create_lit(lit);
}

CoreExpr *core_string(char *val) {
    CoreLit *lit = core_lit_create_string(val);
    return core_expr_create_lit(lit);
}

CoreExpr *core_app2(CoreExpr *fun, CoreExpr *arg1, CoreExpr *arg2) {
    CoreExpr *app1 = core_expr_create_app(fun, arg1);
    return core_expr_create_app(app1, arg2);
}

CoreExpr *core_app3(CoreExpr *fun, CoreExpr *arg1, CoreExpr *arg2, CoreExpr *arg3) {
    CoreExpr *app1 = core_expr_create_app(fun, arg1);
    CoreExpr *app2 = core_expr_create_app(app1, arg2);
    return core_expr_create_app(app2, arg3);
}

CoreExpr *core_lambda(char *var_name, CoreExpr *body) {
    CoreVar *var = core_var_create(var_name, NULL, VAR_LOCAL);
    return core_expr_create_lam(var, body);
}

CoreExpr *core_lambda2(char *var1, char *var2, CoreExpr *body) {
    CoreVar *v2 = core_var_create(var2, NULL, VAR_LOCAL);
    CoreExpr *inner_lam = core_expr_create_lam(v2, body);
    CoreVar *v1 = core_var_create(var1, NULL, VAR_LOCAL);
    return core_expr_create_lam(v1, inner_lam);
}

CoreExpr *core_let_simple(char *var_name, CoreExpr *value, CoreExpr *body) {
    CoreVar *var = core_var_create(var_name, NULL, VAR_LOCAL);
    CoreBind *bind = core_bind_create(var, value);
    CoreBind **binds = (CoreBind **)malloc(sizeof(CoreBind *));
    binds[0] = bind;
    return core_expr_create_let(binds, 1, body, 0);
}

CoreExpr *core_letrec_simple(char *var_name, CoreExpr *value, CoreExpr *body) {
    CoreVar *var = core_var_create(var_name, NULL, VAR_LOCAL);
    CoreBind *bind = core_bind_create(var, value);
    CoreBind **binds = (CoreBind **)malloc(sizeof(CoreBind *));
    binds[0] = bind;
    return core_expr_create_let(binds, 1, body, 1);
}

CoreExpr *core_case_simple(CoreExpr *expr, CoreAlt **alts, int alt_count) {
    return core_expr_create_case(expr, NULL, NULL, alts, alt_count);
}

// ============================================================================
// Core Expression Analysis
// ============================================================================

int core_expr_is_value(CoreExpr *expr) {
    if (!expr) return 0;
    
    switch (expr->expr_type) {
        case CORE_LIT:
        case CORE_LAM:
            return 1;
        case CORE_VAR:
            // Data constructors are values
            return expr->var->var_kind == VAR_DATA_CON;
        default:
            return 0;
    }
}

char *core_expr_get_var_name(CoreExpr *expr) {
    if (!expr || expr->expr_type != CORE_VAR) return NULL;
    return expr->var->name;
}

int core_expr_count_lambdas(CoreExpr *expr) {
    if (!expr || expr->expr_type != CORE_LAM) return 0;
    return 1 + core_expr_count_lambdas(expr->lam.body);
}

// ============================================================================
// Core Expression Transformation (Basic implementation)
// ============================================================================

CoreExpr *ast_to_core(ASTNode *ast) {
    if (!ast) return NULL;
    
    switch (ast->type) {
        case AST_NUMBER:
            return core_double(ast->number);
            
        case AST_STRING:
            return core_string(ast->string_value);
            
        case AST_VARIABLE:
            return core_var(ast->name);
            
        case AST_BINOP: {
            // Convert binary operations to function applications
            // x + y becomes (+) x y
            char *op_name;
            switch (ast->binop.op) {
                case TOKEN_PLUS: op_name = "+"; break;
                case TOKEN_MINUS: op_name = "-"; break;
                case TOKEN_MUL: op_name = "*"; break;
                case TOKEN_DIV: op_name = "/"; break;
                case TOKEN_EQUAL: op_name = "=="; break;
                default: op_name = "unknown_op"; break;
            }
            
            CoreExpr *op_var = core_var(op_name);
            CoreExpr *left = ast_to_core(ast->binop.left);
            CoreExpr *right = ast_to_core(ast->binop.right);
            return core_app2(op_var, left, right);
        }
        
        case AST_FUNCTION_CALL: {
            // f(a, b, c) becomes f a b c
            CoreExpr *fun = core_var(ast->function_call.name);
            CoreExpr *result = fun;
            
            for (int i = 0; i < ast->function_call.arg_count; i++) {
                CoreExpr *arg = ast_to_core(ast->function_call.arguments[i]);
                result = core_expr_create_app(result, arg);
            }
            return result;
        }
        
        case AST_LET_BINDING: {
            // let x = v in body end becomes let x = v in body
            CoreExpr *value = ast_to_core(ast->let_binding.value);
            CoreExpr *body = ast_to_core(ast->let_binding.body);
            return core_let_simple(ast->let_binding.name, value, body);
        }
        
        case AST_IF_EXPR: {
            // if c then a else b becomes case c of True -> a; False -> b
            CoreExpr *cond = ast_to_core(ast->if_expr.condition);
            CoreExpr *then_expr = ast_to_core(ast->if_expr.then_branch);
            CoreExpr *else_expr = ast_to_core(ast->if_expr.else_branch);
            
            // Create alternatives
            CoreAlt **alts = (CoreAlt **)malloc(2 * sizeof(CoreAlt *));
            alts[0] = core_alt_create_con("True", NULL, 0, then_expr);
            alts[1] = core_alt_create_con("False", NULL, 0, else_expr);
            
            return core_case_simple(cond, alts, 2);
        }
        
        // For now, return a placeholder for unimplemented AST nodes
        default: {
            // Create a placeholder variable with the AST node type name
            char placeholder[100];
            snprintf(placeholder, sizeof(placeholder), "TODO_%s", ast_node_type_to_string(ast->type));
            return core_var(placeholder);
        }
    }
}

CoreExpr *core_expr_simplify(CoreExpr *expr) {
    // Basic simplification - for now just return the expression as-is
    // TODO: Implement optimizations like beta reduction, constant folding, etc.
    return expr;
}

// ============================================================================
// Simple Core Evaluator (for basic expressions)
// ============================================================================

double core_eval_simple(CoreExpr *expr) {
    if (!expr) return 0.0;
    
    switch (expr->expr_type) {
        case CORE_LIT:
            if (expr->lit->lit_kind == LIT_DOUBLE) {
                return expr->lit->double_val;
            } else if (expr->lit->lit_kind == LIT_INT) {
                return (double)expr->lit->int_val;
            }
            break;
            
        case CORE_APP: {
            // Special case: Handle curried function application f a b
            // This parses as ((f a) b), where (f a) should be a partial application
            if (expr->app.fun->expr_type == CORE_APP) {
                CoreExpr *inner_app = expr->app.fun;
                
                // Check if this is a curried lambda application: ((λx.λy.body a) b)
                if (inner_app->app.fun->expr_type == CORE_LAM) {
                    CoreExpr *outer_lambda = inner_app->app.fun;
                    if (outer_lambda->lam.body->expr_type == CORE_LAM) {
                        // This is a curried function: λx.λy.body applied to two arguments
                        CoreExpr *inner_lambda = outer_lambda->lam.body;
                        CoreExpr *arg1 = inner_app->app.arg;
                        CoreExpr *arg2 = expr->app.arg;
                        
                        // Evaluate both arguments
                        double arg1_val = core_eval_simple(arg1);
                        double arg2_val = core_eval_simple(arg2);
                        
                        // Apply both substitutions to the inner body
                        CoreExpr *body_with_arg1 = core_substitute_simple(inner_lambda->lam.body,
                                                                         outer_lambda->lam.var->name,
                                                                         arg1_val);
                        CoreExpr *final_body = core_substitute_simple(body_with_arg1,
                                                                     inner_lambda->lam.var->name,
                                                                     arg2_val);
                        
                        double result = core_eval_simple(final_body);
                        core_expr_free(body_with_arg1);
                        core_expr_free(final_body);
                        return result;
                    }
                }
                
                // Handle binary operations: op a b
                if (inner_app->app.fun->expr_type == CORE_VAR) {
                    char *op_name = inner_app->app.fun->var->name;
                    double left = core_eval_simple(inner_app->app.arg);
                    double right = core_eval_simple(expr->app.arg);
                    
                    if (strcmp(op_name, "+") == 0) {
                        return left + right;
                    } else if (strcmp(op_name, "-") == 0) {
                        return left - right;
                    } else if (strcmp(op_name, "*") == 0) {
                        return left * right;
                    } else if (strcmp(op_name, "/") == 0) {
                        if (right == 0.0) {
                            fprintf(stderr, "Error: Division by zero\n");
                            exit(EXIT_FAILURE);
                        }
                        return left / right;
                    }
                }
                
                // Other nested applications
                fprintf(stderr, "Error: Cannot evaluate complex application\n");
                exit(EXIT_FAILURE);
            }
            
            // Handle single lambda application: (\x. body) arg
            if (expr->app.fun->expr_type == CORE_LAM) {
                CoreExpr *lambda = expr->app.fun;
                CoreExpr *arg = expr->app.arg;
                
                double arg_val = core_eval_simple(arg);
                
                // Substitute the parameter with the argument value in the lambda body
                CoreExpr *substituted_body = core_substitute_simple(lambda->lam.body, 
                                                                   lambda->lam.var->name, 
                                                                   arg_val);
                double result = core_eval_simple(substituted_body);
                core_expr_free(substituted_body);
                return result;
            }
            
            // If neither of the above, it might be a variable application
            fprintf(stderr, "Error: Cannot evaluate application - function type %s\n", 
                    core_expr_type_to_string(expr->app.fun->expr_type));
            exit(EXIT_FAILURE);
        }
        
        case CORE_LET: {
            // Let evaluation with lambda substitution
            // Instead of evaluating the bound value, substitute it directly
            CoreExpr *substituted_body = core_substitute_expr(expr->let.body, 
                                                            expr->let.binds[0]->var->name, 
                                                            expr->let.binds[0]->expr);
            double result = core_eval_simple(substituted_body);
            core_expr_free(substituted_body);
            return result;
        }
        
        case CORE_VAR: {
            // Variables should have been substituted by now
            // If we reach here, it might be an unbound variable or operator
            fprintf(stderr, "Error: Unbound variable '%s'\n", expr->var->name);
            exit(EXIT_FAILURE);
        }
        
        case CORE_LAM: {
            // Lambdas can't be evaluated to a number directly
            // This suggests we need a different evaluation strategy
            fprintf(stderr, "Error: Cannot evaluate lambda to number directly\n");
            exit(EXIT_FAILURE);
        }
        
        default:
            fprintf(stderr, "Error: Core evaluation not implemented for expression type %s\n", 
                    core_expr_type_to_string(expr->expr_type));
            exit(EXIT_FAILURE);
    }
    
    return 0.0;
}

// Simple substitution: replace variable with literal value
CoreExpr *core_substitute_simple(CoreExpr *expr, char *var_name, double value) {
    if (!expr) return NULL;
    
    switch (expr->expr_type) {
        case CORE_VAR: {
            if (strcmp(expr->var->name, var_name) == 0) {
                // Replace the variable with the literal value
                return core_double(value);
            } else {
                // Return a copy of the variable
                return core_var(strdup(expr->var->name));
            }
        }
        
        case CORE_LIT: {
            // Copy the literal
            if (expr->lit->lit_kind == LIT_DOUBLE) {
                return core_double(expr->lit->double_val);
            } else if (expr->lit->lit_kind == LIT_INT) {
                return core_int(expr->lit->int_val);
            } else if (expr->lit->lit_kind == LIT_STRING) {
                return core_string(strdup(expr->lit->string_val));
            }
            break;
        }
        
        case CORE_APP: {
            // Recursively substitute in both function and argument
            CoreExpr *fun = core_substitute_simple(expr->app.fun, var_name, value);
            CoreExpr *arg = core_substitute_simple(expr->app.arg, var_name, value);
            return core_expr_create_app(fun, arg);
        }
        
        case CORE_LAM: {
            // Don't substitute if lambda parameter shadows the variable
            if (strcmp(expr->lam.var->name, var_name) == 0) {
                // Variable is shadowed, return copy of lambda
                CoreExpr *body_copy = core_substitute_simple(expr->lam.body, "___never_match___", 0.0);
                return core_lambda(strdup(expr->lam.var->name), body_copy);
            } else {
                // Substitute in body
                CoreExpr *body = core_substitute_simple(expr->lam.body, var_name, value);
                return core_lambda(strdup(expr->lam.var->name), body);
            }
        }
        
        case CORE_LET: {
            // For now, don't substitute inside nested lets (complex)
            // This is a simplified version
            CoreExpr *binds_expr = core_substitute_simple(expr->let.binds[0]->expr, var_name, value);
            CoreExpr *body = core_substitute_simple(expr->let.body, var_name, value);
            return core_let_simple(strdup(expr->let.binds[0]->var->name), binds_expr, body);
        }
        
        default:
            fprintf(stderr, "Error: Substitution not implemented for expression type %s\n", 
                    core_expr_type_to_string(expr->expr_type));
            exit(EXIT_FAILURE);
    }
    
    return NULL;
}

// Expression substitution: replace variable with another expression
CoreExpr *core_substitute_expr(CoreExpr *expr, char *var_name, CoreExpr *replacement) {
    if (!expr) return NULL;
    
    switch (expr->expr_type) {
        case CORE_VAR: {
            if (strcmp(expr->var->name, var_name) == 0) {
                // Replace the variable with a copy of the replacement expression
                return core_expr_copy(replacement);
            } else {
                // Return a copy of the variable
                return core_var(strdup(expr->var->name));
            }
        }
        
        case CORE_LIT: {
            // Copy the literal
            if (expr->lit->lit_kind == LIT_DOUBLE) {
                return core_double(expr->lit->double_val);
            } else if (expr->lit->lit_kind == LIT_INT) {
                return core_int(expr->lit->int_val);
            } else if (expr->lit->lit_kind == LIT_STRING) {
                return core_string(strdup(expr->lit->string_val));
            }
            break;
        }
        
        case CORE_APP: {
            // Recursively substitute in both function and argument
            CoreExpr *fun = core_substitute_expr(expr->app.fun, var_name, replacement);
            CoreExpr *arg = core_substitute_expr(expr->app.arg, var_name, replacement);
            return core_expr_create_app(fun, arg);
        }
        
        case CORE_LAM: {
            // Don't substitute if lambda parameter shadows the variable
            if (strcmp(expr->lam.var->name, var_name) == 0) {
                // Variable is shadowed, return copy of lambda without substituting
                CoreExpr *body_copy = core_expr_copy(expr->lam.body);
                return core_lambda(strdup(expr->lam.var->name), body_copy);
            } else {
                // Substitute in body
                CoreExpr *body = core_substitute_expr(expr->lam.body, var_name, replacement);
                return core_lambda(strdup(expr->lam.var->name), body);
            }
        }
        
        case CORE_LET: {
            // For now, don't substitute inside nested lets (complex)
            CoreExpr *binds_expr = core_substitute_expr(expr->let.binds[0]->expr, var_name, replacement);
            CoreExpr *body = core_substitute_expr(expr->let.body, var_name, replacement);
            return core_let_simple(strdup(expr->let.binds[0]->var->name), binds_expr, body);
        }
        
        default:
            fprintf(stderr, "Error: Expression substitution not implemented for expression type %s\n", 
                    core_expr_type_to_string(expr->expr_type));
            exit(EXIT_FAILURE);
    }
    
    return NULL;
}

// Deep copy of Core expression
CoreExpr *core_expr_copy(CoreExpr *expr) {
    if (!expr) return NULL;
    
    switch (expr->expr_type) {
        case CORE_VAR:
            return core_var(strdup(expr->var->name));
            
        case CORE_LIT:
            if (expr->lit->lit_kind == LIT_DOUBLE) {
                return core_double(expr->lit->double_val);
            } else if (expr->lit->lit_kind == LIT_INT) {
                return core_int(expr->lit->int_val);
            } else if (expr->lit->lit_kind == LIT_STRING) {
                return core_string(strdup(expr->lit->string_val));
            }
            break;
            
        case CORE_APP:
            return core_expr_create_app(core_expr_copy(expr->app.fun), 
                                       core_expr_copy(expr->app.arg));
            
        case CORE_LAM:
            return core_lambda(strdup(expr->lam.var->name), 
                              core_expr_copy(expr->lam.body));
            
        case CORE_LET:
            return core_let_simple(strdup(expr->let.binds[0]->var->name),
                                  core_expr_copy(expr->let.binds[0]->expr),
                                  core_expr_copy(expr->let.body));
            
        default:
            fprintf(stderr, "Error: Copy not implemented for expression type %s\n", 
                    core_expr_type_to_string(expr->expr_type));
            exit(EXIT_FAILURE);
    }
    
    return NULL;
}