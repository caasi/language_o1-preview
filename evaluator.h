#ifndef EVALUATOR_H
#define EVALUATOR_H

#include "parser.h"
#include "env.h"
#include "symbol_table.h"

Value *evaluate(ASTNode *node, Env *env, SymbolTable *sym_table, int depth);

#endif // EVALUATOR_H
