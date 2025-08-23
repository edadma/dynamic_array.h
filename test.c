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

/* Peek Operations Tests */
void test_da_peek_basic(void) {
    da_array arr = da_new(sizeof(int), 3);
    
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

void test_da_peek_macros(void) {
    da_array arr = da_new(sizeof(int), 2);
    
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

void test_da_peek_single_element(void) {
    da_array arr = da_new(sizeof(int), 1);
    
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
void test_da_append_raw_basic(void) {
    da_array arr = da_new(sizeof(int), 2);
    
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

void test_da_append_raw_empty(void) {
    da_array arr = da_new(sizeof(int), 2);
    
    int val = 42;
    da_push(arr, &val);
    
    // Append zero elements - should do nothing
    int dummy[] = {1, 2, 3};
    da_append_raw(arr, dummy, 0);
    
    TEST_ASSERT_EQUAL_INT(1, da_length(arr));
    TEST_ASSERT_EQUAL_INT(42, DA_AT(arr, 0, int));
    
    da_release(&arr);
}

void test_da_append_raw_with_growth(void) {
    da_array arr = da_new(sizeof(int), 2);
    
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

void test_da_fill_basic(void) {
    da_array arr = da_new(sizeof(int), 10);
    
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

void test_da_fill_empty_count(void) {
    da_array arr = da_new(sizeof(int), 2);
    
    int val = 42;
    da_push(arr, &val);
    
    // Fill with zero count - should do nothing
    int dummy = 123;
    da_fill(arr, &dummy, 0);
    
    TEST_ASSERT_EQUAL_INT(1, da_length(arr));
    TEST_ASSERT_EQUAL_INT(42, DA_AT(arr, 0, int));
    
    da_release(&arr);
}

void test_da_fill_with_growth(void) {
    da_array arr = da_new(sizeof(int), 2);
    
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
void test_da_slice_basic(void) {
    da_array arr = da_new(sizeof(int), 5);
    
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

void test_da_slice_empty_range(void) {
    da_array arr = da_new(sizeof(int), 3);
    
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

void test_da_slice_full_array(void) {
    da_array arr = da_new(sizeof(int), 3);
    
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

void test_da_remove_range_basic(void) {
    da_array arr = da_new(sizeof(int), 6);
    
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

void test_da_remove_range_empty(void) {
    da_array arr = da_new(sizeof(int), 3);
    
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

void test_da_remove_range_from_end(void) {
    da_array arr = da_new(sizeof(int), 5);
    
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
void test_da_reverse_basic(void) {
    da_array arr = da_new(sizeof(int), 5);
    
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

void test_da_reverse_even_length(void) {
    da_array arr = da_new(sizeof(int), 4);
    
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

void test_da_reverse_single_element(void) {
    da_array arr = da_new(sizeof(int), 1);
    
    int val = 42;
    da_push(arr, &val);
    
    da_reverse(arr);
    
    // Should be unchanged
    TEST_ASSERT_EQUAL_INT(1, da_length(arr));
    TEST_ASSERT_EQUAL_INT(42, DA_AT(arr, 0, int));
    
    da_release(&arr);
}

void test_da_reverse_empty(void) {
    da_array arr = da_new(sizeof(int), 2);
    
    // Empty array
    da_reverse(arr);
    
    // Should remain empty
    TEST_ASSERT_EQUAL_INT(0, da_length(arr));
    
    da_release(&arr);
}

void test_da_swap_basic(void) {
    da_array arr = da_new(sizeof(int), 5);
    
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

void test_da_swap_same_index(void) {
    da_array arr = da_new(sizeof(int), 3);
    
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

void test_da_swap_adjacent(void) {
    da_array arr = da_new(sizeof(int), 3);
    
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

void test_da_is_empty_basic(void) {
    da_array arr = da_new(sizeof(int), 5);
    
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

void test_da_is_empty_after_clear(void) {
    da_array arr = da_new(sizeof(int), 3);
    
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
void test_da_copy_basic(void) {
    da_array original = da_new(sizeof(int), 5);
    
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

void test_da_copy_empty_array(void) {
    da_array original = da_new(sizeof(int), 10);
    
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

void test_da_copy_single_element(void) {
    da_array original = da_new(sizeof(int), 1);
    
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

void test_da_copy_exact_capacity(void) {
    da_array original = da_new(sizeof(int), 100);  // Large capacity
    
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

void test_da_copy_different_types(void) {
    // Test copying arrays of different types
    da_array float_arr = da_new(sizeof(float), 3);
    da_array char_arr = da_new(sizeof(char), 3);
    
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

void test_da_copy_independence(void) {
    da_array original = da_new(sizeof(int), 5);
    
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

void test_da_copy_sorting_scenario(void) {
    // Test the main use case: copying for sorting without affecting original
    da_array numbers = da_new(sizeof(int), 5);
    
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

void test_da_copy_reference_counting(void) {
    da_array original = da_new(sizeof(int), 3);
    
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

void test_da_filter_basic(void) {
    da_array numbers = da_new(sizeof(int), 5);
    
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

void test_da_filter_empty_result(void) {
    da_array numbers = da_new(sizeof(int), 3);
    
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

void test_da_filter_all_match(void) {
    da_array numbers = da_new(sizeof(int), 4);
    
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

void test_da_filter_with_context(void) {
    da_array numbers = da_new(sizeof(int), 6);
    
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

void test_da_filter_empty_source(void) {
    da_array empty = da_new(sizeof(int), 0);
    
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

void test_da_filter_different_types(void) {
    da_array chars = da_new(sizeof(char), 5);
    
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

void test_da_filter_independence(void) {
    da_array numbers = da_new(sizeof(int), 4);
    
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

void test_da_map_basic(void) {
    da_array numbers = da_new(sizeof(int), 4);
    
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

void test_da_map_empty_array(void) {
    da_array empty = da_new(sizeof(int), 0);
    
    // Map empty array
    da_array result = da_map(empty, double_int, NULL);
    
    TEST_ASSERT_EQUAL_INT(0, da_length(result));
    TEST_ASSERT_EQUAL_INT(0, da_capacity(result));
    TEST_ASSERT_NULL(da_data(result));
    TEST_ASSERT_EQUAL_INT(1, DA_ATOMIC_LOAD(&result->ref_count));
    
    da_release(&empty);
    da_release(&result);
}

void test_da_map_with_context(void) {
    da_array numbers = da_new(sizeof(int), 3);
    
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

void test_da_map_single_element(void) {
    da_array single = da_new(sizeof(int), 1);
    
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

void test_da_map_different_types(void) {
    da_array floats = da_new(sizeof(float), 3);
    
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

void test_da_map_independence(void) {
    da_array numbers = da_new(sizeof(int), 3);
    
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

void test_da_map_chain_operations(void) {
    da_array numbers = da_new(sizeof(int), 4);
    
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
void test_da_reduce_sum_basic(void) {
    da_array numbers = da_new(sizeof(int), 5);
    
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
void test_da_reduce_product(void) {
    da_array numbers = da_new(sizeof(int), 4);
    
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
void test_da_reduce_empty_array(void) {
    da_array numbers = da_new(sizeof(int), 0);
    
    int initial = 42;
    int result;
    da_reduce(numbers, &initial, &result, sum_ints, NULL);
    
    TEST_ASSERT_EQUAL(42, result);  // Should return initial value
    
    da_release(&numbers);
}

// Test da_reduce with single element
void test_da_reduce_single_element(void) {
    da_array numbers = da_new(sizeof(int), 1);
    
    int value = 99;
    da_push(numbers, &value);
    
    int initial = 0;
    int result;
    da_reduce(numbers, &initial, &result, sum_ints, NULL);
    
    TEST_ASSERT_EQUAL(99, result);  // 0 + 99 = 99
    
    da_release(&numbers);
}

// Test da_reduce with custom context
void test_da_reduce_with_context(void) {
    da_array numbers = da_new(sizeof(float), 3);
    
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
void test_da_reduce_count_matching(void) {
    da_array numbers = da_new(sizeof(int), 6);
    
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
void test_da_reduce_accumulator_is_result(void) {
    da_array numbers = da_new(sizeof(int), 3);
    
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

    // Peek operations
    RUN_TEST(test_da_peek_basic);
    RUN_TEST(test_da_peek_macros);
    RUN_TEST(test_da_peek_single_element);

    // Bulk operations
    RUN_TEST(test_da_append_raw_basic);
    RUN_TEST(test_da_append_raw_empty);
    RUN_TEST(test_da_append_raw_with_growth);
    RUN_TEST(test_da_fill_basic);
    RUN_TEST(test_da_fill_empty_count);
    RUN_TEST(test_da_fill_with_growth);

    // Range operations
    RUN_TEST(test_da_slice_basic);
    RUN_TEST(test_da_slice_empty_range);
    RUN_TEST(test_da_slice_full_array);
    RUN_TEST(test_da_remove_range_basic);
    RUN_TEST(test_da_remove_range_empty);
    RUN_TEST(test_da_remove_range_from_end);

    // Utility operations
    RUN_TEST(test_da_reverse_basic);
    RUN_TEST(test_da_reverse_even_length);
    RUN_TEST(test_da_reverse_single_element);
    RUN_TEST(test_da_reverse_empty);
    RUN_TEST(test_da_swap_basic);
    RUN_TEST(test_da_swap_same_index);
    RUN_TEST(test_da_swap_adjacent);
    RUN_TEST(test_da_is_empty_basic);
    RUN_TEST(test_da_is_empty_after_clear);

    // Copy operations
    RUN_TEST(test_da_copy_basic);
    RUN_TEST(test_da_copy_empty_array);
    RUN_TEST(test_da_copy_single_element);
    RUN_TEST(test_da_copy_exact_capacity);
    RUN_TEST(test_da_copy_different_types);
    RUN_TEST(test_da_copy_independence);
    RUN_TEST(test_da_copy_sorting_scenario);
    RUN_TEST(test_da_copy_reference_counting);

    // Filter operations
    RUN_TEST(test_da_filter_basic);
    RUN_TEST(test_da_filter_empty_source);
    RUN_TEST(test_da_filter_empty_result);
    RUN_TEST(test_da_filter_all_match);
    RUN_TEST(test_da_filter_with_context);
    RUN_TEST(test_da_filter_different_types);
    RUN_TEST(test_da_filter_independence);

    // Map operations
    RUN_TEST(test_da_map_basic);
    RUN_TEST(test_da_map_empty_array);
    RUN_TEST(test_da_map_with_context);
    RUN_TEST(test_da_map_single_element);
    RUN_TEST(test_da_map_different_types);
    RUN_TEST(test_da_map_independence);
    RUN_TEST(test_da_map_chain_operations);

    // Reduce operations
    RUN_TEST(test_da_reduce_sum_basic);
    RUN_TEST(test_da_reduce_product);
    RUN_TEST(test_da_reduce_empty_array);
    RUN_TEST(test_da_reduce_single_element);
    RUN_TEST(test_da_reduce_with_context);
    RUN_TEST(test_da_reduce_count_matching);
    RUN_TEST(test_da_reduce_accumulator_is_result);

    return UNITY_END();
}