#include "om_type.h"

OM* OMInit() {
    OM* om = RedisModule_Alloc(sizeof(OM));
    om->nodes = NULL;
    om->lsentinel = RedisModule_Alloc(sizeof(LNode));
    om->lsentinel->label = 0;
    om->lsentinel->next = om->lsentinel;
    om->lsentinel->prev = om->lsentinel;
    om->lsentinel->upper = NULL;
    om->lsentinel->key = NULL;
    om->lsentinel->keylen = 0;
    om->usentinel = RedisModule_Alloc(sizeof(UNode));
    om->usentinel->label = 0;
    om->usentinel->next = om->usentinel;
    om->usentinel->prev = om->usentinel;
    om->usentinel->lower = NULL;
    om->usentinel->lsize = 0;
    om->height = 1;
    om->threshold = INITIAL_THRESHOLD;
    return om;
}

void* OMRdbLoad(RedisModuleIO *io, int encver) {
    if (encver > OM_ENCODING_VERSION) {
        return NULL;
    }

    OM* om = OMInit();
    int size = RedisModule_LoadUnsigned(io);
    om->height = RedisModule_LoadUnsigned(io);
    om->threshold = RedisModule_LoadDouble(io);
    UNode* curr_upper;
    UNode* prev_upper = om->usentinel;
    LNode* curr_lower;
    LNode* prev_lower = om->lsentinel;

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
            HASH_ADD_KEYPTR(hh, om->nodes, curr_lower->key, curr_lower->keylen, nh);
        }
        prev_upper = curr_upper;
    }
    prev_upper->next = om->usentinel;
    om->usentinel->prev = prev_upper;
    prev_lower->next = om->lsentinel;
    om->lsentinel->prev = prev_lower;

    return om;
}

void OMRdbSave(RedisModuleIO *io, void *obj) {

    OM* om = obj;
    RedisModule_SaveUnsigned(io, HASH_COUNT(om->nodes));
    RedisModule_SaveUnsigned(io, om->height);
    RedisModule_SaveDouble(io, om->threshold);

    UNode* curr_upper = om->usentinel->next;
    LNode* curr_lower;

    while (curr_upper != om->usentinel) {
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

void OMAofRewrite(RedisModuleIO *aof, RedisModuleString *key, void *value) {

    OM* om = value;
    LNode* curr_lower = om->lsentinel->next;
    while (curr_lower != om->lsentinel) {
        RedisModule_EmitAOF(aof, "OM.PUSHLAST", "sb", key, curr_lower->key, curr_lower->keylen);
    }
}

void OMFree(void *value) {

    OM* om = value;
    Node* curr; 
    Node* tmp;

    HASH_ITER(hh, om->nodes, curr, tmp) {
        HASH_DEL(om->nodes, curr);
        RedisModule_Free(curr);
    }

    LNode* curr_lower = om->lsentinel->next;
    UNode* curr_upper = om->usentinel->next;
    
    while (curr_lower != om->lsentinel) {
        curr_lower->next->prev = om->lsentinel;
        om->lsentinel->next = curr_lower->next;
        RedisModule_Free(curr_lower->key);
        RedisModule_Free(curr_lower);
        curr_lower = om->lsentinel->next;
    }

    while (curr_upper != om->usentinel) {
        curr_upper->next->prev = om->usentinel;
        om->usentinel->next = curr_upper->next;
        RedisModule_Free(curr_upper);
        curr_upper = om->usentinel->next;
    }

    RedisModule_Free(om->lsentinel);
    RedisModule_Free(om->usentinel);
    RedisModule_Free(om);
}

size_t OMMemUsage(const void *value) {

    const OM* om = value;
    const size_t lower_size = sizeof(LNode) + sizeof(Node);
    const size_t upper_size = sizeof(UNode);
    size_t size = sizeof(LNode); // Lower sentinel
    size += upper_size; // Upper sentinel
    UNode* curr_upper = om->usentinel->next;
    LNode* curr_lower;

    while (curr_upper != om->usentinel) {
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