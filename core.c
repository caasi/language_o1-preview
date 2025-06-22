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