/*
 * DYNAMIC_ARRAY - Reference Counted Dynamic Arrays with ArrayBuffer-style Builder
 *
 * Single header library for mutable dynamic arrays with reference counting.
 * Includes Scala ArrayBuffer-style builder for efficient construction.
 * Designed for portability across PC and microcontroller targets.
 *
 * Configuration:
 *   #define DA_MALLOC malloc     // custom allocator
 *   #define DA_REALLOC realloc   // custom reallocator
 *   #define DA_FREE free         // custom deallocator
 *   #define DA_ASSERT assert     // custom assert macro
 *   #define DA_GROWTH 16         // fixed growth increment (default: doubling)
 *   #define DA_THREAD_SAFE 1     // enable thread-safe reference counting (C11 required)
 *
 * Usage:
 *   #define DYNAMIC_ARRAY_IMPLEMENTATION
 *   #include "dynamic_array.h"
 *
 *   // Regular arrays
 *   da_array arr = da_create(sizeof(int), 10);
 *   DA_PUSH(arr, 42);
 *   int val = DA_AT(arr, 0, int);
 *   da_release(&arr);
 *
 *   // Builder pattern (like Scala's ArrayBuffer)
 *   da_builder builder = DA_BUILDER_CREATE(int);
 *   DA_BUILDER_APPEND(builder, 42);
 *   DA_BUILDER_APPEND(builder, 99);
 *   da_array arr = da_builder_to_array(&builder);  // Exact size, builder becomes NULL
 *   da_release(&arr);
 */

#ifndef DYNAMIC_ARRAY_H
#define DYNAMIC_ARRAY_H

#include <stdlib.h>
#include <string.h>
#include <assert.h>

/* User-configurable defines */
#ifndef DA_MALLOC
#define DA_MALLOC malloc
#endif

#ifndef DA_REALLOC
#define DA_REALLOC realloc
#endif

#ifndef DA_FREE
#define DA_FREE free
#endif

#ifndef DA_ASSERT
#define DA_ASSERT assert
#endif

#ifndef DA_THREAD_SAFE
#define DA_THREAD_SAFE 0
#endif

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

/* Core types */
typedef struct {
    DA_ATOMIC_INT ref_count;
    int length;
    int capacity;
    int element_size;
    void *data;
} da_array_t, *da_array;

typedef struct {
    int length;
    int capacity;
    int element_size;
    void *data;
} da_builder_t, *da_builder;

/* Array function declarations */
da_array da_create(int element_size, int initial_capacity);
void da_release(da_array* arr);
da_array da_retain(da_array arr);

void* da_get(da_array arr, int index);
void* da_data(da_array arr);
void da_set(da_array arr, int index, const void* element);

void da_push(da_array arr, const void* element);
void da_pop(da_array arr, void* out);
void da_clear(da_array arr);

int da_length(da_array arr);
int da_capacity(da_array arr);
void da_reserve(da_array arr, int new_capacity);
void da_resize(da_array arr, int new_length);

/* Builder function declarations */
da_builder da_builder_create(int element_size);
void da_builder_append(da_builder builder, const void* element);
da_array da_builder_to_array(da_builder* builder);
void da_builder_clear(da_builder builder);
void da_builder_destroy(da_builder* builder);

int da_builder_length(da_builder builder);
int da_builder_capacity(da_builder builder);
void* da_builder_get(da_builder builder, int index);
void da_builder_set(da_builder builder, int index, const void* element);

/* Type-safe array macros */
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

/* Type-safe builder macros */
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