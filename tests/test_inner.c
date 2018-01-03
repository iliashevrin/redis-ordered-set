#include <stdio.h>
#include <string.h>
#include "minunit.h"
#include "../src/om_api.h"
#include "../src/om_type.h"
#include "../src/om_hash.h"

static OM* om;

void mock_remove(LNode* node) {}

void test_setup(void) {
	om = OMInit();
}

void test_teardown(void) {
	OMFree(om);
}

MU_TEST(test_hash) {

	mu_check(get_node_read_mod(om->nodes, "foo") == NULL);
	mu_check(get_node_write_mod(&om->nodes, "foo", 3, mock_remove) != NULL);
	mu_assert_int_eq(1, table_size(om->nodes));
	mu_check(get_node_read_mod(om->nodes, "foo") != NULL);
	mu_check(get_node_write_mod(&om->nodes, "bar!", 4, mock_remove) != NULL);
	mu_assert_int_eq(2, table_size(om->nodes));
	mu_check(get_node_write_mod(&om->nodes, "foo", 4, mock_remove) != NULL);
	mu_assert_int_eq(2, table_size(om->nodes));
	mu_assert_int_eq(1, remove_node(&om->nodes, "bar!", mock_remove));
	mu_assert_int_eq(1, table_size(om->nodes));
	mu_assert_int_eq(0, remove_node(&om->nodes, "bar!", mock_remove));
	mu_assert_int_eq(1, remove_node(&om->nodes, "foo", mock_remove));
	mu_assert_int_eq(0, table_size(om->nodes));

}

MU_TEST(test_push_first) {

	mu_assert_int_eq(SUCCESS, push_first(om, "foo", 3));
	mu_assert_string_eq("foo", om->lsentinel->next->key);
	mu_assert_int_eq(SUCCESS, push_first(om, "bar!", 4));
	mu_assert_string_eq("bar!", om->lsentinel->next->key);

}

MU_TEST(test_push_last) {

	mu_assert_int_eq(SUCCESS, push_last(om, "foo", 3));
	mu_assert_string_eq("foo", om->lsentinel->prev->key);
	mu_assert_int_eq(SUCCESS, push_last(om, "bar!", 4));
	mu_assert_string_eq("bar!", om->lsentinel->prev->key);

}

MU_TEST(test_push_after) {

	push_first(om, "foo", 3);
	mu_assert_int_eq(SUCCESS, push_after(om, "foo", "bar!", 4));
	mu_assert_string_eq("bar!", om->lsentinel->next->next->key);
	mu_assert_int_eq(ELEMENT_NOT_FOUND, push_after(om, "notexist", "new", 3));
	mu_assert_int_eq(SUCCESS, push_after(om, "bar!", "baz", 3));
	mu_assert_int_eq(SUCCESS, push_after(om, "baz", "foo", 3));
	mu_assert_string_eq("bar!", om->lsentinel->next->key);
	mu_assert_string_eq("foo", om->lsentinel->prev->key);

}

MU_TEST(test_push_before) {

	push_last(om, "foo", 3);
	mu_assert_int_eq(SUCCESS, push_before(om, "foo", "bar!", 4));
	mu_assert_string_eq("bar!", om->lsentinel->prev->prev->key);
	mu_assert_int_eq(ELEMENT_NOT_FOUND, push_before(om, "notexist", "new", 3));
	mu_assert_int_eq(SUCCESS, push_before(om, "bar!", "baz", 3));
	mu_assert_int_eq(SUCCESS, push_before(om, "baz", "foo", 3));
	mu_assert_string_eq("bar!", om->lsentinel->prev->key);
	mu_assert_string_eq("foo", om->lsentinel->next->key);

}

MU_TEST(test_remove) {

	push_first(om, "foo", 3);
	push_after(om, "foo", "bar!", 4);
	push_after(om, "bar!", "baz", 3);
	mu_assert_int_eq(SUCCESS, remove_item(om, "bar!"));
	mu_assert_int_eq(ELEMENT_NOT_FOUND, remove_item(om, "notexist"));
	mu_assert_int_eq(SUCCESS, remove_item(om, "foo"));
	mu_assert_int_eq(LIST_EMPTY, remove_item(om, "baz"));

}

MU_TEST(test_compare) {

	push_first(om, "foo", 3);
	push_after(om, "foo", "bar!", 4);
	push_after(om, "bar!", "baz", 3);
	mu_assert_int_eq(-1, compare(om, "foo", "bar!"));
	mu_assert_int_eq(0, compare(om, "bar!", "bar!"));
	mu_assert_int_eq(1, compare(om, "baz", "bar!"));
	mu_assert_int_eq(ELEMENT_NOT_FOUND, compare(om, "notexist", "bar!"));

}

MU_TEST_SUITE(test_suite) {

	MU_SUITE_CONFIGURE(&test_setup, &test_teardown);
	MU_RUN_TEST(test_hash);
	MU_RUN_TEST(test_push_first);
	MU_RUN_TEST(test_push_last);
	MU_RUN_TEST(test_push_after);
	MU_RUN_TEST(test_push_before);
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