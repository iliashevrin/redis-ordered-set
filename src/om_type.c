#include "om_type.h"

RedisOS* OSInit() {
    RedisOS* redis_os = RedisModule_Alloc(sizeof(RedisOS));
    redis_os->hash = NULL;
    OrderedSet* os = RedisModule_Alloc(sizeof(OrderedSet));
    os->lsentinel = RedisModule_Alloc(sizeof(LNode));
    os->lsentinel->label = 0;
    os->lsentinel->next = os->lsentinel;
    os->lsentinel->prev = os->lsentinel;
    os->lsentinel->upper = NULL;
    os->lsentinel->key = NULL;
    os->lsentinel->keylen = 0;
    os->usentinel = RedisModule_Alloc(sizeof(UNode));
    os->usentinel->label = 0;
    os->usentinel->next = os->usentinel;
    os->usentinel->prev = os->usentinel;
    os->usentinel->lower = NULL;
    os->usentinel->lsize = 0;
    os->height = 1;
    os->threshold = INITIAL_THRESHOLD;
    redis_os->oset = os;
    return redis_os;
}

void* OSRdbLoad(RedisModuleIO *io, int encver) {
    if (encver > OS_ENCODING_VERSION) {
        return NULL;
    }

    RedisOS* redis_os = OSInit();
    OrderedSet* os = redis_os->oset;
    size_t size = RedisModule_LoadUnsigned(io);
    os->height = RedisModule_LoadUnsigned(io);
    os->threshold = RedisModule_LoadDouble(io);
    UNode* curr_upper;
    UNode* prev_upper = os->usentinel;
    LNode* curr_lower;
    LNode* prev_lower = os->lsentinel;

    size_t i = 0;
    while (i < size) {
        curr_upper = RedisModule_Alloc(sizeof(UNode));
        curr_upper->label = RedisModule_LoadUnsigned(io);
        curr_upper->lsize = RedisModule_LoadUnsigned(io);
        curr_upper->prev = prev_upper;
        prev_upper->next = curr_upper;

        size_t j = 0;
        while (j < curr_upper->lsize) {
            curr_lower = RedisModule_Alloc(sizeof(LNode));
            curr_lower->label = RedisModule_LoadUnsigned(io);
            size_t keylen;
            curr_lower->key = RedisModule_LoadStringBuffer(io, &keylen);
            curr_lower->keylen = keylen;
            curr_lower->prev = prev_lower;
            curr_lower->upper = curr_upper;
            prev_lower->next = curr_lower;

            if (j == 0) {
                curr_upper->lower = curr_lower;
            }
            ++j;
            ++i;
            prev_lower = curr_lower;
            Node* nh = RedisModule_Alloc(sizeof(Node));
            nh->key = curr_lower->key;
            nh->lnode = curr_lower;
            HASH_ADD_KEYPTR(hh, redis_os->hash, curr_lower->key, curr_lower->keylen, nh);
        }
        prev_upper = curr_upper;
    }
    prev_upper->next = os->usentinel;
    os->usentinel->prev = prev_upper;
    prev_lower->next = os->lsentinel;
    os->lsentinel->prev = prev_lower;
    return redis_os;
}

void OSRdbSave(RedisModuleIO *io, void *obj) {

    RedisOS* redis_os = obj;
    OrderedSet* os = redis_os->oset;
    RedisModule_SaveUnsigned(io, HASH_COUNT(redis_os->hash));
    RedisModule_SaveUnsigned(io, os->height);
    RedisModule_SaveDouble(io, os->threshold);

    UNode* curr_upper = os->usentinel->next;
    LNode* curr_lower;

    while (curr_upper != os->usentinel) {
        RedisModule_SaveUnsigned(io, curr_upper->label);
        RedisModule_SaveUnsigned(io, curr_upper->lsize);
        curr_lower = curr_upper->lower;

        while (curr_lower->upper == curr_upper) {
            RedisModule_SaveUnsigned(io, curr_lower->label);
            RedisModule_SaveStringBuffer(io, curr_lower->key, curr_lower->keylen);
            curr_lower = curr_lower->next;
        }

        curr_upper = curr_upper->next;
    }
}

void OSAofRewrite(RedisModuleIO *aof, RedisModuleString *key, void *value) {

    RedisOS* redis_os = value;
    LNode* curr_lower = redis_os->oset->lsentinel->next;
    while (curr_lower != redis_os->oset->lsentinel) {
        RedisModule_EmitAOF(aof, "OS.ADDLAST", "sb", key, curr_lower->key, curr_lower->keylen);
    }
}

void OSFree(void *value) {

    RedisOS* redis_os = value;
    OrderedSet* os = redis_os->oset;
    Node* curr; 
    Node* tmp;

    HASH_ITER(hh, redis_os->hash, curr, tmp) {
        HASH_DEL(redis_os->hash, curr);
        RedisModule_Free(curr);
    }

    LNode* curr_lower = os->lsentinel->next;
    UNode* curr_upper = os->usentinel->next;
    
    while (curr_lower != os->lsentinel) {
        curr_lower->next->prev = os->lsentinel;
        os->lsentinel->next = curr_lower->next;
        RedisModule_Free(curr_lower->key);
        RedisModule_Free(curr_lower);
        curr_lower = os->lsentinel->next;
    }

    while (curr_upper != os->usentinel) {
        curr_upper->next->prev = os->usentinel;
        os->usentinel->next = curr_upper->next;
        RedisModule_Free(curr_upper);
        curr_upper = os->usentinel->next;
    }

    RedisModule_Free(os->lsentinel);
    RedisModule_Free(os->usentinel);
    RedisModule_Free(os);
    RedisModule_Free(redis_os);
}

size_t OSMemUsage(const void *value) {

    const RedisOS* redis_os = value;
    const size_t lower_size = sizeof(LNode) + sizeof(Node);
    const size_t upper_size = sizeof(UNode);
    size_t size = sizeof(LNode); // Lower sentinel
    size += upper_size; // Upper sentinel
    UNode* curr_upper = redis_os->oset->usentinel->next;
    LNode* curr_lower;

    while (curr_upper != redis_os->oset->usentinel) {
        size += upper_size;
        curr_lower = curr_upper->lower;

        while (curr_lower->upper == curr_upper) {
            size += curr_lower->keylen;
            size += lower_size;
            curr_lower = curr_lower->next;
        }

        curr_upper = curr_upper->next;
    }

    return size;
}