#ifndef __MODULE_H__
#define __MODULE_H__

#include "oset.h"
#include "hash.h"
#include "os_type.h"
#include "redismodule.h"

#define GET_OS_SUCCESS 0
#define GET_OS_NOT_EXIST 1
#define GET_OS_WRONG_TYPE 2
#define OS_INVALID_COUNT "ERR value is not a valid integer"

#define LSENTINEL os->oset->lsentinel

#endif