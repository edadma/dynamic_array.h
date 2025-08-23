#include <stdio.h>

// Test with type inference disabled to ensure backward compatibility
#define DA_NOT_USE_TYPE_GENERIC
#define DYNAMIC_ARRAY_IMPLEMENTATION
#include "dynamic_array.h"

int main() {
    // Test that old-style macros still work
    da_array arr = DA_CREATE(int, 10);
    
    // Should require explicit type parameter
    DA_PUSH(arr, 42, int);
    DA_PUT(arr, 0, 100, int);
    DA_INSERT(arr, 0, 5, int);
    
    // Check values
    int val = DA_AT(arr, 0, int);
    printf("First value: %d (expected 5)\n", val);
    
    val = DA_AT(arr, 1, int);
    printf("Second value: %d (expected 100)\n", val);
    
    // Test builder macros
    da_builder builder = DA_BUILDER_CREATE(int);
    DA_BUILDER_APPEND(builder, 10, int);
    DA_BUILDER_PUT(builder, 0, 20, int);
    
    da_array result = da_builder_to_array(&builder);
    printf("Builder result: %d (expected 20)\n", DA_AT(result, 0, int));
    
    da_release(&arr);
    da_release(&result);
    
    printf("Backward compatibility test passed!\n");
    return 0;
}