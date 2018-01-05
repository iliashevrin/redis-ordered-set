#include "om_internal.h"

static inline uint64_t step(const size_t height, const long double threshold) {
    if (height < UNSIGNED_SIZE) {
        return (uint64_t) (((uint64_t)1 << (height-1)) / threshold);
    } else {
        return (((uint64_t)1 << (UNSIGNED_SIZE-1)) / threshold) * ((uint64_t)1 << (height-UNSIGNED_SIZE));
    }
}

static void urelabel(UNode* left, UNode* right, size_t* height, double* threshold) {

    size_t i = 1;
    size_t items = 2;
    unsigned __int128 tlabel = left->label;
    long double items_threshold = 1;
    double tfactor = 2 / *threshold;

    do {
        if ((tlabel & 1) != 0) { // Check left subrange
            tlabel = tlabel >> 1;
            while (left->prev->lsize != 0 && (left->prev->label >> i) >= tlabel) {
                left = left->prev;
                ++items;
            }
        } else { // Check right subrange
            tlabel = tlabel >> 1;
            while (right->next->lsize != 0 && (right->next->label >> i) <= tlabel) {
                right = right->next;
                ++items;
            }
        }

        items_threshold *= tfactor;
        ++i;

    } while (items > items_threshold);

    // Update height and threshold if needed
    if (i >= *height) {
        *height = i;
        *threshold = 2 / pow(items, 1./i);
    }

    // The relabeling
    uint64_t lstep = step(i, items_threshold);
    tlabel = tlabel << (i-1);
    while (left != right->next) {
        left->label = tlabel;
        tlabel += lstep;
        left = left->next;
  }
}

static inline UNode* uadd_initial(UNode* usentinel) { 
    UNode* new_node = RedisModule_Alloc(sizeof(UNode));
    new_node->next = usentinel;
    new_node->prev = usentinel;
    new_node->lsize = 0;
    new_node->label = 0;
    usentinel->next = new_node;
    usentinel->prev = new_node;
    return new_node;
}

static UNode* uadd(UNode* this, LNode* lower, size_t* height, double* threshold) {

    // Adding new upper node
    UNode* new_node = RedisModule_Alloc(sizeof(UNode));
    new_node->lower = lower;
    new_node->prev = this;
    new_node->next = this->next;
    new_node->lsize = 0;
    this->next->prev = new_node;
    this->next = new_node;

    // This is the last node, we simply increment by one
    if (new_node->next->lsize == 0) {
    	new_node->label = this->label + 1;
    } else {
        if (this->label + 1 == new_node->next->label) {
           urelabel(this, new_node, height, threshold);
        } else {
            new_node->label = (this->label + new_node->next->label) >> 1;
        }
    }

    return new_node;
}

static inline LNode* lpopulate(LNode* head, UNode* upper, const size_t list_size, const size_t size) {

    uint64_t new_label = 0; 
    size_t i = 0; 

    while (++i <= list_size) {
        head->label = new_label;
        head->upper = upper;
        new_label += size;
        head = head->next;
    }
    upper->lsize = list_size;
    return head;
}

static void lsplit(LNode* this, const size_t size, size_t* height, double* threshold) {

    size_t initial_size = this->upper->lsize;
    size_t log_size = LOG2(size);
    size_t dbl_log_size = log_size << 1;
    // In case no split is needed and only label spacing
    if (dbl_log_size > initial_size) {
        lpopulate(this->upper->lower, this->upper, initial_size, size);
    } else {
        UNode* curr_upper = this->upper;
        size_t all = log_size;
        LNode* head = this->upper->lower;
        head = lpopulate(head, curr_upper, log_size, size);

        // Split list into lists of size log N
        while (all + dbl_log_size <= initial_size) {
            curr_upper = uadd(curr_upper, head, height, threshold);
            all += log_size;
            head = lpopulate(head, curr_upper, log_size, size);
        }

        // Create the last list with remaining nodes
        curr_upper = uadd(curr_upper, head, height, threshold);
        lpopulate(head, curr_upper, initial_size - all, size);
    }
}

static inline void linit(LNode* node, unsigned long long label, LNode* next, LNode* prev, UNode* upper) {
    node->label = label;
    node->next = next;
    node->prev = prev;
    node->upper = upper;
}

void lrelabel(LNode* this, const size_t size, size_t* height, double* threshold) {
    if (this->next->upper == this->upper) {
        if (this->label + 1 == this->next->label) {
            // In this case list size should be approx O(logN)
            lsplit(this, size, height, threshold);
        } else {
            this->label = (this->label + this->next->label) >> 1;
        }
    } else {
        // Prevent lists of O(N) length
        if (this->upper->lsize > LOG2(size)) {
            lsplit(this, size, height, threshold);
        } else {
            // This is the last node, we simply increment by N
            this->label = this->label + size;
        }
    }
}

void ladd_initial(LNode* x, LNode* lsentinel, UNode* usentinel) {
    UNode* initial_upper = uadd_initial(usentinel);
    linit(x, 0, lsentinel, lsentinel, initial_upper);
    ++(x->upper->lsize);

    x->upper->lower = x;
    lsentinel->next = x;
    lsentinel->prev = x;
}

void ladd_first(LNode* x, LNode* lsentinel) {
    LNode* first = lsentinel->next;
    linit(x, first->label, first, lsentinel, first->upper);
    ++(x->upper->lsize);

    // Swap first and new one
    x->upper->lower = x;
    first->prev = x;
    lsentinel->next = x;
}

void ladd(LNode* x, LNode* y) {
    linit(y, x->label, x->next, x, x->upper);
    ++(x->upper->lsize);

    x->next->prev = y;
    x->next = y;
}

void lremove(LNode* x) {

    --(x->upper->lsize);
    // List is done, need to delete upper label
    if (x->upper->lsize == 0) {
        x->upper->next->prev = x->upper->prev;
        x->upper->prev->next = x->upper->next;
    } else {
    	// If we're deleting first representative node in list
        if (x->upper->lower == x) {
            x->upper->lower = x->next;
        }
    }

    x->next->prev = x->prev;
    x->prev->next = x->next;
}
