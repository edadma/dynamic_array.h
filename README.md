# dynamic_array.h

A high-performance, thread-safe, reference-counted dynamic array library for C. Designed for maximum portability across PC and microcontroller targets.

## Features

🚀 **High Performance**
- Lock-free atomic reference counting
- Configurable growth strategies (fixed increment or doubling)
- Zero-copy data access with direct pointer operations

🛡️ **Memory Safe**
- Reference counting prevents memory leaks and use-after-free
- Automatic cleanup when last reference is released
- Pointers always NULLed after release for safety

🔧 **Developer Friendly**
- Type-safe macros with `typeof` support
- JavaScript-like array semantics
- Comprehensive assert-based error checking
- Single header-only library

🌐 **Cross-Platform**
- Works on Linux, Windows, macOS
- Microcontroller support: Raspberry Pi Pico, ESP32-C3, ARM Cortex-M
- Both GCC and Clang compatible
- Graceful fallback for compilers without `typeof`

⚡ **Thread-Safe**
- Optional atomic reference counting (C11 required)
- Lock-free performance on multi-core systems
- Safe sharing between threads without mutexes

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
    
    // Cleanup (decrements ref count, frees when count reaches 0)
    da_release(&arr);     // arr becomes NULL
    da_release(&shared);  // shared becomes NULL, memory freed
    
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

// Thread safety (requires C11)
#define DA_THREAD_SAFE 1

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
#define DA_LEN(arr)                 // Get length
#define DA_CAP(arr)                 // Get capacity
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

## Thread Safety

Enable thread-safe reference counting:

```c
#define DA_THREAD_SAFE 1
#define DYNAMIC_ARRAY_IMPLEMENTATION
#include "dynamic_array.h"
```

**Thread-Safe Operations (Lock-Free):**
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
- C99 minimum (C11 for thread safety)
- `<stdlib.h>`, `<string.h>`, `<assert.h>`
- `<stdatomic.h>` (only if `DA_THREAD_SAFE=1`)

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

All 20+ tests should pass, covering:
- Creation and destruction
- Reference counting behavior
- Growth strategies
- Type-safe macros
- Edge cases and stress tests
- Thread safety validation

## Use Cases

**Perfect for:**
- Interpreters and virtual machines
- Game engines (entity lists, render queues)
- Embedded systems with dynamic data
- Multi-threaded applications sharing read-only data
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