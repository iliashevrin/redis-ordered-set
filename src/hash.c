#include "hash.h"

// Should be called only if redis_key does not exist
inline LNode* HASH_create_node(Node** hash, const char* key, const size_t keylen) {
	char* hh_key = RedisModule_Strdup(key);
	LNode* lnode = RedisModule_Alloc(sizeof(LNode));
	lnode->key = hh_key;
	lnode->keylen = keylen;
	Node* nh = RedisModule_Alloc(sizeof(Node));
	nh->key = hh_key;
	nh->lnode = lnode;
	HASH_ADD_KEYPTR(hh, *hash, hh_key, keylen, nh);
	return lnode;
}

inline LNode* HASH_get_node(Node* hash, const char* key) {
	Node* nh;
	HASH_FIND_STR(hash, key, nh);
	if (nh != NULL) {
		return nh->lnode;
	} else {
		return NULL;
	}
}

inline void HASH_remove_node(Node** hash, const char* key) {
	Node* nh;
	HASH_FIND_STR(*hash, key, nh);
	if (nh != NULL) {
		HASH_DEL(*hash, nh);
	}
}

inline size_t HASH_table_size(Node* hash) {
	return HASH_COUNT(hash);
}