#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#include "../src/om_internal.h"
#include "../src/om_type.h"
#include "../src/om_hash.h"

#define INITIAL_ARRAY 1000
#define ITERATIONS 1000000
#define STR_SIZE 20

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

	RedisOS* redis_os = OSInit();

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
		LNode* curr = HASH_create_node(&redis_os->hash, initial_array[i], STR_SIZE);
		if (i % 2) {
			OSET_add_head(redis_os->oset, curr, i+1);
		} else {
			OSET_add_tail(redis_os->oset, curr, i+1);
		}
	}
	
	// Validate labels
	LNode* curr = redis_os->oset->lsentinel->next;
	while (curr != redis_os->oset->lsentinel->prev) {
		if (OSET_compare(curr, curr->next) >= 0) {
			printf(" !! Item labeling is invalid (check overflow?): label %lu-%llu > second label %lu-%llu \n",
				curr->label, curr->upper->label, curr->next->label, curr->next->upper->label);
		}
		curr = curr->next;
	}

	size_t size_so_far = HASH_table_size(redis_os->hash);

	// Randomly choose item and add_before / add_after
	for (size_t j = 0; j < ITERATIONS; ++j) {
		int rand_index = rand() % INITIAL_ARRAY;
		LNode* new = HASH_create_node(&redis_os->hash, big_array[j], STR_SIZE);
		LNode* existing = HASH_get_node(redis_os->hash, initial_array[rand_index]);
		if (j % 2) {
			OSET_add_after(redis_os->oset, existing, new, size_so_far+j+1);
		} else {
			OSET_add_before(redis_os->oset, existing, new, size_so_far+j+1);
		}
	}

	// Validate labels
	curr = redis_os->oset->lsentinel->next;
	while (curr != redis_os->oset->lsentinel->prev) {
		if (OSET_compare(curr, curr->next) >= 0) {
			printf(" !! Item labeling is invalid (check overflow?): label %lu-%llu > second label %lu-%llu \n",
				curr->label, curr->upper->label, curr->next->label, curr->next->upper->label);
		}
		curr = curr->next;
	}

	gettimeofday(&end, NULL);
	printf(" -- Finish performance test \n");
	printf(" -- %d initial inserts \n", INITIAL_ARRAY);
	printf(" -- %d load inserts \n", ITERATIONS);
	printf(" -- All labels are valid \n");
	printf(" -- Structure size after test: %zd \n", HASH_table_size(redis_os->hash));
    secs_used = end.tv_sec - start.tv_sec;
    micros_used = (secs_used * 1000000) + end.tv_usec - start.tv_usec;
    printf(" -- %lu seconds %lu microseconds \n", secs_used, micros_used);

    OSFree(redis_os);

    // Clean after ourselves
    for (size_t i = 0; i < INITIAL_ARRAY; ++i) {
    	free(initial_array[i]);
	}

	for (size_t j = 0; j < ITERATIONS; ++j) {
		free(big_array[j]);
	}

    return 0;
}