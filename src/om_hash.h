#ifndef __OM_HASH_H__
#define __OM_HASH_H__

#include "hash/uthash.h"
#include "om_node.h"
#include "redismodule.h"

typedef void (*om_remove_fn)(LNode*);
typedef struct {
	const char* key;
    LNode* lnode;
    UT_hash_handle hh;
} Node;

LNode* get_node_write_mod(Node**, const char*, size_t, om_remove_fn);
LNode* get_node_read_mod(Node*, const char*);
int remove_node(Node**, const char*, om_remove_fn);
size_t table_size(Node*);

#endif