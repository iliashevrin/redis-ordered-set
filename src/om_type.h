#ifndef __OM_TYPE_H__
#define __OM_TYPE_H__

#include "redismodule.h"
#include "om_internal.h"
#include "om_hash.h"

#define OS_ENCODING_VERSION 1
#define INITIAL_THRESHOLD 1.3

typedef struct {
	Node* hash; // The hash table
	OrderedSet* oset; // The linked ordered set
} RedisOS;

void* OSRdbLoad(RedisModuleIO*, int);
void OSRdbSave(RedisModuleIO*, void*);
void OSAofRewrite(RedisModuleIO*, RedisModuleString*, void*);
void OSFree(void*);
size_t OSMemUsage(const void*);
RedisOS* OSInit();


#endif