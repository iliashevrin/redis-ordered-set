#ifndef __OM_NODE_H__
#define __OM_NODE_H__

typedef struct lower_node LNode;
typedef struct upper_node UNode;

struct upper_node {
	unsigned __int128 label;
	UNode* next;
	UNode* prev;
	LNode* lower;
	size_t lsize;
};

struct lower_node {
	uint64_t label;
	LNode* next;
	LNode* prev;
	UNode* upper;
	char* key;
	size_t keylen;
};

#endif