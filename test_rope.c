#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "lib/rope.h"

/* -------------------- Global Rope Memory -------------------- */

static mem_for_special g_mem;

/* -------------------- Test Utilities -------------------- */

#define ASSERT_STR_EQ(actual, expected)                          \
    do {                                                         \
        if (strcmp((char *)(actual), (expected)) != 0) {        \
            fprintf(stderr,                                     \
                "\nFAIL\nExpected: \"%s\"\nGot: \"%s\"\n",      \
                (expected), (actual));                          \
            exit(1);                                            \
        }                                                        \
    } while (0)

static void print_test(const char *name) {
    printf("[TEST] %s ... ", name);
    fflush(stdout);
}

static void ok(void) {
    printf("OK\n");
}

/* -------------------- Tests -------------------- */

void test_make_leaf(void) {
    print_test("make_leaf + flatten");

    rope_node *r = make_leaf("Hello Rope");
    unsigned char *s = flatten_to_string(r);

    ASSERT_STR_EQ(s, "Hello Rope");

    free(s);
    free_ropes(r, &g_mem);
    ok();
}

void test_append(void) {
    print_test("rope_append");

    rope_node *r = NULL;

    rope_append(&r, "Hello");
    rope_append(&r, " ");
    rope_append(&r, "World");

    unsigned char *s = flatten_to_string(r);
    ASSERT_STR_EQ(s, "Hello World");

    free(s);
    free_ropes(r, &g_mem);
    ok();
}

void test_insert(void) {
    print_test("insert_rope");

    rope_node *r = make_leaf("Helo World");
    insert_rope(r, 2, "l", &r, &g_mem);

    unsigned char *s = flatten_to_string(r);
    ASSERT_STR_EQ(s, "Hello World");

    free(s);
    free_ropes(r, &g_mem);
    ok();
}

void test_delete(void) {
    print_test("delete_rope");

    rope_node *r = make_leaf("Hello cruel World");
    rope_node *deleted = NULL;

    delete_rope(r, 6, &r, 6, &g_mem, &deleted);

    unsigned char *s = flatten_to_string(r);
    ASSERT_STR_EQ(s, "Hello World");

    free(s);
    free_ropes(r, &g_mem);
    ok();
}

void test_split(void) {
    print_test("split_rope");

    rope_node *r = make_leaf("HelloWorld");
    rope_node *left = NULL;
    rope_node *right = NULL;

    split_rope(r, 5, &left, &right, &g_mem);

    unsigned char *ls = flatten_to_string(left);
    unsigned char *rs = flatten_to_string(right);

    ASSERT_STR_EQ(ls, "Hello");
    ASSERT_STR_EQ(rs, "World");

    free(ls);
    free(rs);
    free_ropes(left, &g_mem);
    free_ropes(right, &g_mem);
    ok();
}

void test_substr(void) {
    print_test("substr (start + length)");

    rope_node *r = make_leaf("Hello Wonderful World");
    rope_node *sub = NULL;

    /* start = 6, length = 15 */
    substr(r, 6, 15, &sub, &g_mem);

    unsigned char *s = flatten_to_string(sub);
    ASSERT_STR_EQ(s, "Wonderful World");

    free(s);
    free_ropes(r, &g_mem);
    free_ropes(sub, &g_mem);
    ok();
}

void test_balance(void) {
    print_test("rebalance");

    rope_node *r = NULL;

    for (int i = 0; i < 1000; i++) {
        rope_append(&r, "x");
    }

    rebalance(r, &r, &g_mem);

    assert(is_balanced(r) && "rope is not balanced");

    free_ropes(r, &g_mem);
    ok();
}

/* -------------------- Main -------------------- */

int main(void) {
    init_mem_f_s(&g_mem, 128);

    test_make_leaf();
    test_append();
    test_insert();
    test_delete();
    test_split();
    test_substr();
    test_balance();

    free_mem(&g_mem);

    printf("\nAll rope tests passed ✅\n");
    return 0;
}
