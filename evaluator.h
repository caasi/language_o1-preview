#ifndef EVALUATOR_H
#define EVALUATOR_H

#include "parser.h"
#include "env.h"

Value *evaluate(ASTNode *node, Env *env);

#endif // EVALUATOR_H
