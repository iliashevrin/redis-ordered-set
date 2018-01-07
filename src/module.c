#include "module.h"

static RedisModuleType *RedisOSType;

// Helper function to get LNode object from RedisModuleString for update purposes
static LNode* getNodeFromHash(Node** hash, RedisModuleString* key, size_t* ret_count) {
    size_t member_len;
    const char* member_str = RedisModule_StringPtrLen(key, &member_len);
    LNode* member = HASH_get_node(*hash, member_str);
    if (member == NULL) {
        member = HASH_create_node(hash, member_str, member_len);
        ++(*ret_count);
    } else { // If already found remove from structure
        OSET_remove(member);
    }
    return member;
}

static int getOS(RedisModuleKey* key, RedisOS** os) {

    *os = NULL;
    int type = RedisModule_KeyType(key);
    if (REDISMODULE_KEYTYPE_EMPTY == type) {
        return GET_OS_NOT_EXIST;
    } else if (RedisModule_ModuleTypeGetType(key) != RedisOSType) {
        return GET_OS_WRONG_TYPE;
    } else {
        *os = RedisModule_ModuleTypeGetValue(key);
        return GET_OS_SUCCESS;
    }
}

static int OSAddAfter_RedisCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {

    if (argc < 4) {
        RedisModule_WrongArity(ctx);
        return REDISMODULE_ERR;
    }

    RedisModule_AutoMemory(ctx);
    RedisModule_ReplicateVerbatim(ctx);

    RedisOS* os;
    int result = getOS(RedisModule_OpenKey(ctx, argv[1], REDISMODULE_READ | REDISMODULE_WRITE), &os);

    if (result == GET_OS_WRONG_TYPE) {
        RedisModule_ReplyWithError(ctx, REDISMODULE_ERRORMSG_WRONGTYPE);
        return REDISMODULE_ERR;
    } else if (result == GET_OS_NOT_EXIST) {
        RedisModule_ReplyWithNull(ctx);
        return REDISMODULE_OK;
    }

    size_t existing_len;
    const char* existing_str = RedisModule_StringPtrLen(argv[2], &existing_len);
    LNode* existing = HASH_get_node(os->hash, existing_str);
    if (existing == NULL) {
        RedisModule_ReplyWithNull(ctx);
        return REDISMODULE_OK;
    }

    LNode* member;
    size_t added = 0;

    for (size_t i = 3; i < argc; ++i) {
        // No point adding element right after itself
        if (RedisModule_StringCompare(argv[i-1], argv[i]) != 0) {
            member = getNodeFromHash(&os->hash, argv[i], &added);
            OSET_add_after(os->oset, existing, member, HASH_table_size(os->hash));
            existing = member;
        }
    }

    RedisModule_ReplyWithLongLong(ctx, added);
    return REDISMODULE_OK;
}

static int OSAddBefore_RedisCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {

    if (argc < 4) {
        RedisModule_WrongArity(ctx);
        return REDISMODULE_ERR;
    }

    RedisModule_AutoMemory(ctx);
    RedisModule_ReplicateVerbatim(ctx);

    RedisOS* os;
    int result = getOS(RedisModule_OpenKey(ctx, argv[1], REDISMODULE_READ | REDISMODULE_WRITE), &os);

    if (result == GET_OS_WRONG_TYPE) {
        RedisModule_ReplyWithError(ctx, REDISMODULE_ERRORMSG_WRONGTYPE);
        return REDISMODULE_ERR;
    } else if (result == GET_OS_NOT_EXIST) {
        RedisModule_ReplyWithNull(ctx);
        return REDISMODULE_OK;
    }

    size_t existing_len;
    const char* existing_str = RedisModule_StringPtrLen(argv[2], &existing_len);
    LNode* existing = HASH_get_node(os->hash, existing_str);
    if (existing == NULL) {
        RedisModule_ReplyWithNull(ctx);
        return REDISMODULE_OK;
    }

    size_t added = 0;
    LNode* member = getNodeFromHash(&os->hash, argv[3], &added);
    OSET_add_before(os->oset, existing, member, HASH_table_size(os->hash));
    existing = member;

    for (size_t i = 4; i < argc; ++i) {
        // No point adding element right after itself
        if (RedisModule_StringCompare(argv[i-1], argv[i]) != 0) {
            member = getNodeFromHash(&os->hash, argv[i], &added);
            OSET_add_after(os->oset, existing, member, HASH_table_size(os->hash));
            existing = member;
        }
    }

    RedisModule_ReplyWithLongLong(ctx, added);
    return REDISMODULE_OK;
    
}

static int OSRemove_RedisCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {

    if (argc < 3) {
        RedisModule_WrongArity(ctx);
        return REDISMODULE_ERR;
    }

    RedisModule_AutoMemory(ctx);
    RedisModule_ReplicateVerbatim(ctx);

    RedisOS* os;
    RedisModuleKey* key = RedisModule_OpenKey(ctx, argv[1], REDISMODULE_READ | REDISMODULE_WRITE);
    int result = getOS(key, &os);

    if (result == GET_OS_WRONG_TYPE) {
        RedisModule_ReplyWithError(ctx, REDISMODULE_ERRORMSG_WRONGTYPE);
        return REDISMODULE_ERR;
    } else if (result == GET_OS_NOT_EXIST) {
        RedisModule_ReplyWithNull(ctx);
        return REDISMODULE_OK;
    }

    size_t removed = 0;

    for (size_t i = 2; i < argc; ++i) {
        size_t existing_len;
        const char* existing_str = RedisModule_StringPtrLen(argv[i], &existing_len);
        LNode* existing = HASH_get_node(os->hash, existing_str);
        if (existing != NULL) {
            HASH_remove_node(&os->hash, existing_str);
            OSET_remove(existing);
            ++removed;

            // Removed last node so we delete structure from redis keyset
            if (!HASH_table_size(os->hash)) {
                RedisModule_DeleteKey(key);
                break;
            }
        }
    }

    RedisModule_ReplyWithLongLong(ctx, removed);
    return REDISMODULE_OK;
}

static int OSAddHead_RedisCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {

    if (argc < 3) {
        RedisModule_WrongArity(ctx);
        return REDISMODULE_ERR;
    }

    RedisModule_AutoMemory(ctx);
    RedisModule_ReplicateVerbatim(ctx);

    RedisOS* os;
    RedisModuleKey* key = RedisModule_OpenKey(ctx, argv[1], REDISMODULE_READ | REDISMODULE_WRITE);
    int result = getOS(key, &os);

    if (result == GET_OS_WRONG_TYPE) {
        RedisModule_ReplyWithError(ctx, REDISMODULE_ERRORMSG_WRONGTYPE);
        return REDISMODULE_ERR;
    } else if (result == GET_OS_NOT_EXIST) {
        os = OSInit();
        RedisModule_ModuleTypeSetValue(key, RedisOSType, os);
    }

    size_t added = 0;
    LNode* member = getNodeFromHash(&os->hash, argv[2], &added);
    OSET_add_head(os->oset, member, HASH_table_size(os->hash));
    LNode* existing = member;

    for (size_t i = 3; i < argc; ++i) {
        // No point adding element right after itself
        if (RedisModule_StringCompare(argv[i-1], argv[i]) != 0) {
            member = getNodeFromHash(&os->hash, argv[i], &added);
            OSET_add_after(os->oset, existing, member, HASH_table_size(os->hash));
            existing = member;
        }
    }

    RedisModule_ReplyWithLongLong(ctx, added);
    return REDISMODULE_OK;
}

static int OSAddTail_RedisCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {

    if (argc < 3) {
        RedisModule_WrongArity(ctx);
        return REDISMODULE_ERR;
    }

    RedisModule_AutoMemory(ctx);
    RedisModule_ReplicateVerbatim(ctx);

    RedisOS* os;
    RedisModuleKey* key = RedisModule_OpenKey(ctx, argv[1], REDISMODULE_READ | REDISMODULE_WRITE);
    int result = getOS(key, &os);

    if (result == GET_OS_WRONG_TYPE) {
        RedisModule_ReplyWithError(ctx, REDISMODULE_ERRORMSG_WRONGTYPE);
        return REDISMODULE_ERR;
    } else if (result == GET_OS_NOT_EXIST) {
        os = OSInit();
        RedisModule_ModuleTypeSetValue(key, RedisOSType, os);
    }

    size_t added = 0;
    LNode* member = getNodeFromHash(&os->hash, argv[2], &added);
    OSET_add_tail(os->oset, member, HASH_table_size(os->hash));
    LNode* existing = member;

    for (size_t i = 3; i < argc; ++i) {
        // No point adding element right after itself
        if (RedisModule_StringCompare(argv[i-1], argv[i]) != 0) {
            member = getNodeFromHash(&os->hash, argv[i], &added);
            OSET_add_after(os->oset, existing, member, HASH_table_size(os->hash));
            existing = member;
        }
    }

    RedisModule_ReplyWithLongLong(ctx, added);
    return REDISMODULE_OK;
}

static int OSCompare_RedisCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {

    if (argc != 4) {
        RedisModule_WrongArity(ctx);
        return REDISMODULE_ERR;
    }

    RedisModule_AutoMemory(ctx);

    RedisOS* os;
    int result = getOS(RedisModule_OpenKey(ctx, argv[1], REDISMODULE_READ), &os);

    if (result == GET_OS_WRONG_TYPE) {
        RedisModule_ReplyWithError(ctx, REDISMODULE_ERRORMSG_WRONGTYPE);
        return REDISMODULE_ERR;
    } else if (result == GET_OS_NOT_EXIST) {
        RedisModule_ReplyWithNull(ctx);
        return REDISMODULE_OK;
    }

    size_t x_len;
    const char* x_str = RedisModule_StringPtrLen(argv[2], &x_len);
    size_t y_len;
    const char* y_str = RedisModule_StringPtrLen(argv[3], &y_len);
    LNode* x = HASH_get_node(os->hash, x_str);
    LNode* y = HASH_get_node(os->hash, y_str);

    if (x == NULL || y == NULL) {
        RedisModule_ReplyWithNull(ctx);
        return REDISMODULE_OK;
    }

    RedisModule_ReplyWithLongLong(ctx, OSET_compare(x, y));
    return REDISMODULE_OK;
}

static int OSNext_RedisCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {

    if (argc != 4) {
        RedisModule_WrongArity(ctx);
        return REDISMODULE_ERR;
    }

    RedisModule_AutoMemory(ctx);

    RedisOS* os;
    int result = getOS(RedisModule_OpenKey(ctx, argv[1], REDISMODULE_READ), &os);

    if (result == GET_OS_WRONG_TYPE) {
        RedisModule_ReplyWithError(ctx, REDISMODULE_ERRORMSG_WRONGTYPE);
        return REDISMODULE_ERR;
    } else if (result == GET_OS_NOT_EXIST) {
        RedisModule_ReplyWithArray(ctx, 0);
        return REDISMODULE_OK;
    }

    size_t curr_len;
    const char* curr_str = RedisModule_StringPtrLen(argv[2], &curr_len);
    LNode* curr = HASH_get_node(os->hash, curr_str);

    long long ll;
    long arr_len = 0;
    int long_result = RedisModule_StringToLongLong(argv[3], &ll);

    if (curr == NULL) {
        RedisModule_ReplyWithArray(ctx, 0);
        return REDISMODULE_OK;
    } else if (long_result == REDISMODULE_ERR || ll < 0) {
        RedisModule_ReplyWithError(ctx, OS_INVALID_COUNT);
        return REDISMODULE_ERR;
    }

    if (ll == 0) {
        ll = HASH_table_size(os->hash);
    }

    RedisModule_ReplyWithArray(ctx, REDISMODULE_POSTPONED_ARRAY_LEN);

    for (size_t i = 0; i < ll; ++i) {
        curr = curr->next;
        if (curr == LSENTINEL) {
            break;
        }
        ++arr_len;
        RedisModule_ReplyWithStringBuffer(ctx, curr->key, curr->keylen);
    }

    RedisModule_ReplySetArrayLength(ctx, arr_len);
    return REDISMODULE_OK;
}

static int OSPrev_RedisCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {

    if (argc != 4) {
        RedisModule_WrongArity(ctx);
        return REDISMODULE_ERR;
    }

    RedisModule_AutoMemory(ctx);

    RedisOS* os;
    int result = getOS(RedisModule_OpenKey(ctx, argv[1], REDISMODULE_READ), &os);

    if (result == GET_OS_WRONG_TYPE) {
        RedisModule_ReplyWithError(ctx, REDISMODULE_ERRORMSG_WRONGTYPE);
        return REDISMODULE_ERR;
    } else if (result == GET_OS_NOT_EXIST) {
        RedisModule_ReplyWithArray(ctx, 0);
        return REDISMODULE_OK;
    }

    size_t curr_len;
    const char* curr_str = RedisModule_StringPtrLen(argv[2], &curr_len);
    LNode* curr = HASH_get_node(os->hash, curr_str);

    long long ll;
    long arr_len = 0;
    int long_result = RedisModule_StringToLongLong(argv[3], &ll);

    if (curr == NULL) {
        RedisModule_ReplyWithArray(ctx, 0);
        return REDISMODULE_OK;
    } else if (long_result == REDISMODULE_ERR || ll < 0) {
        RedisModule_ReplyWithError(ctx, OS_INVALID_COUNT);
        return REDISMODULE_ERR;
    }

    if (ll == 0) {
        ll = HASH_table_size(os->hash);
    }

    RedisModule_ReplyWithArray(ctx, REDISMODULE_POSTPONED_ARRAY_LEN);

    for (size_t i = 0; i < ll; ++i) {
        curr = curr->prev;
        if (curr == LSENTINEL) {
            break;
        }
        ++arr_len;
        RedisModule_ReplyWithStringBuffer(ctx, curr->key, curr->keylen);
    }

    RedisModule_ReplySetArrayLength(ctx, arr_len);
    return REDISMODULE_OK;
}

static int OSHead_RedisCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {

    if (argc != 3) {
        RedisModule_WrongArity(ctx);
        return REDISMODULE_ERR;
    }

    RedisModule_AutoMemory(ctx);

    RedisOS* os;
    int result = getOS(RedisModule_OpenKey(ctx, argv[1], REDISMODULE_READ), &os);

    if (result == GET_OS_WRONG_TYPE) {
        RedisModule_ReplyWithError(ctx, REDISMODULE_ERRORMSG_WRONGTYPE);
        return REDISMODULE_ERR;
    } else if (result == GET_OS_NOT_EXIST) {
        RedisModule_ReplyWithArray(ctx, 0);
        return REDISMODULE_OK;
    }

    LNode* curr = LSENTINEL->next;

    long long ll;
    long arr_len = 0;
    int long_result = RedisModule_StringToLongLong(argv[2], &ll);

    if (long_result == REDISMODULE_ERR || ll < 0) {
        RedisModule_ReplyWithError(ctx, OS_INVALID_COUNT);
        return REDISMODULE_ERR;
    }

    if (ll == 0) {
        ll = HASH_table_size(os->hash);
    }

    RedisModule_ReplyWithArray(ctx, REDISMODULE_POSTPONED_ARRAY_LEN);

    for (size_t i = 0; i < ll; ++i) {
        RedisModule_ReplyWithStringBuffer(ctx, curr->key, curr->keylen);
        ++arr_len;
        curr = curr->next;
        if (curr == LSENTINEL) {
            break;
        }
    }

    RedisModule_ReplySetArrayLength(ctx, arr_len);
    return REDISMODULE_OK;
}

static int OSTail_RedisCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {

    if (argc != 3) {
        RedisModule_WrongArity(ctx);
        return REDISMODULE_ERR;
    }

    RedisModule_AutoMemory(ctx);

    RedisOS* os;
    int result = getOS(RedisModule_OpenKey(ctx, argv[1], REDISMODULE_READ), &os);

    if (result == GET_OS_WRONG_TYPE) {
        RedisModule_ReplyWithError(ctx, REDISMODULE_ERRORMSG_WRONGTYPE);
        return REDISMODULE_ERR;
    } else if (result == GET_OS_NOT_EXIST) {
        RedisModule_ReplyWithArray(ctx, 0);
        return REDISMODULE_OK;
    }

    LNode* curr = LSENTINEL->prev;

    long long ll;
    long arr_len = 0;
    int long_result = RedisModule_StringToLongLong(argv[2], &ll);

    if (long_result == REDISMODULE_ERR || ll < 0) {
        RedisModule_ReplyWithError(ctx, OS_INVALID_COUNT);
        return REDISMODULE_ERR;
    }

    if (ll == 0) {
        ll = HASH_table_size(os->hash);
    }

    RedisModule_ReplyWithArray(ctx, REDISMODULE_POSTPONED_ARRAY_LEN);

    for (size_t i = 0; i < ll; ++i) {
        RedisModule_ReplyWithStringBuffer(ctx, curr->key, curr->keylen);
        ++arr_len;
        curr = curr->prev;
        if (curr == LSENTINEL) {
            break;
        }
    }

    RedisModule_ReplySetArrayLength(ctx, arr_len);
    return REDISMODULE_OK;
}

static int OSMembers_RedisCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {

    if (argc != 2) {
        RedisModule_WrongArity(ctx);
        return REDISMODULE_ERR;
    }

    RedisModule_AutoMemory(ctx);

    RedisOS* os;
    int result = getOS(RedisModule_OpenKey(ctx, argv[1], REDISMODULE_READ), &os);

    if (result == GET_OS_WRONG_TYPE) {
        RedisModule_ReplyWithError(ctx, REDISMODULE_ERRORMSG_WRONGTYPE);
        return REDISMODULE_ERR;
    } else if (result == GET_OS_NOT_EXIST) {
        RedisModule_ReplyWithArray(ctx, 0);
        return REDISMODULE_OK;
    }

    size_t size = HASH_table_size(os->hash);
    LNode* curr = LSENTINEL->next;
    RedisModule_ReplyWithArray(ctx, size);

    for (size_t i = 0; i < size; ++i) {
        RedisModule_ReplyWithStringBuffer(ctx, curr->key, curr->keylen);
        curr = curr->next;
    }

    return REDISMODULE_OK;
}

static int OSCard_RedisCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {

    if (argc != 2) {
        RedisModule_WrongArity(ctx);
        return REDISMODULE_ERR;
    }

    RedisModule_AutoMemory(ctx);

    RedisOS* os;
    int result = getOS(RedisModule_OpenKey(ctx, argv[1], REDISMODULE_READ), &os);

    if (result == GET_OS_WRONG_TYPE) {
        RedisModule_ReplyWithError(ctx, REDISMODULE_ERRORMSG_WRONGTYPE);
        return REDISMODULE_ERR;
    } else if (result == GET_OS_NOT_EXIST) {
        RedisModule_ReplyWithLongLong(ctx, 0);
        return REDISMODULE_OK;
    }

    RedisModule_ReplyWithLongLong(ctx, HASH_table_size(os->hash));
    return REDISMODULE_OK;
}

int RedisModule_OnLoad(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
    if (RedisModule_Init(ctx, "OrderedSet", 1, REDISMODULE_APIVER_1) != REDISMODULE_OK) {
        return REDISMODULE_ERR;
    }

    if (RedisModule_CreateCommand(ctx, "OS.ADDAFTER", OSAddAfter_RedisCommand, "write deny-oom fast", 1, 1, 1) != REDISMODULE_OK) {
        return REDISMODULE_ERR;
    }
    if (RedisModule_CreateCommand(ctx, "OS.ADDBEFORE", OSAddBefore_RedisCommand, "write deny-oom fast", 1, 1, 1) != REDISMODULE_OK) {
        return REDISMODULE_ERR;
    }
    if (RedisModule_CreateCommand(ctx, "OS.REM", OSRemove_RedisCommand, "write deny-oom fast", 1, 1, 1) != REDISMODULE_OK) {
        return REDISMODULE_ERR;
    }
    if (RedisModule_CreateCommand(ctx, "OS.COMPARE", OSCompare_RedisCommand, "readonly fast", 1, 1, 1) != REDISMODULE_OK) {
        return REDISMODULE_ERR;
    }
    if (RedisModule_CreateCommand(ctx, "OS.ADDHEAD", OSAddHead_RedisCommand, "write deny-oom fast", 1, 1, 1) != REDISMODULE_OK) {
        return REDISMODULE_ERR;
    }
    if (RedisModule_CreateCommand(ctx, "OS.ADDTAIL", OSAddTail_RedisCommand, "write deny-oom fast", 1, 1, 1) != REDISMODULE_OK) {
        return REDISMODULE_ERR;
    }
    if (RedisModule_CreateCommand(ctx, "OS.NEXT", OSNext_RedisCommand, "readonly", 1, 1, 1) != REDISMODULE_OK) {
        return REDISMODULE_ERR;
    }
    if (RedisModule_CreateCommand(ctx, "OS.PREV", OSPrev_RedisCommand, "readonly", 1, 1, 1) != REDISMODULE_OK) {
        return REDISMODULE_ERR;
    }
    if (RedisModule_CreateCommand(ctx, "OS.HEAD", OSHead_RedisCommand, "readonly", 1, 1, 1) != REDISMODULE_OK) {
        return REDISMODULE_ERR;
    }
    if (RedisModule_CreateCommand(ctx, "OS.TAIL", OSTail_RedisCommand, "readonly", 1, 1, 1) != REDISMODULE_OK) {
        return REDISMODULE_ERR;
    }
    if (RedisModule_CreateCommand(ctx, "OS.MEMBERS", OSMembers_RedisCommand, "readonly", 1, 1, 1) != REDISMODULE_OK) {
        return REDISMODULE_ERR;
    }
    if (RedisModule_CreateCommand(ctx, "OS.CARD", OSCard_RedisCommand, "readonly fast", 1, 1, 1) != REDISMODULE_OK) {
        return REDISMODULE_ERR;
    }

    static RedisModuleTypeMethods type_methods = {.version = REDISMODULE_TYPE_METHOD_VERSION,
                                               .rdb_load = OSRdbLoad,
                                               .rdb_save = OSRdbSave,
                                               .aof_rewrite = OSAofRewrite,
                                               .free = OSFree,
                                               .mem_usage = OSMemUsage};
    RedisOSType = RedisModule_CreateDataType(ctx, "OS-iliash", OS_ENCODING_VERSION, &type_methods);
    return RedisOSType == NULL ? REDISMODULE_ERR : REDISMODULE_OK;
}