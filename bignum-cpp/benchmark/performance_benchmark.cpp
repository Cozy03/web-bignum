#include "bignum.h"
#include <iostream>
#include <vector>
#include <chrono>
#include <iomanip>
#include <string>
#include <functional>
#include <algorithm>
#include <numeric>

// ANSI color codes for beautiful output
#define RESET   "\033[0m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define BLUE    "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN    "\033[36m"
#define WHITE   "\033[37m"
#define BOLD    "\033[1m"

struct BenchmarkResult {
    std::string name;
    double avg_time_us;
    double median_time_us;
    double min_time_us;
    double max_time_us;
    double ops_per_sec;
    int total_iterations;
    double benchmark_duration_ms;
};

class CleanBenchmarkSuite {
private:
    std::vector<BenchmarkResult> results;
    
    double calculate_std_dev(const std::vector<double>& times, double mean) {
        double sum_sq_diff = 0.0;
        for (double time : times) {
            double diff = time - mean;
            sum_sq_diff += diff * diff;
        }
        return std::sqrt(sum_sq_diff / times.size());
    }
    
public:
    void benchmark(const std::string& name, std::function<void()> func, double duration_seconds = 2.0) {
        std::vector<double> times;
        auto start_benchmark = std::chrono::high_resolution_clock::now();
        auto end_time = start_benchmark + std::chrono::duration<double>(duration_seconds);
        
        // Warmup runs
        for (int i = 0; i < 10; ++i) {
            func();
        }
        
        // Actual benchmark runs
        int iterations = 0;
        while (std::chrono::high_resolution_clock::now() < end_time) {
            auto start = std::chrono::high_resolution_clock::now();
            func();
            auto end = std::chrono::high_resolution_clock::now();
            
            double time_us = std::chrono::duration<double, std::micro>(end - start).count();
            times.push_back(time_us);
            iterations++;
        }
        
        auto actual_duration = std::chrono::high_resolution_clock::now() - start_benchmark;
        double duration_ms = std::chrono::duration<double, std::milli>(actual_duration).count();
        
        // Calculate statistics
        std::sort(times.begin(), times.end());
        double avg = std::accumulate(times.begin(), times.end(), 0.0) / times.size();
        double median = times[times.size() / 2];
        double min_t = times.front();
        double max_t = times.back();
        double std_dev = calculate_std_dev(times, avg);
        double ops_per_sec = iterations / (duration_ms / 1000.0);
        
        BenchmarkResult result = {name, avg, median, min_t, max_t, ops_per_sec, iterations, duration_ms};
        results.push_back(result);
        
        // Print result immediately
        print_result(result, std_dev);
    }
    
    void print_result(const BenchmarkResult& result, double std_dev) {
        std::string time_unit = "μs";
        double display_time = result.avg_time_us;
        double display_std_dev = std_dev;
        
        if (display_time >= 1000.0) {
            display_time /= 1000.0;
            display_std_dev /= 1000.0;
            time_unit = "ms";
        }
        
        std::string perf_color = GREEN;
        if (display_time > 100 && time_unit == "ms") perf_color = RED;
        else if (display_time > 10 && time_unit == "ms") perf_color = YELLOW;
        else if (display_time > 100 && time_unit == "μs") perf_color = CYAN;
        
        std::cout << "  " << CYAN "►" RESET " " << std::setw(40) << std::left << result.name
                  << " " << perf_color << std::fixed << std::setprecision(2) << display_time << time_unit << RESET
                  << " " << YELLOW "±" << std::fixed << std::setprecision(2) << display_std_dev << time_unit << RESET
                  << " " << WHITE "(" << std::setw(10) << std::right << std::fixed << std::setprecision(0) 
                  << result.ops_per_sec << " ops/s)" << RESET "\n";
    }
    
    void print_header(const std::string& category) {
        std::cout << BOLD CYAN "\n╔══════════════════════════════════════════════════════════════╗\n";
        std::cout << "║ " << std::setw(58) << std::left << ("Benchmarking: " + category) << " ║\n";
        std::cout << "╚══════════════════════════════════════════════════════════════╝" RESET "\n";
    }
    
    void print_summary() {
        std::cout << BOLD WHITE "\n╔══════════════════════════════════════════════════════════════╗\n";
        std::cout << "║                      PERFORMANCE SUMMARY                    ║\n";
        std::cout << "╠══════════════════════════════════════════════════════════════╣\n" RESET;
        
        for (const auto& result : results) {
            std::string time_unit = "μs";
            double display_time = result.avg_time_us;
            if (display_time >= 1000.0) {
                display_time /= 1000.0;
                time_unit = "ms";
            }
            
            std::string perf_color = GREEN;
            if (display_time > 100 && time_unit == "ms") perf_color = RED;
            else if (display_time > 10 && time_unit == "ms") perf_color = YELLOW;
            else if (display_time > 100 && time_unit == "μs") perf_color = CYAN;
            
            std::cout << "║ " << std::setw(35) << std::left << result.name
                      << " " << perf_color << std::setw(8) << std::right 
                      << std::fixed << std::setprecision(2) << display_time << time_unit << RESET
                      << " " << std::setw(12) << std::right << std::fixed << std::setprecision(0) 
                      << result.ops_per_sec << " ops/s ║\n";
        }
        
        std::cout << "╚══════════════════════════════════════════════════════════════╝\n";
        
        // Print top performers
        print_top_performers();
    }
    
    void print_top_performers() {
        if (results.empty()) return;
        
        std::cout << BOLD CYAN "\n╔══════════════════════════════════════════════════════════════╗\n";
        std::cout << "║                      TOP PERFORMERS                         ║\n";
        std::cout << "╚══════════════════════════════════════════════════════════════╝" RESET "\n";
        
        // Sort by operations per second
        auto sorted_results = results;
        std::sort(sorted_results.begin(), sorted_results.end(),
                 [](const auto& a, const auto& b) { return a.ops_per_sec > b.ops_per_sec; });
        
        std::cout << "Fastest operations (by throughput):\n";
        for (int i = 0; i < std::min(5, (int)sorted_results.size()); ++i) {
            const auto& result = sorted_results[i];
            std::cout << "  " << (i + 1) << ". " << BOLD << result.name << RESET 
                      << " - " << GREEN << std::fixed << std::setprecision(0) 
                      << result.ops_per_sec << " ops/s" << RESET "\n";
        }
        
        std::cout << "\nLowest latency operations (by time):\n";
        std::sort(sorted_results.begin(), sorted_results.end(),
                 [](const auto& a, const auto& b) { return a.avg_time_us < b.avg_time_us; });
        
        for (int i = 0; i < std::min(5, (int)sorted_results.size()); ++i) {
            const auto& result = sorted_results[i];
            std::string time_unit = "μs";
            double display_time = result.avg_time_us;
            if (display_time >= 1000.0) {
                display_time /= 1000.0;
                time_unit = "ms";
            }
            
            std::cout << "  " << (i + 1) << ". " << BOLD << result.name << RESET 
                      << " - " << GREEN << std::fixed << std::setprecision(2) 
                      << display_time << time_unit << RESET "\n";
        }
    }
};

// Generate test data for benchmarks
std::vector<BigNum> generate_test_numbers(size_t bit_size, int count) {
    std::vector<BigNum> numbers;
    numbers.reserve(count);
    
    for (int i = 0; i < count; ++i) {
        numbers.push_back(BigNum::random(bit_size));
    }
    
    return numbers;
}

void benchmark_basic_arithmetic(CleanBenchmarkSuite& suite) {
    suite.print_header("Basic Arithmetic Operations");
    
    // Generate test data for different sizes
    auto nums_64 = generate_test_numbers(64, 20);
    auto nums_128 = generate_test_numbers(128, 20);
    auto nums_256 = generate_test_numbers(256, 20);
    auto nums_512 = generate_test_numbers(512, 20);
    auto nums_1024 = generate_test_numbers(1024, 20);
    
    // Addition benchmarks
    suite.benchmark("64-bit Addition", [&nums_64]() {
        static int idx = 0;
        BigNum result = nums_64[idx % 20] + nums_64[(idx + 1) % 20];
        idx++;
    }, 2.0);
    
    suite.benchmark("128-bit Addition", [&nums_128]() {
        static int idx = 0;
        BigNum result = nums_128[idx % 20] + nums_128[(idx + 1) % 20];
        idx++;
    }, 2.0);
    
    suite.benchmark("256-bit Addition", [&nums_256]() {
        static int idx = 0;
        BigNum result = nums_256[idx % 20] + nums_256[(idx + 1) % 20];
        idx++;
    }, 2.0);
    
    suite.benchmark("512-bit Addition", [&nums_512]() {
        static int idx = 0;
        BigNum result = nums_512[idx % 20] + nums_512[(idx + 1) % 20];
        idx++;
    }, 2.0);
    
    // Multiplication benchmarks
    suite.benchmark("64-bit Multiplication", [&nums_64]() {
        static int idx = 0;
        BigNum result = nums_64[idx % 20] * nums_64[(idx + 1) % 20];
        idx++;
    }, 2.0);
    
    suite.benchmark("128-bit Multiplication", [&nums_128]() {
        static int idx = 0;
        BigNum result = nums_128[idx % 20] * nums_128[(idx + 1) % 20];
        idx++;
    }, 2.0);
    
    suite.benchmark("256-bit Multiplication", [&nums_256]() {
        static int idx = 0;
        BigNum result = nums_256[idx % 20] * nums_256[(idx + 1) % 20];
        idx++;
    }, 2.0);
    
    suite.benchmark("512-bit Multiplication", [&nums_512]() {
        static int idx = 0;
        BigNum result = nums_512[idx % 20] * nums_512[(idx + 1) % 20];
        idx++;
    }, 2.0);
    
    // Division benchmarks (smaller sizes for speed)
    suite.benchmark("128-bit Division", [&nums_128]() {
        static int idx = 0;
        BigNum result = nums_128[idx % 20] / nums_128[(idx + 1) % 20];
        idx++;
    }, 2.0);
    
    suite.benchmark("256-bit Division", [&nums_256]() {
        static int idx = 0;
        BigNum result = nums_256[idx % 20] / nums_256[(idx + 1) % 20];
        idx++;
    }, 2.0);
}

void benchmark_cryptographic_operations(CleanBenchmarkSuite& suite) {
    suite.print_header("Cryptographic Operations");
    
    // Generate test data
    auto bases_128 = generate_test_numbers(128, 10);
    auto bases_256 = generate_test_numbers(256, 10);
    auto moduli_128 = generate_test_numbers(128, 5);
    auto moduli_256 = generate_test_numbers(256, 5);
    auto exponents_64 = generate_test_numbers(64, 5);
    auto exponents_128 = generate_test_numbers(128, 5);
    
    // Make moduli odd for Montgomery arithmetic
    for (auto& mod : moduli_128) {
        if (mod.isEven()) mod += BigNum(1LL);
    }
    for (auto& mod : moduli_256) {
        if (mod.isEven()) mod += BigNum(1LL);
    }
    
    // GCD benchmarks
    suite.benchmark("128-bit GCD", [&bases_128]() {
        static int idx = 0;
        BigNum result = bases_128[idx % 10].gcd(bases_128[(idx + 1) % 10]);
        idx++;
    }, 2.0);
    
    suite.benchmark("256-bit GCD", [&bases_256]() {
        static int idx = 0;
        BigNum result = bases_256[idx % 10].gcd(bases_256[(idx + 1) % 10]);
        idx++;
    }, 2.0);
    
    // Modular exponentiation benchmarks
    suite.benchmark("128-bit ModPow (64-bit exp)", [&bases_128, &exponents_64, &moduli_128]() {
        static int idx = 0;
        BigNum result = bases_128[idx % 10].modPow(exponents_64[idx % 5], moduli_128[idx % 5]);
        idx++;
    }, 3.0);
    
    suite.benchmark("256-bit ModPow (64-bit exp)", [&bases_256, &exponents_64, &moduli_256]() {
        static int idx = 0;
        BigNum result = bases_256[idx % 10].modPow(exponents_64[idx % 5], moduli_256[idx % 5]);
        idx++;
    }, 3.0);
    
    suite.benchmark("256-bit ModPow (128-bit exp)", [&bases_256, &exponents_128, &moduli_256]() {
        static int idx = 0;
        BigNum result = bases_256[idx % 10].modPow(exponents_128[idx % 5], moduli_256[idx % 5]);
        idx++;
    }, 3.0);
    
    // Modular inverse (smaller numbers only)
    suite.benchmark("128-bit Modular Inverse", [&bases_128, &moduli_128]() {
        static int idx = 0;
        try {
            BigNum result = bases_128[idx % 10].modInverse(moduli_128[idx % 5]);
        } catch (...) {
            // Skip if not invertible
        }
        idx++;
    }, 2.0);
}

void benchmark_bit_operations(CleanBenchmarkSuite& suite) {
    suite.print_header("Bit Operations");
    
    auto nums_256 = generate_test_numbers(256, 20);
    auto nums_512 = generate_test_numbers(512, 20);
    auto nums_1024 = generate_test_numbers(1024, 20);
    
    // Bit shift operations
    suite.benchmark("256-bit Left Shift", [&nums_256]() {
        static int idx = 0;
        BigNum result = nums_256[idx % 20] << 32;
        idx++;
    }, 2.0);
    
    suite.benchmark("512-bit Left Shift", [&nums_512]() {
        static int idx = 0;
        BigNum result = nums_512[idx % 20] << 64;
        idx++;
    }, 2.0);
    
    suite.benchmark("1024-bit Right Shift", [&nums_1024]() {
        static int idx = 0;
        BigNum result = nums_1024[idx % 20] >> 64;
        idx++;
    }, 2.0);
    
    // Bitwise operations
    suite.benchmark("512-bit Bitwise AND", [&nums_512]() {
        static int idx = 0;
        BigNum result = nums_512[idx % 20] & nums_512[(idx + 1) % 20];
        idx++;
    }, 2.0);
    
    suite.benchmark("512-bit Bitwise OR", [&nums_512]() {
        static int idx = 0;
        BigNum result = nums_512[idx % 20] | nums_512[(idx + 1) % 20];
        idx++;
    }, 2.0);
    
    suite.benchmark("512-bit Bitwise XOR", [&nums_512]() {
        static int idx = 0;
        BigNum result = nums_512[idx % 20] ^ nums_512[(idx + 1) % 20];
        idx++;
    }, 2.0);
}

void benchmark_conversion_operations(CleanBenchmarkSuite& suite) {
    suite.print_header("Conversion Operations");
    
    auto nums_128 = generate_test_numbers(128, 20);
    auto nums_256 = generate_test_numbers(256, 20);
    auto nums_512 = generate_test_numbers(512, 20);
    
    // String conversion benchmarks
    suite.benchmark("128-bit To Hex String", [&nums_128]() {
        static int idx = 0;
        std::string hex = nums_128[idx % 20].toHexString();
        idx++;
    }, 2.0);
    
    suite.benchmark("256-bit To Hex String", [&nums_256]() {
        static int idx = 0;
        std::string hex = nums_256[idx % 20].toHexString();
        idx++;
    }, 2.0);
    
    suite.benchmark("512-bit To Hex String", [&nums_512]() {
        static int idx = 0;
        std::string hex = nums_512[idx % 20].toHexString();
        idx++;
    }, 2.0);
    
    // From hex string conversion
    std::vector<std::string> hex_strings_128, hex_strings_256;
    for (const auto& num : nums_128) {
        hex_strings_128.push_back(num.toHexString());
    }
    for (const auto& num : nums_256) {
        hex_strings_256.push_back(num.toHexString());
    }
    
    suite.benchmark("128-bit From Hex String", [&hex_strings_128]() {
        static int idx = 0;
        BigNum num = BigNum::fromHexString(hex_strings_128[idx % 20]);
        idx++;
    }, 2.0);
    
    suite.benchmark("256-bit From Hex String", [&hex_strings_256]() {
        static int idx = 0;
        BigNum num = BigNum::fromHexString(hex_strings_256[idx % 20]);
        idx++;
    }, 2.0);
    
    // Byte array conversion
    suite.benchmark("256-bit To Byte Array", [&nums_256]() {
        static int idx = 0;
        auto bytes = nums_256[idx % 20].toByteArray();
        idx++;
    }, 2.0);
    
    suite.benchmark("512-bit To Byte Array", [&nums_512]() {
        static int idx = 0;
        auto bytes = nums_512[idx % 20].toByteArray();
        idx++;
    }, 2.0);
}

void benchmark_prime_operations(CleanBenchmarkSuite& suite) {
    suite.print_header("Prime Operations");
    
    auto candidates_64 = generate_test_numbers(64, 20);
    auto candidates_128 = generate_test_numbers(128, 20);
    
    // Primality testing
    suite.benchmark("64-bit Primality Test", [&candidates_64]() {
        static int idx = 0;
        candidates_64[idx % 20].isProbablePrime(5);
        idx++;
    }, 3.0);
    
    suite.benchmark("128-bit Primality Test", [&candidates_128]() {
        static int idx = 0;
        candidates_128[idx % 20].isProbablePrime(5);
        idx++;
    }, 3.0);
    
    // Prime generation (smaller sizes for reasonable time)
    suite.benchmark("32-bit Prime Generation", []() {
        BigNum prime = BigNum::randomPrime(32);
    }, 3.0);
    
    suite.benchmark("64-bit Prime Generation", []() {
        BigNum prime = BigNum::randomPrime(64);
    }, 5.0);
    
    suite.benchmark("96-bit Prime Generation", []() {
        BigNum prime = BigNum::randomPrime(96);
    }, 5.0);
}

int main() {
    CleanBenchmarkSuite suite;
    
    std::cout << BOLD CYAN "╔══════════════════════════════════════════════════════════════╗\n";
    std::cout << "║                    BIGNUM PERFORMANCE BENCHMARK             ║\n";
    std::cout << "║                    Clean & Comprehensive                    ║\n";
    std::cout << "╚══════════════════════════════════════════════════════════════╝" RESET "\n";
    
    std::cout << YELLOW "\nTime-based benchmarking - each test runs for fixed duration" RESET "\n";
    std::cout << YELLOW "Results show average time ± standard deviation and throughput" RESET "\n\n";
    
    try {
        // Run all benchmark categories
        benchmark_basic_arithmetic(suite);
        benchmark_cryptographic_operations(suite);
        benchmark_bit_operations(suite);
        benchmark_conversion_operations(suite);
        benchmark_prime_operations(suite);
        
        // Print comprehensive summary
        suite.print_summary();
        
        std::cout << GREEN BOLD "\nBenchmark completed successfully! 🚀" RESET "\n";
        std::cout << CYAN "All operations measured with statistical analysis." RESET "\n";
        std::cout << CYAN "Higher ops/sec indicates better performance." RESET "\n";
        
    } catch (const std::exception& e) {
        std::cout << RED BOLD "\nBenchmark failed with error: " << e.what() << RESET "\n";
        return 1;
    } catch (...) {
        std::cout << RED BOLD "\nBenchmark failed with unknown error" RESET "\n";
        return 1;
    }
    
    return 0;
}