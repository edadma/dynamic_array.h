#include "unity/unity.h"

#define DYNAMIC_ARRAY_IMPLEMENTATION
#include "dynamic_array.h"

void setUp(void) {
    // Set up before each test
}

void tearDown(void) {
    // Clean up after each test
}

/* Creation and Basic Properties Tests */
void test_da_create_basic(void) {
    da_array arr = da_create(sizeof(int), 10);

    TEST_ASSERT_NOT_NULL(arr);
    TEST_ASSERT_EQUAL_INT(0, da_length(arr));
    TEST_ASSERT_EQUAL_INT(10, da_capacity(arr));
    TEST_ASSERT_NOT_NULL(da_data(arr));

    da_release(&arr);
    TEST_ASSERT_NULL(arr);
}

void test_da_create_zero_capacity(void) {
    da_array arr = da_create(sizeof(int), 0);

    TEST_ASSERT_NOT_NULL(arr);
    TEST_ASSERT_EQUAL_INT(0, da_length(arr));
    TEST_ASSERT_EQUAL_INT(0, da_capacity(arr));

    da_release(&arr);
    TEST_ASSERT_NULL(arr);
}

void test_da_create_typed_macro(void) {
    da_array arr = DA_CREATE(int, 5);

    TEST_ASSERT_NOT_NULL(arr);
    TEST_ASSERT_EQUAL_INT(0, DA_LEN(arr));
    TEST_ASSERT_EQUAL_INT(5, DA_CAP(arr));

    da_release(&arr);
}

/* Reference Counting Tests */
void test_da_reference_counting(void) {
    da_array arr = da_create(sizeof(int), 5);
    TEST_ASSERT_EQUAL_INT(1, DA_ATOMIC_LOAD(&arr->ref_count));

    da_array arr2 = da_retain(arr);
    TEST_ASSERT_EQUAL_PTR(arr, arr2);
    TEST_ASSERT_EQUAL_INT(2, DA_ATOMIC_LOAD(&arr->ref_count));

    da_release(&arr2);
    TEST_ASSERT_NULL(arr2);
    TEST_ASSERT_EQUAL_INT(1, DA_ATOMIC_LOAD(&arr->ref_count));

    da_release(&arr);
    TEST_ASSERT_NULL(arr);
}

void test_da_multiple_retains(void) {
    da_array arr = da_create(sizeof(int), 3);
    da_array arr2 = da_retain(arr);
    da_array arr3 = da_retain(arr);

    TEST_ASSERT_EQUAL_INT(3, DA_ATOMIC_LOAD(&arr->ref_count));

    da_release(&arr3);
    TEST_ASSERT_EQUAL_INT(2, DA_ATOMIC_LOAD(&arr->ref_count));

    da_release(&arr2);
    TEST_ASSERT_EQUAL_INT(1, DA_ATOMIC_LOAD(&arr->ref_count));

    da_release(&arr);
    TEST_ASSERT_NULL(arr);
}

/* Push and Growth Tests */
void test_da_push_basic(void) {
    da_array arr = da_create(sizeof(int), 2);

    int val1 = 42;
    int val2 = 99;

    da_push(arr, &val1);
    TEST_ASSERT_EQUAL_INT(1, da_length(arr));

    da_push(arr, &val2);
    TEST_ASSERT_EQUAL_INT(2, da_length(arr));

    da_release(&arr);
}

void test_da_push_with_growth(void) {
    da_array arr = da_create(sizeof(int), 1);

    int val1 = 10;
    int val2 = 20;

    da_push(arr, &val1);
    TEST_ASSERT_EQUAL_INT(1, da_capacity(arr));

    da_push(arr, &val2);  // Should trigger growth
    TEST_ASSERT_TRUE(da_capacity(arr) > 1);
    TEST_ASSERT_EQUAL_INT(2, da_length(arr));

    da_release(&arr);
}

void test_da_push_from_zero_capacity(void) {
    da_array arr = da_create(sizeof(int), 0);

    int val = 123;
    da_push(arr, &val);

    TEST_ASSERT_TRUE(da_capacity(arr) > 0);
    TEST_ASSERT_EQUAL_INT(1, da_length(arr));

    da_release(&arr);
}

/* Access Tests */
void test_da_get_and_set(void) {
    da_array arr = da_create(sizeof(int), 5);

    int val1 = 42;
    int val2 = 99;

    da_push(arr, &val1);
    da_push(arr, &val2);

    int* ptr1 = (int*)da_get(arr, 0);
    int* ptr2 = (int*)da_get(arr, 1);

    TEST_ASSERT_EQUAL_INT(42, *ptr1);
    TEST_ASSERT_EQUAL_INT(99, *ptr2);

    int new_val = 123;
    da_set(arr, 0, &new_val);

    int* updated_ptr = (int*)da_get(arr, 0);
    TEST_ASSERT_EQUAL_INT(123, *updated_ptr);

    da_release(&arr);
}

void test_da_data_access(void) {
    da_array arr = da_create(sizeof(int), 3);

    int val1 = 10;
    int val2 = 20;

    da_push(arr, &val1);
    da_push(arr, &val2);

    int* data = (int*)da_data(arr);
    TEST_ASSERT_EQUAL_INT(10, data[0]);
    TEST_ASSERT_EQUAL_INT(20, data[1]);

    // Direct modification
    data[0] = 100;
    int* check = (int*)da_get(arr, 0);
    TEST_ASSERT_EQUAL_INT(100, *check);

    da_release(&arr);
}

/* Pop Tests */
void test_da_pop_basic(void) {
    da_array arr = da_create(sizeof(int), 3);

    int val1 = 42;
    int val2 = 99;

    da_push(arr, &val1);
    da_push(arr, &val2);
    TEST_ASSERT_EQUAL_INT(2, da_length(arr));

    int popped;
    da_pop(arr, &popped);
    TEST_ASSERT_EQUAL_INT(99, popped);
    TEST_ASSERT_EQUAL_INT(1, da_length(arr));

    da_pop(arr, &popped);
    TEST_ASSERT_EQUAL_INT(42, popped);
    TEST_ASSERT_EQUAL_INT(0, da_length(arr));

    da_release(&arr);
}

void test_da_pop_ignore_output(void) {
    da_array arr = da_create(sizeof(int), 2);

    int val = 123;
    da_push(arr, &val);
    TEST_ASSERT_EQUAL_INT(1, da_length(arr));

    da_pop(arr, NULL);  // Ignore popped value
    TEST_ASSERT_EQUAL_INT(0, da_length(arr));

    da_release(&arr);
}

/* Clear and Resize Tests */
void test_da_clear(void) {
    da_array arr = da_create(sizeof(int), 5);

    int val1 = 10;
    int val2 = 20;

    da_push(arr, &val1);
    da_push(arr, &val2);
    TEST_ASSERT_EQUAL_INT(2, da_length(arr));

    da_clear(arr);
    TEST_ASSERT_EQUAL_INT(0, da_length(arr));
    TEST_ASSERT_EQUAL_INT(5, da_capacity(arr));  // Capacity unchanged

    da_release(&arr);
}

void test_da_reserve(void) {
    da_array arr = da_create(sizeof(int), 2);

    da_reserve(arr, 10);
    TEST_ASSERT_EQUAL_INT(10, da_capacity(arr));
    TEST_ASSERT_EQUAL_INT(0, da_length(arr));

    // Reserve smaller than current capacity should do nothing
    da_reserve(arr, 5);
    TEST_ASSERT_EQUAL_INT(10, da_capacity(arr));

    da_release(&arr);
}

void test_da_resize_grow(void) {
    da_array arr = da_create(sizeof(int), 3);

    int val = 42;
    da_push(arr, &val);
    TEST_ASSERT_EQUAL_INT(1, da_length(arr));

    da_resize(arr, 5);
    TEST_ASSERT_EQUAL_INT(5, da_length(arr));
    TEST_ASSERT_TRUE(da_capacity(arr) >= 5);

    // Check that new elements are zero-filled
    int* data = (int*)da_data(arr);
    TEST_ASSERT_EQUAL_INT(42, data[0]);
    TEST_ASSERT_EQUAL_INT(0, data[1]);
    TEST_ASSERT_EQUAL_INT(0, data[4]);

    da_release(&arr);
}

void test_da_resize_shrink(void) {
    da_array arr = da_create(sizeof(int), 5);

    int val1 = 10;
    int val2 = 20;
    int val3 = 30;

    da_push(arr, &val1);
    da_push(arr, &val2);
    da_push(arr, &val3);
    TEST_ASSERT_EQUAL_INT(3, da_length(arr));

    da_resize(arr, 1);
    TEST_ASSERT_EQUAL_INT(1, da_length(arr));

    int* data = (int*)da_data(arr);
    TEST_ASSERT_EQUAL_INT(10, data[0]);

    da_release(&arr);
}

/* Macro Tests */
void test_da_typed_macros(void) {
    da_array arr = DA_CREATE(int, 3);

#if DA_HAS_TYPEOF
    DA_PUSH(arr, 42);
    DA_PUSH(arr, 99);

    TEST_ASSERT_EQUAL_INT(42, DA_AT(arr, 0, int));
    TEST_ASSERT_EQUAL_INT(99, DA_AT(arr, 1, int));

    DA_PUT(arr, 0, 123);
    TEST_ASSERT_EQUAL_INT(123, DA_AT(arr, 0, int));
#else
    DA_PUSH(arr, 42, int);
    DA_PUSH(arr, 99, int);

    TEST_ASSERT_EQUAL_INT(42, DA_AT(arr, 0, int));
    TEST_ASSERT_EQUAL_INT(99, DA_AT(arr, 1, int));

    DA_PUT(arr, 0, 123, int);
    TEST_ASSERT_EQUAL_INT(123, DA_AT(arr, 0, int));
#endif

    int popped;
    DA_POP(arr, &popped);
    TEST_ASSERT_EQUAL_INT(99, popped);

    DA_CLEAR(arr);
    TEST_ASSERT_EQUAL_INT(0, DA_LEN(arr));

    DA_RESERVE(arr, 10);
    TEST_ASSERT_EQUAL_INT(10, DA_CAP(arr));

    DA_RESIZE(arr, 5);
    TEST_ASSERT_EQUAL_INT(5, DA_LEN(arr));

    da_release(&arr);
}

/* Stress Tests */
void test_da_many_operations(void) {
    da_array arr = DA_CREATE(int, 1);

    // Push many elements to test growth
    for (int i = 0; i < 100; i++) {
#if DA_HAS_TYPEOF
        DA_PUSH(arr, i);
#else
        DA_PUSH(arr, i, int);
#endif
    }

    TEST_ASSERT_EQUAL_INT(100, DA_LEN(arr));

    // Check all values
    for (int i = 0; i < 100; i++) {
        TEST_ASSERT_EQUAL_INT(i, DA_AT(arr, i, int));
    }

    // Pop half
    for (int i = 0; i < 50; i++) {
        int popped;
        DA_POP(arr, &popped);
        TEST_ASSERT_EQUAL_INT(99 - i, popped);
    }

    TEST_ASSERT_EQUAL_INT(50, DA_LEN(arr));

    da_release(&arr);
}

void test_da_different_types(void) {
    // Test with different data types
    da_array float_arr = DA_CREATE(float, 2);
    da_array char_arr = DA_CREATE(char, 2);

    float f_val = 3.14f;
    char c_val = 'A';

    da_push(float_arr, &f_val);
    da_push(char_arr, &c_val);

    float* f_ptr = (float*)da_get(float_arr, 0);
    char* c_ptr = (char*)da_get(char_arr, 0);

    TEST_ASSERT_FLOAT_WITHIN(0.01f, 3.14f, *f_ptr);
    TEST_ASSERT_EQUAL_INT('A', *c_ptr);

    da_release(&float_arr);
    da_release(&char_arr);
}

void test_da_thread_safety_basic(void) {
    da_array arr = da_create(sizeof(int), 5);

    // Test that atomic operations work (even in single thread)
    TEST_ASSERT_EQUAL_INT(1, DA_ATOMIC_LOAD(&arr->ref_count));

    // Multiple retains should be atomic
    da_retain(arr);
    da_retain(arr);
    TEST_ASSERT_EQUAL_INT(3, DA_ATOMIC_LOAD(&arr->ref_count));

    // Releases should be atomic
    da_array temp = arr;
    da_release(&temp);  // Should not free, ref_count = 2
    TEST_ASSERT_NULL(temp);
    TEST_ASSERT_EQUAL_INT(2, DA_ATOMIC_LOAD(&arr->ref_count));

    temp = arr;
    da_release(&temp);  // Should not free, ref_count = 1
    TEST_ASSERT_NULL(temp);
    TEST_ASSERT_EQUAL_INT(1, DA_ATOMIC_LOAD(&arr->ref_count));

    da_release(&arr);  // Should free
    TEST_ASSERT_NULL(arr);
}

int main(void) {
    UNITY_BEGIN();

    // Creation and basic properties
    RUN_TEST(test_da_create_basic);
    RUN_TEST(test_da_create_zero_capacity);
    RUN_TEST(test_da_create_typed_macro);

    // Reference counting
    RUN_TEST(test_da_reference_counting);
    RUN_TEST(test_da_multiple_retains);

    // Push and growth
    RUN_TEST(test_da_push_basic);
    RUN_TEST(test_da_push_with_growth);
    RUN_TEST(test_da_push_from_zero_capacity);

    // Access
    RUN_TEST(test_da_get_and_set);
    RUN_TEST(test_da_data_access);

    // Pop
    RUN_TEST(test_da_pop_basic);
    RUN_TEST(test_da_pop_ignore_output);

    // Clear and resize
    RUN_TEST(test_da_clear);
    RUN_TEST(test_da_reserve);
    RUN_TEST(test_da_resize_grow);
    RUN_TEST(test_da_resize_shrink);

    // Macros
    RUN_TEST(test_da_typed_macros);

    // Stress tests
    RUN_TEST(test_da_many_operations);
    RUN_TEST(test_da_different_types);
    RUN_TEST(test_da_thread_safety_basic);

    return UNITY_END();
}