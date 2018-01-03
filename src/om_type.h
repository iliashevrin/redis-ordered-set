#ifndef __OM_TYPE_H__
#define __OM_TYPE_H__

#include "redismodule.h"
#include "om_api.h"

#define OM_ENCODING_VERSION 1
#define INITIAL_THRESHOLD 1.3

void* OMRdbLoad(RedisModuleIO*, int);
void OMRdbSave(RedisModuleIO*, void*);
void OMAofRewrite(RedisModuleIO*, RedisModuleString*, void*);
void OMFree(void*);
size_t OMMemUsage(const void*);
OM* OMInit();


#endif