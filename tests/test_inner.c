#include <stdio.h>
#include <string.h>
#include "minunit.h"
#include "../src/oset.h"
#include "../src/os_type.h"
#include "../src/hash.h"

static RedisOS* redis_os;

void test_setup(void) {
	redis_os = OSInit();
}

void test_teardown(void) {
	OSFree(redis_os);
}

MU_TEST(test_hash) {

	mu_check(HASH_get_node(redis_os->hash, "foo") == NULL);
	mu_check(HASH_create_node(&redis_os->hash, "foo", 3) != NULL);
	mu_assert_int_eq(1, HASH_table_size(redis_os->hash));

	mu_check(HASH_get_node(redis_os->hash, "foo") != NULL);
	mu_check(HASH_create_node(&redis_os->hash, "bar!", 4) != NULL);
	mu_assert_int_eq(2, HASH_table_size(redis_os->hash));

	HASH_remove_node(&redis_os->hash, "bar!");
	mu_assert_int_eq(1, HASH_table_size(redis_os->hash));
	HASH_remove_node(&redis_os->hash, "foo");
	mu_assert_int_eq(0, HASH_table_size(redis_os->hash));

}

MU_TEST(test_add_head) {

	LNode* foo = HASH_create_node(&redis_os->hash, "foo", 3);
	OSET_add_head(redis_os->oset, foo, 1);
	mu_assert_string_eq("foo", redis_os->oset->lsentinel->next->key);

	LNode* bar = HASH_create_node(&redis_os->hash, "bar!", 4);
	OSET_add_head(redis_os->oset, bar, 2);
	mu_assert_string_eq("bar!", redis_os->oset->lsentinel->next->key);

	mu_assert_int_eq(1, OSET_compare(foo, bar));

}

MU_TEST(test_add_tail) {

	LNode* foo = HASH_create_node(&redis_os->hash, "foo", 3);
	OSET_add_tail(redis_os->oset, foo, 1);
	mu_assert_string_eq("foo", redis_os->oset->lsentinel->prev->key);

	LNode* bar = HASH_create_node(&redis_os->hash, "bar!", 4);
	OSET_add_tail(redis_os->oset, bar, 2);
	mu_assert_string_eq("bar!", redis_os->oset->lsentinel->prev->key);

	mu_assert_int_eq(-1, OSET_compare(foo, bar));

}

MU_TEST(test_add_after) {

	LNode* foo = HASH_create_node(&redis_os->hash, "foo", 3);
	OSET_add_head(redis_os->oset, foo, 1);

	LNode* bar = HASH_create_node(&redis_os->hash, "bar!", 4);
	OSET_add_after(redis_os->oset, foo, bar, 2);
	mu_assert_string_eq("bar!", redis_os->oset->lsentinel->next->next->key);

	LNode* baz = HASH_create_node(&redis_os->hash, "baz", 3);
	OSET_add_after(redis_os->oset, bar, baz, 3);
	mu_assert_string_eq("bar!", redis_os->oset->lsentinel->next->next->key);
	mu_assert_string_eq("baz", redis_os->oset->lsentinel->prev->key);

	mu_assert_int_eq(-1, OSET_compare(foo, bar));
	mu_assert_int_eq(-1, OSET_compare(bar, baz));

}

MU_TEST(test_add_before) {

	LNode* foo = HASH_create_node(&redis_os->hash, "foo", 3);
	OSET_add_tail(redis_os->oset, foo, 1);
	LNode* bar = HASH_create_node(&redis_os->hash, "bar!", 4);
	OSET_add_before(redis_os->oset, foo, bar, 2);
	mu_assert_string_eq("bar!", redis_os->oset->lsentinel->prev->prev->key);

	LNode* baz = HASH_create_node(&redis_os->hash, "baz", 3);
	OSET_add_before(redis_os->oset, bar, baz, 3);
	mu_assert_string_eq("bar!", redis_os->oset->lsentinel->prev->prev->key);
	mu_assert_string_eq("baz", redis_os->oset->lsentinel->next->key);

	mu_assert_int_eq(1, OSET_compare(foo, bar));
	mu_assert_int_eq(1, OSET_compare(bar, baz));

}

MU_TEST(test_remove) {

	LNode* foo = HASH_create_node(&redis_os->hash, "foo", 3);
	OSET_add_head(redis_os->oset, foo, 1);
	LNode* bar = HASH_create_node(&redis_os->hash, "bar!", 4);
	OSET_add_after(redis_os->oset, foo, bar, 2);
	LNode* baz = HASH_create_node(&redis_os->hash, "baz", 3);
	OSET_add_after(redis_os->oset, bar, baz, 3);

	OSET_remove(bar);
	mu_assert_string_eq("baz", redis_os->oset->lsentinel->next->next->key);

	OSET_remove(foo);
	mu_assert_string_eq("baz", redis_os->oset->lsentinel->next->key);

}

MU_TEST(test_compare) {

	LNode* foo = HASH_create_node(&redis_os->hash, "foo", 3);
	OSET_add_head(redis_os->oset, foo, 1);

	mu_assert_int_eq(0, OSET_compare(foo, foo));

}

MU_TEST_SUITE(test_suite) {

	MU_SUITE_CONFIGURE(&test_setup, &test_teardown);
	MU_RUN_TEST(test_hash);
	MU_RUN_TEST(test_add_head);
	MU_RUN_TEST(test_add_tail);
	MU_RUN_TEST(test_add_after);
	MU_RUN_TEST(test_add_before);
	MU_RUN_TEST(test_remove);
	MU_RUN_TEST(test_compare);

}

int main(int argc, char **argv) {

	RedisModule_Alloc = malloc;
	RedisModule_Free = free;
	RedisModule_Strdup = strdup;

	MU_RUN_SUITE(test_suite);
	MU_REPORT();
	return 0;

}