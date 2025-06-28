# bignum-cpp: A High-Performance Arbitrary-Precision Integer Library

[![Build & Test CI](https://github.com/Cozy03/bignum-cpp/actions/workflows/main.yml/badge.svg)](https://github.com/Cozy03/bignum-cpp/actions)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
![Language](https://img.shields.io/github/languages/top/Cozy03/bignum-cpp)
![Repo Size](https://img.shields.io/github/repo-size/Cozy03/bignum-cpp)
![Latest Release](https://img.shields.io/github/v/release/Cozy03/bignum-cpp?display_name=tag)
![CMake Version](https://img.shields.io/badge/cmake-%3E%3D3.12-blue)

**`bignum-cpp`** is a modern, clean, and highly efficient C++17 library for handling integers of arbitrary size. It is designed with performance and ease of use in mind, making it an ideal choice for applications in cryptography, scientific computing, and number theory.

The library provides a simple `BigNum` class that overloads standard arithmetic, comparison, and bitwise operators, allowing it to be used as a natural replacement for built-in integer types.

## Key Features

- **High-Performance Algorithms**: Utilizes **Karatsuba** multiplication for large numbers and features optimized modular exponentiation with **Montgomery reduction**.
- **Modern C++17 Design**: Clean, header-and-source implementation with a focus on readability and modern practices.
- **Comprehensive Functionality**: Includes all standard arithmetic, bitwise operations, modular inverse, GCD, and Miller-Rabin primality testing.
- **Interactive CLI Tool**: Comes with a powerful command-line calculator for interactive number theory and cryptographic exploration, similar to tools like PARI/GP.
- **Robust Build System**: Powered by CMake, with built-in support for testing, benchmarking, and easy integration into other projects.
- **Extensively Tested**: Comes with a comprehensive test suite to ensure correctness and reliability.

## Quick Start

Get up and running in under a minute.

```bash
# 1. Clone the repository
git clone [https://github.com/Cozy03/bignum-cpp.git](https://github.com/Cozy03/bignum-cpp.git)
cd bignum-cpp

# 2. Configure and build the project (including the CLI tool)
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build .

# 3. Run the tests to verify the build
ctest --output-on-failure

# 4. Use the interactive calculator
./bignum-cli
````

For detailed build, installation, and usage instructions, please see [**BUILD.md**](BUILD.md).

## Usage Example (Library)

Using `bignum-cpp` in your own C++ code feels as natural as using standard integer types.

```cpp
#include "bignum.h"
#include <iostream>

int main() {
    // Create large numbers from hex strings
    BigNum a("0x1234567890ABCDEF1234567890ABCDEF");
    BigNum b("0xFEDCBA0987654321FEDCBA0987654321");

    // Perform arithmetic operations
    BigNum sum = a + b;
    BigNum product = a * b;

    std::cout << "a = " << a << std::endl;
    std::cout << "b = " << b << std::endl;
    std::cout << "Sum = " << sum.toHexString() << std::endl;
    std::cout << "Product = " << product.toHexString() << std::endl;

    // Perform a cryptographic operation: (a^b) mod modulus
    BigNum modulus = BigNum::randomPrime(256);
    BigNum result = a.modPow(b, modulus);

    std::cout << "ModPow Result: " << result.toHexString() << std::endl;

    return 0;
}
```

## Integrating into Your Project

The easiest way to use `bignum-cpp` is with CMake's `FetchContent` module. Add this to your `CMakeLists.txt`:

```cmake
include(FetchContent)

FetchContent_Declare(
    bignum_cpp
    GIT_REPOSITORY [https://github.com/Cozy03/bignum-cpp.git](https://github.com/Cozy03/bignum-cpp.git)
    GIT_TAG        main # Or a specific release tag
)
FetchContent_MakeAvailable(bignum_cpp)

# Link to your executable
add_executable(my_app main.cpp)
target_link_libraries(my_app PRIVATE BigNum::bignum)
```

For other integration methods (like `find_package` or `add_subdirectory`), see [**BUILD.md**](BUILD.md).

## Performance

This library is designed for speed. Here are some sample benchmark results on a modern CPU:

| Operation                      | Average Time | Throughput (ops/s) |
| ------------------------------ | ------------ | ------------------ |
| 512-bit Multiplication         | `0.60μs`     | \~1,560,000         |
| 256-bit Division               | `0.14μs`     | \~5,630,000         |
| 256-bit GCD                    | `63.28μs`    | \~15,700            |
| 256-bit Modular Inverse        | `~45μs`      | \~21,700            |
| 256-bit `modPow` (128-bit exp) | `270.04μs`   | \~3,700             |
| 96-bit Prime Generation        | `75.42ms`    | \~11                |

## License

This project is licensed under the **MIT License**.

## Contributing

Contributions are welcome\! If you have a suggestion or find a bug, please feel free to open an issue or submit a pull request.
