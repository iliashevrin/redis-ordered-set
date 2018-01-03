#ifndef __OM_MODULE_H__
#define __OM_MODULE_H__

#include "om_api.h"
#include "om_hash.h"
#include "om_type.h"
#include "redismodule.h"

#define GET_OM_SUCCESS 0
#define GET_OM_NOT_EXIST 1
#define GET_OM_WRONG_TYPE 2
#define OM_NOT_EXIST_ERR "ERR structure does not exist"
#define OM_ELEMENT_NOT_FOUND "ERR element not found in structure"
#define OM_INVALID_COUNT "ERR invalid count"

#endif