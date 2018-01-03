#ifndef __OM_API_H__
#define __OM_API_H__

#include "om_internal.h"
#include "om_hash.h"

typedef struct {
	Node* nodes;
	UNode* usentinel;
	LNode* lsentinel;
	size_t height;
	double threshold;
} OM;

int compare(OM*, const char*, const char*);
int push_after(OM*, const char*, const char*, size_t);
int push_before(OM*, const char*, const char*, size_t);
int push_first(OM*, const char*, size_t);
int push_last(OM*, const char*, size_t);
int remove_item(OM*, const char*);

#define SUCCESS 10
#define ELEMENT_NOT_FOUND 11
#define LIST_EMPTY 12

#endif