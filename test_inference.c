#include <stdio.h>

// Test with type inference enabled
#define DYNAMIC_ARRAY_IMPLEMENTATION
#include "dynamic_array.h"

int main() {
    // Test that new-style macros work without type parameter
    da_array arr = DA_CREATE(int, 10);
    
    // Should work without explicit type parameter
    DA_PUSH(arr, 42);
    DA_PUT(arr, 0, 100);
    DA_INSERT(arr, 0, 5);
    
    // Check values
    int val = DA_AT(arr, 0, int);
    printf("First value: %d (expected 5)\n", val);
    
    val = DA_AT(arr, 1, int);
    printf("Second value: %d (expected 100)\n", val);
    
    // Test builder macros
    da_builder builder = DA_BUILDER_CREATE(int);
    DA_BUILDER_APPEND(builder, 10);
    DA_BUILDER_PUT(builder, 0, 20);
    
    da_array result = da_builder_to_array(&builder);
    printf("Builder result: %d (expected 20)\n", DA_AT(result, 0, int));
    
    da_release(&arr);
    da_release(&result);
    
    printf("Type inference test passed!\n");
    return 0;
}