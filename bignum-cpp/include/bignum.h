#ifndef BIGNUM_H
#define BIGNUM_H

#include <vector>
#include <string>
#include <iostream>

// Check for 128-bit integer support
#if defined(__SIZEOF_INT128__) || (defined(_INTEGRAL_MAX_BITS) && _INTEGRAL_MAX_BITS >= 128)
    // 128-bit integers are supported
#else
    #error "This implementation requires 128-bit integer support for 64-bit radix"
#endif

// Forward declarations for optimization contexts
class MontgomeryContext;
class BarrettContext;

class BigNum {
private:
    std::vector<uint64_t> digits;  // Little-endian representation
    bool negative;
    static const uint32_t BASE_BITS = 64;
    
    // Algorithm thresholds
    static const size_t KARATSUBA_THRESHOLD = 8;    // Switch to Karatsuba above this digit count
    static const size_t MONTGOMERY_THRESHOLD = 4;   // Use Montgomery for modpow above this size
    static const size_t BARRETT_THRESHOLD = 8;      // Use Barrett reduction above this size
    
    // Helper functions
    void removeLeadingZeros();
    int compare(const BigNum& other) const;
    BigNum addUnsigned(const BigNum& other) const;
    BigNum subtractUnsigned(const BigNum& other) const;
    BigNum multiplyUnsigned(const BigNum& other) const;
    std::pair<BigNum, BigNum> divideUnsigned(const BigNum& divisor) const;
    
    // Optimized multiplication algorithms
    BigNum multiplyKaratsuba(const BigNum& other) const;
    BigNum multiplySchoolbook(const BigNum& other) const;
    
    // Optimized modular arithmetic
    BigNum modPowMontgomery(const BigNum& exponent, const BigNum& modulus) const;
    BigNum modPowBinary(const BigNum& exponent, const BigNum& modulus) const;
    
public:
    // Constructors
    BigNum();
    BigNum(int64_t value);
    BigNum(const std::string& hexStr);
    BigNum(const std::vector<uint64_t>& digits, bool neg = false);
    
    // Copy constructor and assignment
    BigNum(const BigNum& other);
    BigNum& operator=(const BigNum& other);
    
    // Move constructor and assignment
    BigNum(BigNum&& other) noexcept;
    BigNum& operator=(BigNum&& other) noexcept;
    
    // Destructor
    ~BigNum() = default;
    
    // Basic arithmetic operations
    BigNum operator+(const BigNum& other) const;
    BigNum operator-(const BigNum& other) const;
    BigNum operator*(const BigNum& other) const;
    BigNum operator/(const BigNum& other) const;
    BigNum operator%(const BigNum& other) const;
    
    // In-place operations
    BigNum& operator+=(const BigNum& other);
    BigNum& operator-=(const BigNum& other);
    BigNum& operator*=(const BigNum& other);
    BigNum& operator/=(const BigNum& other);
    BigNum& operator%=(const BigNum& other);
    
    // Comparison operations
    bool operator==(const BigNum& other) const;
    bool operator!=(const BigNum& other) const;
    bool operator<(const BigNum& other) const;
    bool operator<=(const BigNum& other) const;
    bool operator>(const BigNum& other) const;
    bool operator>=(const BigNum& other) const;
    
    // Unary operations
    BigNum operator-() const;
    BigNum operator+() const;
    
    // Bit operations
    BigNum operator<<(int shift) const;
    BigNum operator>>(int shift) const;
    BigNum& operator<<=(int shift);
    BigNum& operator>>=(int shift);
    BigNum operator&(const BigNum& other) const;
    BigNum operator|(const BigNum& other) const;
    BigNum operator^(const BigNum& other) const;
    BigNum& operator&=(const BigNum& other);
    BigNum& operator|=(const BigNum& other);
    BigNum& operator^=(const BigNum& other);
    
    // Cryptographic operations
    BigNum modPow(const BigNum& exponent, const BigNum& modulus) const;
    BigNum modInverse(const BigNum& modulus) const;
    BigNum gcd(const BigNum& other) const;
    std::pair<BigNum, std::pair<BigNum, BigNum>> extendedGcd(const BigNum& other) const;
    
    // Utility functions
    bool isZero() const;
    bool isOne() const;
    bool isNegative() const;
    bool isEven() const;
    bool isOdd() const;
    size_t bitLength() const;
    size_t byteLength() const;
    
    // Conversion functions
    std::string toHexString() const;
    std::string toDecimalString() const;
    int64_t toInt64() const;
    std::vector<uint8_t> toByteArray() const;
    
    // Random number generation
    static BigNum random(size_t bitLength);
    static BigNum randomPrime(size_t bitLength);
    
    // Prime testing
    bool isProbablePrime(int rounds = 20) const;
    
    // Factory methods
    static BigNum fromByteArray(const std::vector<uint8_t>& bytes);
    static BigNum fromHexString(const std::string& hexStr);
    static BigNum zero();
    static BigNum one();
    static BigNum two();
    
    // Stream operations
    friend std::ostream& operator<<(std::ostream& os, const BigNum& num);
    friend std::istream& operator>>(std::istream& is, BigNum& num);
    
    // Access to internal representation (for advanced users)
    const std::vector<uint64_t>& getDigits() const { return digits; }
    bool isNeg() const { return negative; }
};

// Montgomery arithmetic context for fast modular operations
class MontgomeryContext {
public:
    BigNum modulus;
    BigNum r;          // R = 2^(k*64) where k = modulus.digits.size()
    BigNum r_inv;      // R^(-1) mod modulus
    BigNum n_prime;    // -modulus^(-1) mod R
    size_t k;          // Number of digits
    
    MontgomeryContext(const BigNum& mod);
    BigNum reduce(const BigNum& a) const;
    BigNum multiply(const BigNum& a, const BigNum& b) const;
    BigNum toMontgomery(const BigNum& a) const;
    BigNum fromMontgomery(const BigNum& a) const;
};

// Barrett reduction context for fast modular reduction
class BarrettContext {
public:
    BigNum modulus;
    BigNum mu;         // floor(R^2 / modulus)
    size_t k;          // bit length of modulus
    
    BarrettContext(const BigNum& mod);
    BigNum reduce(const BigNum& a) const;
};

#endif // BIGNUM_H