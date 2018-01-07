#ifndef __OM_MODULE_H__
#define __OM_MODULE_H__

#include "om_internal.h"
#include "om_hash.h"
#include "om_type.h"
#include "redismodule.h"

#define GET_OS_SUCCESS 0
#define GET_OS_NOT_EXIST 1
#define GET_OS_WRONG_TYPE 2
#define OS_INVALID_COUNT "ERR value is not a valid integer"

#define LSENTINEL os->oset->lsentinel

#endif