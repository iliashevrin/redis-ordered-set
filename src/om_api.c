#include "om_api.h"

int compare(OM* om, const char* x, const char* y) {
	LNode* x_node = get_node_read_mod(om->nodes, x);
	LNode* y_node = get_node_read_mod(om->nodes, y);

	if (x_node == NULL || y_node == NULL) {
		return ELEMENT_NOT_FOUND;
	} 

	return lcompare(x_node, y_node);
}

int push_after(OM* om, const char* x, const char* y, size_t y_len) {

	LNode* x_node = get_node_read_mod(om->nodes, x);
	if (x_node == NULL) {
		return ELEMENT_NOT_FOUND;
	}

	LNode* y_node = get_node_write_mod(&om->nodes, y, y_len, &lremove);
	lpush(x_node, y_node);
	lrelabel(y_node, table_size(om->nodes), &om->height, &om->threshold);
	return SUCCESS;
}

int push_before(OM* om, const char* x, const char* y, size_t y_len) {

	LNode* x_node = get_node_read_mod(om->nodes, x);
	if (x_node == NULL) {
		return ELEMENT_NOT_FOUND;
	}

	LNode* y_node = get_node_write_mod(&om->nodes, y, y_len, &lremove);
	if (x_node == om->lsentinel->next) { // Check if x is first
        lpush_first(y_node, om->lsentinel);
        lrelabel(y_node->next, table_size(om->nodes), &om->height, &om->threshold);
    } else {
        lpush(x_node->prev, y_node);
        lrelabel(y_node, table_size(om->nodes), &om->height, &om->threshold);
	}

    return SUCCESS;
}

int push_first(OM* om, const char* x, size_t x_len) {
	LNode* x_node = get_node_write_mod(&om->nodes, x, x_len, &lremove);
	int size = table_size(om->nodes);

	// If size is one push first and push last are the same
	if (size == 1) {
        lpush_initial(x_node, om->lsentinel, om->usentinel);
    } else {
        lpush_first(x_node, om->lsentinel);
        lrelabel(x_node->next, size, &om->height, &om->threshold);
    }

    return SUCCESS;
}

int push_last(OM* om, const char* x, size_t x_len) {
	LNode* x_node = get_node_write_mod(&om->nodes, x, x_len, &lremove);
	int size = table_size(om->nodes);

	// If size is one push first and push last are the same
	if (size == 1) {
        lpush_initial(x_node, om->lsentinel, om->usentinel);
    } else {
        lpush(om->lsentinel->prev, x_node);
        lrelabel(x_node, size, &om->height, &om->threshold);
    }

    return SUCCESS;
}

int remove_item(OM* om, const char* x) {

	if (!remove_node(&om->nodes, x, &lremove)) {
		return ELEMENT_NOT_FOUND;
	} else {
		// Empty list is also success but it signals redis layer to remove key
		return table_size(om->nodes) ? SUCCESS : LIST_EMPTY;
	}
}