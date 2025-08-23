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
    da_array arr = da_new(sizeof(int), 10);

    TEST_ASSERT_NOT_NULL(arr);
    TEST_ASSERT_EQUAL_INT(0, da_length(arr));
    TEST_ASSERT_EQUAL_INT(10, da_capacity(arr));
    TEST_ASSERT_NOT_NULL(da_data(arr));

    da_release(&arr);
    TEST_ASSERT_NULL(arr);
}

void test_da_create_zero_capacity(void) {
    da_array arr = da_new(sizeof(int), 0);

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
    da_array arr = da_new(sizeof(int), 5);
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
    da_array arr = da_new(sizeof(int), 3);
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
    da_array arr = da_new(sizeof(int), 2);

    int val1 = 42;
    int val2 = 99;

    da_push(arr, &val1);
    TEST_ASSERT_EQUAL_INT(1, da_length(arr));

    da_push(arr, &val2);
    TEST_ASSERT_EQUAL_INT(2, da_length(arr));

    da_release(&arr);
}

void test_da_push_with_growth(void) {
    da_array arr = da_new(sizeof(int), 1);

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
    da_array arr = da_new(sizeof(int), 0);

    int val = 123;
    da_push(arr, &val);

    TEST_ASSERT_TRUE(da_capacity(arr) > 0);
    TEST_ASSERT_EQUAL_INT(1, da_length(arr));

    da_release(&arr);
}

/* Access Tests */
void test_da_get_and_set(void) {
    da_array arr = da_new(sizeof(int), 5);

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
    da_array arr = da_new(sizeof(int), 3);

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
    da_array arr = da_new(sizeof(int), 3);

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
    da_array arr = da_new(sizeof(int), 2);

    int val = 123;
    da_push(arr, &val);
    TEST_ASSERT_EQUAL_INT(1, da_length(arr));

    da_pop(arr, NULL);  // Ignore popped value
    TEST_ASSERT_EQUAL_INT(0, da_length(arr));

    da_release(&arr);
}

/* Clear and Resize Tests */
void test_da_clear(void) {
    da_array arr = da_new(sizeof(int), 5);

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
    da_array arr = da_new(sizeof(int), 2);

    da_reserve(arr, 10);
    TEST_ASSERT_EQUAL_INT(10, da_capacity(arr));
    TEST_ASSERT_EQUAL_INT(0, da_length(arr));

    // Reserve smaller than current capacity should do nothing
    da_reserve(arr, 5);
    TEST_ASSERT_EQUAL_INT(10, da_capacity(arr));

    da_release(&arr);
}

void test_da_resize_grow(void) {
    da_array arr = da_new(sizeof(int), 3);

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
    da_array arr = da_new(sizeof(int), 5);

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
    da_array arr = da_new(sizeof(int), 5);

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
void test_da_insert_basic(void) {
    da_array arr = da_new(sizeof(int), 5);
    
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

void test_da_insert_at_beginning(void) {
    da_array arr = da_new(sizeof(int), 3);
    
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

void test_da_insert_at_end(void) {
    da_array arr = da_new(sizeof(int), 3);
    
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

void test_da_insert_with_growth(void) {
    da_array arr = da_new(sizeof(int), 2);
    
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

void test_da_remove_basic(void) {
    da_array arr = da_new(sizeof(int), 5);
    
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

void test_da_remove_first(void) {
    da_array arr = da_new(sizeof(int), 3);
    
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

void test_da_remove_last(void) {
    da_array arr = da_new(sizeof(int), 3);
    
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

void test_da_remove_ignore_output(void) {
    da_array arr = da_new(sizeof(int), 3);
    
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
void test_da_trim_basic(void) {
    da_array arr = da_new(sizeof(int), 100);
    
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

void test_da_trim_to_zero(void) {
    da_array arr = da_new(sizeof(int), 10);
    
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

void test_da_shrink_to_fit_macro(void) {
    da_array arr = da_new(sizeof(int), 50);
    
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
void test_da_append_array_basic(void) {
    da_array arr1 = da_new(sizeof(int), 3);
    da_array arr2 = da_new(sizeof(int), 3);
    
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

void test_da_append_array_empty(void) {
    da_array arr1 = da_new(sizeof(int), 2);
    da_array arr2 = da_new(sizeof(int), 2);
    
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

void test_da_append_array_with_growth(void) {
    da_array arr1 = da_new(sizeof(int), 2);
    da_array arr2 = da_new(sizeof(int), 3);
    
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

void test_da_concat_basic(void) {
    da_array arr1 = da_new(sizeof(int), 2);
    da_array arr2 = da_new(sizeof(int), 2);
    
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

void test_da_concat_empty_arrays(void) {
    da_array arr1 = da_new(sizeof(int), 2);
    da_array arr2 = da_new(sizeof(int), 2);
    
    // Both arrays empty
    da_array result = da_concat(arr1, arr2);
    
    TEST_ASSERT_EQUAL_INT(0, da_length(result));
    TEST_ASSERT_EQUAL_INT(0, da_capacity(result));
    TEST_ASSERT_NULL(da_data(result));
    
    da_release(&result);
    da_release(&arr1);
    da_release(&arr2);
}

void test_da_concat_one_empty(void) {
    da_array arr1 = da_new(sizeof(int), 2);
    da_array arr2 = da_new(sizeof(int), 2);
    
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
void test_da_builder_create_basic(void) {
    da_builder builder = da_builder_create(sizeof(int));

    TEST_ASSERT_NOT_NULL(builder);
    TEST_ASSERT_EQUAL_INT(0, da_builder_length(builder));
    TEST_ASSERT_EQUAL_INT(0, da_builder_capacity(builder));

    da_builder_destroy(&builder);
    TEST_ASSERT_NULL(builder);
}

void test_da_builder_create_typed_macro(void) {
    da_builder builder = DA_BUILDER_CREATE(int);

    TEST_ASSERT_NOT_NULL(builder);
    TEST_ASSERT_EQUAL_INT(0, DA_BUILDER_LEN(builder));
    TEST_ASSERT_EQUAL_INT(0, DA_BUILDER_CAP(builder));

    da_builder_destroy(&builder);
}

void test_da_builder_append_basic(void) {
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

void test_da_builder_append_typed_macro(void) {
    da_builder builder = DA_BUILDER_CREATE(int);

#if DA_HAS_TYPEOF
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

void test_da_builder_growth_doubling(void) {
    da_builder builder = DA_BUILDER_CREATE(int);

    // Builder should always use doubling strategy
    int previous_capacity = 0;

    for (int i = 0; i < 20; i++) {
#if DA_HAS_TYPEOF
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

void test_da_builder_access_operations(void) {
    da_builder builder = DA_BUILDER_CREATE(int);

    // Append some values
    for (int i = 0; i < 5; i++) {
#if DA_HAS_TYPEOF
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

void test_da_builder_clear(void) {
    da_builder builder = DA_BUILDER_CREATE(int);

    // Add some elements
    for (int i = 0; i < 10; i++) {
#if DA_HAS_TYPEOF
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

void test_da_builder_to_array_basic(void) {
    da_builder builder = DA_BUILDER_CREATE(int);

    // Build up some data
    for (int i = 0; i < 10; i++) {
#if DA_HAS_TYPEOF
        DA_BUILDER_APPEND(builder, i * 2);
#else
        DA_BUILDER_APPEND(builder, i * 2, int);
#endif
    }

    int builder_length = DA_BUILDER_LEN(builder);
    int builder_capacity = DA_BUILDER_CAP(builder);

    // Convert to array
    da_array arr = da_builder_to_array(&builder);
    TEST_ASSERT_NULL(builder);  // Builder should be consumed

    // Check array properties
    TEST_ASSERT_NOT_NULL(arr);
    TEST_ASSERT_EQUAL_INT(builder_length, DA_LEN(arr));
    TEST_ASSERT_EQUAL_INT(builder_length, DA_CAP(arr));  // Exact capacity!
    TEST_ASSERT_EQUAL_INT(1, DA_ATOMIC_LOAD(&arr->ref_count));

    // Check that capacity was shrunk to exact size
    TEST_ASSERT_TRUE(DA_CAP(arr) <= builder_capacity);

    // Verify data integrity
    for (int i = 0; i < 10; i++) {
        TEST_ASSERT_EQUAL_INT(i * 2, DA_AT(arr, i, int));
    }

    da_release(&arr);
}

void test_da_builder_to_array_empty(void) {
    da_builder builder = DA_BUILDER_CREATE(int);

    // Don't add any elements
    TEST_ASSERT_EQUAL_INT(0, DA_BUILDER_LEN(builder));

    da_array arr = da_builder_to_array(&builder);
    TEST_ASSERT_NULL(builder);

    TEST_ASSERT_NOT_NULL(arr);
    TEST_ASSERT_EQUAL_INT(0, DA_LEN(arr));
    TEST_ASSERT_EQUAL_INT(0, DA_CAP(arr));
    TEST_ASSERT_EQUAL_INT(1, DA_ATOMIC_LOAD(&arr->ref_count));

    da_release(&arr);
}

void test_da_builder_to_array_exact_sizing(void) {
    da_builder builder = DA_BUILDER_CREATE(int);

    // Add elements to trigger multiple capacity doublings
    for (int i = 0; i < 100; i++) {
#if DA_HAS_TYPEOF
        DA_BUILDER_APPEND(builder, i);
#else
        DA_BUILDER_APPEND(builder, i, int);
#endif
    }

    int builder_capacity = DA_BUILDER_CAP(builder);
    TEST_ASSERT_TRUE(builder_capacity > 100);  // Should have excess capacity

    da_array arr = da_builder_to_array(&builder);

    // Array should have exact capacity = length = 100
    TEST_ASSERT_EQUAL_INT(100, DA_LEN(arr));
    TEST_ASSERT_EQUAL_INT(100, DA_CAP(arr));  // No wasted memory!

    da_release(&arr);
}

void test_da_builder_integration_with_arrays(void) {
    // Test that arrays created from builders work normally
    da_builder builder = DA_BUILDER_CREATE(int);

    for (int i = 0; i < 5; i++) {
#if DA_HAS_TYPEOF
        DA_BUILDER_APPEND(builder, i * 10);
#else
        DA_BUILDER_APPEND(builder, i * 10, int);
#endif
    }

    da_array arr = da_builder_to_array(&builder);

    // Test reference counting
    da_array arr2 = da_retain(arr);
    TEST_ASSERT_EQUAL_INT(2, DA_ATOMIC_LOAD(&arr->ref_count));

    // Test array operations
#if DA_HAS_TYPEOF
    DA_PUSH(arr, 999);
#else
    DA_PUSH(arr, 999, int);
#endif

    TEST_ASSERT_EQUAL_INT(6, DA_LEN(arr));
    TEST_ASSERT_EQUAL_INT(999, DA_AT(arr, 5, int));

    // Test that shared reference sees the change
    TEST_ASSERT_EQUAL_INT(6, DA_LEN(arr2));
    TEST_ASSERT_EQUAL_INT(999, DA_AT(arr2, 5, int));

    da_release(&arr);
    da_release(&arr2);
}

void test_da_builder_different_types(void) {
    // Test builder with different data types
    da_builder float_builder = DA_BUILDER_CREATE(float);
    da_builder char_builder = DA_BUILDER_CREATE(char);

    float f_vals[] = {3.14f, 2.71f, 1.41f};
    char c_vals[] = {'A', 'B', 'C'};

    for (int i = 0; i < 3; i++) {
        da_builder_append(float_builder, &f_vals[i]);
        da_builder_append(char_builder, &c_vals[i]);
    }

    da_array float_arr = da_builder_to_array(&float_builder);
    da_array char_arr = da_builder_to_array(&char_builder);

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

void test_da_builder_stress_test(void) {
    da_builder builder = DA_BUILDER_CREATE(int);

    // Add many elements to stress test growth
    const int num_elements = 1000;
    for (int i = 0; i < num_elements; i++) {
#if DA_HAS_TYPEOF
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
    da_array arr = da_builder_to_array(&builder);

    TEST_ASSERT_EQUAL_INT(num_elements, DA_LEN(arr));
    TEST_ASSERT_EQUAL_INT(num_elements, DA_CAP(arr));  // Exact capacity

    // Verify data integrity after conversion
    for (int i = 0; i < num_elements; i++) {
        TEST_ASSERT_EQUAL_INT(i, DA_AT(arr, i, int));
    }

    da_release(&arr);
}

int main(void) {
    UNITY_BEGIN();

    // Array creation and basic properties
    RUN_TEST(test_da_create_basic);
    RUN_TEST(test_da_create_zero_capacity);
    RUN_TEST(test_da_create_typed_macro);

    // Array reference counting
    RUN_TEST(test_da_reference_counting);
    RUN_TEST(test_da_multiple_retains);

    // Array push and growth
    RUN_TEST(test_da_push_basic);
    RUN_TEST(test_da_push_with_growth);
    RUN_TEST(test_da_push_from_zero_capacity);

    // Array access
    RUN_TEST(test_da_get_and_set);
    RUN_TEST(test_da_data_access);

    // Array pop
    RUN_TEST(test_da_pop_basic);
    RUN_TEST(test_da_pop_ignore_output);

    // Array clear and resize
    RUN_TEST(test_da_clear);
    RUN_TEST(test_da_reserve);
    RUN_TEST(test_da_resize_grow);
    RUN_TEST(test_da_resize_shrink);

    // Array macros
    RUN_TEST(test_da_typed_macros);

    // Array insert and remove
    RUN_TEST(test_da_insert_basic);
    RUN_TEST(test_da_insert_at_beginning);
    RUN_TEST(test_da_insert_at_end);
    RUN_TEST(test_da_insert_with_growth);
    RUN_TEST(test_da_remove_basic);
    RUN_TEST(test_da_remove_first);
    RUN_TEST(test_da_remove_last);
    RUN_TEST(test_da_remove_ignore_output);

    // Array memory optimization
    RUN_TEST(test_da_trim_basic);
    RUN_TEST(test_da_trim_to_zero);
    RUN_TEST(test_da_shrink_to_fit_macro);

    // Array concatenation
    RUN_TEST(test_da_append_array_basic);
    RUN_TEST(test_da_append_array_empty);
    RUN_TEST(test_da_append_array_with_growth);
    RUN_TEST(test_da_concat_basic);
    RUN_TEST(test_da_concat_empty_arrays);
    RUN_TEST(test_da_concat_one_empty);

    // Array stress tests
    RUN_TEST(test_da_many_operations);
    RUN_TEST(test_da_different_types);
    RUN_TEST(test_da_thread_safety_basic);

    // Builder creation and basic operations
    RUN_TEST(test_da_builder_create_basic);
    RUN_TEST(test_da_builder_create_typed_macro);
    RUN_TEST(test_da_builder_append_basic);
    RUN_TEST(test_da_builder_append_typed_macro);

    // Builder growth and access
    RUN_TEST(test_da_builder_growth_doubling);
    RUN_TEST(test_da_builder_access_operations);
    RUN_TEST(test_da_builder_clear);

    // Builder to array conversion
    RUN_TEST(test_da_builder_to_array_basic);
    RUN_TEST(test_da_builder_to_array_empty);
    RUN_TEST(test_da_builder_to_array_exact_sizing);

    // Builder integration and stress tests
    RUN_TEST(test_da_builder_integration_with_arrays);
    RUN_TEST(test_da_builder_different_types);
    RUN_TEST(test_da_builder_stress_test);

    return UNITY_END();
}