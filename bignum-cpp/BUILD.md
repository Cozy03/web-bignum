# bignum-cpp - Build, Installation, and Usage Guide

This document provides comprehensive instructions for building, installing, and using the `bignum-cpp` arbitrary-precision arithmetic library.

## Prerequisites

- **C++17 Compiler with `__int128` Support**: The library explicitly requires 128-bit integer support.
  - **Primary Targets**: GCC (7+) or Clang (5+) on a 64-bit architecture (x86_64, ARM64).
  - **Unsupported**: MSVC on Windows is currently not supported as it lacks native `__int128` type support, which this library's build system requires.
- **CMake**: Version 3.12 or higher.
- **Git**: Required for cloning the repository.

## Quick Start

The fastest way to get the library built and tested.

```bash
# 1. Clone the repository
git clone [https://github.com/Cozy03/bignum-cpp.git](https://github.com/Cozy03/bignum-cpp.git)
cd bignum-cpp

# 2. Create a build directory
mkdir build && cd build

# 3. Configure the project (for a Release build)
cmake -DCMAKE_BUILD_TYPE=Release ..

# 4. Build the library and tests (using all available CPU cores)
cmake --build . -j

# 5. Run tests
ctest --output-on-failure
````

## Build Options

You can customize the build by passing options to CMake during the configuration step.

```bash
# Example: Configure a minimal build as a shared library
cmake -DCMAKE_BUILD_TYPE=Release \
      -DBIGNUM_BUILD_SHARED=ON \
      -DBIGNUM_BUILD_TESTS=OFF \
      -DBIGNUM_BUILD_BENCHMARKS=OFF \
      ..
```

| Option                      | Description                            | Default |
| --------------------------- | -------------------------------------- | ------- |
| `CMAKE_BUILD_TYPE`          | `Release`, `Debug`, `RelWithDebInfo`   | `None`  |
| `BIGNUM_BUILD_SHARED`       | Build as a shared library (`.so`/`.dll`) | `OFF`   |
| `BIGNUM_BUILD_TESTS`        | Build the test suite executable        | `ON`    |
| `BIGNUM_BUILD_BENCHMARKS`   | Build the performance benchmark executable | `ON`    |
| `CMAKE_INSTALL_PREFIX`      | Path for installation                  | System-dependent |

## Installation

After building the library, you can install it on your system.

```bash
# From your build directory:

# 1. Build the project first
cmake --build .

# 2. Install the library and headers
# On Linux/macOS, this may require administrator privileges
sudo cmake --build . --target install
```

This will install the library headers and static/shared library files to the `CMAKE_INSTALL_PREFIX`.

## Using `bignum-cpp` in Your Project

There are three common ways to integrate `bignum-cpp` into your own CMake project.

### Method 1: `find_package` (Recommended after installation)

If you have installed `bignum-cpp`, other projects can find it with `find_package`.

```cmake
# In your CMakeLists.txt
find_package(BigNum REQUIRED)

add_executable(my_app main.cpp)
target_link_libraries(my_app PRIVATE BigNum::bignum)
```

### Method 2: `add_subdirectory`

If you have cloned `bignum-cpp` into your project's source tree, you can add it as a subdirectory.

```cmake
# In your CMakeLists.txt
add_subdirectory(path/to/bignum-cpp)

add_executable(my_app main.cpp)
target_link_libraries(my_app PRIVATE BigNum::bignum)
```

### Method 3: `FetchContent` (Recommended for modern CMake)

CMake can download and configure the library for you automatically. **This is the easiest method.**

```cmake
# In your CMakeLists.txt
include(FetchContent)

FetchContent_Declare(
    bignum_cpp
    GIT_REPOSITORY [https://github.com/Cozy03/bignum-cpp.git](https://github.com/Cozy03/bignum-cpp.git)
    GIT_TAG        main # Or a specific release tag/commit
)
FetchContent_MakeAvailable(bignum_cpp)

add_executable(my_app main.cpp)
target_link_libraries(my_app PRIVATE BigNum::bignum)
```

## Available CMake Targets

  - **`BigNum::bignum`**: The main library interface target. Always link against this.
  - **`test_bignum`**: The test suite executable (if `BIGNUM_BUILD_TESTS=ON`).
  - **`performance_benchmark`**: The benchmark executable (if `BIGNUM_BUILD_BENCHMARKS=ON`).

### Custom Convenience Targets

Your build environment provides several helpful commands:

```bash
# Format all C++ source files using clang-format
cmake --build . --target format

# Build and run the test suite
cmake --build . --target check

# Build the performance benchmarks
cmake --build . --target perf
```

## Troubleshooting

#### "128-bit integer support required"

The build system has detected that your compiler does not support the `__int128` type. Please use a compatible compiler like GCC 7+ or Clang 5+ on a 64-bit system.

#### Tests fail with sanitizer errors

This usually indicates a real bug. Re-run the build in `Debug` mode to get detailed error messages and stack traces.

```bash
# From an empty build directory
cmake -DCMAKE_BUILD_TYPE=Debug ..
cmake --build . && ctest --verbose
```

## Example Usage

Here is a simple example of how to use the library:

```cpp
#include "bignum.h"
#include <iostream>

int main() {
    // Create large numbers from hex strings
    BigNum a("0x1234567890ABCDEF1234567890ABCDEF");
    BigNum b("0xFEDCBA0987654321FEDCBA0987654321");

    // Perform arithmetic
    BigNum sum = a + b;
    BigNum product = a * b;

    std::cout << "a = " << a << std::endl;
    std::cout << "b = " << b << std::endl;
    std::cout << "Sum = " << sum.toHexString() << std::endl;
    std::cout << "Product = " << product.toHexString() << std::endl;

    // Cryptographic operations
    BigNum modulus = BigNum::randomPrime(256);
    BigNum result = a.modPow(b, modulus);

    std::cout << "ModPow Result: " << result.toHexString() << std::endl;

    return 0;
}
```
