#include "om_module.h"

static RedisModuleType *OMType;

static int getOM(RedisModuleKey* key, OM** om) {

    *om = NULL;
    int type = RedisModule_KeyType(key);
    if (REDISMODULE_KEYTYPE_EMPTY == type) {
        return GET_OM_NOT_EXIST;
    } else if (RedisModule_ModuleTypeGetType(key) != OMType) {
        return GET_OM_WRONG_TYPE;
    } else {
        *om = RedisModule_ModuleTypeGetValue(key);
        return GET_OM_SUCCESS;
    }
}

static int OMAddAfter_RedisCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {

    if (argc != 4) {
        RedisModule_WrongArity(ctx);
        return REDISMODULE_ERR;
    }

    RedisModule_AutoMemory(ctx);
    RedisModule_ReplicateVerbatim(ctx);

    OM* om;
    int result = getOM(RedisModule_OpenKey(ctx, argv[1], REDISMODULE_READ | REDISMODULE_WRITE), &om);

    if (result == GET_OM_WRONG_TYPE) {
        RedisModule_ReplyWithError(ctx, REDISMODULE_ERRORMSG_WRONGTYPE);
        return REDISMODULE_ERR;
    } else if (result == GET_OM_NOT_EXIST) {
        RedisModule_ReplyWithNull(ctx);
        return REDISMODULE_ERR;
    }

    if (RedisModule_StringCompare(argv[2], argv[3]) == 0) {
        RedisModule_ReplyWithSimpleString(ctx, "OK");
        return REDISMODULE_OK;
    }

    size_t x_len, y_len;
    const char* x = RedisModule_StringPtrLen(argv[2], &x_len);
    const char* y = RedisModule_StringPtrLen(argv[3], &y_len);
    int add_result = add_after(om, x, y, y_len);

    if (add_result == ELEMENT_NOT_FOUND) {
        RedisModule_ReplyWithError(ctx, OM_ELEMENT_NOT_FOUND);
        return REDISMODULE_ERR;
    }

    RedisModule_ReplyWithSimpleString(ctx, "OK");
    return REDISMODULE_OK;
}

static int OMAddBefore_RedisCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {

    if (argc != 4) {
        RedisModule_WrongArity(ctx);
        return REDISMODULE_ERR;
    }

    RedisModule_AutoMemory(ctx);
    RedisModule_ReplicateVerbatim(ctx);

    OM* om;
    int result = getOM(RedisModule_OpenKey(ctx, argv[1], REDISMODULE_READ | REDISMODULE_WRITE), &om);

    if (result == GET_OM_WRONG_TYPE) {
        RedisModule_ReplyWithError(ctx, REDISMODULE_ERRORMSG_WRONGTYPE);
        return REDISMODULE_ERR;
    } else if (result == GET_OM_NOT_EXIST) {
        RedisModule_ReplyWithNull(ctx);
        return REDISMODULE_ERR;
    }

    if (RedisModule_StringCompare(argv[2], argv[3]) == 0) {
        RedisModule_ReplyWithSimpleString(ctx, "OK");
        return REDISMODULE_OK;
    }

    size_t x_len, y_len;
    const char* x = RedisModule_StringPtrLen(argv[2], &x_len);
    const char* y = RedisModule_StringPtrLen(argv[3], &y_len);
    int add_result = add_before(om, x, y, y_len);

    if (add_result == ELEMENT_NOT_FOUND) {
        RedisModule_ReplyWithError(ctx, OM_ELEMENT_NOT_FOUND);
        return REDISMODULE_ERR;
    }
    RedisModule_ReplyWithSimpleString(ctx, "OK");
    return REDISMODULE_OK;
    
}

static int OMRemove_RedisCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {

    if (argc != 3) {
        RedisModule_WrongArity(ctx);
        return REDISMODULE_ERR;
    }

    RedisModule_AutoMemory(ctx);
    RedisModule_ReplicateVerbatim(ctx);

    OM* om;
    RedisModuleKey* key = RedisModule_OpenKey(ctx, argv[1], REDISMODULE_READ | REDISMODULE_WRITE);
    int result = getOM(key, &om);

    if (result == GET_OM_WRONG_TYPE) {
        RedisModule_ReplyWithError(ctx, REDISMODULE_ERRORMSG_WRONGTYPE);
        return REDISMODULE_ERR;
    } else if (result == GET_OM_NOT_EXIST) {
        RedisModule_ReplyWithNull(ctx);
        return REDISMODULE_ERR;
    }

    size_t x_len;
    const char* x = RedisModule_StringPtrLen(argv[2], &x_len);
    int remove_result = remove_item(om, x);

    if (remove_result == ELEMENT_NOT_FOUND) {
        RedisModule_ReplyWithError(ctx, OM_ELEMENT_NOT_FOUND);
        return REDISMODULE_ERR;
    }

    if (remove_result == LIST_EMPTY) {
        RedisModule_DeleteKey(key);
    }

    RedisModule_ReplyWithSimpleString(ctx, "OK");
    return REDISMODULE_OK;
}

static int OMAddFirst_RedisCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {

    if (argc != 3) {
        RedisModule_WrongArity(ctx);
        return REDISMODULE_ERR;
    }

    RedisModule_AutoMemory(ctx);
    RedisModule_ReplicateVerbatim(ctx);

    OM* om;
    RedisModuleKey* key = RedisModule_OpenKey(ctx, argv[1], REDISMODULE_READ | REDISMODULE_WRITE);
    int result = getOM(key, &om);

    if (result == GET_OM_WRONG_TYPE) {
        RedisModule_ReplyWithError(ctx, REDISMODULE_ERRORMSG_WRONGTYPE);
        return REDISMODULE_ERR;
    } else if (result == GET_OM_NOT_EXIST) {
        om = OMInit();
        RedisModule_ModuleTypeSetValue(key, OMType, om);
    }

    size_t x_len;
    const char* x = RedisModule_StringPtrLen(argv[2], &x_len);
    add_first(om, x, x_len);
    RedisModule_ReplyWithSimpleString(ctx, "OK");
    return REDISMODULE_OK;
}

static int OMAddLast_RedisCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {

    if (argc != 3) {
        RedisModule_WrongArity(ctx);
        return REDISMODULE_ERR;
    }

    RedisModule_AutoMemory(ctx);
    RedisModule_ReplicateVerbatim(ctx);

    OM* om;
    RedisModuleKey* key = RedisModule_OpenKey(ctx, argv[1], REDISMODULE_READ | REDISMODULE_WRITE);
    int result = getOM(key, &om);

    if (result == GET_OM_WRONG_TYPE) {
        RedisModule_ReplyWithError(ctx, REDISMODULE_ERRORMSG_WRONGTYPE);
        return REDISMODULE_ERR;
    } else if (result == GET_OM_NOT_EXIST) {
        om = OMInit();
        RedisModule_ModuleTypeSetValue(key, OMType, om);
    }

    size_t x_len;
    const char* x = RedisModule_StringPtrLen(argv[2], &x_len);
    add_last(om, x, x_len);
    RedisModule_ReplyWithSimpleString(ctx, "OK");
    return REDISMODULE_OK;
}

static int OMCompare_RedisCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {

    if (argc != 4) {
        RedisModule_WrongArity(ctx);
        return REDISMODULE_ERR;
    }

    RedisModule_AutoMemory(ctx);

    OM* om;
    int result = getOM(RedisModule_OpenKey(ctx, argv[1], REDISMODULE_READ), &om);

    if (result == GET_OM_WRONG_TYPE) {
        RedisModule_ReplyWithError(ctx, REDISMODULE_ERRORMSG_WRONGTYPE);
        return REDISMODULE_ERR;
    } else if (result == GET_OM_NOT_EXIST) {
        RedisModule_ReplyWithNull(ctx);
        return REDISMODULE_ERR;
    }

    if (RedisModule_StringCompare(argv[2], argv[3]) == 0) {
        RedisModule_ReplyWithLongLong(ctx, 0);
        return REDISMODULE_OK;
    }

    size_t x_len, y_len;
    const char* x = RedisModule_StringPtrLen(argv[2], &x_len);
    const char* y = RedisModule_StringPtrLen(argv[3], &y_len);
    int cmp_result = compare(om, x, y);

    if (cmp_result == ELEMENT_NOT_FOUND) {
        RedisModule_ReplyWithError(ctx, OM_ELEMENT_NOT_FOUND);
        return REDISMODULE_ERR;
    }

    RedisModule_ReplyWithLongLong(ctx, cmp_result);
    return REDISMODULE_OK;
}

static int OMNext_RedisCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {

    if (argc != 4) {
        RedisModule_WrongArity(ctx);
        return REDISMODULE_ERR;
    }

    RedisModule_AutoMemory(ctx);

    OM* om;
    int result = getOM(RedisModule_OpenKey(ctx, argv[1], REDISMODULE_READ), &om);

    if (result == GET_OM_WRONG_TYPE) {
        RedisModule_ReplyWithError(ctx, REDISMODULE_ERRORMSG_WRONGTYPE);
        return REDISMODULE_ERR;
    } else if (result == GET_OM_NOT_EXIST) {
        RedisModule_ReplyWithArray(ctx, 0);
        return REDISMODULE_ERR;
    }

    size_t x_len;
    const char* x = RedisModule_StringPtrLen(argv[2], &x_len);
    LNode* curr = get_node_read_mod(om->nodes, x);

    long long ll;
    long arr_len = 0;
    int long_result = RedisModule_StringToLongLong(argv[3], &ll);

    if (curr == NULL) {
        RedisModule_ReplyWithError(ctx, OM_ELEMENT_NOT_FOUND);
        return REDISMODULE_ERR;
    } else if (long_result == REDISMODULE_ERR || ll <= 0) {
        RedisModule_ReplyWithError(ctx, OM_INVALID_COUNT);
        return REDISMODULE_ERR;
    }

    RedisModule_ReplyWithArray(ctx, REDISMODULE_POSTPONED_ARRAY_LEN);

    for (size_t i = 0; i < ll; ++i) {
        curr = curr->next;
        if (curr == om->lsentinel) {
            break;
        }
        ++arr_len;
        RedisModule_ReplyWithStringBuffer(ctx, curr->key, curr->keylen);
    }

    RedisModule_ReplySetArrayLength(ctx, arr_len);
    return REDISMODULE_OK;
}

static int OMPrev_RedisCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {

    if (argc != 4) {
        RedisModule_WrongArity(ctx);
        return REDISMODULE_ERR;
    }

    RedisModule_AutoMemory(ctx);

    OM* om;
    int result = getOM(RedisModule_OpenKey(ctx, argv[1], REDISMODULE_READ), &om);

    if (result == GET_OM_WRONG_TYPE) {
        RedisModule_ReplyWithError(ctx, REDISMODULE_ERRORMSG_WRONGTYPE);
        return REDISMODULE_ERR;
    } else if (result == GET_OM_NOT_EXIST) {
        RedisModule_ReplyWithArray(ctx, 0);
        return REDISMODULE_ERR;
    }

    size_t x_len;
    const char* x = RedisModule_StringPtrLen(argv[2], &x_len);
    LNode* curr = get_node_read_mod(om->nodes, x);

    long long ll;
    long arr_len = 0;
    int long_result = RedisModule_StringToLongLong(argv[3], &ll);

    if (curr == NULL) {
        RedisModule_ReplyWithError(ctx, OM_ELEMENT_NOT_FOUND);
        return REDISMODULE_ERR;
    } else if (long_result == REDISMODULE_ERR || ll <= 0) {
        RedisModule_ReplyWithError(ctx, OM_INVALID_COUNT);
        return REDISMODULE_ERR;
    }

    RedisModule_ReplyWithArray(ctx, REDISMODULE_POSTPONED_ARRAY_LEN);

    for (size_t i = 0; i < ll; ++i) {
        curr = curr->prev;
        if (curr == om->lsentinel) {
            break;
        }
        ++arr_len;
        RedisModule_ReplyWithStringBuffer(ctx, curr->key, curr->keylen);
    }

    RedisModule_ReplySetArrayLength(ctx, arr_len);
    return REDISMODULE_OK;
}

static int OMFirst_RedisCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {

    if (argc != 3) {
        RedisModule_WrongArity(ctx);
        return REDISMODULE_ERR;
    }

    RedisModule_AutoMemory(ctx);

    OM* om;
    int result = getOM(RedisModule_OpenKey(ctx, argv[1], REDISMODULE_READ), &om);

    if (result == GET_OM_WRONG_TYPE) {
        RedisModule_ReplyWithError(ctx, REDISMODULE_ERRORMSG_WRONGTYPE);
        return REDISMODULE_ERR;
    } else if (result == GET_OM_NOT_EXIST) {
        RedisModule_ReplyWithArray(ctx, 0);
        return REDISMODULE_ERR;
    }

    LNode* curr = om->lsentinel->next;

    long long ll;
    long arr_len = 0;
    int long_result = RedisModule_StringToLongLong(argv[2], &ll);

    if (long_result == REDISMODULE_ERR || ll <= 0) {
        RedisModule_ReplyWithError(ctx, OM_INVALID_COUNT);
        return REDISMODULE_ERR;
    }

    RedisModule_ReplyWithArray(ctx, REDISMODULE_POSTPONED_ARRAY_LEN);

    for (size_t i = 0; i < ll; ++i) {
        RedisModule_ReplyWithStringBuffer(ctx, curr->key, curr->keylen);
        ++arr_len;
        curr = curr->next;
        if (curr == om->lsentinel) {
            break;
        }
    }

    RedisModule_ReplySetArrayLength(ctx, arr_len);
    return REDISMODULE_OK;
}

static int OMLast_RedisCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {

    if (argc != 3) {
        RedisModule_WrongArity(ctx);
        return REDISMODULE_ERR;
    }

    RedisModule_AutoMemory(ctx);

    OM* om;
    int result = getOM(RedisModule_OpenKey(ctx, argv[1], REDISMODULE_READ), &om);

    if (result == GET_OM_WRONG_TYPE) {
        RedisModule_ReplyWithError(ctx, REDISMODULE_ERRORMSG_WRONGTYPE);
        return REDISMODULE_ERR;
    } else if (result == GET_OM_NOT_EXIST) {
        RedisModule_ReplyWithArray(ctx, 0);
        return REDISMODULE_ERR;
    }

    LNode* curr = om->lsentinel->prev;

    long long ll;
    long arr_len = 0;
    int long_result = RedisModule_StringToLongLong(argv[2], &ll);

    if (long_result == REDISMODULE_ERR || ll <= 0) {
        RedisModule_ReplyWithError(ctx, OM_INVALID_COUNT);
        return REDISMODULE_ERR;
    }

    RedisModule_ReplyWithArray(ctx, REDISMODULE_POSTPONED_ARRAY_LEN);

    for (size_t i = 0; i < ll; ++i) {
        RedisModule_ReplyWithStringBuffer(ctx, curr->key, curr->keylen);
        ++arr_len;
        curr = curr->prev;
        if (curr == om->lsentinel) {
            break;
        }
    }

    RedisModule_ReplySetArrayLength(ctx, arr_len);
    return REDISMODULE_OK;
}

static int OMList_RedisCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {

    if (argc != 2) {
        RedisModule_WrongArity(ctx);
        return REDISMODULE_ERR;
    }

    RedisModule_AutoMemory(ctx);

    OM* om;
    int result = getOM(RedisModule_OpenKey(ctx, argv[1], REDISMODULE_READ), &om);

    if (result == GET_OM_WRONG_TYPE) {
        RedisModule_ReplyWithError(ctx, REDISMODULE_ERRORMSG_WRONGTYPE);
        return REDISMODULE_ERR;
    } else if (result == GET_OM_NOT_EXIST) {
        RedisModule_ReplyWithArray(ctx, 0);
        return REDISMODULE_ERR;
    }

    size_t size = table_size(om->nodes);
    LNode* curr = om->lsentinel->next;
    RedisModule_ReplyWithArray(ctx, size);

    for (size_t i = 0; i < size; ++i) {
        RedisModule_ReplyWithStringBuffer(ctx, curr->key, curr->keylen);
        curr = curr->next;
    }

    return REDISMODULE_OK;
}

static int OMCard_RedisCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {

    if (argc != 2) {
        RedisModule_WrongArity(ctx);
        return REDISMODULE_ERR;
    }

    RedisModule_AutoMemory(ctx);

    OM* om;
    int result = getOM(RedisModule_OpenKey(ctx, argv[1], REDISMODULE_READ), &om);

    if (result == GET_OM_WRONG_TYPE) {
        RedisModule_ReplyWithError(ctx, REDISMODULE_ERRORMSG_WRONGTYPE);
        return REDISMODULE_ERR;
    } else if (result == GET_OM_NOT_EXIST) {
        RedisModule_ReplyWithLongLong(ctx, 0);
        return REDISMODULE_ERR;
    }

    RedisModule_ReplyWithLongLong(ctx, table_size(om->nodes));
    return REDISMODULE_OK;
}

int RedisModule_OnLoad(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
    if (RedisModule_Init(ctx, "OrderedSet", 1, REDISMODULE_APIVER_1) != REDISMODULE_OK) {
        return REDISMODULE_ERR;
    }

    if (RedisModule_CreateCommand(ctx, "OM.ADDAFTER", OMAddAfter_RedisCommand, "write deny-oom fast", 1, 1, 1) != REDISMODULE_OK) {
        return REDISMODULE_ERR;
    }
    if (RedisModule_CreateCommand(ctx, "OM.ADDBEFORE", OMAddBefore_RedisCommand, "write deny-oom fast", 1, 1, 1) != REDISMODULE_OK) {
        return REDISMODULE_ERR;
    }
    if (RedisModule_CreateCommand(ctx, "OM.REMOVE", OMRemove_RedisCommand, "write deny-oom fast", 1, 1, 1) != REDISMODULE_OK) {
        return REDISMODULE_ERR;
    }
    if (RedisModule_CreateCommand(ctx, "OM.COMPARE", OMCompare_RedisCommand, "readonly fast", 1, 1, 1) != REDISMODULE_OK) {
        return REDISMODULE_ERR;
    }
    if (RedisModule_CreateCommand(ctx, "OM.ADDFIRST", OMAddFirst_RedisCommand, "write deny-oom fast", 1, 1, 1) != REDISMODULE_OK) {
        return REDISMODULE_ERR;
    }
    if (RedisModule_CreateCommand(ctx, "OM.ADDLAST", OMAddLast_RedisCommand, "write deny-oom fast", 1, 1, 1) != REDISMODULE_OK) {
        return REDISMODULE_ERR;
    }
    if (RedisModule_CreateCommand(ctx, "OM.NEXT", OMNext_RedisCommand, "readonly", 1, 1, 1) != REDISMODULE_OK) {
        return REDISMODULE_ERR;
    }
    if (RedisModule_CreateCommand(ctx, "OM.PREV", OMPrev_RedisCommand, "readonly", 1, 1, 1) != REDISMODULE_OK) {
        return REDISMODULE_ERR;
    }
    if (RedisModule_CreateCommand(ctx, "OM.FIRST", OMFirst_RedisCommand, "readonly", 1, 1, 1) != REDISMODULE_OK) {
        return REDISMODULE_ERR;
    }
    if (RedisModule_CreateCommand(ctx, "OM.LAST", OMLast_RedisCommand, "readonly", 1, 1, 1) != REDISMODULE_OK) {
        return REDISMODULE_ERR;
    }
    if (RedisModule_CreateCommand(ctx, "OM.LIST", OMList_RedisCommand, "readonly", 1, 1, 1) != REDISMODULE_OK) {
        return REDISMODULE_ERR;
    }
    if (RedisModule_CreateCommand(ctx, "OM.CARD", OMCard_RedisCommand, "readonly fast", 1, 1, 1) != REDISMODULE_OK) {
        return REDISMODULE_ERR;
    }

    static RedisModuleTypeMethods type_methods = {.version = REDISMODULE_TYPE_METHOD_VERSION,
                                               .rdb_load = OMRdbLoad,
                                               .rdb_save = OMRdbSave,
                                               .aof_rewrite = OMAofRewrite,
                                               .free = OMFree,
                                               .mem_usage = OMMemUsage};
    OMType = RedisModule_CreateDataType(ctx, "OM-iliash", OM_ENCODING_VERSION, &type_methods);
    return OMType == NULL ? REDISMODULE_ERR : REDISMODULE_OK;
}