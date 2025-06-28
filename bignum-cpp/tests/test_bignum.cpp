#include "bignum.h"
#include <iostream>
#include <vector>
#include <stdexcept>
#include <iomanip>
#include <chrono>
#include <map>
#include <string>
#include <cassert>

// ANSI color codes for better visual output
#define RESET   "\033[0m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define BLUE    "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN    "\033[36m"
#define WHITE   "\033[37m"
#define BOLD    "\033[1m"

class TestSuite {
private:
    struct TestCategory {
        std::string name;
        int passed = 0;
        int failed = 0;
        int total = 0;
        double total_time = 0.0;
        std::vector<std::string> failures;
    };
    
    std::map<std::string, TestCategory> categories;
    std::string current_category;
    std::chrono::high_resolution_clock::time_point test_start;
    
public:
    void start_category(const std::string& name) {
        current_category = name;
        if (categories.find(name) == categories.end()) {
            categories[name] = TestCategory{name, 0, 0, 0, 0.0, {}};
        }
        std::cout << BOLD CYAN "\n╔══════════════════════════════════════════════════════════════╗\n";
        std::cout << "║ " << std::setw(58) << std::left << ("Testing: " + name) << " ║\n";
        std::cout << "╚══════════════════════════════════════════════════════════════╝" RESET "\n";
    }
    
    void test(const std::string& test_name, std::function<void()> test_func) {
        categories[current_category].total++;
        test_start = std::chrono::high_resolution_clock::now();
        
        std::cout << "  " << YELLOW "►" RESET " " << std::setw(45) << std::left << test_name << " ";
        
        try {
            test_func();
            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration<double, std::milli>(end - test_start).count();
            categories[current_category].total_time += duration;
            categories[current_category].passed++;
            
            std::cout << GREEN "✓ PASSED" RESET;
            if (duration > 1.0) {
                std::cout << " " << CYAN "(" << std::fixed << std::setprecision(2) << duration << "ms)" RESET;
            }
            std::cout << "\n";
        } catch (const std::exception& e) {
            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration<double, std::milli>(end - test_start).count();
            categories[current_category].total_time += duration;
            categories[current_category].failed++;
            categories[current_category].failures.push_back(test_name + ": " + e.what());
            
            std::cout << RED "✗ FAILED" RESET " " << RED "(" << e.what() << ")" RESET "\n";
        }
    }
    
    void assert_equals(const std::string& expected, const std::string& actual, const std::string& context = "") {
        if (expected != actual) {
            std::string msg = "Expected '" + expected + "', got '" + actual + "'";
            if (!context.empty()) msg = context + ": " + msg;
            throw std::runtime_error(msg);
        }
    }
    
    void assert_true(bool condition, const std::string& message) {
        if (!condition) {
            throw std::runtime_error(message);
        }
    }
    
    void print_summary() {
        std::cout << BOLD WHITE "\n╔══════════════════════════════════════════════════════════════╗\n";
        std::cout << "║                        TEST SUMMARY                          ║\n";
        std::cout << "╠══════════════════════════════════════════════════════════════╣\n" RESET;
        
        int total_passed = 0, total_failed = 0, total_tests = 0;
        double total_time = 0.0;
        
        for (const auto& [name, cat] : categories) {
            total_passed += cat.passed;
            total_failed += cat.failed;
            total_tests += cat.total;
            total_time += cat.total_time;
            
            double pass_rate = cat.total > 0 ? (100.0 * cat.passed / cat.total) : 0.0;
            std::string status_color = (cat.failed == 0) ? GREEN : (pass_rate >= 80.0) ? YELLOW : RED;
            
            std::cout << "║ " << std::setw(25) << std::left << name 
                      << " " << status_color << std::setw(3) << cat.passed << "/" << std::setw(3) << cat.total 
                      << RESET " (" << status_color << std::fixed << std::setprecision(1) << pass_rate << "%" RESET ")"
                      << " " << CYAN << std::setw(8) << std::right << std::fixed << std::setprecision(1) << cat.total_time << "ms" RESET " ║\n";
        }
        
        std::cout << "╠══════════════════════════════════════════════════════════════╣\n";
        
        double overall_pass_rate = total_tests > 0 ? (100.0 * total_passed / total_tests) : 0.0;
        std::string overall_color = (total_failed == 0) ? GREEN : (overall_pass_rate >= 80.0) ? YELLOW : RED;
        
        std::cout << BOLD "║ " << std::setw(25) << std::left << "OVERALL RESULTS"
                  << " " << overall_color << std::setw(3) << total_passed << "/" << std::setw(3) << total_tests 
                  << RESET BOLD " (" << overall_color << std::fixed << std::setprecision(1) << overall_pass_rate << "%" RESET BOLD ")"
                  << " " << CYAN << std::setw(8) << std::right << std::fixed << std::setprecision(1) << total_time << "ms" RESET BOLD " ║" RESET "\n";
        
        std::cout << "╚══════════════════════════════════════════════════════════════╝\n";
        
        // Print failures if any
        if (total_failed > 0) {
            std::cout << RED BOLD "\n╔══════════════════════════════════════════════════════════════╗\n";
            std::cout << "║                          FAILURES                           ║\n";
            std::cout << "╚══════════════════════════════════════════════════════════════╝" RESET "\n";
            
            for (const auto& [name, cat] : categories) {
                if (!cat.failures.empty()) {
                    std::cout << RED BOLD "\n" << name << ":" RESET "\n";
                    for (const auto& failure : cat.failures) {
                        std::cout << RED "  • " << failure << RESET "\n";
                    }
                }
            }
        }
        
        if (total_failed > 0) {
            std::cout << RED BOLD "\nSome tests failed!" RESET "\n";
            exit(EXIT_FAILURE);
        } else {
            std::cout << GREEN BOLD "\nAll tests passed! 🎉" RESET "\n";
        }
    }
};

TestSuite test_suite;

void test_basic_constructors() {
    test_suite.start_category("Basic Constructors & Properties");
    
    test_suite.test("Default constructor creates zero", []() {
        BigNum z;
        test_suite.assert_true(z.isZero(), "Default constructor should create zero");
        test_suite.assert_equals("0", z.toHexString(), "Default hex string");
        test_suite.assert_true(!z.isNegative(), "Zero should not be negative");
        test_suite.assert_true(z.isEven(), "Zero should be even");
    });
    
    test_suite.test("Positive integer constructor", []() {
        BigNum a(42);
        test_suite.assert_equals("2a", a.toHexString(), "42 in hex");
        test_suite.assert_true(!a.isNegative(), "42 should be positive");
        test_suite.assert_true(a.isEven(), "42 should be even");
    });
    
    test_suite.test("Negative integer constructor", []() {
        BigNum b(-17);
        test_suite.assert_equals("-11", b.toHexString(), "-17 in hex");
        test_suite.assert_true(b.isNegative(), "-17 should be negative");
        test_suite.assert_true(b.isOdd(), "-17 should be odd");
    });
    
    test_suite.test("Large positive integer", []() {
        BigNum large(0x123456789ABCDEFULL);
        test_suite.assert_equals("123456789abcdef", large.toHexString(), "Large positive hex");
    });
    
    test_suite.test("Vector constructor", []() {
        std::vector<uint64_t> vec{0x1111222233334444ULL, 0x00000000deadbeefULL};
        BigNum v(vec, false);
        test_suite.assert_true(v.getDigits() == vec, "Vector digits should match");
        test_suite.assert_true(!v.isNegative(), "Vector-constructed number should be positive");
    });
    
    test_suite.test("Vector constructor with negative", []() {
        std::vector<uint64_t> vec{0x123ULL};
        BigNum v(vec, true);
        test_suite.assert_true(v.isNegative(), "Should be negative");
        test_suite.assert_equals("-123", v.toHexString(), "Negative vector hex");
    });
    
    test_suite.test("Copy constructor", []() {
        BigNum original(12345);
        BigNum copy(original);
        test_suite.assert_equals(original.toHexString(), copy.toHexString(), "Copy should equal original");
    });
}

void test_hex_conversions() {
    test_suite.start_category("Hex String Conversions");
    
    test_suite.test("Simple hex parsing", []() {
        BigNum h = BigNum::fromHexString("1abcdef");
        test_suite.assert_equals("1abcdef", h.toHexString(), "Simple hex roundtrip");
    });
    
    test_suite.test("Hex with 0x prefix", []() {
        BigNum h = BigNum::fromHexString("0x1abcdef");
        test_suite.assert_equals("1abcdef", h.toHexString(), "0x prefix handling");
    });
    
    test_suite.test("Negative hex", []() {
        BigNum h = BigNum::fromHexString("-ff");
        test_suite.assert_equals("-ff", h.toHexString(), "Negative hex");
        test_suite.assert_true(h.isNegative(), "Should be negative");
    });
    
    test_suite.test("Large hex number", []() {
        std::string large_hex = "123456789abcdef0123456789abcdef0123456789abcdef";
        BigNum h = BigNum::fromHexString(large_hex);
        test_suite.assert_equals(large_hex, h.toHexString(), "Large hex roundtrip");
    });
    
    test_suite.test("Mixed case hex", []() {
        BigNum h = BigNum::fromHexString("0xAbCdEf");
        test_suite.assert_equals("abcdef", h.toHexString(), "Mixed case handling");
    });
    
    test_suite.test("Single digit hex", []() {
        BigNum h = BigNum::fromHexString("a");
        test_suite.assert_equals("a", h.toHexString(), "Single digit");
    });
}

void test_basic_arithmetic() {
    test_suite.start_category("Basic Arithmetic Operations");
    
    test_suite.test("Simple addition", []() {
        BigNum a(12345), b(67890);
        BigNum c = a + b;
        // 12345 + 67890 = 80235 = 0x1396b
        test_suite.assert_equals("1396b", c.toHexString(), "12345 + 67890");
    });
    
    test_suite.test("Addition with carry", []() {
        BigNum a = BigNum::fromHexString("ffffffffffffffff");
        BigNum b(1);
        BigNum c = a + b;
        test_suite.assert_equals("10000000000000000", c.toHexString(), "Max uint64 + 1");
    });
    
    test_suite.test("Negative addition", []() {
        BigNum a(-100), b(-50);
        BigNum c = a + b;
        test_suite.assert_equals("-96", c.toHexString(), "-100 + (-50)");
    });
    
    test_suite.test("Mixed sign addition", []() {
        BigNum a(100), b(-30);
        BigNum c = a + b;
        test_suite.assert_equals("46", c.toHexString(), "100 + (-30)");
    });
    
    test_suite.test("Simple subtraction", []() {
        BigNum a(100), b(40);
        BigNum c = a - b;
        // 100 - 40 = 60 = 0x3c
        test_suite.assert_equals("3c", c.toHexString(), "100 - 40");
    });
    
    test_suite.test("Subtraction resulting in negative", []() {
        BigNum a(50), b(100);
        BigNum c = a - b;
        test_suite.assert_equals("-32", c.toHexString(), "50 - 100");
    });
    
    test_suite.test("Large number subtraction", []() {
        BigNum a = BigNum::fromHexString("10000000000000000");
        BigNum b(1);
        BigNum c = a - b;
        test_suite.assert_equals("ffffffffffffffff", c.toHexString(), "2^64 - 1");
    });
    
    test_suite.test("Simple multiplication", []() {
        BigNum a(123), b(456);
        BigNum c = a * b;
        // 123 * 456 = 56088 = 0xdb18
        test_suite.assert_equals("db18", c.toHexString(), "123 * 456");
    });
    
    test_suite.test("Multiplication by zero", []() {
        BigNum a(12345), b(0);
        BigNum c = a * b;
        test_suite.assert_equals("0", c.toHexString(), "12345 * 0");
        test_suite.assert_true(c.isZero(), "Should be zero");
    });
    
    test_suite.test("Negative multiplication", []() {
        BigNum a(-12), b(5);
        BigNum c = a * b;
        test_suite.assert_equals("-3c", c.toHexString(), "-12 * 5");
    });
    
    test_suite.test("Large multiplication", []() {
        BigNum a = BigNum::fromHexString("123456789");
        BigNum b = BigNum::fromHexString("abcdef");
        BigNum c = a * b;
        test_suite.assert_equals("c379aaaa375de7", c.toHexString(), "Large multiplication");
    });
}

void test_division_modulo() {
    test_suite.start_category("Division & Modulo Operations");
    
    test_suite.test("Simple division", []() {
        BigNum a(100), b(30);
        BigNum q = a / b;
        test_suite.assert_equals("3", q.toHexString(), "100 / 30");
    });
    
    test_suite.test("Simple modulo", []() {
        BigNum a(100), b(30);
        BigNum r = a % b;
        // 100 % 30 = 10 = 0xa
        test_suite.assert_equals("a", r.toHexString(), "100 % 30");
    });
    
    test_suite.test("Division by same number", []() {
        BigNum a(42), b(42);
        BigNum q = a / b;
        BigNum r = a % b;
        test_suite.assert_equals("1", q.toHexString(), "42 / 42");
        test_suite.assert_equals("0", r.toHexString(), "42 % 42");
    });
    
    test_suite.test("Division by larger number", []() {
        BigNum a(50), b(100);
        BigNum q = a / b;
        BigNum r = a % b;
        test_suite.assert_equals("0", q.toHexString(), "50 / 100");
        test_suite.assert_equals("32", r.toHexString(), "50 % 100");
    });
    
    test_suite.test("Large number division", []() {
        BigNum a = BigNum::fromHexString("123456789abcdef");
        BigNum b = BigNum::fromHexString("123456");
        BigNum q = a / b;
        BigNum r = a % b;
        // Verify: a = q * b + r
        BigNum check = q * b + r;
        test_suite.assert_equals(a.toHexString(), check.toHexString(), "Division check: a = q*b + r");
    });
    
    test_suite.test("Power of 2 division", []() {
        BigNum a = BigNum::fromHexString("12345678");
        BigNum b = BigNum::fromHexString("100"); // 256
        BigNum q = a / b;
        test_suite.assert_equals("123456", q.toHexString(), "Division by 256");
    });
}

void test_bit_operations() {
    test_suite.start_category("Bit Operations");
    
    test_suite.test("Left shift small", []() {
        BigNum a(5); // 0b101
        BigNum shifted = a << 2;
        test_suite.assert_equals("14", shifted.toHexString(), "5 << 2"); // 0b10100 = 20 = 0x14
    });
    
    test_suite.test("Right shift small", []() {
        BigNum a(20); // 0b10100
        BigNum shifted = a >> 2;
        test_suite.assert_equals("5", shifted.toHexString(), "20 >> 2"); // 0b101 = 5
    });
    
    test_suite.test("Left shift by 64", []() {
        BigNum a(1);
        BigNum shifted = a << 64;
        test_suite.assert_equals("10000000000000000", shifted.toHexString(), "1 << 64");
    });
    
    test_suite.test("Bitwise AND", []() {
        BigNum a = BigNum::fromHexString("ff00");
        BigNum b = BigNum::fromHexString("0ff0");
        BigNum result = a & b;
        test_suite.assert_equals("f00", result.toHexString(), "0xff00 & 0x0ff0");
    });
    
    test_suite.test("Bitwise OR", []() {
        BigNum a = BigNum::fromHexString("f0f0");
        BigNum b = BigNum::fromHexString("0f0f");
        BigNum result = a | b;
        test_suite.assert_equals("ffff", result.toHexString(), "0xf0f0 | 0x0f0f");
    });
    
    test_suite.test("Bitwise XOR", []() {
        BigNum a = BigNum::fromHexString("ff00");
        BigNum b = BigNum::fromHexString("00ff");
        BigNum result = a ^ b;
        test_suite.assert_equals("ffff", result.toHexString(), "0xff00 ^ 0x00ff");
    });
    
    test_suite.test("Bit length calculation", []() {
        BigNum a = BigNum::fromHexString("ff");
        test_suite.assert_true(a.bitLength() == 8, "0xff should have 8 bits");
        
        BigNum b = BigNum::fromHexString("100");
        test_suite.assert_true(b.bitLength() == 9, "0x100 should have 9 bits");
        
        BigNum c(0);
        test_suite.assert_true(c.bitLength() == 0, "0 should have 0 bits");
    });
}

void test_cryptographic_operations() {
    test_suite.start_category("Cryptographic Operations");
    
    test_suite.test("GCD small numbers", []() {
        BigNum g = BigNum(48).gcd(BigNum(18));
        test_suite.assert_equals("6", g.toHexString(), "gcd(48, 18)");
    });
    
    test_suite.test("GCD coprime", []() {
        BigNum g = BigNum(17).gcd(BigNum(13));
        test_suite.assert_equals("1", g.toHexString(), "gcd(17, 13)");
    });
    
    test_suite.test("GCD with zero", []() {
        BigNum g = BigNum(42).gcd(BigNum(0));
        test_suite.assert_equals("2a", g.toHexString(), "gcd(42, 0)");
    });
    
    test_suite.test("Modular inverse small", []() {
        BigNum inv = BigNum(3).modInverse(BigNum(11));
        test_suite.assert_equals("4", inv.toHexString(), "3^(-1) mod 11");
        // Verify: (3 * 4) % 11 = 1
        BigNum check = (BigNum(3) * inv) % BigNum(11);
        test_suite.assert_equals("1", check.toHexString(), "Inverse verification");
    });
    
    test_suite.test("Modular inverse larger", []() {
        BigNum inv = BigNum(7).modInverse(BigNum(26));
        test_suite.assert_equals("f", inv.toHexString(), "7^(-1) mod 26"); // 15
        // Verify: (7 * 15) % 26 = 1
        BigNum check = (BigNum(7) * inv) % BigNum(26);
        test_suite.assert_equals("1", check.toHexString(), "Larger inverse verification");
    });
    
    test_suite.test("ModPow small", []() {
        BigNum result = BigNum(3).modPow(BigNum(4), BigNum(5));
        // 3^4 = 81, 81 % 5 = 1
        test_suite.assert_equals("1", result.toHexString(), "3^4 mod 5");
    });
    
    test_suite.test("ModPow medium", []() {
        BigNum result = BigNum(2).modPow(BigNum(10), BigNum(1000));
        // 2^10 = 1024, 1024 % 1000 = 24
        test_suite.assert_equals("18", result.toHexString(), "2^10 mod 1000"); // 24 = 0x18
    });
    
    test_suite.test("ModPow large exponent", []() {
        BigNum base = BigNum::fromHexString("123");
        BigNum exp = BigNum::fromHexString("456");
        BigNum mod = BigNum::fromHexString("789");
        BigNum result = base.modPow(exp, mod);
        // Just verify it completes without error and result < mod
        test_suite.assert_true(result < mod, "ModPow result should be < modulus");
    });
    
    test_suite.test("ModPow with Montgomery context", []() {
        // Test with a larger modulus that would trigger Montgomery
        BigNum base = BigNum::fromHexString("123456789");
        BigNum exp = BigNum::fromHexString("abc");
        BigNum mod = BigNum::fromHexString("123456789abcdef123456789abcdef123456789abcdef"); // Large odd modulus
        BigNum result = base.modPow(exp, mod);
        test_suite.assert_true(result < mod, "Montgomery ModPow result should be < modulus");
    });
}

void test_edge_cases() {
    test_suite.start_category("Edge Cases & Error Handling");
    
    test_suite.test("Division by zero throws", []() {
        BigNum a(100), b(0);
        bool threw = false;
        try {
            BigNum c = a / b;
        } catch (const std::invalid_argument&) {
            threw = true;
        }
        test_suite.assert_true(threw, "Division by zero should throw");
    });
    
    test_suite.test("Modulo by zero throws", []() {
        BigNum a(100), b(0);
        bool threw = false;
        try {
            BigNum c = a % b;
        } catch (const std::invalid_argument&) {
            threw = true;
        }
        test_suite.assert_true(threw, "Modulo by zero should throw");
    });
    
    test_suite.test("Mod inverse non-invertible throws", []() {
        bool threw = false;
        try {
            BigNum inv = BigNum(6).modInverse(BigNum(9)); // gcd(6,9) = 3 ≠ 1
        } catch (const std::invalid_argument&) {
            threw = true;
        }
        test_suite.assert_true(threw, "Non-invertible modInverse should throw");
    });
    
    test_suite.test("Zero operations", []() {
        BigNum zero(0);
        BigNum a(42);
        
        test_suite.assert_equals("0", (zero + zero).toHexString(), "0 + 0");
        test_suite.assert_equals("2a", (a + zero).toHexString(), "42 + 0");
        test_suite.assert_equals("0", (zero * a).toHexString(), "0 * 42");
        test_suite.assert_equals("0", (zero / a).toHexString(), "0 / 42");
        test_suite.assert_equals("0", (zero % a).toHexString(), "0 % 42");
    });
    
    test_suite.test("Self operations", []() {
        BigNum a(42);
        test_suite.assert_equals("54", (a + a).toHexString(), "42 + 42"); // 84 = 0x54
        test_suite.assert_equals("0", (a - a).toHexString(), "42 - 42");
        test_suite.assert_equals("6e4", (a * a).toHexString(), "42 * 42"); // 1764 = 0x6e4
        test_suite.assert_equals("1", (a / a).toHexString(), "42 / 42");
        test_suite.assert_equals("0", (a % a).toHexString(), "42 % 42");
    });
    
    test_suite.test("Sign preservation", []() {
        BigNum pos(42);
        BigNum neg(-42);
        
        test_suite.assert_true(!pos.isNegative(), "Positive should not be negative");
        test_suite.assert_true(neg.isNegative(), "Negative should be negative");
        test_suite.assert_equals("2a", pos.toHexString(), "Positive hex");
        test_suite.assert_equals("-2a", neg.toHexString(), "Negative hex");
        
        BigNum double_neg = -neg;
        test_suite.assert_true(!double_neg.isNegative(), "Double negative should be positive");
        test_suite.assert_equals("2a", double_neg.toHexString(), "Double negative hex");
    });
}

void test_random_and_primes() {
    test_suite.start_category("Random Numbers & Primality");
    
    test_suite.test("Random number generation", []() {
        BigNum r1 = BigNum::random(64);
        BigNum r2 = BigNum::random(64);
        test_suite.assert_true(r1 != r2, "Random numbers should be different");
        test_suite.assert_true(r1.bitLength() <= 64, "Random number bit length check");
        test_suite.assert_true(r1.bitLength() == 64, "Random number should use exact bit length");
    });
    
    test_suite.test("Small prime generation", []() {
        BigNum p = BigNum::randomPrime(8);
        test_suite.assert_true(p.isProbablePrime(5), "Generated number should be prime");
        test_suite.assert_true(p.bitLength() == 8, "Prime should have exact bit length");
        test_suite.assert_true(p.isOdd(), "Prime should be odd");
    });
    
    test_suite.test("Known prime tests", []() {
        test_suite.assert_true(BigNum(2).isProbablePrime(5), "2 should be prime");
        test_suite.assert_true(BigNum(3).isProbablePrime(5), "3 should be prime");
        test_suite.assert_true(BigNum(17).isProbablePrime(5), "17 should be prime");
        test_suite.assert_true(BigNum(97).isProbablePrime(5), "97 should be prime");
        
        test_suite.assert_true(!BigNum(4).isProbablePrime(5), "4 should not be prime");
        test_suite.assert_true(!BigNum(15).isProbablePrime(5), "15 should not be prime");
        test_suite.assert_true(!BigNum(100).isProbablePrime(5), "100 should not be prime");
    });
    
    test_suite.test("Medium prime test", []() {
        // Test a smaller known prime: 2^31 - 1 (Mersenne prime)
        BigNum medium_prime = (BigNum(1) << 31) - BigNum(1);
        test_suite.assert_true(medium_prime.isProbablePrime(5), "2^31 - 1 should be prime");
    });
}

void test_byte_arrays() {
    test_suite.start_category("Byte Array Conversions");
    
    test_suite.test("Simple byte array conversion", []() {
        std::vector<uint8_t> bytes = {0x01, 0x23, 0x45, 0x67};
        BigNum num = BigNum::fromByteArray(bytes);
        test_suite.assert_equals("1234567", num.toHexString(), "Byte array to BigNum");
        
        std::vector<uint8_t> result = num.toByteArray();
        test_suite.assert_true(result == bytes, "Round-trip byte array conversion");
    });
    
    test_suite.test("Single byte", []() {
        std::vector<uint8_t> bytes = {0xff};
        BigNum num = BigNum::fromByteArray(bytes);
        test_suite.assert_equals("ff", num.toHexString(), "Single byte conversion");
    });
    
    test_suite.test("Empty byte array", []() {
        std::vector<uint8_t> empty;
        BigNum num = BigNum::fromByteArray(empty);
        test_suite.assert_true(num.isZero(), "Empty byte array should be zero");
    });
    
    test_suite.test("Large byte array", []() {
        std::vector<uint8_t> bytes(32, 0xff); // 32 bytes of 0xff
        BigNum num = BigNum::fromByteArray(bytes);
        std::string expected(64, 'f'); // 64 'f' characters
        test_suite.assert_equals(expected, num.toHexString(), "Large byte array");
    });
}

int main() {
    std::cout << BOLD CYAN "╔══════════════════════════════════════════════════════════════╗\n";
    std::cout << "║                    BIGNUM TEST SUITE                        ║\n";
    std::cout << "║                   Comprehensive Testing                     ║\n";
    std::cout << "╚══════════════════════════════════════════════════════════════╝" RESET "\n";
    
    test_basic_constructors();
    test_hex_conversions();
    test_basic_arithmetic();
    test_division_modulo();
    test_bit_operations();
    test_cryptographic_operations();
    test_edge_cases();
    test_random_and_primes();
    test_byte_arrays();
    
    test_suite.print_summary();
    return 0;
}