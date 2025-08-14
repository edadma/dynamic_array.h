/**
 * @file dynamic_array.h
 * @brief Thread-safe reference-counted dynamic arrays with ArrayBuffer-style builder
 * @author dynamic_array.h contributors
 * @version 0.1.0
 * @date 2025
 *
 * Single header library for mutable dynamic arrays with reference counting.
 * Includes Scala ArrayBuffer-style builder for efficient construction.
 * Designed for portability across PC and microcontroller targets.
 *
 * @section config Configuration
 *
 * Customize the library by defining these macros before including:
 *
 * @code
 * #define DA_MALLOC malloc         // custom allocator
 * #define DA_REALLOC realloc       // custom reallocator
 * #define DA_FREE free             // custom deallocator
 * #define DA_ASSERT assert         // custom assert macro
 * #define DA_GROWTH 16             // fixed growth increment (default: doubling)
 * #define DA_THREAD_SAFE 1         // enable thread-safe reference counting (C11 required)
 *
 * #define DYNAMIC_ARRAY_IMPLEMENTATION
 * #include "dynamic_array.h"
 * @endcode
 *
 * @section usage Basic Usage
 *
 * @subsection arrays Regular Arrays
 * @code
 * da_array arr = da_create(sizeof(int), 10);
 * DA_PUSH(arr, 42);
 * int val = DA_AT(arr, 0, int);
 * da_release(&arr);  // arr becomes NULL
 * @endcode
 *
 * @subsection builders Builder Pattern (like Scala's ArrayBuffer)
 * @code
 * da_builder builder = DA_BUILDER_CREATE(int);
 * DA_BUILDER_APPEND(builder, 42);
 * DA_BUILDER_APPEND(builder, 99);
 * da_array arr = da_builder_to_array(&builder);  // Exact size, builder becomes NULL
 * da_release(&arr);
 * @endcode
 *
 * @section threading Thread Safety
 *
 * When DA_THREAD_SAFE=1 (requires C11):
 * - Reference counting operations (da_retain/da_release) are lock-free and thread-safe
 * - Array content modifications require external synchronization
 * - Builders are not thread-safe and should be used by single threads
 *
 * @section platforms Supported Platforms
 *
 * - Linux (GCC, Clang)
 * - Windows (MinGW, MSVC)
 * - Raspberry Pi Pico (arm-none-eabi-gcc)
 * - ESP32-C3 (Espressif toolchain)
 * - ARM Cortex-M (various toolchains)
 */

#ifndef DYNAMIC_ARRAY_H
#define DYNAMIC_ARRAY_H

#include <stdlib.h>
#include <string.h>
#include <assert.h>

/**
 * @defgroup config Configuration Macros
 * @brief Customizable macros for memory allocation and behavior
 * @{
 */

/** @brief Custom memory allocator (default: malloc) */
#ifndef DA_MALLOC
#define DA_MALLOC malloc
#endif

/** @brief Custom memory reallocator (default: realloc) */
#ifndef DA_REALLOC
#define DA_REALLOC realloc
#endif

/** @brief Custom memory deallocator (default: free) */
#ifndef DA_FREE
#define DA_FREE free
#endif

/** @brief Custom assertion macro (default: assert) */
#ifndef DA_ASSERT
#define DA_ASSERT assert
#endif

/**
 * @brief Enable thread-safe reference counting (default: 0)
 * @note Requires C11 and stdatomic.h support
 * @note Only reference counting is thread-safe, not array operations
 */
#ifndef DA_THREAD_SAFE
#define DA_THREAD_SAFE 0
#endif

/** @} */ // end of config group

/* Thread-safe atomic operations */
#if DA_THREAD_SAFE && __STDC_VERSION__ >= 201112L
    #include <stdatomic.h>
    #define DA_ATOMIC_INT _Atomic int
    #define DA_ATOMIC_FETCH_ADD(ptr, val) atomic_fetch_add(ptr, val)
    #define DA_ATOMIC_FETCH_SUB(ptr, val) atomic_fetch_sub(ptr, val)
    #define DA_ATOMIC_LOAD(ptr) atomic_load(ptr)
    #define DA_ATOMIC_STORE(ptr, val) atomic_store(ptr, val)
#else
    #define DA_ATOMIC_INT int
    #define DA_ATOMIC_FETCH_ADD(ptr, val) (*(ptr) += (val), *(ptr) - (val))
    #define DA_ATOMIC_FETCH_SUB(ptr, val) (*(ptr) -= (val), *(ptr) + (val))
    #define DA_ATOMIC_LOAD(ptr) (*(ptr))
    #define DA_ATOMIC_STORE(ptr, val) (*(ptr) = (val))
#endif

/* Detect typeof support */
#if defined(__GNUC__) || defined(__clang__)
    #define DA_TYPEOF(x) typeof(x)
    #define DA_HAS_TYPEOF 1
#else
    #define DA_HAS_TYPEOF 0
#endif

/**
 * @defgroup types Core Types
 * @brief Main data structures for arrays and builders
 * @{
 */

/**
 * @brief Reference-counted dynamic array structure
 * @note Do not access fields directly - use provided functions and macros
 * @note Thread-safe reference counting when DA_THREAD_SAFE=1
 */
typedef struct {
    DA_ATOMIC_INT ref_count;  /**< @brief Reference count (atomic if DA_THREAD_SAFE=1) */
    int length;               /**< @brief Current number of elements */
    int capacity;             /**< @brief Allocated capacity */
    int element_size;         /**< @brief Size of each element in bytes */
    void *data;               /**< @brief Pointer to element data */
} da_array_t, *da_array;

/**
 * @brief ArrayBuffer-style builder for efficient array construction
 * @note Not thread-safe - use from single thread only
 * @note Always uses doubling growth strategy for fast construction
 * @note Convert to da_array with da_builder_to_array() for sharing/efficiency
 */
typedef struct {
    int length;               /**< @brief Current number of elements */
    int capacity;             /**< @brief Allocated capacity */
    int element_size;         /**< @brief Size of each element in bytes */
    void *data;               /**< @brief Pointer to element data */
} da_builder_t, *da_builder;

/** @} */ // end of types group

/**
 * @defgroup array_lifecycle Array Lifecycle
 * @brief Functions for creating, sharing, and destroying arrays
 * @{
 */

/**
 * @brief Creates a new dynamic array with reference counting
 * @param element_size Size in bytes of each element (must be > 0)
 * @param initial_capacity Initial capacity (0 is valid for deferred allocation)
 * @return New array with ref_count = 1
 * @note Asserts on allocation failure or invalid parameters
 * @note Uses configured growth strategy (DA_GROWTH) for expansions
 * @note Thread-safe reference counting if DA_THREAD_SAFE=1
 *
 * @code
 * da_array arr = da_create(sizeof(int), 10);
 * DA_PUSH(arr, 42);
 * da_release(&arr);
 * @endcode
 */
da_array da_create(int element_size, int initial_capacity);

/**
 * @brief Releases a reference to an array, potentially freeing it
 * @param arr Pointer to array pointer (will be set to NULL)
 * @note Always sets *arr to NULL for safety, regardless of ref count
 * @note Only frees memory when ref_count reaches 0
 * @note Thread-safe if DA_THREAD_SAFE=1
 * @note Asserts if arr or *arr is NULL
 *
 * @code
 * da_array arr = da_create(sizeof(int), 5);
 * da_release(&arr);  // arr becomes NULL, memory freed
 * @endcode
 */
void da_release(da_array* arr);

/**
 * @brief Increments reference count for sharing an array
 * @param arr Array to retain (must not be NULL)
 * @return The same array pointer (for convenience)
 * @note Thread-safe if DA_THREAD_SAFE=1
 * @note Use da_release() to decrement reference count
 *
 * @code
 * da_array shared = da_retain(original_array);
 * pass_to_worker_thread(shared);
 * da_release(&shared);  // Decrements count, shared becomes NULL
 * @endcode
 */
da_array da_retain(da_array arr);

/** @} */ // end of array_lifecycle group

/**
 * @defgroup array_access Array Access
 * @brief Functions for reading and writing array elements
 * @{
 */

/**
 * @brief Gets a pointer to an element at the specified index
 * @param arr Array to access (must not be NULL)
 * @param index Element index (must be >= 0 and < length)
 * @return Pointer to element at index
 * @note Asserts on out-of-bounds access
 * @note Returned pointer is valid until array is modified or released
 *
 * @code
 * int* ptr = (int*)da_get(arr, 0);
 * *ptr = 42;  // Direct modification
 * @endcode
 */
void* da_get(da_array arr, int index);

/**
 * @brief Gets direct pointer to the underlying data array
 * @param arr Array to access (must not be NULL)
 * @return Pointer to raw data array (like stb_ds.h style)
 * @note Enables direct indexing: ((int*)da_data(arr))[i]
 * @note Pointer is valid until array is modified or released
 * @note No bounds checking - use with care
 *
 * @code
 * int* data = (int*)da_data(arr);
 * data[0] = 42;  // Direct array-style access
 * data[1] = 99;
 * @endcode
 */
void* da_data(da_array arr);

/**
 * @brief Sets the value of an element at the specified index
 * @param arr Array to modify (must not be NULL)
 * @param index Element index (must be >= 0 and < length)
 * @param element Pointer to element data to copy (must not be NULL)
 * @note Asserts on out-of-bounds access or NULL parameters
 * @note Copies element_size bytes from element pointer
 *
 * @code
 * int value = 42;
 * da_set(arr, 0, &value);
 * @endcode
 */
void da_set(da_array arr, int index, const void* element);

/** @} */ // end of array_access group

/**
 * @defgroup array_modification Array Modification
 * @brief Functions for adding, removing, and clearing elements
 * @{
 */

/**
 * @brief Appends an element to the end of the array
 * @param arr Array to modify (must not be NULL)
 * @param element Pointer to element data to copy (must not be NULL)
 * @note Automatically grows array capacity if needed
 * @note Asserts on allocation failure or NULL parameters
 * @note Uses configured growth strategy (DA_GROWTH)
 *
 * @code
 * int value = 42;
 * da_push(arr, &value);
 * @endcode
 */
void da_push(da_array arr, const void* element);

/**
 * @brief Removes and optionally returns the last element
 * @param arr Array to modify (must not be NULL)
 * @param out Optional pointer to store popped element (can be NULL)
 * @note Asserts if array is empty
 * @note If out is NULL, element is discarded
 * @note Does not shrink capacity
 *
 * @code
 * int popped;
 * da_pop(arr, &popped);  // Get the value
 * da_pop(arr, NULL);     // Discard the value
 * @endcode
 */
void da_pop(da_array arr, void* out);

/**
 * @brief Removes all elements from the array
 * @param arr Array to clear (must not be NULL)
 * @note Sets length to 0 but preserves capacity
 * @note Does not free allocated memory (use da_resize(arr, 0) for that)
 *
 * @code
 * da_clear(arr);
 * assert(da_length(arr) == 0);
 * @endcode
 */
void da_clear(da_array arr);

/** @} */ // end of array_modification group

/**
 * @defgroup array_utility Array Utility
 * @brief Functions for querying and managing array size and capacity
 * @{
 */

/**
 * @brief Gets the current number of elements in the array
 * @param arr Array to query (must not be NULL)
 * @return Number of elements currently in the array
 *
 * @code
 * for (int i = 0; i < da_length(arr); i++) {
 *     process_element(da_get(arr, i));
 * }
 * @endcode
 */
int da_length(da_array arr);

/**
 * @brief Gets the current allocated capacity of the array
 * @param arr Array to query (must not be NULL)
 * @return Number of elements that can be stored without reallocation
 *
 * @code
 * printf("Array using %d/%d slots\n", da_length(arr), da_capacity(arr));
 * @endcode
 */
int da_capacity(da_array arr);

/**
 * @brief Ensures the array has at least the specified capacity
 * @param arr Array to modify (must not be NULL)
 * @param new_capacity Minimum capacity required (must be >= 0)
 * @note Only increases capacity, never decreases
 * @note Asserts on allocation failure
 * @note Useful for avoiding multiple reallocations when size is known
 *
 * @code
 * da_reserve(arr, 1000);  // Ensure space for 1000 elements
 * for (int i = 0; i < 1000; i++) {
 *     da_push(arr, &i);  // No reallocations needed
 * }
 * @endcode
 */
void da_reserve(da_array arr, int new_capacity);

/**
 * @brief Changes the array length, growing or shrinking as needed
 * @param arr Array to modify (must not be NULL)
 * @param new_length New length for the array (must be >= 0)
 * @note If growing, new elements are zero-initialized
 * @note If shrinking, excess elements are discarded
 * @note Automatically adjusts capacity if needed
 * @note Asserts on allocation failure
 *
 * @code
 * da_resize(arr, 100);  // Array now has exactly 100 elements
 * // Elements 0..old_length-1 preserve their values
 * // Elements old_length..99 are zero-initialized
 * @endcode
 */
void da_resize(da_array arr, int new_length);

/** @} */ // end of array_utility group

/**
 * @defgroup builder_lifecycle Builder Lifecycle
 * @brief Functions for creating and managing ArrayBuffer-style builders
 * @{
 */

/**
 * @brief Creates a new array builder for efficient construction
 * @param element_size Size in bytes of each element (must be > 0)
 * @return New builder with length = 0 and capacity = 0
 * @note Builders always use doubling growth strategy for fast construction
 * @note Not thread-safe - use from single thread only
 * @note Use da_builder_to_array() to convert to ref-counted array
 * @note Asserts on allocation failure
 *
 * @code
 * da_builder builder = da_builder_create(sizeof(int));
 * DA_BUILDER_APPEND(builder, 42);
 * da_array arr = da_builder_to_array(&builder);
 * @endcode
 */
da_builder da_builder_create(int element_size);

/**
 * @brief Converts builder to a ref-counted array with exact capacity
 * @param builder Pointer to builder pointer (will be set to NULL)
 * @return New da_array with capacity = length (no wasted memory)
 * @note Builder is consumed and *builder is set to NULL
 * @note Resulting array has ref_count = 1
 * @note Memory is reallocated to exact size for efficiency
 * @note Perfect for microcontroller memory optimization
 *
 * @code
 * da_builder builder = DA_BUILDER_CREATE(int);
 * DA_BUILDER_APPEND(builder, 42);
 * da_array arr = da_builder_to_array(&builder);  // builder becomes NULL
 * assert(da_capacity(arr) == da_length(arr));    // Exact sizing
 * @endcode
 */
da_array da_builder_to_array(da_builder* builder);

/**
 * @brief Removes all elements from the builder
 * @param builder Builder to clear (must not be NULL)
 * @note Sets length to 0 but preserves capacity
 * @note Allows reusing builder for multiple constructions
 *
 * @code
 * da_builder_clear(builder);
 * assert(da_builder_length(builder) == 0);
 * @endcode
 */
void da_builder_clear(da_builder builder);

/**
 * @brief Destroys a builder and frees its memory
 * @param builder Pointer to builder pointer (will be set to NULL)
 * @note Use this if you don't want to convert to da_array
 * @note Always sets *builder to NULL
 *
 * @code
 * da_builder builder = da_builder_create(sizeof(int));
 * // ... use builder ...
 * da_builder_destroy(&builder);  // builder becomes NULL
 * @endcode
 */
void da_builder_destroy(da_builder* builder);

/** @} */ // end of builder_lifecycle group

/**
 * @defgroup builder_modification Builder Modification
 * @brief Functions for adding elements to builders
 * @{
 */

/**
 * @brief Appends an element to the builder
 * @param builder Builder to modify (must not be NULL)
 * @param element Pointer to element data to copy (must not be NULL)
 * @note Always uses doubling growth strategy for fast construction
 * @note Asserts on allocation failure or NULL parameters
 * @note Much faster than da_push() for bulk construction
 *
 * @code
 * da_builder builder = DA_BUILDER_CREATE(int);
 * int value = 42;
 * da_builder_append(builder, &value);
 * @endcode
 */
void da_builder_append(da_builder builder, const void* element);

/** @} */ // end of builder_modification group

/**
 * @defgroup builder_utility Builder Utility
 * @brief Functions for querying and accessing builder elements
 * @{
 */

/**
 * @brief Gets the current number of elements in the builder
 * @param builder Builder to query (must not be NULL)
 * @return Number of elements currently in the builder
 */
int da_builder_length(da_builder builder);

/**
 * @brief Gets the current allocated capacity of the builder
 * @param builder Builder to query (must not be NULL)
 * @return Number of elements that can be stored without reallocation
 */
int da_builder_capacity(da_builder builder);

/**
 * @brief Gets a pointer to an element at the specified index
 * @param builder Builder to access (must not be NULL)
 * @param index Element index (must be >= 0 and < length)
 * @return Pointer to element at index
 * @note Asserts on out-of-bounds access
 */
void* da_builder_get(da_builder builder, int index);

/**
 * @brief Sets the value of an element at the specified index
 * @param builder Builder to modify (must not be NULL)
 * @param index Element index (must be >= 0 and < length)
 * @param element Pointer to element data to copy (must not be NULL)
 * @note Asserts on out-of-bounds access or NULL parameters
 */
void da_builder_set(da_builder builder, int index, const void* element);

/** @} */ // end of builder_utility group

/**
 * @defgroup array_macros Type-Safe Array Macros
 * @brief Convenient type-safe macros for array operations
 * @{
 */

/**
 * @def DA_CREATE(T, cap)
 * @brief Type-safe array creation
 * @param T Element type (e.g., int, float, struct mytype)
 * @param cap Initial capacity
 * @return New da_array sized for type T
 *
 * @code
 * da_array arr = DA_CREATE(int, 10);
 * @endcode
 */

/**
 * @def DA_PUSH(arr, val)
 * @brief Type-safe element append (with typeof support)
 * @param arr Array to modify
 * @param val Value to append
 * @note With typeof support: DA_PUSH(arr, 42)
 * @note Without typeof: DA_PUSH(arr, 42, int)
 */

/**
 * @def DA_PUT(arr, i, val)
 * @brief Type-safe element assignment (with typeof support)
 * @param arr Array to modify
 * @param i Index to set
 * @param val Value to assign
 * @note With typeof support: DA_PUT(arr, 0, 42)
 * @note Without typeof: DA_PUT(arr, 0, 42, int)
 */

/**
 * @def DA_AT(arr, i, T)
 * @brief Type-safe element access by value
 * @param arr Array to access
 * @param i Index to get
 * @param T Element type
 * @return Element value at index i
 *
 * @code
 * int value = DA_AT(arr, 0, int);
 * @endcode
 */

/**
 * @def DA_LEN(arr)
 * @brief Get array length (shorthand for da_length)
 */

/**
 * @def DA_CAP(arr)
 * @brief Get array capacity (shorthand for da_capacity)
 */

/**
 * @def DA_POP(arr, out_ptr)
 * @brief Pop last element (shorthand for da_pop)
 */

/**
 * @def DA_CLEAR(arr)
 * @brief Clear array (shorthand for da_clear)
 */

/**
 * @def DA_RESERVE(arr, cap)
 * @brief Reserve capacity (shorthand for da_reserve)
 */

/**
 * @def DA_RESIZE(arr, len)
 * @brief Resize array (shorthand for da_resize)
 */

/** @} */ // end of array_macros group
#if DA_HAS_TYPEOF
    #define DA_CREATE(T, cap) da_create(sizeof(T), cap)
    #define DA_PUSH(arr, val) do { DA_TYPEOF(val) _temp = (val); da_push(arr, &_temp); } while(0)
    #define DA_PUT(arr, i, val) do { DA_TYPEOF(val) _temp = (val); da_set(arr, i, &_temp); } while(0)
#else
    #define DA_CREATE(T, cap) da_create(sizeof(T), cap)
    #define DA_PUSH(arr, val, T) do { T _temp = (val); da_push(arr, &_temp); } while(0)
    #define DA_PUT(arr, i, val, T) do { T _temp = (val); da_set(arr, i, &_temp); } while(0)
#endif

#define DA_LEN(arr) da_length(arr)
#define DA_CAP(arr) da_capacity(arr)
#define DA_AT(arr, i, T) (*(T*)da_get(arr, i))
#define DA_POP(arr, out_ptr) da_pop(arr, out_ptr)
#define DA_CLEAR(arr) da_clear(arr)
#define DA_RESERVE(arr, cap) da_reserve(arr, cap)
#define DA_RESIZE(arr, len) da_resize(arr, len)

/**
 * @defgroup builder_macros Type-Safe Builder Macros
 * @brief Convenient type-safe macros for builder operations
 * @{
 */

/**
 * @def DA_BUILDER_CREATE(T)
 * @brief Type-safe builder creation
 * @param T Element type (e.g., int, float, struct mytype)
 * @return New da_builder sized for type T
 *
 * @code
 * da_builder builder = DA_BUILDER_CREATE(int);
 * @endcode
 */

/**
 * @def DA_BUILDER_APPEND(builder, val)
 * @brief Type-safe element append to builder (with typeof support)
 * @param builder Builder to modify
 * @param val Value to append
 * @note With typeof support: DA_BUILDER_APPEND(builder, 42)
 * @note Without typeof: DA_BUILDER_APPEND(builder, 42, int)
 */

/**
 * @def DA_BUILDER_PUT(builder, i, val)
 * @brief Type-safe element assignment in builder (with typeof support)
 * @param builder Builder to modify
 * @param i Index to set
 * @param val Value to assign
 * @note With typeof support: DA_BUILDER_PUT(builder, 0, 42)
 * @note Without typeof: DA_BUILDER_PUT(builder, 0, 42, int)
 */

/**
 * @def DA_BUILDER_AT(builder, i, T)
 * @brief Type-safe element access by value from builder
 * @param builder Builder to access
 * @param i Index to get
 * @param T Element type
 * @return Element value at index i
 *
 * @code
 * int value = DA_BUILDER_AT(builder, 0, int);
 * @endcode
 */

/**
 * @def DA_BUILDER_LEN(builder)
 * @brief Get builder length (shorthand for da_builder_length)
 */

/**
 * @def DA_BUILDER_CAP(builder)
 * @brief Get builder capacity (shorthand for da_builder_capacity)
 */

/**
 * @def DA_BUILDER_CLEAR(builder)
 * @brief Clear builder (shorthand for da_builder_clear)
 */

/**
 * @def DA_BUILDER_TO_ARRAY(builder)
 * @brief Convert builder to array (shorthand for da_builder_to_array)
 */

/** @} */ // end of builder_macros group
#if DA_HAS_TYPEOF
    #define DA_BUILDER_CREATE(T) da_builder_create(sizeof(T))
    #define DA_BUILDER_APPEND(builder, val) do { DA_TYPEOF(val) _temp = (val); da_builder_append(builder, &_temp); } while(0)
    #define DA_BUILDER_PUT(builder, i, val) do { DA_TYPEOF(val) _temp = (val); da_builder_set(builder, i, &_temp); } while(0)
#else
    #define DA_BUILDER_CREATE(T) da_builder_create(sizeof(T))
    #define DA_BUILDER_APPEND(builder, val, T) do { T _temp = (val); da_builder_append(builder, &_temp); } while(0)
    #define DA_BUILDER_PUT(builder, i, val, T) do { T _temp = (val); da_builder_set(builder, i, &_temp); } while(0)
#endif

#define DA_BUILDER_LEN(builder) da_builder_length(builder)
#define DA_BUILDER_CAP(builder) da_builder_capacity(builder)
#define DA_BUILDER_AT(builder, i, T) (*(T*)da_builder_get(builder, i))
#define DA_BUILDER_CLEAR(builder) da_builder_clear(builder)
#define DA_BUILDER_TO_ARRAY(builder) da_builder_to_array(builder)

/* Implementation */
#ifdef DYNAMIC_ARRAY_IMPLEMENTATION

static int da_grow_capacity(int current_capacity, int min_needed) {
    int new_capacity = current_capacity;

#ifdef DA_GROWTH
    /* Fixed growth strategy */
    while (new_capacity < min_needed) {
        new_capacity += DA_GROWTH;
    }
#else
    /* Doubling strategy */
    if (new_capacity == 0) new_capacity = 1;
    while (new_capacity < min_needed) {
        new_capacity *= 2;
    }
#endif

    return new_capacity;
}

static int da_builder_grow_capacity(int current_capacity, int min_needed) {
    /* Builders always use doubling strategy for fast construction */
    int new_capacity = current_capacity;
    if (new_capacity == 0) new_capacity = 1;
    while (new_capacity < min_needed) {
        new_capacity *= 2;
    }
    return new_capacity;
}

/* Array Implementation */

da_array da_create(int element_size, int initial_capacity) {
    DA_ASSERT(element_size > 0);
    DA_ASSERT(initial_capacity >= 0);

    da_array arr = (da_array)DA_MALLOC(sizeof(da_array_t));
    DA_ASSERT(arr != NULL);

    DA_ATOMIC_STORE(&arr->ref_count, 1);
    arr->length = 0;
    arr->capacity = initial_capacity;
    arr->element_size = element_size;

    if (initial_capacity > 0) {
        arr->data = DA_MALLOC(initial_capacity * element_size);
        DA_ASSERT(arr->data != NULL);
    } else {
        arr->data = NULL;
    }

    return arr;
}

void da_release(da_array* arr) {
    DA_ASSERT(arr != NULL);
    DA_ASSERT(*arr != NULL);

    int old_count = DA_ATOMIC_FETCH_SUB(&(*arr)->ref_count, 1);

    if (old_count == 1) {  /* We were the last reference */
        if ((*arr)->data) {
            DA_FREE((*arr)->data);
        }
        DA_FREE(*arr);
    }

    *arr = NULL;  /* Always NULL the pointer for safety */
}

da_array da_retain(da_array arr) {
    DA_ASSERT(arr != NULL);
    DA_ATOMIC_FETCH_ADD(&arr->ref_count, 1);
    return arr;
}

void* da_get(da_array arr, int index) {
    DA_ASSERT(arr != NULL);
    DA_ASSERT(index >= 0 && index < arr->length);
    return (char*)arr->data + (index * arr->element_size);
}

void* da_data(da_array arr) {
    DA_ASSERT(arr != NULL);
    return arr->data;
}

void da_set(da_array arr, int index, const void* element) {
    DA_ASSERT(arr != NULL);
    DA_ASSERT(element != NULL);
    DA_ASSERT(index >= 0 && index < arr->length);

    void* dest = (char*)arr->data + (index * arr->element_size);
    memcpy(dest, element, arr->element_size);
}

void da_push(da_array arr, const void* element) {
    DA_ASSERT(arr != NULL);
    DA_ASSERT(element != NULL);

    if (arr->length >= arr->capacity) {
        int new_capacity = da_grow_capacity(arr->capacity, arr->length + 1);
        arr->data = DA_REALLOC(arr->data, new_capacity * arr->element_size);
        DA_ASSERT(arr->data != NULL);
        arr->capacity = new_capacity;
    }

    void* dest = (char*)arr->data + (arr->length * arr->element_size);
    memcpy(dest, element, arr->element_size);
    arr->length++;
}

void da_pop(da_array arr, void* out) {
    DA_ASSERT(arr != NULL);
    DA_ASSERT(arr->length > 0);

    arr->length--;

    if (out != NULL) {
        void* src = (char*)arr->data + (arr->length * arr->element_size);
        memcpy(out, src, arr->element_size);
    }
}

void da_clear(da_array arr) {
    DA_ASSERT(arr != NULL);
    arr->length = 0;
}

int da_length(da_array arr) {
    DA_ASSERT(arr != NULL);
    return arr->length;
}

int da_capacity(da_array arr) {
    DA_ASSERT(arr != NULL);
    return arr->capacity;
}

void da_reserve(da_array arr, int new_capacity) {
    DA_ASSERT(arr != NULL);
    DA_ASSERT(new_capacity >= 0);

    if (new_capacity > arr->capacity) {
        arr->data = DA_REALLOC(arr->data, new_capacity * arr->element_size);
        DA_ASSERT(arr->data != NULL);
        arr->capacity = new_capacity;
    }
}

void da_resize(da_array arr, int new_length) {
    DA_ASSERT(arr != NULL);
    DA_ASSERT(new_length >= 0);

    if (new_length > arr->capacity) {
        da_reserve(arr, new_length);
    }

    if (new_length > arr->length) {
        /* Zero-fill new elements */
        void* start = (char*)arr->data + (arr->length * arr->element_size);
        int bytes_to_zero = (new_length - arr->length) * arr->element_size;
        memset(start, 0, bytes_to_zero);
    }

    arr->length = new_length;
}

/* Builder Implementation */

da_builder da_builder_create(int element_size) {
    DA_ASSERT(element_size > 0);

    da_builder builder = (da_builder)DA_MALLOC(sizeof(da_builder_t));
    DA_ASSERT(builder != NULL);

    builder->length = 0;
    builder->capacity = 0;
    builder->element_size = element_size;
    builder->data = NULL;

    return builder;
}

void da_builder_append(da_builder builder, const void* element) {
    DA_ASSERT(builder != NULL);
    DA_ASSERT(element != NULL);

    if (builder->length >= builder->capacity) {
        int new_capacity = da_builder_grow_capacity(builder->capacity, builder->length + 1);
        builder->data = DA_REALLOC(builder->data, new_capacity * builder->element_size);
        DA_ASSERT(builder->data != NULL);
        builder->capacity = new_capacity;
    }

    void* dest = (char*)builder->data + (builder->length * builder->element_size);
    memcpy(dest, element, builder->element_size);
    builder->length++;
}

da_array da_builder_to_array(da_builder* builder) {
    DA_ASSERT(builder != NULL);
    DA_ASSERT(*builder != NULL);

    da_builder b = *builder;

    /* Create new da_array */
    da_array arr = (da_array)DA_MALLOC(sizeof(da_array_t));
    DA_ASSERT(arr != NULL);

    DA_ATOMIC_STORE(&arr->ref_count, 1);
    arr->length = b->length;
    arr->capacity = b->length;  /* Exact capacity = length */
    arr->element_size = b->element_size;

    if (b->length > 0) {
        /* Shrink to exact size */
        arr->data = DA_REALLOC(b->data, b->length * b->element_size);
        DA_ASSERT(arr->data != NULL);
    } else {
        arr->data = NULL;
        if (b->data) {
            DA_FREE(b->data);
        }
    }

    /* Free builder */
    DA_FREE(b);
    *builder = NULL;

    return arr;
}

void da_builder_clear(da_builder builder) {
    DA_ASSERT(builder != NULL);
    builder->length = 0;
}

void da_builder_destroy(da_builder* builder) {
    DA_ASSERT(builder != NULL);
    DA_ASSERT(*builder != NULL);

    if ((*builder)->data) {
        DA_FREE((*builder)->data);
    }
    DA_FREE(*builder);
    *builder = NULL;
}

int da_builder_length(da_builder builder) {
    DA_ASSERT(builder != NULL);
    return builder->length;
}

int da_builder_capacity(da_builder builder) {
    DA_ASSERT(builder != NULL);
    return builder->capacity;
}

void* da_builder_get(da_builder builder, int index) {
    DA_ASSERT(builder != NULL);
    DA_ASSERT(index >= 0 && index < builder->length);
    return (char*)builder->data + (index * builder->element_size);
}

void da_builder_set(da_builder builder, int index, const void* element) {
    DA_ASSERT(builder != NULL);
    DA_ASSERT(element != NULL);
    DA_ASSERT(index >= 0 && index < builder->length);

    void* dest = (char*)builder->data + (index * builder->element_size);
    memcpy(dest, element, builder->element_size);
}

#endif /* DYNAMIC_ARRAY_IMPLEMENTATION */

#endif /* DYNAMIC_ARRAY_H */