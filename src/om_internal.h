#ifndef __OM_INTERNAL_H__
#define __OM_INTERNAL_H__

#include <stdlib.h>
#include <math.h>
#include "redismodule.h"
#include "om_node.h"

// #include <stdio.h>

#define LOG2(X) ((unsigned) (8 * sizeof(unsigned long long) - __builtin_clzll((X)) - 1))
#define UNSIGNED_SIZE 64

int lcompare(LNode*, LNode*);
void lpush(LNode*, LNode*);
void lrelabel(LNode*, const size_t, size_t*, double*);
void lremove(LNode*);
void lpush_first(LNode*, LNode*);
void lpush_initial(LNode*, LNode*, UNode*);

#endif