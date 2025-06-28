cmake_minimum_required(VERSION 3.12)

project(BigNum
    VERSION 1.0.0
    DESCRIPTION "High-performance arbitrary precision arithmetic library for cryptographic applications"
    LANGUAGES CXX
)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Include CMake modules
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

# Find required packages
find_package(Threads REQUIRED)

# Options
option(BIGNUM_BUILD_TESTS "Build tests" ON)
option(BIGNUM_BUILD_BENCHMARKS "Build benchmarks" ON)
option(BIGNUM_BUILD_CLI "Build the interactive CLI tool" ON)
option(BIGNUM_BUILD_SHARED "Build shared library" OFF)

# Compiler-specific options
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set(BIGNUM_COMPILE_OPTIONS
        -Wall
        -Wextra
        -Wpedantic
        -Wno-unknown-pragmas
    )
    
    # Release-specific optimizations
    set(BIGNUM_COMPILE_OPTIONS_RELEASE
        -O3
        -march=native
        -DNDEBUG
    )
    
    # Debug-specific options
    set(BIGNUM_COMPILE_OPTIONS_DEBUG
        -g
        -O0
        -DDEBUG
        -fsanitize=address
        -fsanitize=undefined
    )
elseif(MSVC)
    set(BIGNUM_COMPILE_OPTIONS
        /W4
        /permissive-
    )
    
    set(BIGNUM_COMPILE_OPTIONS_RELEASE
        /O2
        /DNDEBUG
    )
    
    set(BIGNUM_COMPILE_OPTIONS_DEBUG
        /Od
        /DDEBUG
    )
endif()

# Check for 128-bit integer support
include(CheckCXXSourceCompiles)
check_cxx_source_compiles(
    "
    int main() {
        __uint128_t x = 1;
        __int128_t y = 1;
        return 0;
    }
    "
    HAS_INT128_SUPPORT
)

if(NOT HAS_INT128_SUPPORT)
    message(FATAL_ERROR "This BigNum implementation requires 128-bit integer support (__int128). "
                        "Please use a compiler and platform that supports 128-bit integers.")
endif()

# Check for threading support
check_cxx_source_compiles(
    "
    #include <thread>
    #include <atomic>
    #include <future>
    int main() {
        std::thread t([](){});
        t.join();
        std::atomic<int> x{0};
        auto f = std::async(std::launch::async, [](){});
        return 0;
    }
    "
    HAS_THREADING_SUPPORT
)

if(NOT HAS_THREADING_SUPPORT)
    message(WARNING "Threading support not detected. Multithreaded benchmarks may not work properly.")
endif()

# Create the main library
set(BIGNUM_SOURCES
    src/bignum.cpp
)

set(BIGNUM_HEADERS
    include/bignum.h
)

# Choose library type
if(BIGNUM_BUILD_SHARED)
    add_library(bignum SHARED ${BIGNUM_SOURCES})
    target_compile_definitions(bignum PRIVATE BIGNUM_BUILDING_DLL)
    target_compile_definitions(bignum INTERFACE BIGNUM_USING_DLL)
else()
    add_library(bignum STATIC ${BIGNUM_SOURCES})
endif()

# Add alias for consistent usage
add_library(BigNum::bignum ALIAS bignum)

# Set target properties
target_include_directories(bignum
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_compile_features(bignum PUBLIC cxx_std_17)

target_compile_options(bignum
    PRIVATE
        ${BIGNUM_COMPILE_OPTIONS}
        $<$<CONFIG:Release>:${BIGNUM_COMPILE_OPTIONS_RELEASE}>
        $<$<CONFIG:Debug>:${BIGNUM_COMPILE_OPTIONS_DEBUG}>
)

# Link threading library to the main library
target_link_libraries(bignum PUBLIC Threads::Threads)

# Link libraries (if any additional libraries are needed)
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_link_options(bignum
        PRIVATE
            $<$<CONFIG:Debug>:-fsanitize=address>
            $<$<CONFIG:Debug>:-fsanitize=undefined>
    )
endif()

# Set library properties
set_target_properties(bignum PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    OUTPUT_NAME "bignum"
    EXPORT_NAME "bignum"
)

# Tests
if(BIGNUM_BUILD_TESTS)
    enable_testing()
    
    add_executable(test_bignum tests/test_bignum.cpp)
    target_link_libraries(test_bignum PRIVATE BigNum::bignum)
    
    target_compile_options(test_bignum
        PRIVATE
            ${BIGNUM_COMPILE_OPTIONS}
            $<$<CONFIG:Release>:${BIGNUM_COMPILE_OPTIONS_RELEASE}>
            $<$<CONFIG:Debug>:${BIGNUM_COMPILE_OPTIONS_DEBUG}>
    )
    
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_link_options(test_bignum
            PRIVATE
                $<$<CONFIG:Debug>:-fsanitize=address>
                $<$<CONFIG:Debug>:-fsanitize=undefined>
        )
    endif()
    
    # Register test with CTest
    add_test(NAME BigNumTests COMMAND test_bignum)
    
    # Set test properties
    set_tests_properties(BigNumTests PROPERTIES
        TIMEOUT 300
        LABELS "unit"
    )
endif()

# Benchmarks
if(BIGNUM_BUILD_BENCHMARKS)
    # Check if benchmark directory has any source files
    file(GLOB BENCHMARK_SOURCES "benchmark/*.cpp")
    
    if(BENCHMARK_SOURCES)
        foreach(benchmark_file ${BENCHMARK_SOURCES})
            get_filename_component(benchmark_name ${benchmark_file} NAME_WE)
            add_executable(${benchmark_name} ${benchmark_file})
            target_link_libraries(${benchmark_name} PRIVATE BigNum::bignum)
            
            target_compile_options(${benchmark_name}
                PRIVATE
                    ${BIGNUM_COMPILE_OPTIONS}
                    ${BIGNUM_COMPILE_OPTIONS_RELEASE}  # Always optimize benchmarks
            )
            
            # Ensure benchmarks have threading support
            if(HAS_THREADING_SUPPORT)
                target_compile_definitions(${benchmark_name} PRIVATE BIGNUM_HAS_THREADING)
            endif()
            
            # Add to a custom target for running all benchmarks
            if(NOT TARGET benchmarks)
                add_custom_target(benchmarks)
            endif()
            add_dependencies(benchmarks ${benchmark_name})
        endforeach()
        
        # Add convenience target to run all benchmarks
        add_custom_target(run_benchmarks
            COMMENT "Running all benchmarks"
            VERBATIM
        )
        
        foreach(benchmark_file ${BENCHMARK_SOURCES})
            get_filename_component(benchmark_name ${benchmark_file} NAME_WE)
            add_custom_command(TARGET run_benchmarks POST_BUILD
                COMMAND ${CMAKE_CURRENT_BINARY_DIR}/${benchmark_name}
                COMMENT "Running ${benchmark_name}"
                VERBATIM
            )
        endforeach()
        
    else()
        message(STATUS "No benchmark source files found in benchmark/ directory")
    endif()
endif()

# ===================================================================
# CLI Tool
# ===================================================================
if(BIGNUM_BUILD_CLI)
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/cli-tool/cli.cpp")
        add_executable(bignum-cli cli-tool/cli.cpp)
        target_link_libraries(bignum-cli PRIVATE BigNum::bignum)
        message(STATUS "Configured target: bignum-cli")
    else()
        message(STATUS "CLI source file not found at cli-tool/cli.cpp")
    endif()
endif()

# Installation
install(TARGETS bignum
    EXPORT BiNumTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(FILES ${BIGNUM_HEADERS}
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# Export targets for use by other projects
install(EXPORT BiNumTargets
    FILE BigNumTargets.cmake
    NAMESPACE BigNum::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/BigNum
)

# Create package config files
configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/BigNumConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/BigNumConfig.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/BigNum
)

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/BigNumConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/BigNumConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/BigNumConfigVersion.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/BigNum
)

# Export for build tree
export(EXPORT BiNumTargets
    FILE "${CMAKE_CURRENT_BINARY_DIR}/BigNumTargets.cmake"
    NAMESPACE BigNum::
)

# Package configuration
set(CMAKE_EXPORT_PACKAGE_REGISTRY ON)
export(PACKAGE BigNum)

# Print configuration summary
message(STATUS "")
message(STATUS "BigNum Configuration Summary:")
message(STATUS "  Version:              ${PROJECT_VERSION}")
message(STATUS "  Build type:           ${CMAKE_BUILD_TYPE}")
message(STATUS "  C++ standard:         ${CMAKE_CXX_STANDARD}")
message(STATUS "  Compiler:             ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "  Build shared library: ${BIGNUM_BUILD_SHARED}")
message(STATUS "  Build tests:          ${BIGNUM_BUILD_TESTS}")
message(STATUS "  Build benchmarks:     ${BIGNUM_BUILD_BENCHMARKS}")
message(STATUS "  Build CLI tool:       ${BIGNUM_BUILD_CLI}")
message(STATUS "  128-bit support:      ${HAS_INT128_SUPPORT}")
message(STATUS "  Threading support:    ${HAS_THREADING_SUPPORT}")
message(STATUS "  Install prefix:       ${CMAKE_INSTALL_PREFIX}")
if(Threads_FOUND)
    message(STATUS "  Threading library:    ${CMAKE_THREAD_LIBS_INIT}")
endif()
message(STATUS "")

# Add custom targets for convenience
add_custom_target(format
    COMMAND find ${CMAKE_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/tests ${CMAKE_SOURCE_DIR}/cli-tool -name "*.cpp" -o -name "*.h" | xargs clang-format -i
    COMMENT "Formatting source code"
    VERBATIM
)

add_custom_target(check
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
    DEPENDS test_bignum
    COMMENT "Running tests"
    VERBATIM
)

# Performance testing target
if(BIGNUM_BUILD_BENCHMARKS)
    add_custom_target(perf
        DEPENDS benchmarks
        COMMENT "Building performance benchmarks"
        VERBATIM
    )
endif()
