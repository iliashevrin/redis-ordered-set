#include "om_hash.h"

inline LNode* get_node_write_mod(Node** nodes, const char* key, size_t keylen, om_remove_fn rem) {
	Node* nh;
	HASH_FIND_STR(*nodes, key, nh);
	if (nh != NULL) { // If already found remove from structure
		rem(nh->lnode);
		return nh->lnode;
	} else { // Else create a new node
		char* hh_key = RedisModule_Strdup(key);
		LNode* lnode = RedisModule_Alloc(sizeof(LNode));
		lnode->key = hh_key;
		lnode->keylen = keylen;
		Node* nh = RedisModule_Alloc(sizeof(Node));
		nh->key = hh_key;
		nh->lnode = lnode;
		HASH_ADD_KEYPTR(hh, *nodes, hh_key, keylen, nh);
		return lnode;
	}
}

inline LNode* get_node_read_mod(Node* nodes, const char* key) {
	Node* nh;
	HASH_FIND_STR(nodes, key, nh);
	return nh != NULL ? nh->lnode : NULL;
}

inline int remove_node(Node** nodes, const char* key, om_remove_fn rem) {
	Node* nh;
	HASH_FIND_STR(*nodes, key, nh);
	if (nh == NULL) {
		return 0;
	} else {
		rem(nh->lnode);
		HASH_DEL(*nodes, nh);
		return 1;
	}
}

inline size_t table_size(Node* nodes) {
	return HASH_COUNT(nodes);
}