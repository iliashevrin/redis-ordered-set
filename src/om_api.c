// #include "om_api.h"

// int compare(LNode* x_node, LNode* y_node) {

// 	if (x_node->upper->label > y_node->upper->label) {
// 		return 1;
// 	} else if (x_node->upper->label < y_node->upper->label) {
// 		return -1;
// 	} else {
// 		if (x_node->label > y_node->label) {
// 			return 1;
// 		} else if (x_node->label < y_node->label) {
// 			return -1;
// 		} else {
// 			return 0;
// 		}
// 	}
// }

// void add_after(OM* om, LNode* x_node, LNode* y_node) {

// 	ladd(x_node, y_node);
// 	lrelabel(y_node, table_size(om->nodes), &om->height, &om->threshold);
// }

// void add_before(OM* om, LNode* x_node, LNode* y_node) {

// 	int size = table_size(om->nodes);

// 	// Check if x is first
// 	if (x_node == om->lsentinel->next) {
// 		ladd_head(y_node, om->lsentinel);
// 		lrelabel(y_node->next, size, &om->height, &om->threshold);
// 	} else {
// 		ladd(x_node->prev, y_node);
// 		lrelabel(y_node, size, &om->height, &om->threshold);
// 	}
// }

// void add_head(OM* om, LNode* x_node) {

// 	int size = table_size(om->nodes);

// 	// If size is one add first and add last are the same
// 	if (size == 1) {
// 		ladd_initial(x_node, om->lsentinel, om->usentinel);
// 	} else {
// 		ladd_head(x_node, om->lsentinel);
// 		lrelabel(x_node->next, size, &om->height, &om->threshold);
// 	}
// }

// void add_tail(OM* om, LNode* x_node) {

// 	int size = table_size(om->nodes);

// 	// If size is one add first and add last are the same
// 	if (size == 1) {
// 		ladd_initial(x_node, om->lsentinel, om->usentinel);
// 	} else {
// 		ladd(om->lsentinel->prev, x_node);
// 		lrelabel(x_node, size, &om->height, &om->threshold);
// 	}
// }