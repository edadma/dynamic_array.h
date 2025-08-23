#include "unity.h"

#define DYNAMIC_ARRAY_IMPLEMENTATION
/* uncomment as needed */
/* #define DA_NOT_USE_TYPE_GENERIC */
#include "dynamic_array.h"

void setUp(void) {
    // Set up before each test
}

void tearDown(void) {
    // Clean up after each test
}

/* Creation and Basic Properties Tests */
void test_create_basic(void) {
    da_array arr = da_create(sizeof(int), 10, NULL, NULL);

    TEST_ASSERT_NOT_NULL(arr);
    TEST_ASSERT_EQUAL_INT(0, da_length(arr));
    TEST_ASSERT_EQUAL_INT(10, da_capacity(arr));
    TEST_ASSERT_NOT_NULL(da_data(arr));

    da_release(&arr);
    TEST_ASSERT_NULL(arr);
}

void test_create_zero_capacity(void) {
    da_array arr = da_new(sizeof(int));

    TEST_ASSERT_NOT_NULL(arr);
    TEST_ASSERT_EQUAL_INT(0, da_length(arr));
    TEST_ASSERT_EQUAL_INT(0, da_capacity(arr));

    da_release(&arr);
    TEST_ASSERT_NULL(arr);
}

void test_create_typed_macro(void) {
    da_array arr = da_create(sizeof(int), 5, NULL, NULL);

    TEST_ASSERT_NOT_NULL(arr);
    TEST_ASSERT_EQUAL_INT(0, DA_LENGTH(arr));
    TEST_ASSERT_EQUAL_INT(5, DA_CAPACITY(arr));

    da_release(&arr);
}

/* Reference Counting Tests */
void test_reference_counting(void) {
    da_array arr = da_new(sizeof(int));
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

void test_multiple_retains(void) {
    da_array arr = da_new(sizeof(int));
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
void test_push_basic(void) {
    da_array arr = da_new(sizeof(int));

    int val1 = 42;
    int val2 = 99;

    da_push(arr, &val1);
    TEST_ASSERT_EQUAL_INT(1, da_length(arr));

    da_push(arr, &val2);
    TEST_ASSERT_EQUAL_INT(2, da_length(arr));

    da_release(&arr);
}

void test_push_with_growth(void) {
    da_array arr = da_new(sizeof(int));

    int val1 = 10;
    int val2 = 20;

    da_push(arr, &val1);
    TEST_ASSERT_EQUAL_INT(1, da_capacity(arr));

    da_push(arr, &val2);  // Should trigger growth
    TEST_ASSERT_TRUE(da_capacity(arr) > 1);
    TEST_ASSERT_EQUAL_INT(2, da_length(arr));

    da_release(&arr);
}

void test_push_from_zero_capacity(void) {
    da_array arr = da_new(sizeof(int));

    int val = 123;
    da_push(arr, &val);

    TEST_ASSERT_TRUE(da_capacity(arr) > 0);
    TEST_ASSERT_EQUAL_INT(1, da_length(arr));

    da_release(&arr);
}

/* Access Tests */
void test_get_and_set(void) {
    da_array arr = da_new(sizeof(int));

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

void test_data_access(void) {
    da_array arr = da_new(sizeof(int));

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
void test_pop_basic(void) {
    da_array arr = da_new(sizeof(int));

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

void test_pop_ignore_output(void) {
    da_array arr = da_new(sizeof(int));

    int val = 123;
    da_push(arr, &val);
    TEST_ASSERT_EQUAL_INT(1, da_length(arr));

    da_pop(arr, NULL);  // Ignore popped value
    TEST_ASSERT_EQUAL_INT(0, da_length(arr));

    da_release(&arr);
}

/* Clear and Resize Tests */
void test_clear(void) {
    da_array arr = da_create(sizeof(int), 5, NULL, NULL);

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

void test_reserve(void) {
    da_array arr = da_new(sizeof(int));

    da_reserve(arr, 10);
    TEST_ASSERT_EQUAL_INT(10, da_capacity(arr));
    TEST_ASSERT_EQUAL_INT(0, da_length(arr));

    // Reserve smaller than current capacity should do nothing
    da_reserve(arr, 5);
    TEST_ASSERT_EQUAL_INT(10, da_capacity(arr));

    da_release(&arr);
}

void test_resize_grow(void) {
    da_array arr = da_new(sizeof(int));

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

void test_resize_shrink(void) {
    da_array arr = da_new(sizeof(int));

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
void test_typed_macros(void) {
    da_array arr = da_create(sizeof(int), 3, NULL, NULL);

#if DA_SUPPORT_TYPE_INFERENCE && !defined(DA_NOT_USE_TYPE_GENERIC)
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
    TEST_ASSERT_EQUAL_INT(0, DA_LENGTH(arr));

    DA_RESERVE(arr, 10);
    TEST_ASSERT_EQUAL_INT(10, DA_CAPACITY(arr));

    DA_RESIZE(arr, 5);
    TEST_ASSERT_EQUAL_INT(5, DA_LENGTH(arr));

    da_release(&arr);
}

/* Stress Tests */
void test_many_operations(void) {
    da_array arr = da_create(sizeof(int), 1, NULL, NULL);

    // Push many elements to test growth
    for (int i = 0; i < 100; i++) {
#if DA_SUPPORT_TYPE_INFERENCE && !defined(DA_NOT_USE_TYPE_GENERIC)
        DA_PUSH(arr, i);
#else
        DA_PUSH(arr, i, int);
#endif
    }

    TEST_ASSERT_EQUAL_INT(100, DA_LENGTH(arr));

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

    TEST_ASSERT_EQUAL_INT(50, DA_LENGTH(arr));

    da_release(&arr);
}

void test_different_types(void) {
    // Test with different data types
    da_array float_arr = da_create(sizeof(float), 2, NULL, NULL);
    da_array char_arr = da_create(sizeof(char), 2, NULL, NULL);

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

void test_atomic_refcount_basic(void) {
    da_array arr = da_new(sizeof(int));

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

/* Insert and Remove Tests */
void test_insert_basic(void) {
    da_array arr = da_new(sizeof(int));

    // Add initial elements
    int vals[] = {10, 20, 30};
    for (int i = 0; i < 3; i++) {
        da_push(arr, &vals[i]);
    }

    // Insert in the middle
    int insert_val = 25;
    da_insert(arr, 2, &insert_val);

    TEST_ASSERT_EQUAL_INT(4, da_length(arr));
    TEST_ASSERT_EQUAL_INT(10, DA_AT(arr, 0, int));
    TEST_ASSERT_EQUAL_INT(20, DA_AT(arr, 1, int));
    TEST_ASSERT_EQUAL_INT(25, DA_AT(arr, 2, int));
    TEST_ASSERT_EQUAL_INT(30, DA_AT(arr, 3, int));

    da_release(&arr);
}

void test_insert_at_beginning(void) {
    da_array arr = da_new(sizeof(int));

    int val1 = 20;
    int val2 = 30;
    da_push(arr, &val1);
    da_push(arr, &val2);

    int insert_val = 10;
    da_insert(arr, 0, &insert_val);

    TEST_ASSERT_EQUAL_INT(3, da_length(arr));
    TEST_ASSERT_EQUAL_INT(10, DA_AT(arr, 0, int));
    TEST_ASSERT_EQUAL_INT(20, DA_AT(arr, 1, int));
    TEST_ASSERT_EQUAL_INT(30, DA_AT(arr, 2, int));

    da_release(&arr);
}

void test_insert_at_end(void) {
    da_array arr = da_new(sizeof(int));

    int val1 = 10;
    int val2 = 20;
    da_push(arr, &val1);
    da_push(arr, &val2);

    // Insert at end (equivalent to push)
    int insert_val = 30;
    da_insert(arr, da_length(arr), &insert_val);

    TEST_ASSERT_EQUAL_INT(3, da_length(arr));
    TEST_ASSERT_EQUAL_INT(10, DA_AT(arr, 0, int));
    TEST_ASSERT_EQUAL_INT(20, DA_AT(arr, 1, int));
    TEST_ASSERT_EQUAL_INT(30, DA_AT(arr, 2, int));

    da_release(&arr);
}

void test_insert_with_growth(void) {
    da_array arr = da_new(sizeof(int));

    int val1 = 10;
    int val2 = 30;
    da_push(arr, &val1);
    da_push(arr, &val2);

    // Array is full, insert should trigger growth
    int insert_val = 20;
    da_insert(arr, 1, &insert_val);

    TEST_ASSERT_TRUE(da_capacity(arr) > 2);
    TEST_ASSERT_EQUAL_INT(3, da_length(arr));
    TEST_ASSERT_EQUAL_INT(10, DA_AT(arr, 0, int));
    TEST_ASSERT_EQUAL_INT(20, DA_AT(arr, 1, int));
    TEST_ASSERT_EQUAL_INT(30, DA_AT(arr, 2, int));

    da_release(&arr);
}

void test_remove_basic(void) {
    da_array arr = da_new(sizeof(int));

    int vals[] = {10, 20, 30, 40};
    for (int i = 0; i < 4; i++) {
        da_push(arr, &vals[i]);
    }

    // Remove from middle
    int removed;
    da_remove(arr, 2, &removed);

    TEST_ASSERT_EQUAL_INT(30, removed);
    TEST_ASSERT_EQUAL_INT(3, da_length(arr));
    TEST_ASSERT_EQUAL_INT(10, DA_AT(arr, 0, int));
    TEST_ASSERT_EQUAL_INT(20, DA_AT(arr, 1, int));
    TEST_ASSERT_EQUAL_INT(40, DA_AT(arr, 2, int));

    da_release(&arr);
}

void test_remove_first(void) {
    da_array arr = da_new(sizeof(int));

    int vals[] = {10, 20, 30};
    for (int i = 0; i < 3; i++) {
        da_push(arr, &vals[i]);
    }

    int removed;
    da_remove(arr, 0, &removed);

    TEST_ASSERT_EQUAL_INT(10, removed);
    TEST_ASSERT_EQUAL_INT(2, da_length(arr));
    TEST_ASSERT_EQUAL_INT(20, DA_AT(arr, 0, int));
    TEST_ASSERT_EQUAL_INT(30, DA_AT(arr, 1, int));

    da_release(&arr);
}

void test_remove_last(void) {
    da_array arr = da_new(sizeof(int));

    int vals[] = {10, 20, 30};
    for (int i = 0; i < 3; i++) {
        da_push(arr, &vals[i]);
    }

    int removed;
    da_remove(arr, 2, &removed);

    TEST_ASSERT_EQUAL_INT(30, removed);
    TEST_ASSERT_EQUAL_INT(2, da_length(arr));
    TEST_ASSERT_EQUAL_INT(10, DA_AT(arr, 0, int));
    TEST_ASSERT_EQUAL_INT(20, DA_AT(arr, 1, int));

    da_release(&arr);
}

void test_remove_ignore_output(void) {
    da_array arr = da_new(sizeof(int));

    int vals[] = {10, 20, 30};
    for (int i = 0; i < 3; i++) {
        da_push(arr, &vals[i]);
    }

    // Remove without capturing the value
    da_remove(arr, 1, NULL);

    TEST_ASSERT_EQUAL_INT(2, da_length(arr));
    TEST_ASSERT_EQUAL_INT(10, DA_AT(arr, 0, int));
    TEST_ASSERT_EQUAL_INT(30, DA_AT(arr, 1, int));

    da_release(&arr);
}

/* Memory Optimization Tests */
void test_trim_basic(void) {
    da_array arr = da_create(sizeof(int), 100, NULL, NULL);

    // Add some elements
    for (int i = 0; i < 10; i++) {
        da_push(arr, &i);
    }

    TEST_ASSERT_EQUAL_INT(10, da_length(arr));
    TEST_ASSERT_EQUAL_INT(100, da_capacity(arr));

    // Trim to smaller capacity
    da_trim(arr, 20);
    TEST_ASSERT_EQUAL_INT(10, da_length(arr));
    TEST_ASSERT_EQUAL_INT(20, da_capacity(arr));

    // Verify data integrity
    for (int i = 0; i < 10; i++) {
        TEST_ASSERT_EQUAL_INT(i, DA_AT(arr, i, int));
    }

    da_release(&arr);
}

void test_trim_to_zero(void) {
    da_array arr = da_create(sizeof(int), 10, NULL, NULL);

    // Don't add any elements
    TEST_ASSERT_EQUAL_INT(0, da_length(arr));
    TEST_ASSERT_EQUAL_INT(10, da_capacity(arr));

    // Trim to zero capacity
    da_trim(arr, 0);
    TEST_ASSERT_EQUAL_INT(0, da_length(arr));
    TEST_ASSERT_EQUAL_INT(0, da_capacity(arr));
    TEST_ASSERT_NULL(da_data(arr));

    da_release(&arr);
}

void test_shrink_to_fit_macro(void) {
    da_array arr = da_create(sizeof(int), 50, NULL, NULL);

    // Add some elements
    for (int i = 0; i < 15; i++) {
        da_push(arr, &i);
    }

    TEST_ASSERT_EQUAL_INT(15, da_length(arr));
    TEST_ASSERT_EQUAL_INT(50, da_capacity(arr));

    // Shrink to fit using macro
    DA_SHRINK_TO_FIT(arr);
    TEST_ASSERT_EQUAL_INT(15, da_length(arr));
    TEST_ASSERT_EQUAL_INT(15, da_capacity(arr));

    // Verify data integrity
    for (int i = 0; i < 15; i++) {
        TEST_ASSERT_EQUAL_INT(i, DA_AT(arr, i, int));
    }

    da_release(&arr);
}

/* Array Concatenation Tests */
void test_append_array_basic(void) {
    da_array arr1 = da_new(sizeof(int));
    da_array arr2 = da_new(sizeof(int));

    // Setup arr1: [10, 20]
    int vals1[] = {10, 20};
    for (int i = 0; i < 2; i++) {
        da_push(arr1, &vals1[i]);
    }

    // Setup arr2: [30, 40]
    int vals2[] = {30, 40};
    for (int i = 0; i < 2; i++) {
        da_push(arr2, &vals2[i]);
    }

    // Append arr2 to arr1
    da_append_array(arr1, arr2);

    // Check arr1: [10, 20, 30, 40]
    TEST_ASSERT_EQUAL_INT(4, da_length(arr1));
    TEST_ASSERT_EQUAL_INT(10, DA_AT(arr1, 0, int));
    TEST_ASSERT_EQUAL_INT(20, DA_AT(arr1, 1, int));
    TEST_ASSERT_EQUAL_INT(30, DA_AT(arr1, 2, int));
    TEST_ASSERT_EQUAL_INT(40, DA_AT(arr1, 3, int));

    // Check arr2 is unchanged: [30, 40]
    TEST_ASSERT_EQUAL_INT(2, da_length(arr2));
    TEST_ASSERT_EQUAL_INT(30, DA_AT(arr2, 0, int));
    TEST_ASSERT_EQUAL_INT(40, DA_AT(arr2, 1, int));

    da_release(&arr1);
    da_release(&arr2);
}

void test_append_array_empty(void) {
    da_array arr1 = da_new(sizeof(int));
    da_array arr2 = da_new(sizeof(int));

    // arr1 has elements
    int val = 42;
    da_push(arr1, &val);

    // arr2 is empty - appending should do nothing
    da_append_array(arr1, arr2);

    TEST_ASSERT_EQUAL_INT(1, da_length(arr1));
    TEST_ASSERT_EQUAL_INT(42, DA_AT(arr1, 0, int));

    da_release(&arr1);
    da_release(&arr2);
}

void test_append_array_with_growth(void) {
    da_array arr1 = da_new(sizeof(int));
    da_array arr2 = da_new(sizeof(int));

    // Fill arr1 to capacity: [10, 20]
    int vals1[] = {10, 20};
    for (int i = 0; i < 2; i++) {
        da_push(arr1, &vals1[i]);
    }

    // Setup arr2: [30, 40, 50]
    int vals2[] = {30, 40, 50};
    for (int i = 0; i < 3; i++) {
        da_push(arr2, &vals2[i]);
    }

    TEST_ASSERT_EQUAL_INT(2, da_capacity(arr1));

    // Append should trigger growth
    da_append_array(arr1, arr2);

    TEST_ASSERT_TRUE(da_capacity(arr1) >= 5);
    TEST_ASSERT_EQUAL_INT(5, da_length(arr1));

    // Verify data: [10, 20, 30, 40, 50]
    TEST_ASSERT_EQUAL_INT(10, DA_AT(arr1, 0, int));
    TEST_ASSERT_EQUAL_INT(20, DA_AT(arr1, 1, int));
    TEST_ASSERT_EQUAL_INT(30, DA_AT(arr1, 2, int));
    TEST_ASSERT_EQUAL_INT(40, DA_AT(arr1, 3, int));
    TEST_ASSERT_EQUAL_INT(50, DA_AT(arr1, 4, int));

    da_release(&arr1);
    da_release(&arr2);
}

void test_concat_basic(void) {
    da_array arr1 = da_new(sizeof(int));
    da_array arr2 = da_new(sizeof(int));

    // Setup arr1: [10, 20]
    int vals1[] = {10, 20};
    for (int i = 0; i < 2; i++) {
        da_push(arr1, &vals1[i]);
    }

    // Setup arr2: [30, 40]
    int vals2[] = {30, 40};
    for (int i = 0; i < 2; i++) {
        da_push(arr2, &vals2[i]);
    }

    // Concatenate (functional style)
    da_array result = da_concat(arr1, arr2);

    // Check result: [10, 20, 30, 40]
    TEST_ASSERT_EQUAL_INT(4, da_length(result));
    TEST_ASSERT_EQUAL_INT(4, da_capacity(result)); // Exact capacity!
    TEST_ASSERT_EQUAL_INT(10, DA_AT(result, 0, int));
    TEST_ASSERT_EQUAL_INT(20, DA_AT(result, 1, int));
    TEST_ASSERT_EQUAL_INT(30, DA_AT(result, 2, int));
    TEST_ASSERT_EQUAL_INT(40, DA_AT(result, 3, int));

    // Check originals are unchanged
    TEST_ASSERT_EQUAL_INT(2, da_length(arr1));
    TEST_ASSERT_EQUAL_INT(2, da_length(arr2));
    TEST_ASSERT_EQUAL_INT(10, DA_AT(arr1, 0, int));
    TEST_ASSERT_EQUAL_INT(30, DA_AT(arr2, 0, int));

    da_release(&result);
    da_release(&arr1);
    da_release(&arr2);
}

void test_concat_empty_arrays(void) {
    da_array arr1 = da_new(sizeof(int));
    da_array arr2 = da_new(sizeof(int));

    // Both arrays empty
    da_array result = da_concat(arr1, arr2);

    TEST_ASSERT_EQUAL_INT(0, da_length(result));
    TEST_ASSERT_EQUAL_INT(0, da_capacity(result));
    TEST_ASSERT_NULL(da_data(result));

    da_release(&result);
    da_release(&arr1);
    da_release(&arr2);
}

void test_concat_one_empty(void) {
    da_array arr1 = da_new(sizeof(int));
    da_array arr2 = da_new(sizeof(int));

    // Only arr1 has elements: [42, 99]
    int vals[] = {42, 99};
    for (int i = 0; i < 2; i++) {
        da_push(arr1, &vals[i]);
    }

    // arr2 is empty
    da_array result = da_concat(arr1, arr2);

    TEST_ASSERT_EQUAL_INT(2, da_length(result));
    TEST_ASSERT_EQUAL_INT(2, da_capacity(result));
    TEST_ASSERT_EQUAL_INT(42, DA_AT(result, 0, int));
    TEST_ASSERT_EQUAL_INT(99, DA_AT(result, 1, int));

    da_release(&result);
    da_release(&arr1);
    da_release(&arr2);
}

/* Builder Tests */
void test_builder_create_basic(void) {
    da_builder builder = da_builder_create(sizeof(int));

    TEST_ASSERT_NOT_NULL(builder);
    TEST_ASSERT_EQUAL_INT(0, da_builder_length(builder));
    TEST_ASSERT_EQUAL_INT(0, da_builder_capacity(builder));

    da_builder_destroy(&builder);
    TEST_ASSERT_NULL(builder);
}

void test_builder_create_typed_macro(void) {
    da_builder builder = DA_BUILDER_CREATE(int);

    TEST_ASSERT_NOT_NULL(builder);
    TEST_ASSERT_EQUAL_INT(0, DA_BUILDER_LEN(builder));
    TEST_ASSERT_EQUAL_INT(0, DA_BUILDER_CAP(builder));

    da_builder_destroy(&builder);
}

void test_builder_append_basic(void) {
    da_builder builder = DA_BUILDER_CREATE(int);

    int val1 = 42;
    int val2 = 99;

    da_builder_append(builder, &val1);
    TEST_ASSERT_EQUAL_INT(1, DA_BUILDER_LEN(builder));
    TEST_ASSERT_TRUE(DA_BUILDER_CAP(builder) >= 1);

    da_builder_append(builder, &val2);
    TEST_ASSERT_EQUAL_INT(2, DA_BUILDER_LEN(builder));
    TEST_ASSERT_TRUE(DA_BUILDER_CAP(builder) >= 2);

    da_builder_destroy(&builder);
}

void test_builder_append_typed_macro(void) {
    da_builder builder = DA_BUILDER_CREATE(int);

#if DA_SUPPORT_TYPE_INFERENCE && !defined(DA_NOT_USE_TYPE_GENERIC)
    DA_BUILDER_APPEND(builder, 42);
    DA_BUILDER_APPEND(builder, 99);
#else
    DA_BUILDER_APPEND(builder, 42, int);
    DA_BUILDER_APPEND(builder, 99, int);
#endif

    TEST_ASSERT_EQUAL_INT(2, DA_BUILDER_LEN(builder));

    TEST_ASSERT_EQUAL_INT(42, DA_BUILDER_AT(builder, 0, int));
    TEST_ASSERT_EQUAL_INT(99, DA_BUILDER_AT(builder, 1, int));

    da_builder_destroy(&builder);
}

void test_builder_growth_doubling(void) {
    da_builder builder = DA_BUILDER_CREATE(int);

    // Builder should always use doubling strategy
    int previous_capacity = 0;

    for (int i = 0; i < 20; i++) {
#if DA_SUPPORT_TYPE_INFERENCE && !defined(DA_NOT_USE_TYPE_GENERIC)
        DA_BUILDER_APPEND(builder, i);
#else
        DA_BUILDER_APPEND(builder, i, int);
#endif

        int current_capacity = DA_BUILDER_CAP(builder);

        // Check doubling behavior (capacity should be power of 2)
        if (current_capacity > previous_capacity) {
            if (previous_capacity > 0) {
                TEST_ASSERT_EQUAL_INT(previous_capacity * 2, current_capacity);
            }
            previous_capacity = current_capacity;
        }
    }

    da_builder_destroy(&builder);
}

void test_builder_access_operations(void) {
    da_builder builder = DA_BUILDER_CREATE(int);

    // Append some values
    for (int i = 0; i < 5; i++) {
#if DA_SUPPORT_TYPE_INFERENCE && !defined(DA_NOT_USE_TYPE_GENERIC)
        DA_BUILDER_APPEND(builder, i * 10);
#else
        DA_BUILDER_APPEND(builder, i * 10, int);
#endif
    }

    // Test get
    for (int i = 0; i < 5; i++) {
        int* ptr = (int*)da_builder_get(builder, i);
        TEST_ASSERT_EQUAL_INT(i * 10, *ptr);
    }

    // Test set
    int new_val = 999;
    da_builder_set(builder, 2, &new_val);
    TEST_ASSERT_EQUAL_INT(999, DA_BUILDER_AT(builder, 2, int));

    da_builder_destroy(&builder);
}

void test_builder_clear(void) {
    da_builder builder = DA_BUILDER_CREATE(int);

    // Add some elements
    for (int i = 0; i < 10; i++) {
#if DA_SUPPORT_TYPE_INFERENCE && !defined(DA_NOT_USE_TYPE_GENERIC)
        DA_BUILDER_APPEND(builder, i);
#else
        DA_BUILDER_APPEND(builder, i, int);
#endif
    }

    TEST_ASSERT_EQUAL_INT(10, DA_BUILDER_LEN(builder));
    int capacity_before = DA_BUILDER_CAP(builder);

    DA_BUILDER_CLEAR(builder);

    TEST_ASSERT_EQUAL_INT(0, DA_BUILDER_LEN(builder));
    TEST_ASSERT_EQUAL_INT(capacity_before, DA_BUILDER_CAP(builder));  // Capacity unchanged

    da_builder_destroy(&builder);
}

void test_builder_to_array_basic(void) {
    da_builder builder = DA_BUILDER_CREATE(int);

    // Build up some data
    for (int i = 0; i < 10; i++) {
#if DA_SUPPORT_TYPE_INFERENCE && !defined(DA_NOT_USE_TYPE_GENERIC)
        DA_BUILDER_APPEND(builder, i * 2);
#else
        DA_BUILDER_APPEND(builder, i * 2, int);
#endif
    }

    int builder_length = DA_BUILDER_LEN(builder);
    int builder_capacity = DA_BUILDER_CAP(builder);

    // Convert to array
    da_array arr = da_builder_to_array(&builder, NULL, NULL);
    TEST_ASSERT_NULL(builder);  // Builder should be consumed

    // Check array properties
    TEST_ASSERT_NOT_NULL(arr);
    TEST_ASSERT_EQUAL_INT(builder_length, DA_LENGTH(arr));
    TEST_ASSERT_EQUAL_INT(builder_length, DA_CAPACITY(arr));  // Exact capacity!
    TEST_ASSERT_EQUAL_INT(1, DA_ATOMIC_LOAD(&arr->ref_count));

    // Check that capacity was shrunk to exact size
    TEST_ASSERT_TRUE(DA_CAPACITY(arr) <= builder_capacity);

    // Verify data integrity
    for (int i = 0; i < 10; i++) {
        TEST_ASSERT_EQUAL_INT(i * 2, DA_AT(arr, i, int));
    }

    da_release(&arr);
}

void test_builder_to_array_empty(void) {
    da_builder builder = DA_BUILDER_CREATE(int);

    // Don't add any elements
    TEST_ASSERT_EQUAL_INT(0, DA_BUILDER_LEN(builder));

    da_array arr = da_builder_to_array(&builder, NULL, NULL);
    TEST_ASSERT_NULL(builder);

    TEST_ASSERT_NOT_NULL(arr);
    TEST_ASSERT_EQUAL_INT(0, DA_LENGTH(arr));
    TEST_ASSERT_EQUAL_INT(0, DA_CAPACITY(arr));
    TEST_ASSERT_EQUAL_INT(1, DA_ATOMIC_LOAD(&arr->ref_count));

    da_release(&arr);
}

void test_builder_to_array_exact_sizing(void) {
    da_builder builder = DA_BUILDER_CREATE(int);

    // Add elements to trigger multiple capacity doublings
    for (int i = 0; i < 100; i++) {
#if DA_SUPPORT_TYPE_INFERENCE && !defined(DA_NOT_USE_TYPE_GENERIC)
        DA_BUILDER_APPEND(builder, i);
#else
        DA_BUILDER_APPEND(builder, i, int);
#endif
    }

    int builder_capacity = DA_BUILDER_CAP(builder);
    TEST_ASSERT_TRUE(builder_capacity > 100);  // Should have excess capacity

    da_array arr = da_builder_to_array(&builder, NULL, NULL);

    // Array should have exact capacity = length = 100
    TEST_ASSERT_EQUAL_INT(100, DA_LENGTH(arr));
    TEST_ASSERT_EQUAL_INT(100, DA_CAPACITY(arr));  // No wasted memory!

    da_release(&arr);
}

void test_builder_integration_with_arrays(void) {
    // Test that arrays created from builders work normally
    da_builder builder = DA_BUILDER_CREATE(int);

    for (int i = 0; i < 5; i++) {
#if DA_SUPPORT_TYPE_INFERENCE && !defined(DA_NOT_USE_TYPE_GENERIC)
        DA_BUILDER_APPEND(builder, i * 10);
#else
        DA_BUILDER_APPEND(builder, i * 10, int);
#endif
    }

    da_array arr = da_builder_to_array(&builder, NULL, NULL);

    // Test reference counting
    da_array arr2 = da_retain(arr);
    TEST_ASSERT_EQUAL_INT(2, DA_ATOMIC_LOAD(&arr->ref_count));

    // Test array operations
#if DA_SUPPORT_TYPE_INFERENCE && !defined(DA_NOT_USE_TYPE_GENERIC)
    DA_PUSH(arr, 999);
#else
    DA_PUSH(arr, 999, int);
#endif

    TEST_ASSERT_EQUAL_INT(6, DA_LENGTH(arr));
    TEST_ASSERT_EQUAL_INT(999, DA_AT(arr, 5, int));

    // Test that shared reference sees the change
    TEST_ASSERT_EQUAL_INT(6, DA_LENGTH(arr2));
    TEST_ASSERT_EQUAL_INT(999, DA_AT(arr2, 5, int));

    da_release(&arr);
    da_release(&arr2);
}

void test_builder_different_types(void) {
    // Test builder with different data types
    da_builder float_builder = DA_BUILDER_CREATE(float);
    da_builder char_builder = DA_BUILDER_CREATE(char);

    float f_vals[] = {3.14f, 2.71f, 1.41f};
    char c_vals[] = {'A', 'B', 'C'};

    for (int i = 0; i < 3; i++) {
        da_builder_append(float_builder, &f_vals[i]);
        da_builder_append(char_builder, &c_vals[i]);
    }

    da_array float_arr = da_builder_to_array(&float_builder, NULL, NULL);
    da_array char_arr = da_builder_to_array(&char_builder, NULL, NULL);

    // Verify data
    for (int i = 0; i < 3; i++) {
        float* f_ptr = (float*)da_get(float_arr, i);
        char* c_ptr = (char*)da_get(char_arr, i);

        TEST_ASSERT_FLOAT_WITHIN(0.01f, f_vals[i], *f_ptr);
        TEST_ASSERT_EQUAL_INT(c_vals[i], *c_ptr);
    }

    da_release(&float_arr);
    da_release(&char_arr);
}

void test_builder_stress_test(void) {
    da_builder builder = DA_BUILDER_CREATE(int);

    // Add many elements to stress test growth
    const int num_elements = 1000;
    for (int i = 0; i < num_elements; i++) {
#if DA_SUPPORT_TYPE_INFERENCE && !defined(DA_NOT_USE_TYPE_GENERIC)
        DA_BUILDER_APPEND(builder, i);
#else
        DA_BUILDER_APPEND(builder, i, int);
#endif
    }

    TEST_ASSERT_EQUAL_INT(num_elements, DA_BUILDER_LEN(builder));

    // Verify all data
    for (int i = 0; i < num_elements; i++) {
        TEST_ASSERT_EQUAL_INT(i, DA_BUILDER_AT(builder, i, int));
    }

    // Convert to array and verify exact sizing
    da_array arr = da_builder_to_array(&builder, NULL, NULL);

    TEST_ASSERT_EQUAL_INT(num_elements, DA_LENGTH(arr));
    TEST_ASSERT_EQUAL_INT(num_elements, DA_CAPACITY(arr));  // Exact capacity

    // Verify data integrity after conversion
    for (int i = 0; i < num_elements; i++) {
        TEST_ASSERT_EQUAL_INT(i, DA_AT(arr, i, int));
    }

    da_release(&arr);
}

void test_builder_reserve_basic(void) {
    da_builder builder = da_builder_create(sizeof(int));

    // Initially should have 0 capacity
    TEST_ASSERT_EQUAL_INT(0, da_builder_capacity(builder));

    // Reserve capacity for 100 elements
    da_builder_reserve(builder, 100);
    TEST_ASSERT_GREATER_OR_EQUAL_INT(100, da_builder_capacity(builder));
    TEST_ASSERT_EQUAL_INT(0, da_builder_length(builder));

    // Add elements - should not trigger reallocations
    for (int i = 0; i < 50; i++) {
        da_builder_append(builder, &i);
    }

    TEST_ASSERT_EQUAL_INT(50, da_builder_length(builder));
    TEST_ASSERT_GREATER_OR_EQUAL_INT(100, da_builder_capacity(builder));

    // Verify no reallocation occurred by checking capacity hasn't grown
    int capacity_after_50 = da_builder_capacity(builder);

    for (int i = 50; i < 100; i++) {
        da_builder_append(builder, &i);
    }

    TEST_ASSERT_EQUAL_INT(100, da_builder_length(builder));
    TEST_ASSERT_EQUAL_INT(capacity_after_50, da_builder_capacity(builder));

    da_builder_destroy(&builder);
}

void test_builder_reserve_no_shrink(void) {
    da_builder builder = da_builder_create(sizeof(int));

    // Reserve large capacity
    da_builder_reserve(builder, 1000);
    int large_capacity = da_builder_capacity(builder);
    TEST_ASSERT_GREATER_OR_EQUAL_INT(1000, large_capacity);

    // Attempt to "reserve" smaller capacity - should not shrink
    da_builder_reserve(builder, 10);
    TEST_ASSERT_EQUAL_INT(large_capacity, da_builder_capacity(builder));

    da_builder_destroy(&builder);
}

void test_builder_append_array_basic(void) {
    da_builder builder = da_builder_create(sizeof(int));

    // Create source array with data
    da_array source = da_new(sizeof(int));
    int values[] = {10, 20, 30};
    for (int i = 0; i < 3; i++) {
        da_push(source, &values[i]);
    }

    // Append array to builder
    da_builder_append_array(builder, source);

    TEST_ASSERT_EQUAL_INT(3, da_builder_length(builder));

    // Verify all elements were copied correctly
    for (int i = 0; i < 3; i++) {
        TEST_ASSERT_EQUAL_INT(values[i], DA_BUILDER_AT(builder, i, int));
    }

    da_release(&source);
    da_builder_destroy(&builder);
}

void test_builder_append_array_empty(void) {
    da_builder builder = da_builder_create(sizeof(int));

    // Create empty source array
    da_array source = da_new(sizeof(int));

    // Append empty array to builder - should be no-op
    da_builder_append_array(builder, source);

    TEST_ASSERT_EQUAL_INT(0, da_builder_length(builder));

    da_release(&source);
    da_builder_destroy(&builder);
}

void test_builder_append_array_multiple(void) {
    da_builder builder = da_builder_create(sizeof(int));

    // First array: [1, 2, 3]
    da_array arr1 = da_new(sizeof(int));
    for (int i = 1; i <= 3; i++) {
        da_push(arr1, &i);
    }

    // Second array: [4, 5]
    da_array arr2 = da_new(sizeof(int));
    for (int i = 4; i <= 5; i++) {
        da_push(arr2, &i);
    }

    // Append both arrays
    da_builder_append_array(builder, arr1);
    da_builder_append_array(builder, arr2);

    // Should have [1, 2, 3, 4, 5]
    TEST_ASSERT_EQUAL_INT(5, da_builder_length(builder));

    for (int i = 0; i < 5; i++) {
        TEST_ASSERT_EQUAL_INT(i + 1, DA_BUILDER_AT(builder, i, int));
    }

    da_release(&arr1);
    da_release(&arr2);
    da_builder_destroy(&builder);
}

void test_builder_append_array_with_existing_data(void) {
    da_builder builder = da_builder_create(sizeof(int));

    // Add some initial data
    int initial[] = {100, 200};
    for (int i = 0; i < 2; i++) {
        da_builder_append(builder, &initial[i]);
    }

    // Create array to append
    da_array source = da_new(sizeof(int));
    int values[] = {300, 400};
    for (int i = 0; i < 2; i++) {
        da_push(source, &values[i]);
    }

    // Append array to existing builder data
    da_builder_append_array(builder, source);

    // Should have [100, 200, 300, 400]
    TEST_ASSERT_EQUAL_INT(4, da_builder_length(builder));
    TEST_ASSERT_EQUAL_INT(100, DA_BUILDER_AT(builder, 0, int));
    TEST_ASSERT_EQUAL_INT(200, DA_BUILDER_AT(builder, 1, int));
    TEST_ASSERT_EQUAL_INT(300, DA_BUILDER_AT(builder, 2, int));
    TEST_ASSERT_EQUAL_INT(400, DA_BUILDER_AT(builder, 3, int));

    da_release(&source);
    da_builder_destroy(&builder);
}

void test_builder_reserve_and_append_array_efficiency(void) {
    da_builder builder = da_builder_create(sizeof(int));

    // Create large array
    da_array large_array = da_new(sizeof(int));
    for (int i = 0; i < 1000; i++) {
        da_push(large_array, &i);
    }

    // Pre-reserve exact space needed
    da_builder_reserve(builder, 1000);
    int reserved_capacity = da_builder_capacity(builder);

    // Append the large array
    da_builder_append_array(builder, large_array);

    // Capacity should not have grown beyond reserved amount
    TEST_ASSERT_EQUAL_INT(reserved_capacity, da_builder_capacity(builder));
    TEST_ASSERT_EQUAL_INT(1000, da_builder_length(builder));

    // Verify all data is correct
    for (int i = 0; i < 1000; i++) {
        TEST_ASSERT_EQUAL_INT(i, DA_BUILDER_AT(builder, i, int));
    }

    da_release(&large_array);
    da_builder_destroy(&builder);
}

/* Peek Operations Tests */
void test_peek_basic(void) {
    da_array arr = da_new(sizeof(int));

    int vals[] = {10, 20, 30};
    for (int i = 0; i < 3; i++) {
        da_push(arr, &vals[i]);
    }

    // Test peek (last element)
    int* last_ptr = (int*)da_peek(arr);
    TEST_ASSERT_EQUAL_INT(30, *last_ptr);
    TEST_ASSERT_EQUAL_INT(3, da_length(arr));  // Length unchanged

    // Test peek_first
    int* first_ptr = (int*)da_peek_first(arr);
    TEST_ASSERT_EQUAL_INT(10, *first_ptr);
    TEST_ASSERT_EQUAL_INT(3, da_length(arr));  // Length unchanged

    da_release(&arr);
}

void test_peek_macros(void) {
    da_array arr = da_new(sizeof(int));

    int vals[] = {42, 99};
    for (int i = 0; i < 2; i++) {
        da_push(arr, &vals[i]);
    }

    // Test typed peek macros
    TEST_ASSERT_EQUAL_INT(99, DA_PEEK(arr, int));      // Last element
    TEST_ASSERT_EQUAL_INT(42, DA_PEEK_FIRST(arr, int)); // First element
    TEST_ASSERT_EQUAL_INT(2, da_length(arr));          // Length unchanged

    da_release(&arr);
}

void test_peek_single_element(void) {
    da_array arr = da_new(sizeof(int));

    int val = 123;
    da_push(arr, &val);

    // Both peek operations should return the same element
    int* last = (int*)da_peek(arr);
    int* first = (int*)da_peek_first(arr);

    TEST_ASSERT_EQUAL_INT(123, *last);
    TEST_ASSERT_EQUAL_INT(123, *first);
    TEST_ASSERT_EQUAL_PTR(first, last);  // Should be same pointer

    da_release(&arr);
}

/* Bulk Operations Tests */
void test_append_raw_basic(void) {
    da_array arr = da_new(sizeof(int));

    // Add initial elements
    int initial[] = {10, 20};
    for (int i = 0; i < 2; i++) {
        da_push(arr, &initial[i]);
    }

    // Append raw array
    int raw_data[] = {30, 40, 50, 60};
    da_append_raw(arr, raw_data, 4);

    TEST_ASSERT_EQUAL_INT(6, da_length(arr));
    TEST_ASSERT_EQUAL_INT(10, DA_AT(arr, 0, int));
    TEST_ASSERT_EQUAL_INT(20, DA_AT(arr, 1, int));
    TEST_ASSERT_EQUAL_INT(30, DA_AT(arr, 2, int));
    TEST_ASSERT_EQUAL_INT(40, DA_AT(arr, 3, int));
    TEST_ASSERT_EQUAL_INT(50, DA_AT(arr, 4, int));
    TEST_ASSERT_EQUAL_INT(60, DA_AT(arr, 5, int));

    da_release(&arr);
}

void test_append_raw_empty(void) {
    da_array arr = da_new(sizeof(int));

    int val = 42;
    da_push(arr, &val);

    // Append zero elements - should do nothing
    int dummy[] = {1, 2, 3};
    da_append_raw(arr, dummy, 0);

    TEST_ASSERT_EQUAL_INT(1, da_length(arr));
    TEST_ASSERT_EQUAL_INT(42, DA_AT(arr, 0, int));

    da_release(&arr);
}

void test_append_raw_with_growth(void) {
    da_array arr = da_new(sizeof(int));

    // Fill to capacity
    int vals[] = {10, 20};
    for (int i = 0; i < 2; i++) {
        da_push(arr, &vals[i]);
    }

    TEST_ASSERT_EQUAL_INT(2, da_capacity(arr));

    // Append more than remaining capacity
    int raw_data[] = {30, 40, 50, 60, 70};
    da_append_raw(arr, raw_data, 5);

    TEST_ASSERT_TRUE(da_capacity(arr) >= 7);
    TEST_ASSERT_EQUAL_INT(7, da_length(arr));

    // Verify all data
    int expected[] = {10, 20, 30, 40, 50, 60, 70};
    for (int i = 0; i < 7; i++) {
        TEST_ASSERT_EQUAL_INT(expected[i], DA_AT(arr, i, int));
    }

    da_release(&arr);
}

void test_fill_basic(void) {
    da_array arr = da_new(sizeof(int));

    // Add initial element
    int initial = 99;
    da_push(arr, &initial);

    // Fill with zeros
    int zero = 0;
    da_fill(arr, &zero, 5);

    TEST_ASSERT_EQUAL_INT(6, da_length(arr));
    TEST_ASSERT_EQUAL_INT(99, DA_AT(arr, 0, int));
    for (int i = 1; i < 6; i++) {
        TEST_ASSERT_EQUAL_INT(0, DA_AT(arr, i, int));
    }

    da_release(&arr);
}

void test_fill_empty_count(void) {
    da_array arr = da_new(sizeof(int));

    int val = 42;
    da_push(arr, &val);

    // Fill with zero count - should do nothing
    int dummy = 123;
    da_fill(arr, &dummy, 0);

    TEST_ASSERT_EQUAL_INT(1, da_length(arr));
    TEST_ASSERT_EQUAL_INT(42, DA_AT(arr, 0, int));

    da_release(&arr);
}

void test_fill_with_growth(void) {
    da_array arr = da_create(sizeof(int), 2, NULL, NULL);

    TEST_ASSERT_EQUAL_INT(0, da_length(arr));
    TEST_ASSERT_EQUAL_INT(2, da_capacity(arr));

    // Fill more than capacity
    int pattern = 777;
    da_fill(arr, &pattern, 10);

    TEST_ASSERT_TRUE(da_capacity(arr) >= 10);
    TEST_ASSERT_EQUAL_INT(10, da_length(arr));

    // Verify all filled elements
    for (int i = 0; i < 10; i++) {
        TEST_ASSERT_EQUAL_INT(777, DA_AT(arr, i, int));
    }

    da_release(&arr);
}

/* Range Operations Tests */
void test_slice_basic(void) {
    da_array arr = da_new(sizeof(int));

    // Setup array: [10, 20, 30, 40, 50]
    int vals[] = {10, 20, 30, 40, 50};
    for (int i = 0; i < 5; i++) {
        da_push(arr, &vals[i]);
    }

    // Slice [1, 4) -> [20, 30, 40]
    da_array slice = da_slice(arr, 1, 4);

    TEST_ASSERT_EQUAL_INT(3, da_length(slice));
    TEST_ASSERT_EQUAL_INT(3, da_capacity(slice));  // Exact capacity
    TEST_ASSERT_EQUAL_INT(20, DA_AT(slice, 0, int));
    TEST_ASSERT_EQUAL_INT(30, DA_AT(slice, 1, int));
    TEST_ASSERT_EQUAL_INT(40, DA_AT(slice, 2, int));

    // Original array unchanged
    TEST_ASSERT_EQUAL_INT(5, da_length(arr));
    TEST_ASSERT_EQUAL_INT(10, DA_AT(arr, 0, int));

    da_release(&slice);
    da_release(&arr);
}

void test_slice_empty_range(void) {
    da_array arr = da_new(sizeof(int));

    int vals[] = {10, 20, 30};
    for (int i = 0; i < 3; i++) {
        da_push(arr, &vals[i]);
    }

    // Empty slice [1, 1)
    da_array slice = da_slice(arr, 1, 1);

    TEST_ASSERT_EQUAL_INT(0, da_length(slice));
    TEST_ASSERT_EQUAL_INT(0, da_capacity(slice));
    TEST_ASSERT_NULL(da_data(slice));

    da_release(&slice);
    da_release(&arr);
}

void test_slice_full_array(void) {
    da_array arr = da_new(sizeof(int));

    int vals[] = {42, 99, 123};
    for (int i = 0; i < 3; i++) {
        da_push(arr, &vals[i]);
    }

    // Full slice [0, 3)
    da_array slice = da_slice(arr, 0, 3);

    TEST_ASSERT_EQUAL_INT(3, da_length(slice));
    TEST_ASSERT_EQUAL_INT(3, da_capacity(slice));

    // Should be identical content
    for (int i = 0; i < 3; i++) {
        TEST_ASSERT_EQUAL_INT(vals[i], DA_AT(slice, i, int));
    }

    da_release(&slice);
    da_release(&arr);
}

void test_remove_range_basic(void) {
    da_array arr = da_new(sizeof(int));

    // Setup array: [10, 20, 30, 40, 50, 60]
    int vals[] = {10, 20, 30, 40, 50, 60};
    for (int i = 0; i < 6; i++) {
        da_push(arr, &vals[i]);
    }

    // Remove range [2, 4) -> remove 30, 40
    da_remove_range(arr, 2, 2);

    TEST_ASSERT_EQUAL_INT(4, da_length(arr));
    TEST_ASSERT_EQUAL_INT(10, DA_AT(arr, 0, int));
    TEST_ASSERT_EQUAL_INT(20, DA_AT(arr, 1, int));
    TEST_ASSERT_EQUAL_INT(50, DA_AT(arr, 2, int));  // 50 moved left
    TEST_ASSERT_EQUAL_INT(60, DA_AT(arr, 3, int));  // 60 moved left

    da_release(&arr);
}

void test_remove_range_empty(void) {
    da_array arr = da_new(sizeof(int));

    int vals[] = {10, 20, 30};
    for (int i = 0; i < 3; i++) {
        da_push(arr, &vals[i]);
    }

    // Remove zero elements - should do nothing
    da_remove_range(arr, 1, 0);

    TEST_ASSERT_EQUAL_INT(3, da_length(arr));
    TEST_ASSERT_EQUAL_INT(10, DA_AT(arr, 0, int));
    TEST_ASSERT_EQUAL_INT(20, DA_AT(arr, 1, int));
    TEST_ASSERT_EQUAL_INT(30, DA_AT(arr, 2, int));

    da_release(&arr);
}

void test_remove_range_from_end(void) {
    da_array arr = da_new(sizeof(int));

    int vals[] = {10, 20, 30, 40, 50};
    for (int i = 0; i < 5; i++) {
        da_push(arr, &vals[i]);
    }

    // Remove last 2 elements
    da_remove_range(arr, 3, 2);

    TEST_ASSERT_EQUAL_INT(3, da_length(arr));
    TEST_ASSERT_EQUAL_INT(10, DA_AT(arr, 0, int));
    TEST_ASSERT_EQUAL_INT(20, DA_AT(arr, 1, int));
    TEST_ASSERT_EQUAL_INT(30, DA_AT(arr, 2, int));

    da_release(&arr);
}

/* Utility Operations Tests */
void test_reverse_basic(void) {
    da_array arr = da_new(sizeof(int));

    // Setup array: [10, 20, 30, 40, 50]
    int vals[] = {10, 20, 30, 40, 50};
    for (int i = 0; i < 5; i++) {
        da_push(arr, &vals[i]);
    }

    da_reverse(arr);

    // Should now be: [50, 40, 30, 20, 10]
    TEST_ASSERT_EQUAL_INT(5, da_length(arr));
    TEST_ASSERT_EQUAL_INT(50, DA_AT(arr, 0, int));
    TEST_ASSERT_EQUAL_INT(40, DA_AT(arr, 1, int));
    TEST_ASSERT_EQUAL_INT(30, DA_AT(arr, 2, int));
    TEST_ASSERT_EQUAL_INT(20, DA_AT(arr, 3, int));
    TEST_ASSERT_EQUAL_INT(10, DA_AT(arr, 4, int));

    da_release(&arr);
}

void test_reverse_even_length(void) {
    da_array arr = da_new(sizeof(int));

    int vals[] = {1, 2, 3, 4};
    for (int i = 0; i < 4; i++) {
        da_push(arr, &vals[i]);
    }

    da_reverse(arr);

    // Should be: [4, 3, 2, 1]
    TEST_ASSERT_EQUAL_INT(4, DA_AT(arr, 0, int));
    TEST_ASSERT_EQUAL_INT(3, DA_AT(arr, 1, int));
    TEST_ASSERT_EQUAL_INT(2, DA_AT(arr, 2, int));
    TEST_ASSERT_EQUAL_INT(1, DA_AT(arr, 3, int));

    da_release(&arr);
}

void test_reverse_single_element(void) {
    da_array arr = da_new(sizeof(int));

    int val = 42;
    da_push(arr, &val);

    da_reverse(arr);

    // Should be unchanged
    TEST_ASSERT_EQUAL_INT(1, da_length(arr));
    TEST_ASSERT_EQUAL_INT(42, DA_AT(arr, 0, int));

    da_release(&arr);
}

void test_reverse_empty(void) {
    da_array arr = da_new(sizeof(int));

    // Empty array
    da_reverse(arr);

    // Should remain empty
    TEST_ASSERT_EQUAL_INT(0, da_length(arr));

    da_release(&arr);
}

void test_swap_basic(void) {
    da_array arr = da_new(sizeof(int));

    int vals[] = {10, 20, 30, 40, 50};
    for (int i = 0; i < 5; i++) {
        da_push(arr, &vals[i]);
    }

    // Swap first and last
    da_swap(arr, 0, 4);

    TEST_ASSERT_EQUAL_INT(50, DA_AT(arr, 0, int));
    TEST_ASSERT_EQUAL_INT(20, DA_AT(arr, 1, int));  // Unchanged
    TEST_ASSERT_EQUAL_INT(30, DA_AT(arr, 2, int));  // Unchanged
    TEST_ASSERT_EQUAL_INT(40, DA_AT(arr, 3, int));  // Unchanged
    TEST_ASSERT_EQUAL_INT(10, DA_AT(arr, 4, int));

    da_release(&arr);
}

void test_swap_same_index(void) {
    da_array arr = da_new(sizeof(int));

    int vals[] = {10, 20, 30};
    for (int i = 0; i < 3; i++) {
        da_push(arr, &vals[i]);
    }

    // Swap element with itself - should be no-op
    da_swap(arr, 1, 1);

    TEST_ASSERT_EQUAL_INT(10, DA_AT(arr, 0, int));
    TEST_ASSERT_EQUAL_INT(20, DA_AT(arr, 1, int));  // Unchanged
    TEST_ASSERT_EQUAL_INT(30, DA_AT(arr, 2, int));

    da_release(&arr);
}

void test_swap_adjacent(void) {
    da_array arr = da_new(sizeof(int));

    int vals[] = {100, 200, 300};
    for (int i = 0; i < 3; i++) {
        da_push(arr, &vals[i]);
    }

    // Swap adjacent elements
    da_swap(arr, 0, 1);

    TEST_ASSERT_EQUAL_INT(200, DA_AT(arr, 0, int));
    TEST_ASSERT_EQUAL_INT(100, DA_AT(arr, 1, int));
    TEST_ASSERT_EQUAL_INT(300, DA_AT(arr, 2, int));  // Unchanged

    da_release(&arr);
}

void test_is_empty_basic(void) {
    da_array arr = da_new(sizeof(int));

    // Initially empty
    TEST_ASSERT_EQUAL_INT(1, da_is_empty(arr));

    // Add element
    int val = 42;
    da_push(arr, &val);
    TEST_ASSERT_EQUAL_INT(0, da_is_empty(arr));

    // Remove element
    da_pop(arr, NULL);
    TEST_ASSERT_EQUAL_INT(1, da_is_empty(arr));

    da_release(&arr);
}

void test_is_empty_after_clear(void) {
    da_array arr = da_new(sizeof(int));

    // Add elements
    for (int i = 0; i < 3; i++) {
        da_push(arr, &i);
    }
    TEST_ASSERT_EQUAL_INT(0, da_is_empty(arr));

    // Clear and check
    da_clear(arr);
    TEST_ASSERT_EQUAL_INT(1, da_is_empty(arr));

    da_release(&arr);
}

/* Copy Operations Tests */
void test_copy_basic(void) {
    da_array original = da_new(sizeof(int));

    // Add some elements to original
    int vals[] = {10, 20, 30, 40};
    for (int i = 0; i < 4; i++) {
        da_push(original, &vals[i]);
    }

    // Create copy
    da_array copy = da_copy(original);

    // Verify copy has same data but different identity
    TEST_ASSERT_NOT_EQUAL(original, copy);
    TEST_ASSERT_EQUAL_INT(4, da_length(copy));
    TEST_ASSERT_EQUAL_INT(4, da_capacity(copy));  // Exact capacity
    TEST_ASSERT_EQUAL_INT(1, DA_ATOMIC_LOAD(&copy->ref_count));  // New ref count

    // Verify data integrity
    for (int i = 0; i < 4; i++) {
        TEST_ASSERT_EQUAL_INT(vals[i], DA_AT(copy, i, int));
    }

    // Verify independence - modify original
    int new_val = 99;
    da_push(original, &new_val);
    TEST_ASSERT_EQUAL_INT(5, da_length(original));
    TEST_ASSERT_EQUAL_INT(4, da_length(copy));  // Copy unchanged

    // Verify independence - modify copy
    DA_PUT(copy, 0, 123);
    TEST_ASSERT_EQUAL_INT(123, DA_AT(copy, 0, int));
    TEST_ASSERT_EQUAL_INT(10, DA_AT(original, 0, int));  // Original unchanged

    da_release(&original);
    da_release(&copy);
}

void test_copy_empty_array(void) {
    da_array original = da_new(sizeof(int));

    // Copy empty array
    da_array copy = da_copy(original);

    TEST_ASSERT_NOT_EQUAL(original, copy);
    TEST_ASSERT_EQUAL_INT(0, da_length(copy));
    TEST_ASSERT_EQUAL_INT(0, da_capacity(copy));  // Exact capacity (zero)
    TEST_ASSERT_NULL(da_data(copy));  // No data allocated
    TEST_ASSERT_EQUAL_INT(1, DA_ATOMIC_LOAD(&copy->ref_count));

    // Verify independence - add to original
    int val = 42;
    da_push(original, &val);
    TEST_ASSERT_EQUAL_INT(1, da_length(original));
    TEST_ASSERT_EQUAL_INT(0, da_length(copy));  // Copy remains empty

    da_release(&original);
    da_release(&copy);
}

void test_copy_single_element(void) {
    da_array original = da_new(sizeof(int));

    int val = 42;
    da_push(original, &val);

    da_array copy = da_copy(original);

    TEST_ASSERT_NOT_EQUAL(original, copy);
    TEST_ASSERT_EQUAL_INT(1, da_length(copy));
    TEST_ASSERT_EQUAL_INT(1, da_capacity(copy));  // Exact capacity
    TEST_ASSERT_EQUAL_INT(42, DA_AT(copy, 0, int));

    // Verify different data pointers
    TEST_ASSERT_NOT_EQUAL(da_data(original), da_data(copy));

    da_release(&original);
    da_release(&copy);
}

void test_copy_exact_capacity(void) {
    da_array original = da_create(sizeof(int), 100, NULL, NULL);  // Large capacity

    // Add only a few elements
    for (int i = 0; i < 10; i++) {
        da_push(original, &i);
    }

    TEST_ASSERT_EQUAL_INT(10, da_length(original));
    TEST_ASSERT_EQUAL_INT(100, da_capacity(original));  // Excess capacity

    da_array copy = da_copy(original);

    // Copy should have exact capacity = length
    TEST_ASSERT_EQUAL_INT(10, da_length(copy));
    TEST_ASSERT_EQUAL_INT(10, da_capacity(copy));  // No wasted space

    // Verify data integrity
    for (int i = 0; i < 10; i++) {
        TEST_ASSERT_EQUAL_INT(i, DA_AT(copy, i, int));
    }

    da_release(&original);
    da_release(&copy);
}

void test_copy_different_types(void) {
    // Test copying arrays of different types
    da_array float_arr = da_new(sizeof(float));
    da_array char_arr = da_new(sizeof(char));

    float f_vals[] = {3.14f, 2.71f, 1.41f};
    char c_vals[] = {'A', 'B', 'C'};

    for (int i = 0; i < 3; i++) {
        da_push(float_arr, &f_vals[i]);
        da_push(char_arr, &c_vals[i]);
    }

    da_array float_copy = da_copy(float_arr);
    da_array char_copy = da_copy(char_arr);

    // Verify float copy
    TEST_ASSERT_EQUAL_INT(3, da_length(float_copy));
    TEST_ASSERT_EQUAL_INT(3, da_capacity(float_copy));
    for (int i = 0; i < 3; i++) {
        float* ptr = (float*)da_get(float_copy, i);
        TEST_ASSERT_FLOAT_WITHIN(0.01f, f_vals[i], *ptr);
    }

    // Verify char copy
    TEST_ASSERT_EQUAL_INT(3, da_length(char_copy));
    TEST_ASSERT_EQUAL_INT(3, da_capacity(char_copy));
    for (int i = 0; i < 3; i++) {
        char* ptr = (char*)da_get(char_copy, i);
        TEST_ASSERT_EQUAL_INT(c_vals[i], *ptr);
    }

    da_release(&float_arr);
    da_release(&char_arr);
    da_release(&float_copy);
    da_release(&char_copy);
}

void test_copy_independence(void) {
    da_array original = da_new(sizeof(int));

    // Setup original array: [10, 20, 30]
    int vals[] = {10, 20, 30};
    for (int i = 0; i < 3; i++) {
        da_push(original, &vals[i]);
    }

    da_array copy = da_copy(original);

    // Extensive independence testing

    // 1. Push to original - copy unaffected
    int new_val = 40;
    da_push(original, &new_val);
    TEST_ASSERT_EQUAL_INT(4, da_length(original));
    TEST_ASSERT_EQUAL_INT(3, da_length(copy));

    // 2. Push to copy - original unaffected
    int copy_val = 99;
    da_push(copy, &copy_val);
    TEST_ASSERT_EQUAL_INT(4, da_length(original));
    TEST_ASSERT_EQUAL_INT(4, da_length(copy));
    TEST_ASSERT_EQUAL_INT(40, DA_AT(original, 3, int));
    TEST_ASSERT_EQUAL_INT(99, DA_AT(copy, 3, int));

    // 3. Modify elements in original - copy unaffected
    DA_PUT(original, 0, 777);
    TEST_ASSERT_EQUAL_INT(777, DA_AT(original, 0, int));
    TEST_ASSERT_EQUAL_INT(10, DA_AT(copy, 0, int));

    // 4. Clear original - copy unaffected
    da_clear(original);
    TEST_ASSERT_EQUAL_INT(0, da_length(original));
    TEST_ASSERT_EQUAL_INT(4, da_length(copy));
    TEST_ASSERT_EQUAL_INT(10, DA_AT(copy, 0, int));  // Still has original data

    da_release(&original);
    da_release(&copy);
}

void test_copy_sorting_scenario(void) {
    // Test the main use case: copying for sorting without affecting original
    da_array numbers = da_new(sizeof(int));

    // Add unsorted numbers
    int vals[] = {50, 20, 80, 10, 30};
    for (int i = 0; i < 5; i++) {
        da_push(numbers, &vals[i]);
    }

    // Create copy for sorting
    da_array sorted_copy = da_copy(numbers);

    // Simple bubble sort on copy
    for (int i = 0; i < da_length(sorted_copy) - 1; i++) {
        for (int j = 0; j < da_length(sorted_copy) - i - 1; j++) {
            int a = DA_AT(sorted_copy, j, int);
            int b = DA_AT(sorted_copy, j + 1, int);
            if (a > b) {
                da_swap(sorted_copy, j, j + 1);
            }
        }
    }

    // Verify original is unchanged
    TEST_ASSERT_EQUAL_INT(50, DA_AT(numbers, 0, int));
    TEST_ASSERT_EQUAL_INT(20, DA_AT(numbers, 1, int));
    TEST_ASSERT_EQUAL_INT(80, DA_AT(numbers, 2, int));
    TEST_ASSERT_EQUAL_INT(10, DA_AT(numbers, 3, int));
    TEST_ASSERT_EQUAL_INT(30, DA_AT(numbers, 4, int));

    // Verify copy is sorted
    TEST_ASSERT_EQUAL_INT(10, DA_AT(sorted_copy, 0, int));
    TEST_ASSERT_EQUAL_INT(20, DA_AT(sorted_copy, 1, int));
    TEST_ASSERT_EQUAL_INT(30, DA_AT(sorted_copy, 2, int));
    TEST_ASSERT_EQUAL_INT(50, DA_AT(sorted_copy, 3, int));
    TEST_ASSERT_EQUAL_INT(80, DA_AT(sorted_copy, 4, int));

    da_release(&numbers);
    da_release(&sorted_copy);
}

void test_copy_reference_counting(void) {
    da_array original = da_new(sizeof(int));

    int vals[] = {10, 20, 30};
    for (int i = 0; i < 3; i++) {
        da_push(original, &vals[i]);
    }

    // Create copy
    da_array copy = da_copy(original);

    // Both should have ref_count = 1
    TEST_ASSERT_EQUAL_INT(1, DA_ATOMIC_LOAD(&original->ref_count));
    TEST_ASSERT_EQUAL_INT(1, DA_ATOMIC_LOAD(&copy->ref_count));

    // Retain original - should not affect copy
    da_array original_ref = da_retain(original);
    TEST_ASSERT_EQUAL_INT(2, DA_ATOMIC_LOAD(&original->ref_count));
    TEST_ASSERT_EQUAL_INT(1, DA_ATOMIC_LOAD(&copy->ref_count));

    // Retain copy - should not affect original
    da_array copy_ref = da_retain(copy);
    TEST_ASSERT_EQUAL_INT(2, DA_ATOMIC_LOAD(&original->ref_count));
    TEST_ASSERT_EQUAL_INT(2, DA_ATOMIC_LOAD(&copy->ref_count));

    // Release references
    da_release(&original_ref);
    da_release(&copy_ref);
    TEST_ASSERT_EQUAL_INT(1, DA_ATOMIC_LOAD(&original->ref_count));
    TEST_ASSERT_EQUAL_INT(1, DA_ATOMIC_LOAD(&copy->ref_count));

    da_release(&original);
    da_release(&copy);
}

/* Filter Operations Tests */
// Helper predicate functions for testing
int is_even(const void* elem, void* ctx) {
    (void)ctx; // Suppress unused parameter warning
    return *(int*)elem % 2 == 0;
}

int is_positive(const void* elem, void* ctx) {
    (void)ctx; // Suppress unused parameter warning
    return *(int*)elem > 0;
}

int greater_than_threshold(const void* elem, void* ctx) {
    int threshold = *(int*)ctx;
    return *(int*)elem > threshold;
}

void test_filter_basic(void) {
    da_array numbers = da_new(sizeof(int));

    // Add mixed numbers: [1, 2, 3, 4, 5]
    for (int i = 1; i <= 5; i++) {
        da_push(numbers, &i);
    }

    // Filter even numbers
    da_array evens = da_filter(numbers, is_even, NULL);

    TEST_ASSERT_EQUAL_INT(2, da_length(evens));  // [2, 4]
    TEST_ASSERT_EQUAL_INT(2, da_capacity(evens)); // Exact capacity
    TEST_ASSERT_EQUAL_INT(1, DA_ATOMIC_LOAD(&evens->ref_count));

    // Verify filtered data
    TEST_ASSERT_EQUAL_INT(2, DA_AT(evens, 0, int));
    TEST_ASSERT_EQUAL_INT(4, DA_AT(evens, 1, int));

    // Original unchanged
    TEST_ASSERT_EQUAL_INT(5, da_length(numbers));
    TEST_ASSERT_EQUAL_INT(1, DA_AT(numbers, 0, int));

    da_release(&numbers);
    da_release(&evens);
}

void test_filter_empty_result(void) {
    da_array numbers = da_new(sizeof(int));

    // Add negative numbers: [-1, -2, -3]
    int vals[] = {-1, -2, -3};
    for (int i = 0; i < 3; i++) {
        da_push(numbers, &vals[i]);
    }

    // Filter positive numbers (should get empty result)
    da_array positives = da_filter(numbers, is_positive, NULL);

    TEST_ASSERT_EQUAL_INT(0, da_length(positives));
    TEST_ASSERT_EQUAL_INT(0, da_capacity(positives)); // Exact capacity = 0
    TEST_ASSERT_NULL(da_data(positives));
    TEST_ASSERT_EQUAL_INT(1, DA_ATOMIC_LOAD(&positives->ref_count));

    da_release(&numbers);
    da_release(&positives);
}

void test_filter_all_match(void) {
    da_array numbers = da_new(sizeof(int));

    // Add positive numbers: [1, 2, 3, 4]
    for (int i = 1; i <= 4; i++) {
        da_push(numbers, &i);
    }

    // Filter positive numbers (all should match)
    da_array positives = da_filter(numbers, is_positive, NULL);

    TEST_ASSERT_EQUAL_INT(4, da_length(positives));
    TEST_ASSERT_EQUAL_INT(4, da_capacity(positives)); // Exact capacity

    // Verify all data copied correctly
    for (int i = 0; i < 4; i++) {
        TEST_ASSERT_EQUAL_INT(i + 1, DA_AT(positives, i, int));
    }

    da_release(&numbers);
    da_release(&positives);
}

void test_filter_with_context(void) {
    da_array numbers = da_new(sizeof(int));

    // Add numbers: [1, 5, 10, 15, 20, 25]
    int vals[] = {1, 5, 10, 15, 20, 25};
    for (int i = 0; i < 6; i++) {
        da_push(numbers, &vals[i]);
    }

    // Filter numbers > 10 using context
    int threshold = 10;
    da_array filtered = da_filter(numbers, greater_than_threshold, &threshold);

    TEST_ASSERT_EQUAL_INT(3, da_length(filtered));  // [15, 20, 25]
    TEST_ASSERT_EQUAL_INT(3, da_capacity(filtered)); // Exact capacity

    TEST_ASSERT_EQUAL_INT(15, DA_AT(filtered, 0, int));
    TEST_ASSERT_EQUAL_INT(20, DA_AT(filtered, 1, int));
    TEST_ASSERT_EQUAL_INT(25, DA_AT(filtered, 2, int));

    da_release(&numbers);
    da_release(&filtered);
}

void test_filter_empty_source(void) {
    da_array empty = da_new(sizeof(int));

    // Filter empty array
    da_array result = da_filter(empty, is_even, NULL);

    TEST_ASSERT_EQUAL_INT(0, da_length(result));
    TEST_ASSERT_EQUAL_INT(0, da_capacity(result));
    TEST_ASSERT_NULL(da_data(result));

    da_release(&empty);
    da_release(&result);
}

// Helper predicate function for char filtering
int is_uppercase(const void* elem, void* ctx) {
    (void)ctx; // Suppress unused parameter warning
    char c = *(char*)elem;
    return c >= 'A' && c <= 'Z';
}

void test_filter_different_types(void) {
    da_array chars = da_new(sizeof(char));

    // Add characters: ['a', 'B', 'c', 'D', 'e']
    char vals[] = {'a', 'B', 'c', 'D', 'e'};
    for (int i = 0; i < 5; i++) {
        da_push(chars, &vals[i]);
    }

    // Filter uppercase characters
    da_array uppers = da_filter(chars, is_uppercase, NULL);

    TEST_ASSERT_EQUAL_INT(2, da_length(uppers));  // ['B', 'D']
    TEST_ASSERT_EQUAL_INT(2, da_capacity(uppers)); // Exact capacity

    TEST_ASSERT_EQUAL_INT('B', DA_AT(uppers, 0, char));
    TEST_ASSERT_EQUAL_INT('D', DA_AT(uppers, 1, char));

    da_release(&chars);
    da_release(&uppers);
}

void test_filter_independence(void) {
    da_array numbers = da_new(sizeof(int));

    // Add numbers: [1, 2, 3, 4]
    for (int i = 1; i <= 4; i++) {
        da_push(numbers, &i);
    }

    da_array evens = da_filter(numbers, is_even, NULL);

    // Modify original - filtered array unaffected
    int new_val = 99;
    da_push(numbers, &new_val);
    DA_PUT(numbers, 0, 100);

    TEST_ASSERT_EQUAL_INT(5, da_length(numbers));
    TEST_ASSERT_EQUAL_INT(100, DA_AT(numbers, 0, int));
    TEST_ASSERT_EQUAL_INT(99, DA_AT(numbers, 4, int));

    // Filtered array unchanged
    TEST_ASSERT_EQUAL_INT(2, da_length(evens));
    TEST_ASSERT_EQUAL_INT(2, DA_AT(evens, 0, int));
    TEST_ASSERT_EQUAL_INT(4, DA_AT(evens, 1, int));

    // Modify filtered - original unaffected
    DA_PUT(evens, 0, 222);
    TEST_ASSERT_EQUAL_INT(222, DA_AT(evens, 0, int));
    TEST_ASSERT_EQUAL_INT(100, DA_AT(numbers, 0, int)); // Original unchanged

    da_release(&numbers);
    da_release(&evens);
}

/* Map Operations Tests */
// Helper mapper functions for testing
void double_int(const void* src, void* dst, void* ctx) {
    (void)ctx; // Suppress unused parameter warning
    *(int*)dst = *(int*)src * 2;
}

void add_offset(const void* src, void* dst, void* ctx) {
    int offset = *(int*)ctx;
    *(int*)dst = *(int*)src + offset;
}

void negate_int(const void* src, void* dst, void* ctx) {
    (void)ctx; // Suppress unused parameter warning
    *(int*)dst = -(*(int*)src);
}

void test_map_basic(void) {
    da_array numbers = da_new(sizeof(int));

    // Add numbers: [1, 2, 3, 4]
    for (int i = 1; i <= 4; i++) {
        da_push(numbers, &i);
    }

    // Double all values
    da_array doubled = da_map(numbers, double_int, NULL);

    TEST_ASSERT_EQUAL_INT(4, da_length(doubled));  // Same length
    TEST_ASSERT_EQUAL_INT(4, da_capacity(doubled)); // Exact capacity
    TEST_ASSERT_EQUAL_INT(1, DA_ATOMIC_LOAD(&doubled->ref_count));

    // Verify transformed data: [2, 4, 6, 8]
    TEST_ASSERT_EQUAL_INT(2, DA_AT(doubled, 0, int));
    TEST_ASSERT_EQUAL_INT(4, DA_AT(doubled, 1, int));
    TEST_ASSERT_EQUAL_INT(6, DA_AT(doubled, 2, int));
    TEST_ASSERT_EQUAL_INT(8, DA_AT(doubled, 3, int));

    // Original unchanged
    TEST_ASSERT_EQUAL_INT(4, da_length(numbers));
    TEST_ASSERT_EQUAL_INT(1, DA_AT(numbers, 0, int));

    da_release(&numbers);
    da_release(&doubled);
}

void test_map_empty_array(void) {
    da_array empty = da_new(sizeof(int));

    // Map empty array
    da_array result = da_map(empty, double_int, NULL);

    TEST_ASSERT_EQUAL_INT(0, da_length(result));
    TEST_ASSERT_EQUAL_INT(0, da_capacity(result));
    TEST_ASSERT_NULL(da_data(result));
    TEST_ASSERT_EQUAL_INT(1, DA_ATOMIC_LOAD(&result->ref_count));

    da_release(&empty);
    da_release(&result);
}

void test_map_with_context(void) {
    da_array numbers = da_new(sizeof(int));

    // Add numbers: [5, 10, 15]
    int vals[] = {5, 10, 15};
    for (int i = 0; i < 3; i++) {
        da_push(numbers, &vals[i]);
    }

    // Add offset of 100 to all values
    int offset = 100;
    da_array offsetted = da_map(numbers, add_offset, &offset);

    TEST_ASSERT_EQUAL_INT(3, da_length(offsetted));
    TEST_ASSERT_EQUAL_INT(3, da_capacity(offsetted)); // Exact capacity

    // Verify transformed data: [105, 110, 115]
    TEST_ASSERT_EQUAL_INT(105, DA_AT(offsetted, 0, int));
    TEST_ASSERT_EQUAL_INT(110, DA_AT(offsetted, 1, int));
    TEST_ASSERT_EQUAL_INT(115, DA_AT(offsetted, 2, int));

    da_release(&numbers);
    da_release(&offsetted);
}

void test_map_single_element(void) {
    da_array single = da_new(sizeof(int));

    int val = 42;
    da_push(single, &val);

    da_array negated = da_map(single, negate_int, NULL);

    TEST_ASSERT_EQUAL_INT(1, da_length(negated));
    TEST_ASSERT_EQUAL_INT(1, da_capacity(negated)); // Exact capacity
    TEST_ASSERT_EQUAL_INT(-42, DA_AT(negated, 0, int));

    // Original unchanged
    TEST_ASSERT_EQUAL_INT(42, DA_AT(single, 0, int));

    da_release(&single);
    da_release(&negated);
}

// Helper mapper function for squaring floats
void square_float(const void* src, void* dst, void* ctx) {
    (void)ctx; // Suppress unused parameter warning
    float val = *(float*)src;
    *(float*)dst = val * val;
}

void test_map_different_types(void) {
    da_array floats = da_new(sizeof(float));

    // Add float numbers: [1.5, 2.5, 3.5]
    float vals[] = {1.5f, 2.5f, 3.5f};
    for (int i = 0; i < 3; i++) {
        da_push(floats, &vals[i]);
    }

    da_array squared = da_map(floats, square_float, NULL);

    TEST_ASSERT_EQUAL_INT(3, da_length(squared));
    TEST_ASSERT_EQUAL_INT(3, da_capacity(squared)); // Exact capacity

    // Verify squared values: [2.25, 6.25, 12.25]
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 2.25f, DA_AT(squared, 0, float));
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 6.25f, DA_AT(squared, 1, float));
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 12.25f, DA_AT(squared, 2, float));

    da_release(&floats);
    da_release(&squared);
}

void test_map_independence(void) {
    da_array numbers = da_new(sizeof(int));

    // Add numbers: [1, 2, 3]
    for (int i = 1; i <= 3; i++) {
        da_push(numbers, &i);
    }

    da_array doubled = da_map(numbers, double_int, NULL);

    // Modify original - mapped array unaffected
    int new_val = 99;
    da_push(numbers, &new_val);
    DA_PUT(numbers, 0, 100);

    TEST_ASSERT_EQUAL_INT(4, da_length(numbers));
    TEST_ASSERT_EQUAL_INT(100, DA_AT(numbers, 0, int));
    TEST_ASSERT_EQUAL_INT(99, DA_AT(numbers, 3, int));

    // Mapped array unchanged
    TEST_ASSERT_EQUAL_INT(3, da_length(doubled));
    TEST_ASSERT_EQUAL_INT(2, DA_AT(doubled, 0, int));
    TEST_ASSERT_EQUAL_INT(4, DA_AT(doubled, 1, int));
    TEST_ASSERT_EQUAL_INT(6, DA_AT(doubled, 2, int));

    // Modify mapped - original unaffected
    DA_PUT(doubled, 0, 222);
    TEST_ASSERT_EQUAL_INT(222, DA_AT(doubled, 0, int));
    TEST_ASSERT_EQUAL_INT(100, DA_AT(numbers, 0, int)); // Original unchanged

    da_release(&numbers);
    da_release(&doubled);
}

void test_map_chain_operations(void) {
    da_array numbers = da_new(sizeof(int));

    // Add numbers: [1, 2, 3, 4]
    for (int i = 1; i <= 4; i++) {
        da_push(numbers, &i);
    }

    // Chain: double -> filter evens -> add 10
    da_array doubled = da_map(numbers, double_int, NULL);        // [2, 4, 6, 8]
    da_array evens = da_filter(doubled, is_even, NULL);          // [2, 4, 6, 8] (all even)

    int offset = 10;
    da_array final = da_map(evens, add_offset, &offset);         // [12, 14, 16, 18]

    TEST_ASSERT_EQUAL_INT(4, da_length(final));
    TEST_ASSERT_EQUAL_INT(4, da_capacity(final)); // Exact capacity

    TEST_ASSERT_EQUAL_INT(12, DA_AT(final, 0, int));
    TEST_ASSERT_EQUAL_INT(14, DA_AT(final, 1, int));
    TEST_ASSERT_EQUAL_INT(16, DA_AT(final, 2, int));
    TEST_ASSERT_EQUAL_INT(18, DA_AT(final, 3, int));

    // All arrays independent
    TEST_ASSERT_EQUAL_INT(4, da_length(numbers));
    TEST_ASSERT_EQUAL_INT(1, DA_AT(numbers, 0, int)); // Original unchanged

    da_release(&numbers);
    da_release(&doubled);
    da_release(&evens);
    da_release(&final);
}

// Helper functions for da_reduce tests
void sum_ints(void* acc, const void* elem, void* ctx) {
    (void)ctx;
    *(int*)acc += *(int*)elem;
}

void product_ints(void* acc, const void* elem, void* ctx) {
    (void)ctx;
    *(int*)acc *= *(int*)elem;
}

void count_evens(void* acc, const void* elem, void* ctx) {
    (void)ctx;
    if (*(int*)elem % 2 == 0) {
        (*(int*)acc)++;
    }
}

void concat_floats_with_multiplier(void* acc, const void* elem, void* ctx) {
    float multiplier = *(float*)ctx;
    *(float*)acc += (*(float*)elem * multiplier);
}

// Basic da_reduce test - sum all elements
void test_reduce_sum_basic(void) {
    da_array numbers = da_new(sizeof(int));

    // Add numbers: [1, 2, 3, 4, 5]
    for (int i = 1; i <= 5; i++) {
        da_push(numbers, &i);
    }

    int initial = 0;
    int result;
    da_reduce(numbers, &initial, &result, sum_ints, NULL);

    TEST_ASSERT_EQUAL(15, result);  // 1+2+3+4+5 = 15

    da_release(&numbers);
}

// Test da_reduce with product operation
void test_reduce_product(void) {
    da_array numbers = da_new(sizeof(int));

    // Add numbers: [2, 3, 4, 5]
    for (int i = 2; i <= 5; i++) {
        da_push(numbers, &i);
    }

    int initial = 1;
    int result;
    da_reduce(numbers, &initial, &result, product_ints, NULL);

    TEST_ASSERT_EQUAL(120, result);  // 2*3*4*5 = 120

    da_release(&numbers);
}

// Test da_reduce with empty array
void test_reduce_empty_array(void) {
    da_array numbers = da_new(sizeof(int));

    int initial = 42;
    int result;
    da_reduce(numbers, &initial, &result, sum_ints, NULL);

    TEST_ASSERT_EQUAL(42, result);  // Should return initial value

    da_release(&numbers);
}

// Test da_reduce with single element
void test_reduce_single_element(void) {
    da_array numbers = da_new(sizeof(int));

    int value = 99;
    da_push(numbers, &value);

    int initial = 0;
    int result;
    da_reduce(numbers, &initial, &result, sum_ints, NULL);

    TEST_ASSERT_EQUAL(99, result);  // 0 + 99 = 99

    da_release(&numbers);
}

// Test da_reduce with custom context
void test_reduce_with_context(void) {
    da_array numbers = da_new(sizeof(float));

    // Add floats: [1.0, 2.0, 3.0]
    float vals[] = {1.0f, 2.0f, 3.0f};
    for (int i = 0; i < 3; i++) {
        da_push(numbers, &vals[i]);
    }

    float initial = 0.0f;
    float result;
    float multiplier = 2.0f;  // Context: multiply each element by 2 before adding
    da_reduce(numbers, &initial, &result, concat_floats_with_multiplier, &multiplier);

    TEST_ASSERT_FLOAT_WITHIN(0.001f, 12.0f, result);  // (1*2) + (2*2) + (3*2) = 12

    da_release(&numbers);
}

// Test da_reduce with counting operation
void test_reduce_count_matching(void) {
    da_array numbers = da_new(sizeof(int));

    // Add mixed numbers: [1, 2, 3, 4, 5, 6]
    for (int i = 1; i <= 6; i++) {
        da_push(numbers, &i);
    }

    int initial = 0;
    int result;
    da_reduce(numbers, &initial, &result, count_evens, NULL);

    TEST_ASSERT_EQUAL(3, result);  // 2, 4, 6 are even

    da_release(&numbers);
}

// Test da_reduce with result same as initial reference
void test_reduce_accumulator_is_result(void) {
    da_array numbers = da_new(sizeof(int));

    // Add numbers: [10, 20, 30]
    int vals[] = {10, 20, 30};
    for (int i = 0; i < 3; i++) {
        da_push(numbers, &vals[i]);
    }

    int accumulator = 5;  // Initial value
    da_reduce(numbers, &accumulator, &accumulator, sum_ints, NULL);

    TEST_ASSERT_EQUAL(65, accumulator);  // 5 + 10 + 20 + 30 = 65

    da_release(&numbers);
}

/* Test structures */
typedef struct {
    int x, y;
} Point;

typedef struct {
    char name[32];
    int age;
    float score;
} Person;

typedef struct {
    Point start;
    Point end;
    char color[16];
} Line;

/* Point struct tests */
void test_point_struct_basic(void) {
    da_array points = da_create(sizeof(Point), 3, NULL, NULL);

    Point p1 = {10, 20};
    Point p2 = {30, 40};
    Point p3 = {50, 60};

#if DA_SUPPORT_TYPE_INFERENCE && !defined(DA_NOT_USE_TYPE_GENERIC)
    DA_PUSH(points, p1);
    DA_PUSH(points, p2);
    DA_PUSH(points, p3);
#else
    DA_PUSH(points, p1, Point);
    DA_PUSH(points, p2, Point);
    DA_PUSH(points, p3, Point);
#endif

    TEST_ASSERT_EQUAL_INT(3, DA_LENGTH(points));
    
    Point retrieved1 = DA_AT(points, 0, Point);
    Point retrieved2 = DA_AT(points, 1, Point);
    Point retrieved3 = DA_AT(points, 2, Point);

    TEST_ASSERT_EQUAL_INT(10, retrieved1.x);
    TEST_ASSERT_EQUAL_INT(20, retrieved1.y);
    TEST_ASSERT_EQUAL_INT(30, retrieved2.x);
    TEST_ASSERT_EQUAL_INT(40, retrieved2.y);
    TEST_ASSERT_EQUAL_INT(50, retrieved3.x);
    TEST_ASSERT_EQUAL_INT(60, retrieved3.y);

    da_release(&points);
}

void test_person_struct_with_strings(void) {
    da_array people = da_create(sizeof(Person), 2, NULL, NULL);

    Person alice = {"Alice", 25, 95.5f};
    Person bob = {"Bob", 30, 87.2f};

    DA_PUSH(people, alice);
    DA_PUSH(people, bob);

    TEST_ASSERT_EQUAL_INT(2, DA_LENGTH(people));
    
    Person retrieved_alice = DA_AT(people, 0, Person);
    Person retrieved_bob = DA_AT(people, 1, Person);

    TEST_ASSERT_EQUAL_STRING("Alice", retrieved_alice.name);
    TEST_ASSERT_EQUAL_INT(25, retrieved_alice.age);
    TEST_ASSERT_EQUAL_FLOAT(95.5f, retrieved_alice.score);

    TEST_ASSERT_EQUAL_STRING("Bob", retrieved_bob.name);
    TEST_ASSERT_EQUAL_INT(30, retrieved_bob.age);
    TEST_ASSERT_EQUAL_FLOAT(87.2f, retrieved_bob.score);

    da_release(&people);
}

void test_nested_struct_complex(void) {
    da_array lines = da_create(sizeof(Line), 2, NULL, NULL);

    Line line1 = {{0, 0}, {10, 10}, "red"};
    Line line2 = {{5, 5}, {15, 15}, "blue"};

    DA_PUSH(lines, line1);
    DA_PUSH(lines, line2);

    TEST_ASSERT_EQUAL_INT(2, DA_LENGTH(lines));
    
    Line retrieved1 = DA_AT(lines, 0, Line);
    Line retrieved2 = DA_AT(lines, 1, Line);

    TEST_ASSERT_EQUAL_INT(0, retrieved1.start.x);
    TEST_ASSERT_EQUAL_INT(0, retrieved1.start.y);
    TEST_ASSERT_EQUAL_INT(10, retrieved1.end.x);
    TEST_ASSERT_EQUAL_INT(10, retrieved1.end.y);
    TEST_ASSERT_EQUAL_STRING("red", retrieved1.color);

    TEST_ASSERT_EQUAL_INT(5, retrieved2.start.x);
    TEST_ASSERT_EQUAL_INT(5, retrieved2.start.y);
    TEST_ASSERT_EQUAL_INT(15, retrieved2.end.x);
    TEST_ASSERT_EQUAL_INT(15, retrieved2.end.y);
    TEST_ASSERT_EQUAL_STRING("blue", retrieved2.color);

    da_release(&lines);
}

void test_struct_modification(void) {
    da_array points = da_create(sizeof(Point), 2, NULL, NULL);

    Point original = {100, 200};
    DA_PUSH(points, original);

    Point modified = {300, 400};
    DA_PUT(points, 0, modified);

    Point retrieved = DA_AT(points, 0, Point);
    TEST_ASSERT_EQUAL_INT(300, retrieved.x);
    TEST_ASSERT_EQUAL_INT(400, retrieved.y);

    da_release(&points);
}

void test_struct_direct_access(void) {
    da_array points = da_create(sizeof(Point), 3, NULL, NULL);

    Point p1 = {1, 2};
    Point p2 = {3, 4};
    DA_PUSH(points, p1);
    DA_PUSH(points, p2);

    /* Direct access via da_get */
    Point* ptr = (Point*)da_get(points, 0);
    TEST_ASSERT_EQUAL_INT(1, ptr->x);
    TEST_ASSERT_EQUAL_INT(2, ptr->y);

    /* Modify through pointer */
    ptr->x = 999;
    ptr->y = 888;

    Point retrieved = DA_AT(points, 0, Point);
    TEST_ASSERT_EQUAL_INT(999, retrieved.x);
    TEST_ASSERT_EQUAL_INT(888, retrieved.y);

    da_release(&points);
}

void test_struct_array_operations(void) {
    da_array people = da_create(sizeof(Person), 0, NULL, NULL);

    Person team[] = {
        {"John", 28, 92.1f},
        {"Jane", 32, 88.5f},
        {"Jim", 26, 95.8f}
    };

    /* Append raw struct array */
    da_append_raw(people, team, 3);
    TEST_ASSERT_EQUAL_INT(3, DA_LENGTH(people));
    
    Person retrieved = DA_AT(people, 1, Person);
    TEST_ASSERT_EQUAL_STRING("Jane", retrieved.name);
    TEST_ASSERT_EQUAL_INT(32, retrieved.age);

    /* Test copy */
    da_array people_copy = da_copy(people);
    TEST_ASSERT_EQUAL_INT(3, DA_LENGTH(people_copy));
    
    Person copy_retrieved = DA_AT(people_copy, 2, Person);
    TEST_ASSERT_EQUAL_STRING("Jim", copy_retrieved.name);
    TEST_ASSERT_EQUAL_FLOAT(95.8f, copy_retrieved.score);

    da_release(&people);
    da_release(&people_copy);
}

void test_struct_builder_pattern(void) {
    da_builder builder = DA_BUILDER_CREATE(Point);

    for (int i = 0; i < 5; i++) {
        Point p = {i * 10, i * 20};
        DA_BUILDER_APPEND(builder, p);
    }

    TEST_ASSERT_EQUAL_INT(5, DA_BUILDER_LEN(builder));

    /* Convert to array */
    da_array points = DA_BUILDER_TO_ARRAY(&builder);
    TEST_ASSERT_NULL(builder); /* Builder consumed */
    TEST_ASSERT_EQUAL_INT(5, DA_LENGTH(points));
    
    /* Verify data integrity */
    for (int i = 0; i < 5; i++) {
        Point p = DA_AT(points, i, Point);
        TEST_ASSERT_EQUAL_INT(i * 10, p.x);
        TEST_ASSERT_EQUAL_INT(i * 20, p.y);
    }

    da_release(&points);
}

/* Filter predicate for Person structs */
int is_adult(const void* elem, void* ctx) {
    const Person* person = (const Person*)elem;
    int min_age = *(int*)ctx;
    return person->age >= min_age;
}

void test_struct_filter_map_operations(void) {
    da_array people = da_create(sizeof(Person), 0, NULL, NULL);

    Person team[] = {
        {"Alice", 17, 85.0f},  /* Minor */
        {"Bob", 25, 90.0f},    /* Adult */
        {"Carol", 16, 95.0f},  /* Minor */
        {"David", 30, 88.0f}   /* Adult */
    };

    da_append_raw(people, team, 4);

    /* Filter adults (age >= 18) */
    int min_age = 18;
    da_array adults = da_filter(people, is_adult, &min_age);

    TEST_ASSERT_EQUAL_INT(2, DA_LENGTH(adults));
    
    Person adult1 = DA_AT(adults, 0, Person);
    Person adult2 = DA_AT(adults, 1, Person);

    TEST_ASSERT_EQUAL_STRING("Bob", adult1.name);
    TEST_ASSERT_EQUAL_INT(25, adult1.age);
    TEST_ASSERT_EQUAL_STRING("David", adult2.name);
    TEST_ASSERT_EQUAL_INT(30, adult2.age);

    da_release(&people);
    da_release(&adults);
}

// ============================================================================
// NEW FUNCTIONS TESTS  
// ============================================================================

// Helper predicates for testing
static int is_negative(const void* element, void* context) {
    (void)context;
    int value = *(const int*)element;
    return value < 0;
}

static int is_greater_than_context(const void* element, void* context) {
    int value = *(const int*)element;
    int threshold = *(const int*)context;
    return value > threshold;
}

static int compare_ints_asc(const void* a, const void* b, void* context) {
    (void)context;
    int ia = *(const int*)a;
    int ib = *(const int*)b;
    return ia - ib;
}

static int compare_ints_desc(const void* a, const void* b, void* context) {
    (void)context;
    int ia = *(const int*)a;
    int ib = *(const int*)b;
    return ib - ia;
}

void test_array_find_index(void) {
    da_array arr = da_new(sizeof(int));
    
    int values[] = {1, 3, 4, 7, 8};
    for (int i = 0; i < 5; i++) {
        da_push(arr, &values[i]);
    }
    
    // Find first even number
    TEST_ASSERT_EQUAL_INT(2, da_find_index(arr, is_even, NULL));  // index of 4
    
    // Find nothing
    TEST_ASSERT_EQUAL_INT(-1, da_find_index(arr, is_negative, NULL));
    
    // Find with context
    int threshold = 5;
    TEST_ASSERT_EQUAL_INT(3, da_find_index(arr, is_greater_than_context, &threshold));  // index of 7
    
    da_release(&arr);
}

void test_array_contains(void) {
    da_array arr = da_new(sizeof(int));
    
    int values[] = {1, 3, 4, 7, 8};
    for (int i = 0; i < 5; i++) {
        da_push(arr, &values[i]);
    }
    
    // Contains even number
    TEST_ASSERT_EQUAL_INT(1, da_contains(arr, is_even, NULL));
    
    // Does not contain negative number
    TEST_ASSERT_EQUAL_INT(0, da_contains(arr, is_negative, NULL));
    
    // Contains number greater than threshold
    int threshold = 5;
    TEST_ASSERT_EQUAL_INT(1, da_contains(arr, is_greater_than_context, &threshold));
    
    threshold = 10;
    TEST_ASSERT_EQUAL_INT(0, da_contains(arr, is_greater_than_context, &threshold));
    
    da_release(&arr);
}

void test_array_sort(void) {
    da_array arr = da_new(sizeof(int));
    
    // Add unsorted values
    int values[] = {7, 1, 8, 3, 4};
    for (int i = 0; i < 5; i++) {
        da_push(arr, &values[i]);
    }
    
    // Sort ascending
    da_sort(arr, compare_ints_asc, NULL);
    
    // Check sorted order
    int expected_asc[] = {1, 3, 4, 7, 8};
    for (int i = 0; i < 5; i++) {
        TEST_ASSERT_EQUAL_INT(expected_asc[i], DA_AT(arr, i, int));
    }
    
    // Sort descending
    da_sort(arr, compare_ints_desc, NULL);
    
    // Check reverse sorted order
    int expected_desc[] = {8, 7, 4, 3, 1};
    for (int i = 0; i < 5; i++) {
        TEST_ASSERT_EQUAL_INT(expected_desc[i], DA_AT(arr, i, int));
    }
    
    // Test edge case: empty array
    da_array empty_arr = da_new(sizeof(int));
    da_sort(empty_arr, compare_ints_asc, NULL);  // Should not crash
    TEST_ASSERT_EQUAL_INT(0, da_length(empty_arr));
    
    // Test edge case: single element
    da_array single_arr = da_new(sizeof(int));
    int single_val = 42;
    da_push(single_arr, &single_val);
    da_sort(single_arr, compare_ints_asc, NULL);  // Should not crash
    TEST_ASSERT_EQUAL_INT(42, DA_AT(single_arr, 0, int));
    
    da_release(&arr);
    da_release(&empty_arr);
    da_release(&single_arr);
}

/* Destructor tests */
static int destructor_call_count = 0;

typedef struct {
    int id;
    char* name;
} TestPerson;

void test_person_destructor(void* p) {
    TestPerson* person = (TestPerson*)p;
    if (person->name) {
        free(person->name);
        person->name = NULL;  // Prevent double free
    }
    destructor_call_count++;
}

void test_person_retain(void* p) {
    TestPerson* person = (TestPerson*)p;
    if (person->name) {
        // Duplicate the string
        size_t len = strlen(person->name);
        char* new_name = malloc(len + 1);
        strcpy(new_name, person->name);
        person->name = new_name;
    }
}

TestPerson create_test_person(int id, const char* name) {
    TestPerson p;
    p.id = id;
    p.name = malloc(strlen(name) + 1);
    strcpy(p.name, name);
    return p;
}

// Helper to transfer ownership - sets original's name to NULL
void transfer_test_person(TestPerson* dest, TestPerson* src) {
    dest->id = src->id;
    dest->name = src->name;
    src->name = NULL;  // Transfer ownership
}

void test_destructor_on_release(void) {
    destructor_call_count = 0;
    da_array people = da_create(sizeof(TestPerson), 0, test_person_retain, test_person_destructor);
    
    TestPerson p1 = create_test_person(1, "Alice");
    TestPerson p2 = create_test_person(2, "Bob");
    TestPerson p3 = create_test_person(3, "Charlie");
    
    da_push(people, &p1);
    da_push(people, &p2);
    da_push(people, &p3);
    
    da_release(&people);
    TEST_ASSERT_EQUAL_INT(3, destructor_call_count);
}

void test_destructor_on_pop(void) {
    destructor_call_count = 0;
    da_array people = da_create(sizeof(TestPerson), 0, test_person_retain, test_person_destructor);
    
    TestPerson p1 = create_test_person(4, "David");
    da_push(people, &p1);
    
    da_pop(people, NULL);
    TEST_ASSERT_EQUAL_INT(1, destructor_call_count);
    
    da_release(&people);
}

void test_destructor_on_clear(void) {
    destructor_call_count = 0;
    da_array people = da_create(sizeof(TestPerson), 0, test_person_retain, test_person_destructor);
    
    TestPerson p1 = create_test_person(5, "Eve");
    TestPerson p2 = create_test_person(6, "Frank");
    
    da_push(people, &p1);
    da_push(people, &p2);
    
    da_clear(people);
    TEST_ASSERT_EQUAL_INT(2, destructor_call_count);
    
    da_release(&people);
}

void test_destructor_on_set(void) {
    destructor_call_count = 0;
    da_array people = da_create(sizeof(TestPerson), 0, test_person_retain, test_person_destructor);
    
    TestPerson p1 = create_test_person(7, "George");
    TestPerson p2 = create_test_person(8, "Helen");
    
    da_push(people, &p1);
    
    da_set(people, 0, &p2);
    TEST_ASSERT_EQUAL_INT(1, destructor_call_count);
    
    destructor_call_count = 0;
    da_release(&people);
    TEST_ASSERT_EQUAL_INT(1, destructor_call_count);
}

void test_destructor_on_remove(void) {
    destructor_call_count = 0;
    da_array people = da_create(sizeof(TestPerson), 0, test_person_retain, test_person_destructor);
    
    TestPerson p1 = create_test_person(9, "Iris");
    TestPerson p2 = create_test_person(10, "Jack");
    TestPerson p3 = create_test_person(11, "Kate");
    
    da_push(people, &p1);
    da_push(people, &p2);
    da_push(people, &p3);
    
    da_remove(people, 1, NULL);
    TEST_ASSERT_EQUAL_INT(1, destructor_call_count);
    
    destructor_call_count = 0;
    da_release(&people);
    TEST_ASSERT_EQUAL_INT(2, destructor_call_count);
}

void test_destructor_on_resize_shrink(void) {
    destructor_call_count = 0;
    da_array people = da_create(sizeof(TestPerson), 0, test_person_retain, test_person_destructor);
    
    TestPerson p1 = create_test_person(12, "Leo");
    TestPerson p2 = create_test_person(13, "Mary");
    TestPerson p3 = create_test_person(14, "Nick");
    
    da_push(people, &p1);
    da_push(people, &p2);
    da_push(people, &p3);
    
    da_resize(people, 1);
    TEST_ASSERT_EQUAL_INT(2, destructor_call_count);
    
    destructor_call_count = 0;
    da_release(&people);
    TEST_ASSERT_EQUAL_INT(1, destructor_call_count);
}

void test_destructor_with_builder(void) {
    destructor_call_count = 0;
    da_builder builder = da_builder_create(sizeof(TestPerson));
    
    TestPerson p1 = create_test_person(15, "Oscar");
    TestPerson p2 = create_test_person(16, "Paula");
    
    da_builder_append(builder, &p1);
    da_builder_append(builder, &p2);
    
    da_array people = da_builder_to_array(&builder, test_person_retain, test_person_destructor);
    
    da_release(&people);
    TEST_ASSERT_EQUAL_INT(2, destructor_call_count);
}

void test_destructor_inheritance_on_copy(void) {
    destructor_call_count = 0;
    da_array original = da_create(sizeof(TestPerson), 0, test_person_retain, test_person_destructor);
    
    TestPerson p1 = create_test_person(17, "Quinn");
    da_push(original, &p1);
    
    da_array copy = da_copy(original);
    
    da_release(&original);
    TEST_ASSERT_EQUAL_INT(1, destructor_call_count);
    
    destructor_call_count = 0;
    da_release(&copy);
    TEST_ASSERT_EQUAL_INT(1, destructor_call_count);
}

int main(void) {
    UNITY_BEGIN();

    // Array creation and basic properties
    RUN_TEST(test_create_basic);
    RUN_TEST(test_create_zero_capacity);
    RUN_TEST(test_create_typed_macro);

    // Array reference counting
    RUN_TEST(test_reference_counting);
    RUN_TEST(test_multiple_retains);

    // Array push and growth
    RUN_TEST(test_push_basic);
    RUN_TEST(test_push_with_growth);
    RUN_TEST(test_push_from_zero_capacity);

    // Array access
    RUN_TEST(test_get_and_set);
    RUN_TEST(test_data_access);

    // Array pop
    RUN_TEST(test_pop_basic);
    RUN_TEST(test_pop_ignore_output);

    // Array clear and resize
    RUN_TEST(test_clear);
    RUN_TEST(test_reserve);
    RUN_TEST(test_resize_grow);
    RUN_TEST(test_resize_shrink);

    // Array macros
    RUN_TEST(test_typed_macros);

    // Array insert and remove
    RUN_TEST(test_insert_basic);
    RUN_TEST(test_insert_at_beginning);
    RUN_TEST(test_insert_at_end);
    RUN_TEST(test_insert_with_growth);
    RUN_TEST(test_remove_basic);
    RUN_TEST(test_remove_first);
    RUN_TEST(test_remove_last);
    RUN_TEST(test_remove_ignore_output);

    // Array memory optimization
    RUN_TEST(test_trim_basic);
    RUN_TEST(test_trim_to_zero);
    RUN_TEST(test_shrink_to_fit_macro);

    // Array concatenation
    RUN_TEST(test_append_array_basic);
    RUN_TEST(test_append_array_empty);
    RUN_TEST(test_append_array_with_growth);
    RUN_TEST(test_concat_basic);
    RUN_TEST(test_concat_empty_arrays);
    RUN_TEST(test_concat_one_empty);

    // Array stress tests
    RUN_TEST(test_many_operations);
    RUN_TEST(test_different_types);
    RUN_TEST(test_atomic_refcount_basic);

    // Builder creation and basic operations
    RUN_TEST(test_builder_create_basic);
    RUN_TEST(test_builder_create_typed_macro);
    RUN_TEST(test_builder_append_basic);
    RUN_TEST(test_builder_append_typed_macro);

    // Builder growth and access
    RUN_TEST(test_builder_growth_doubling);
    RUN_TEST(test_builder_access_operations);
    RUN_TEST(test_builder_clear);

    // Builder to array conversion
    RUN_TEST(test_builder_to_array_basic);
    RUN_TEST(test_builder_to_array_empty);
    RUN_TEST(test_builder_to_array_exact_sizing);

    // Builder integration and stress tests
    RUN_TEST(test_builder_integration_with_arrays);
    RUN_TEST(test_builder_different_types);
    RUN_TEST(test_builder_stress_test);
    
    // Builder reserve and append_array tests
    RUN_TEST(test_builder_reserve_basic);
    RUN_TEST(test_builder_reserve_no_shrink);
    RUN_TEST(test_builder_append_array_basic);
    RUN_TEST(test_builder_append_array_empty);
    RUN_TEST(test_builder_append_array_multiple);
    RUN_TEST(test_builder_append_array_with_existing_data);
    RUN_TEST(test_builder_reserve_and_append_array_efficiency);

    // Peek operations
    RUN_TEST(test_peek_basic);
    RUN_TEST(test_peek_macros);
    RUN_TEST(test_peek_single_element);

    // Bulk operations
    RUN_TEST(test_append_raw_basic);
    RUN_TEST(test_append_raw_empty);
    RUN_TEST(test_append_raw_with_growth);
    RUN_TEST(test_fill_basic);
    RUN_TEST(test_fill_empty_count);
    RUN_TEST(test_fill_with_growth);

    // Range operations
    RUN_TEST(test_slice_basic);
    RUN_TEST(test_slice_empty_range);
    RUN_TEST(test_slice_full_array);
    RUN_TEST(test_remove_range_basic);
    RUN_TEST(test_remove_range_empty);
    RUN_TEST(test_remove_range_from_end);

    // Utility operations
    RUN_TEST(test_reverse_basic);
    RUN_TEST(test_reverse_even_length);
    RUN_TEST(test_reverse_single_element);
    RUN_TEST(test_reverse_empty);
    RUN_TEST(test_swap_basic);
    RUN_TEST(test_swap_same_index);
    RUN_TEST(test_swap_adjacent);
    RUN_TEST(test_is_empty_basic);
    RUN_TEST(test_is_empty_after_clear);

    // Copy operations
    RUN_TEST(test_copy_basic);
    RUN_TEST(test_copy_empty_array);
    RUN_TEST(test_copy_single_element);
    RUN_TEST(test_copy_exact_capacity);
    RUN_TEST(test_copy_different_types);
    RUN_TEST(test_copy_independence);
    RUN_TEST(test_copy_sorting_scenario);
    RUN_TEST(test_copy_reference_counting);

    // Filter operations
    RUN_TEST(test_filter_basic);
    RUN_TEST(test_filter_empty_source);
    RUN_TEST(test_filter_empty_result);
    RUN_TEST(test_filter_all_match);
    RUN_TEST(test_filter_with_context);
    RUN_TEST(test_filter_different_types);
    RUN_TEST(test_filter_independence);

    // Map operations
    RUN_TEST(test_map_basic);
    RUN_TEST(test_map_empty_array);
    RUN_TEST(test_map_with_context);
    RUN_TEST(test_map_single_element);
    RUN_TEST(test_map_different_types);
    RUN_TEST(test_map_independence);
    RUN_TEST(test_map_chain_operations);

    // Reduce operations
    RUN_TEST(test_reduce_sum_basic);
    RUN_TEST(test_reduce_product);
    RUN_TEST(test_reduce_empty_array);
    RUN_TEST(test_reduce_single_element);
    RUN_TEST(test_reduce_with_context);
    RUN_TEST(test_reduce_count_matching);
    RUN_TEST(test_reduce_accumulator_is_result);

    // Struct support tests
    RUN_TEST(test_point_struct_basic);
    RUN_TEST(test_person_struct_with_strings);
    RUN_TEST(test_nested_struct_complex);
    RUN_TEST(test_struct_modification);
    RUN_TEST(test_struct_direct_access);
    RUN_TEST(test_struct_array_operations);
    RUN_TEST(test_struct_builder_pattern);
    RUN_TEST(test_struct_filter_map_operations);

    // New functions tests
    RUN_TEST(test_array_find_index);
    RUN_TEST(test_array_contains);
    RUN_TEST(test_array_sort);
    
    // Destructor tests
    RUN_TEST(test_destructor_on_release);
    RUN_TEST(test_destructor_on_pop);
    RUN_TEST(test_destructor_on_clear);
    RUN_TEST(test_destructor_on_set);
    RUN_TEST(test_destructor_on_remove);
    RUN_TEST(test_destructor_on_resize_shrink);
    RUN_TEST(test_destructor_with_builder);
    RUN_TEST(test_destructor_inheritance_on_copy);

    return UNITY_END();
}
