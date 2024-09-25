#ifndef EVALUATOR_H
#define EVALUATOR_H

#include "parser.h"
#include "env.h"

Value *evaluate(ASTNode *node, Env *env, int depth);

#endif // EVALUATOR_H
