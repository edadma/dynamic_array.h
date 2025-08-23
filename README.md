# dynamic_array.h

A high-performance, (optionally atomic) reference-counted dynamic array library for C. Designed for maximum portability across PC and microcontroller targets.

## Features

**High Performance**
- Lock-free atomic reference counting
- Configurable growth strategies (fixed increment or doubling)
- Zero-copy data access with direct pointer operations

**Memory Safe**
- Reference counting prevents memory leaks and use-after-free
- Automatic cleanup when last reference is released
- Pointers always NULLed after release for safety

**Developer Friendly**
- Type-safe macros with `typeof` support
- JavaScript-like array semantics
- Comprehensive assert-based error checking
- Single header-only library

**Cross-Platform**
- Works on Linux, Windows, macOS
- Microcontroller support: Raspberry Pi Pico, ESP32-C3, ARM Cortex-M
- Both GCC and Clang compatible
- Graceful fallback for compilers without `typeof`

**Atomic reference counting**
- Optional atomic reference counting (C11 required)
- Lock-free performance on multi-core systems
- Safe sharing between threads without mutexes

**Functional Programming**
- Filter, map, and reduce operations with exact capacity allocation
- Single-pass optimizations for memory efficiency
- Perfect for data processing pipelines

**Builder Pattern**
- Efficient construction with pre-allocation
- Bulk append operations for performance
- Zero-waste capacity management

## What's New

### v0.1.1 (Latest)
**Type Inference Support** (Thanks to @Maqi-x via PR #1)
- Added automatic type inference for C23, C++11, and GCC/Clang
- Macros like `DA_PUSH` now work without explicit type parameter when compiler supports it
- Backward compatible - older compilers still work with explicit types
- Improved developer ergonomics with modern C/C++ standards

**API Improvements**
- Renamed `DA_LEN`/`DA_CAP` to `DA_LENGTH`/`DA_CAPACITY` for clarity
- Similarly renamed builder macros for consistency
- Enhanced documentation with comprehensive examples

### v0.1.0
**Functional Programming Complete**
- Added `da_reduce()` function with accumulator pattern for array reduction
- Completes the functional programming trinity: filter → map → reduce
- User-controlled memory management with result buffers

**Builder Pattern Enhancements** 
- New `da_builder_reserve()` for efficient pre-allocation
- New `da_builder_append_array()` for bulk array operations
- Optimized construction patterns for high-performance scenarios

**Performance Optimizations**
- Optimized `da_filter()` to use builder pattern internally (single-pass filtering)
- Improved memory efficiency across all operations
- Exact capacity allocation throughout the API

**Comprehensive Testing**
- Expanded test suite to 110+ tests (from 20+)
- Complete coverage of all new functions and edge cases
- Enhanced reliability and stability validation

### v0.0.1 (Initial Release)
- Core dynamic array functionality with reference counting
- Atomic ref counting
- Type-safe macros and cross-platform compatibility
- Basic builder pattern implementation
- Initial filter and map operations

## Quick Start

```c
#define DYNAMIC_ARRAY_IMPLEMENTATION
#include "dynamic_array.h"

int main() {
    // Create a typed array
    da_array arr = DA_CREATE(int, 10);
    
    // Add elements
    DA_PUSH(arr, 42);
    DA_PUSH(arr, 99);
    
    // Access elements
    int value = DA_AT(arr, 0, int);  // 42
    
    // Direct pointer access (like stb_ds.h)
    int* data = (int*)da_data(arr);
    data[1] = 100;
    
    // Reference counting
    da_array shared = da_retain(arr);
    
    // Functional programming - filter, map, reduce
    da_array evens = da_filter(arr, is_even_predicate, NULL);
    da_array doubled = da_map(evens, double_mapper, NULL);
    
    int sum = 0, total;
    da_reduce(doubled, &sum, &total, sum_reducer, NULL);
    
    // Builder pattern for efficient construction
    da_builder builder = DA_BUILDER_CREATE(int);
    da_builder_reserve(builder, 1000);  // Pre-allocate
    da_builder_append_array(builder, existing_data);
    da_array result = da_builder_to_array(&builder);
    
    // Cleanup (decrements ref count, frees when count reaches 0)
    da_release(&arr);
    da_release(&shared);
    da_release(&evens);
    da_release(&doubled);
    da_release(&result);
    
    return 0;
}
```

## Configuration

Configure the library before including:

```c
// Custom allocators
#define DA_MALLOC my_malloc
#define DA_REALLOC my_realloc  
#define DA_FREE my_free

// Custom assert
#define DA_ASSERT my_assert

// Growth strategy
#define DA_GROWTH 16        // Fixed growth of 16 elements
// #define DA_GROWTH undefined  // Use doubling strategy (default)

// Atomic reference counting (requires C11)
#define DA_ATOMIC_REFCOUNT 1

#define DYNAMIC_ARRAY_IMPLEMENTATION
#include "dynamic_array.h"
```

## API Reference

### Creation and Reference Counting

```c
da_array da_create(int element_size, int initial_capacity);
da_array da_retain(da_array arr);      // Increment reference count
void da_release(da_array* arr);        // Decrement ref count, NULL pointer
```

### Type-Safe Macros

```c
// With typeof support (GCC/Clang)
#define DA_CREATE(T, cap)           // Create typed array
#define DA_PUSH(arr, val)           // Push value
#define DA_PUT(arr, i, val)         // Set element at index

// Without typeof support (strict ISO C)
#define DA_PUSH(arr, val, T)        // Push value with type
#define DA_PUT(arr, i, val, T)      // Set element with type

// Universal macros
#define DA_AT(arr, i, T)            // Get element at index
#define DA_LENGTH(arr)                 // Get length
#define DA_CAPACITY(arr)                 // Get capacity
#define DA_POP(arr, out_ptr)        // Pop last element
#define DA_CLEAR(arr)               // Clear all elements
#define DA_RESERVE(arr, cap)        // Reserve capacity
#define DA_RESIZE(arr, len)         // Resize array
```

### Raw Functions

```c
void* da_get(da_array arr, int index);           // Get element pointer
void* da_data(da_array arr);                     // Get raw data pointer
void da_set(da_array arr, int index, const void* element);
void da_push(da_array arr, const void* element);
void da_pop(da_array arr, void* out);            // out can be NULL
int da_length(da_array arr);
int da_capacity(da_array arr);
void da_clear(da_array arr);
void da_reserve(da_array arr, int new_capacity);
void da_resize(da_array arr, int new_length);
```

## Lock-free Reference Counting

Enable atomic reference counting:

```c
#define DA_ATOMIC_REFCOUNT 1
#define DYNAMIC_ARRAY_IMPLEMENTATION
#include "dynamic_array.h"
```

- `da_retain()` - Atomic reference increment
- `da_release()` - Atomic reference decrement
- Memory cleanup - Only one thread frees when ref count hits 0

**Requires External Synchronization:**
- Array modifications (`DA_PUSH`, `DA_POP`, `da_set`)
- Concurrent access to array contents

**Perfect for sharing immutable or read-mostly data between threads!**

## Growth Strategies

**Fixed Growth** (recommended for microcontrollers):
```c
#define DA_GROWTH 16  // Grow by 16 elements each time
```

**Doubling Growth** (default, good for desktop):
```c
// Don't define DA_GROWTH - uses doubling strategy
```

## Error Handling

The library uses assertions for error detection. All errors fail fast:

- Out-of-bounds access → Assert failure
- Memory allocation failure → Assert failure
- Invalid parameters → Assert failure

This is perfect for embedded systems where you want to catch bugs early rather than handle them gracefully.

## Platform Support

**Tested Platforms:**
- Linux (GCC, Clang)
- Windows (MinGW, MSVC)
- Raspberry Pi Pico (arm-none-eabi-gcc)
- Raspberry Pi Zero/4 (GCC)
- ESP32-C3 (Espressif toolchain)

**Requirements:**
- C11 for atomic reference count operations
- `<stdlib.h>`, `<string.h>`, `<assert.h>`
- `<stdatomic.h>` (only if `DA_ATOMIC_REFCOUNT=1`)

## Building and Testing

The library includes a comprehensive Unity-based test suite:

```bash
# Clone with Unity framework
git clone <your-repo>
cd dynamic_array.h

# Add Unity framework to unity/ directory
# Then build and test:
mkdir build && cd build
cmake ..
make
./test_runner
```

All 110+ tests should pass, covering:
- Creation and destruction
- Reference counting behavior
- Growth strategies and builder patterns
- Type-safe macros
- Functional programming operations
- Edge cases and stress tests
- Atomic reference count validation

## Use Cases

**Perfect for:**
- Interpreters and virtual machines
- Game engines (entity lists, render queues)
- Embedded systems with dynamic data
- Multi-threaded applications sharing read-only data
- Data processing pipelines (ETL operations)
- Scientific computing with functional patterns
- Any C project needing JavaScript-like arrays

**Example - Shared Bytecode in Interpreter:**
```c
// Main thread creates bytecode
da_array bytecode = DA_CREATE(instruction_t, 1000);
compile_to_bytecode(source, bytecode);

// Share with worker threads (lock-free!)
spawn_worker(da_retain(bytecode));
spawn_worker(da_retain(bytecode));

// Main thread releases its reference
da_release(&bytecode);
// Memory freed when last worker finishes
```

**Example - Data Processing Pipeline:**
```c
// Functional data processing with exact memory allocation
da_array sensor_data = load_sensor_readings();
da_array valid = da_filter(sensor_data, is_valid_reading, &threshold);
da_array scaled = da_map(valid, normalize_value, &scale_params);

// Reduce to statistical summary
float stats = {0};
da_reduce(scaled, &stats, &summary, calculate_stats, NULL);

// Efficient bulk construction
da_builder results = DA_BUILDER_CREATE(result_t);
da_builder_reserve(results, expected_count);
da_builder_append_array(results, processed_batch_1);
da_builder_append_array(results, processed_batch_2);
da_array final = da_builder_to_array(&results);  // Exact capacity
```

## Version History

### v0.2.0 (Current)

- **Added**: `da_find_index()` - find first element matching predicate function
- **Added**: `da_contains()` - boolean check for element existence using predicates
- **Added**: `da_sort()` - sort array with custom comparison function and context
- **Improved**: All array construction operations use da_builder internally for efficiency
- **Documentation**: Added CLAUDE.md for AI assistance and comprehensive usage examples
- **Documentation**: Enhanced Doxygen comments with detailed usage examples for new functions

### v0.1.0

- **Foundation**: Complete single-header dynamic array library
- **Features**: Reference counting, atomic operations, type-safe macros, builder pattern
- **Features**: Functional operations (filter, map, reduce)
- **Features**: Advanced manipulation (slice, concat, reverse, swap)
- **Platform Support**: C99+, optional C11 atomic operations, microcontroller support
- **Documentation**: Comprehensive Doxygen documentation with examples

## License

This project is dual-licensed under your choice of:

- [MIT License](LICENSE-MIT)
- [The Unlicense](LICENSE-UNLICENSE)

Choose whichever license works best for your project!

## Contributing

Contributions welcome! Please ensure:
- All tests pass (`make test`)
- Code follows existing style
- New features include tests
- Maintain compatibility across target platforms

---

*Built for performance, designed for portability, crafted for developers.* 🚀