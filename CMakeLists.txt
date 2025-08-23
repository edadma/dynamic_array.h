cmake_minimum_required(VERSION 3.16)
project(dynamic_array C)

# Use C11 for atomic support
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Compiler flags for better debugging and warnings
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -g")

# Unity framework files
set(UNITY_SOURCES
        libs/unity/unity.c
)

# Test executable (can't be named "test" - CMake reserves that name)
add_executable(test_runner
        test.c
        ${UNITY_SOURCES}
)

# Include directories
target_include_directories(test_runner PRIVATE
        .
        libs/unity
)

# Optional: Enable testing with CTest
enable_testing()
add_test(NAME da_array_tests COMMAND test_runner)