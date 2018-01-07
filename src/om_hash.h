#ifndef __OM_HASH_H__
#define __OM_HASH_H__

#include "hash/uthash.h"
#include "om_node.h"
#include "redismodule.h"

typedef struct {
	const char* key;
    LNode* lnode;
    UT_hash_handle hh;
} Node;

LNode* HASH_get_node(Node*, const char*);
LNode* HASH_create_node(Node**, const char*, const size_t);
void HASH_remove_node(Node**, const char*);
size_t HASH_table_size(Node*);

#endif