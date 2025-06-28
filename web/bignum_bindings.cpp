#include "../bignum-cpp/include/bignum.h"
#include <emscripten/bind.h>
#include <emscripten/val.h>
#include <string>
#include <vector>

using namespace emscripten;

// Wrapper class for easier JavaScript integration
class BigNumJS {
private:
    BigNum num;

public:
    BigNumJS() : num(0LL) {}
    BigNumJS(const std::string& hexStr) : num(BigNum::fromHexString(hexStr)) {}
    BigNumJS(const BigNum& bn) : num(bn) {}

    // Basic arithmetic operations
    BigNumJS add(const BigNumJS& other) const {
        return BigNumJS(num + other.num);
    }

    BigNumJS subtract(const BigNumJS& other) const {
        return BigNumJS(num - other.num);
    }

    BigNumJS multiply(const BigNumJS& other) const {
        return BigNumJS(num * other.num);
    }

    BigNumJS divide(const BigNumJS& other) const {
        return BigNumJS(num / other.num);
    }

    BigNumJS modulo(const BigNumJS& other) const {
        return BigNumJS(num % other.num);
    }

    // Cryptographic operations
    BigNumJS modPow(const BigNumJS& exponent, const BigNumJS& modulus) const {
        return BigNumJS(num.modPow(exponent.num, modulus.num));
    }

    BigNumJS gcd(const BigNumJS& other) const {
        return BigNumJS(num.gcd(other.num));
    }

    BigNumJS modInverse(const BigNumJS& modulus) const {
        return BigNumJS(num.modInverse(modulus.num));
    }

    // Extended GCD - returns object with gcd, s, and t
    val extendedGcd(const BigNumJS& other) const {
        auto result = num.extendedGcd(other.num);
        
        val obj = val::object();
        obj.set("gcd", BigNumJS(result.first).toHexString());
        obj.set("s", BigNumJS(result.second.first).toHexString());
        obj.set("t", BigNumJS(result.second.second).toHexString());
        
        return obj;
    }

    // Bit operations
    BigNumJS shiftLeft(int bits) const {
        return BigNumJS(num << bits);
    }

    BigNumJS shiftRight(int bits) const {
        return BigNumJS(num >> bits);
    }

    BigNumJS bitwiseAnd(const BigNumJS& other) const {
        return BigNumJS(num & other.num);
    }

    BigNumJS bitwiseOr(const BigNumJS& other) const {
        return BigNumJS(num | other.num);
    }

    BigNumJS bitwiseXor(const BigNumJS& other) const {
        return BigNumJS(num ^ other.num);
    }

    // Comparison operations
    bool equals(const BigNumJS& other) const {
        return num == other.num;
    }

    bool lessThan(const BigNumJS& other) const {
        return num < other.num;
    }

    bool greaterThan(const BigNumJS& other) const {
        return num > other.num;
    }

    // Properties and utilities
    std::string toHexString() const {
        return num.toHexString();
    }

    bool isZero() const {
        return num.isZero();
    }

    bool isOne() const {
        return num.isOne();
    }

    bool isNegative() const {
        return num.isNegative();
    }

    bool isEven() const {
        return num.isEven();
    }

    bool isOdd() const {
        return num.isOdd();
    }

    size_t bitLength() const {
        return num.bitLength();
    }

    size_t byteLength() const {
        return num.byteLength();
    }

    bool isProbablePrime(int rounds = 20) const {
        return num.isProbablePrime(rounds);
    }

    // Static methods
    static BigNumJS random(size_t bitLength) {
        return BigNumJS(BigNum::random(bitLength));
    }

    static BigNumJS randomPrime(size_t bitLength) {
        return BigNumJS(BigNum::randomPrime(bitLength));
    }

    static BigNumJS fromHexString(const std::string& hexStr) {
        return BigNumJS(BigNum::fromHexString(hexStr));
    }

    static BigNumJS zero() {
        return BigNumJS(BigNum::zero());
    }

    static BigNumJS one() {
        return BigNumJS(BigNum::one());
    }

    static BigNumJS two() {
        return BigNumJS(BigNum::two());
    }
};

// JavaScript bindings
EMSCRIPTEN_BINDINGS(bignum) {
    class_<BigNumJS>("BigNum")
        .constructor<>()
        .constructor<std::string>()
        
        // Basic arithmetic
        .function("add", &BigNumJS::add)
        .function("subtract", &BigNumJS::subtract)
        .function("multiply", &BigNumJS::multiply)
        .function("divide", &BigNumJS::divide)
        .function("modulo", &BigNumJS::modulo)
        
        // Cryptographic operations
        .function("modPow", &BigNumJS::modPow)
        .function("gcd", &BigNumJS::gcd)
        .function("modInverse", &BigNumJS::modInverse)
        .function("extendedGcd", &BigNumJS::extendedGcd)
        
        // Bit operations
        .function("shiftLeft", &BigNumJS::shiftLeft)
        .function("shiftRight", &BigNumJS::shiftRight)
        .function("bitwiseAnd", &BigNumJS::bitwiseAnd)
        .function("bitwiseOr", &BigNumJS::bitwiseOr)
        .function("bitwiseXor", &BigNumJS::bitwiseXor)
        
        // Comparison operations
        .function("equals", &BigNumJS::equals)
        .function("lessThan", &BigNumJS::lessThan)
        .function("greaterThan", &BigNumJS::greaterThan)
        
        // Properties and utilities
        .function("toHexString", &BigNumJS::toHexString)
        .function("isZero", &BigNumJS::isZero)
        .function("isOne", &BigNumJS::isOne)
        .function("isNegative", &BigNumJS::isNegative)
        .function("isEven", &BigNumJS::isEven)
        .function("isOdd", &BigNumJS::isOdd)
        .function("bitLength", &BigNumJS::bitLength)
        .function("byteLength", &BigNumJS::byteLength)
        .function("isProbablePrime", &BigNumJS::isProbablePrime)
        
        // Static methods
        .class_function("random", &BigNumJS::random)
        .class_function("randomPrime", &BigNumJS::randomPrime)
        .class_function("fromHexString", &BigNumJS::fromHexString)
        .class_function("zero", &BigNumJS::zero)
        .class_function("one", &BigNumJS::one)
        .class_function("two", &BigNumJS::two);
}
