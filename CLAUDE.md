# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

This is a single-header C library for thread-safe, reference-counted dynamic arrays with ArrayBuffer-style builder support. The library is designed for maximum portability across PC and microcontroller targets.

## Permissions

You have full permission to perform any non-mutating operations without asking, including:
- Reading files (Read tool)
- Searching/grepping through code (Grep tool)
- Listing directories (LS tool)
- Running read-only commands like `git status`, `git diff`, `git log`
- Checking out branches/PRs for reading and testing purposes
- Compiling code and running unit tests
- Any other operation that doesn't modify files or system state

## Architecture

- **Single Header Library**: The entire implementation is in `dynamic_array.h` - include with `#define DYNAMIC_ARRAY_IMPLEMENTATION` before including
- **Two Main Data Structures**:
  - `da_array`: Reference-counted dynamic arrays with optional thread safety
  - `da_builder`: ArrayBuffer-style builder for efficient construction, always converted to `da_array`
- **Type Safety**: Macros provide type-safe operations when `typeof` support is available (GCC/Clang)
- **Memory Management**: Reference counting prevents leaks, automatic cleanup when ref count reaches 0

## Build Commands

### Build and Test
```bash
# Configure and build
mkdir build && cd build
cmake ..
make

# Run tests
./test_runner

# Run specific test with CTest
ctest
```

### Documentation Generation
```bash
# Generate Doxygen documentation
doxygen
# Output goes to docs/ directory
```

## Configuration Options

The library behavior is controlled by macros defined before including:

- `DA_MALLOC`/`DA_REALLOC`/`DA_FREE`: Custom allocators
- `DA_ASSERT`: Custom assert macro
- `DA_GROWTH`: Fixed growth increment (default: doubling strategy)
- `DA_ATOMIC_REFCOUNT 1`: Enable atomic reference counting (requires C11)

## Testing

- Uses Unity testing framework (included in `unity/` directory)
- Test file: `test.c`
- All tests should pass for any changes
- Covers creation, reference counting, type-safe macros, builders, and thread safety

## Key Implementation Details

### Memory Layout
- Arrays store metadata (ref_count, length, capacity, element_size) + data pointer
- Builders are simpler (no ref counting) and always use doubling growth
- `da_builder_to_array()` creates exact-sized array from builder

### Thread Safety
- When `DA_ATOMIC_REFCOUNT=1`: Reference counting operations are atomic and lock-free
- Array content modifications always require external synchronization
- Builders are never thread-safe

### Platform Support
- C99 minimum (C11 for thread safety)
- Tested on Linux, Windows, Raspberry Pi Pico, ESP32-C3
- Uses `typeof` when available for better macro ergonomics

## Growth Strategies
- **Arrays**: Configurable via `DA_GROWTH` (fixed increment) or doubling (default)
- **Builders**: Always use doubling for fast construction

## Common Patterns

### Basic Array Usage
```c
da_array arr = DA_CREATE(int, 10);
DA_PUSH(arr, 42);
int val = DA_AT(arr, 0, int);
da_release(&arr);  // arr becomes NULL
```

### Builder Pattern
```c
da_builder builder = DA_BUILDER_CREATE(int);
DA_BUILDER_APPEND(builder, 42);
da_array arr = da_builder_to_array(&builder);  // builder becomes NULL
```

### Reference Sharing
```c
da_array shared = da_retain(arr);
pass_to_thread(shared);
da_release(&shared);  // shared becomes NULL
```

### Arbitrary Struct Support
```c
typedef struct { int x, y; } Point;
typedef struct { char name[32]; int age; float score; } Person;

da_array points = DA_CREATE(Point, 10);
Point p = {10, 20};
DA_PUSH(points, p);
Point retrieved = DA_AT(points, 0, Point);

da_array people = DA_CREATE(Person, 5);  
Person alice = {"Alice", 25, 95.5f};
DA_PUSH(people, alice);
Person* ptr = (Person*)da_get(people, 0);  // Direct access
```