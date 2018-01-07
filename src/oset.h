#ifndef __OSET_H__
#define __OSET_H__

#include <stdlib.h>
#include <math.h>
#include "redismodule.h"
#include "node.h"

// #include <stdio.h>

#define LOG2(X) ((unsigned) (8 * sizeof(unsigned long long) - __builtin_clzll((X)) - 1))
#define UNSIGNED_SIZE 64

typedef struct {
	UNode* usentinel;
	LNode* lsentinel;
	size_t height;
	double threshold;
} OrderedSet;

void OSET_remove(LNode*);
int OSET_compare(LNode* x, LNode* y);
void OSET_add_after(OrderedSet*, LNode*, LNode*, const size_t);
void OSET_add_before(OrderedSet*, LNode*, LNode*, const size_t);
void OSET_add_head(OrderedSet*, LNode*, const size_t);
void OSET_add_tail(OrderedSet*, LNode*, const size_t);

#endif