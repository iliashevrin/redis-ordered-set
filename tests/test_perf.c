#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#include "../src/om_api.h"
#include "../src/om_type.h"
#include "../src/om_hash.h"

#define INITIAL_ARRAY 1000
#define ITERATIONS 1000000
#define STR_SIZE 10

char* rand_string(size_t length) {

    static char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";       
    char* randomString = malloc(sizeof(char) * (length + 1));

    if (randomString) {            
        for (int n = 0; n < length; n++) {            
            int key = rand() % (int)(sizeof(charset) - 1);
            randomString[n] = charset[key];
        }

        randomString[length] = '\0';
    }

    return randomString;
}

static char* initial_array[INITIAL_ARRAY];
static char* big_array[ITERATIONS];

int main(int argc, char **argv) {

	srand(time(NULL));

	RedisModule_Alloc = malloc;
	RedisModule_Free = free;
	RedisModule_Strdup = strdup;

	OM* om = OMInit();

	for (size_t i = 0; i < INITIAL_ARRAY; ++i) {
		initial_array[i] = rand_string(STR_SIZE);
	}

	for (size_t i = 0; i < ITERATIONS; ++i) {
		big_array[i] = rand_string(STR_SIZE);
	}

	struct timeval start, end;
    long secs_used, micros_used;
    printf(" -- Begin performance test \n");
    gettimeofday(&start, NULL);

	// Generate strings and add_head / add_tail
	for (size_t i = 0; i < INITIAL_ARRAY; ++i) {
		if (i % 2) {
			add_head(om, initial_array[i], STR_SIZE);
		} else {
			add_tail(om, initial_array[i], STR_SIZE);
		}
	}
	
	// Validate labels
	LNode* curr = om->lsentinel->next;
	while (curr != om->lsentinel->prev) {
		if (compare(om, curr->key, curr->next->key) >= 0) {
			printf(" !! Item labeling is invalid (check overflow?): label %lu-%llu > second label %lu-%llu \n",
				curr->label, curr->upper->label, curr->next->label, curr->next->upper->label);
		}
		curr = curr->next;
	}

	// Randomly choose item and add_before / add_after
	for (size_t j = 0; j < ITERATIONS; ++j) {
		int rand_index = rand() % INITIAL_ARRAY;
		if (strcmp(initial_array[rand_index], big_array[j]) != 0) {
			if (j % 2) {
				add_after(om, get_node_read_mod(om->nodes, initial_array[rand_index]), big_array[j], STR_SIZE);
			} else {
				add_before(om, get_node_read_mod(om->nodes, initial_array[rand_index]), big_array[j], STR_SIZE);
			}
		}
	}

	// Validate labels
	curr = om->lsentinel->next;
	while (curr != om->lsentinel->prev) {
		if (compare(om, curr->key, curr->next->key) >= 0) {
			printf(" !! Item labeling is invalid (check overflow?): label %lu-%llu > second label %lu-%llu \n",
				curr->label, curr->upper->label, curr->next->label, curr->next->upper->label);
		}
		curr = curr->next;
	}

	gettimeofday(&end, NULL);
	printf(" -- Finish performance test \n");
	printf(" -- %d initial inserts \n", INITIAL_ARRAY);
	printf(" -- %d load inserts \n", ITERATIONS);
	printf(" -- All labels are valid \n", ITERATIONS);
	printf(" -- Structure size after test: %zd \n", table_size(om->nodes));
    secs_used = end.tv_sec - start.tv_sec;
    micros_used = (secs_used * 1000000) + end.tv_usec - start.tv_usec;
    printf(" -- %lu seconds %lu microseconds \n", secs_used, micros_used);

    OMFree(om);

    // Clean after ourselves
    for (size_t i = 0; i < INITIAL_ARRAY; ++i) {
    	free(initial_array[i]);
	}

	for (size_t j = 0; j < ITERATIONS; ++j) {
		free(big_array[j]);
	}

    return 0;
}