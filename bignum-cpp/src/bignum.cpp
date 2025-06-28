/**
 * @file bignum.cpp
 * @brief Implements the BigNum class for arbitrary-precision integer arithmetic.
 * @author Debrup Chatterjee
 * @date June 27, 2025
 *
 * This file contains the implementation of the BigNum class, which supports
 * integers of any size, along with helper classes for optimized modular arithmetic.
 */

#include "bignum.h"
#include <algorithm>
#include <random>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <utility>

// Constructors
BigNum::BigNum() : negative(false) {
    digits.push_back(0);
}

BigNum::BigNum(int64_t value) : negative(value < 0) {
    uint64_t absValue = negative ? -static_cast<uint64_t>(value) : static_cast<uint64_t>(value);
    
    if (absValue == 0) {
        digits.push_back(0);
        negative = false;
    } else {
        digits.push_back(absValue);
    }
}

BigNum::BigNum(const std::string& hexStr) : negative(false) {
    *this = fromHexString(hexStr);
}

BigNum::BigNum(const std::vector<uint64_t>& digits, bool neg) 
    : digits(digits), negative(neg) {
    removeLeadingZeros();
}

BigNum::BigNum(const BigNum& other) : digits(other.digits), negative(other.negative) {}

BigNum& BigNum::operator=(const BigNum& other) {
    if (this != &other) {
        digits = other.digits;
        negative = other.negative;
    }
    return *this;
}

BigNum::BigNum(BigNum&& other) noexcept 
    : digits(std::move(other.digits)), negative(other.negative) {}

BigNum& BigNum::operator=(BigNum&& other) noexcept {
    if (this != &other) {
        digits = std::move(other.digits);
        negative = other.negative;
    }
    return *this;
}

// Helper functions
void BigNum::removeLeadingZeros() {
    while (digits.size() > 1 && digits.back() == 0) {
        digits.pop_back();
    }
    if (digits.size() == 1 && digits[0] == 0) {
        negative = false;
    }
}

int BigNum::compare(const BigNum& other) const {
    if (negative != other.negative) {
        return negative ? -1 : 1;
    }
    
    int sign = negative ? -1 : 1;
    
    if (digits.size() != other.digits.size()) {
        return sign * (digits.size() < other.digits.size() ? -1 : 1);
    }
    
    for (int i = digits.size() - 1; i >= 0; --i) {
        if (digits[i] != other.digits[i]) {
            return sign * (digits[i] < other.digits[i] ? -1 : 1);
        }
    }
    
    return 0;
}

BigNum BigNum::addUnsigned(const BigNum& other) const {
    const std::vector<uint64_t>& a = digits;
    const std::vector<uint64_t>& b = other.digits;
    
    size_t maxSize = std::max(a.size(), b.size());
    std::vector<uint64_t> result;
    result.reserve(maxSize + 1);
    
    __uint128_t carry = 0;
    for (size_t i = 0; i < maxSize || carry; ++i) {
        __uint128_t sum = carry;
        if (i < a.size()) sum += a[i];
        if (i < b.size()) sum += b[i];
        
        result.push_back(static_cast<uint64_t>(sum & 0xFFFFFFFFFFFFFFFFULL));
        carry = sum >> 64;
    }
    
    return BigNum(result, false);
}

BigNum BigNum::subtractUnsigned(const BigNum& other) const {
    // This function assumes |this| >= |other|
    const std::vector<uint64_t>& a = digits;
    const std::vector<uint64_t>& b = other.digits;
    
    std::vector<uint64_t> result;
    result.reserve(a.size());
    
    uint64_t borrow = 0;
    for (size_t i = 0; i < a.size(); ++i) {
        uint64_t ai = a[i];
        uint64_t bi = (i < b.size()) ? b[i] : 0;
        
        if (ai >= bi + borrow) {
            result.push_back(ai - bi - borrow);
            borrow = 0;
        } else {
            // Need to borrow
            result.push_back(ai + (UINT64_MAX + 1) - bi - borrow);
            borrow = 1;
        }
    }
    
    return BigNum(result, false);
}

BigNum BigNum::multiplyUnsigned(const BigNum& other) const {
    // Choose algorithm based on input size
    size_t maxSize = std::max(digits.size(), other.digits.size());
    
    if (maxSize >= KARATSUBA_THRESHOLD) {
        return multiplyKaratsuba(other);
    } else {
        return multiplySchoolbook(other);
    }
}

BigNum BigNum::multiplySchoolbook(const BigNum& other) const {
    const std::vector<uint64_t>& a = digits;
    const std::vector<uint64_t>& b = other.digits;
    
    std::vector<uint64_t> result(a.size() + b.size(), 0);
    
    for (size_t i = 0; i < a.size(); ++i) {
        __uint128_t carry = 0;
        for (size_t j = 0; j < b.size() || carry; ++j) {
            __uint128_t prod = static_cast<__uint128_t>(result[i + j]) + carry;
            if (j < b.size()) {
                prod += static_cast<__uint128_t>(a[i]) * b[j];
            }
            
            result[i + j] = static_cast<uint64_t>(prod & 0xFFFFFFFFFFFFFFFFULL);
            carry = prod >> 64;
        }
    }
    
    return BigNum(result, false);
}

BigNum BigNum::multiplyKaratsuba(const BigNum& other) const {
    // Base case: use schoolbook for small numbers
    size_t maxSize = std::max(digits.size(), other.digits.size());
    if (maxSize < KARATSUBA_THRESHOLD) {
        return multiplySchoolbook(other);
    }
    
    // Pad to equal sizes
    size_t n = maxSize;
    if (n % 2 != 0) n++; // Make even for easier splitting
    
    BigNum a = *this;
    BigNum b = other;
    
    // Pad with zeros if necessary
    while (a.digits.size() < n) a.digits.push_back(0);
    while (b.digits.size() < n) b.digits.push_back(0);
    
    // Split in half
    size_t half = n / 2;
    
    // a = a1 * R^half + a0
    BigNum a0(std::vector<uint64_t>(a.digits.begin(), a.digits.begin() + half), false);
    BigNum a1(std::vector<uint64_t>(a.digits.begin() + half, a.digits.end()), false);
    
    // b = b1 * R^half + b0  
    BigNum b0(std::vector<uint64_t>(b.digits.begin(), b.digits.begin() + half), false);
    BigNum b1(std::vector<uint64_t>(b.digits.begin() + half, b.digits.end()), false);
    
    // Recursive calls
    BigNum z0 = a0.multiplyUnsigned(b0);  // Low part
    BigNum z2 = a1.multiplyUnsigned(b1);  // High part
    BigNum z1 = (a1 + a0).multiplyUnsigned(b1 + b0) - z2 - z0;  // Middle part
    
    // Combine: result = z2 * R^n + z1 * R^half + z0
    BigNum result = z0;
    result += z1 << (half * 64);
    result += z2 << (n * 64);

    return result;
}

std::pair<BigNum, BigNum> BigNum::divideUnsigned(const BigNum& divisor) const {
    if (divisor.isZero()) {
        throw std::invalid_argument("Division by zero");
    }
    
    if (compare(divisor) < 0) {
        return {BigNum(0LL), *this};
    }
    
    BigNum remainder = *this;
    BigNum quotient(0LL);
    BigNum temp = divisor;
    
    // Find the highest bit position
    int shift = 0;
    while (temp <= remainder) {
        temp <<= 1;
        shift++;
    }
    
    temp >>= 1;
    shift--;
    
    for (int i = shift; i >= 0; --i) {
        if (remainder >= temp) {
            remainder -= temp;
            quotient += (BigNum(1LL) << i);
        }
        temp >>= 1;
    }
    
    return {quotient, remainder};
}

// Arithmetic operations
BigNum BigNum::operator+(const BigNum& other) const {
    if (negative == other.negative) {
        BigNum result = addUnsigned(other);
        result.negative = negative;
        return result;
    } else {
        // Different signs: this is actually subtraction
        // Compare absolute values
        int abs_compare = 0;
        if (digits.size() != other.digits.size()) {
            abs_compare = digits.size() < other.digits.size() ? -1 : 1;
        } else {
            for (int i = digits.size() - 1; i >= 0; --i) {
                if (digits[i] != other.digits[i]) {
                    abs_compare = digits[i] < other.digits[i] ? -1 : 1;
                    break;
                }
            }
        }
        
        if (abs_compare >= 0) {
            // |this| >= |other|
            BigNum result = subtractUnsigned(other);
            result.negative = negative;
            return result;
        } else {
            // |this| < |other|
            BigNum result = other.subtractUnsigned(*this);
            result.negative = other.negative;
            return result;
        }
    }
}

BigNum BigNum::operator-(const BigNum& other) const {
    return *this + (-other);
}

BigNum BigNum::operator*(const BigNum& other) const {
    BigNum result = multiplyUnsigned(other);
    result.negative = negative ^ other.negative;
    result.removeLeadingZeros();
    return result;
}

BigNum BigNum::operator/(const BigNum& other) const {
    auto result = divideUnsigned(other);
    result.first.negative = negative ^ other.negative;
    result.first.removeLeadingZeros();
    return result.first;
}

BigNum BigNum::operator%(const BigNum& other) const {
    auto result = divideUnsigned(other);
    result.second.negative = negative;
    result.second.removeLeadingZeros();
    return result.second;
}

// In-place operations
BigNum& BigNum::operator+=(const BigNum& other) {
    *this = *this + other;
    return *this;
}

BigNum& BigNum::operator-=(const BigNum& other) {
    *this = *this - other;
    return *this;
}

BigNum& BigNum::operator*=(const BigNum& other) {
    *this = *this * other;
    return *this;
}

BigNum& BigNum::operator/=(const BigNum& other) {
    *this = *this / other;
    return *this;
}

BigNum& BigNum::operator%=(const BigNum& other) {
    *this = *this % other;
    return *this;
}

// Comparison operations
bool BigNum::operator==(const BigNum& other) const {
    return compare(other) == 0;
}

bool BigNum::operator!=(const BigNum& other) const {
    return compare(other) != 0;
}

bool BigNum::operator<(const BigNum& other) const {
    return compare(other) < 0;
}

bool BigNum::operator<=(const BigNum& other) const {
    return compare(other) <= 0;
}

bool BigNum::operator>(const BigNum& other) const {
    return compare(other) > 0;
}

bool BigNum::operator>=(const BigNum& other) const {
    return compare(other) >= 0;
}

// Unary operations
BigNum BigNum::operator-() const {
    BigNum result = *this;
    if (!isZero()) {
        result.negative = !negative;
    }
    return result;
}

BigNum BigNum::operator+() const {
    return *this;
}

// Bit operations
BigNum BigNum::operator<<(int shift) const {
    if (shift <= 0) return *this;
    
    int wordShift = shift / 64;
    int bitShift = shift % 64;
    
    std::vector<uint64_t> result(digits.size() + wordShift + 1, 0);
    
    if (bitShift == 0) {
        std::copy(digits.begin(), digits.end(), result.begin() + wordShift);
    } else {
        uint64_t carry = 0;
        for (size_t i = 0; i < digits.size(); ++i) {
            __uint128_t temp = (static_cast<__uint128_t>(digits[i]) << bitShift) | carry;
            result[i + wordShift] = static_cast<uint64_t>(temp & 0xFFFFFFFFFFFFFFFFULL);
            carry = static_cast<uint64_t>(temp >> 64);
        }
        if (carry) {
            result[digits.size() + wordShift] = carry;
        }
    }
    
    return BigNum(result, negative);
}

BigNum BigNum::operator>>(int shift) const {
    if (shift <= 0) return *this;
    
    int wordShift = shift / 64;
    int bitShift = shift % 64;
    
    if (wordShift >= static_cast<int>(digits.size())) {
        return BigNum::zero();
    }
    
    std::vector<uint64_t> result(digits.size() - wordShift);
    
    if (bitShift == 0) {
        std::copy(digits.begin() + wordShift, digits.end(), result.begin());
    } else {
        uint64_t carry = 0;
        for (int i = digits.size() - 1; i >= wordShift; --i) {
            __uint128_t temp = (static_cast<__uint128_t>(carry) << 64) | digits[i];
            result[i - wordShift] = static_cast<uint64_t>(temp >> bitShift);
            carry = static_cast<uint64_t>(temp & ((1ULL << bitShift) - 1));
        }
    }
    
    return BigNum(result, negative);
}

BigNum& BigNum::operator<<=(int shift) {
    *this = *this << shift;
    return *this;
}

BigNum& BigNum::operator>>=(int shift) {
    *this = *this >> shift;
    return *this;
}

// Bitwise operations
BigNum BigNum::operator&(const BigNum& other) const {
    size_t maxSize = std::max(digits.size(), other.digits.size());
    std::vector<uint64_t> result(maxSize, 0);
    
    for (size_t i = 0; i < maxSize; ++i) {
        uint64_t a_digit = (i < digits.size()) ? digits[i] : 0;
        uint64_t b_digit = (i < other.digits.size()) ? other.digits[i] : 0;
        result[i] = a_digit & b_digit;
    }
    
    return BigNum(result, false);
}

BigNum BigNum::operator|(const BigNum& other) const {
    size_t maxSize = std::max(digits.size(), other.digits.size());
    std::vector<uint64_t> result(maxSize, 0);
    
    for (size_t i = 0; i < maxSize; ++i) {
        uint64_t a_digit = (i < digits.size()) ? digits[i] : 0;
        uint64_t b_digit = (i < other.digits.size()) ? other.digits[i] : 0;
        result[i] = a_digit | b_digit;
    }
    
    return BigNum(result, false);
}

BigNum BigNum::operator^(const BigNum& other) const {
    size_t maxSize = std::max(digits.size(), other.digits.size());
    std::vector<uint64_t> result(maxSize, 0);
    
    for (size_t i = 0; i < maxSize; ++i) {
        uint64_t a_digit = (i < digits.size()) ? digits[i] : 0;
        uint64_t b_digit = (i < other.digits.size()) ? other.digits[i] : 0;
        result[i] = a_digit ^ b_digit;
    }
    
    return BigNum(result, false);
}

BigNum& BigNum::operator&=(const BigNum& other) {
    *this = *this & other;
    return *this;
}

BigNum& BigNum::operator|=(const BigNum& other) {
    *this = *this | other;
    return *this;
}

BigNum& BigNum::operator^=(const BigNum& other) {
    *this = *this ^ other;
    return *this;
}

// Optimized modular exponentiation
BigNum BigNum::modPow(const BigNum& exponent, const BigNum& modulus) const {
    if (modulus.isZero()) {
        throw std::invalid_argument("Modulus cannot be zero");
    }
    
    if (exponent.isZero()) {
        return BigNum::one();
    }
    
    if (modulus.isOne()) {
        return BigNum::zero();
    }
    
    // Choose algorithm based on modulus size and whether it's odd
    if (modulus.getDigits().size() >= MONTGOMERY_THRESHOLD && modulus.isOdd()) {
        return modPowMontgomery(exponent, modulus);
    } else {
        return modPowBinary(exponent, modulus);
    }
}

BigNum BigNum::modPowMontgomery(const BigNum& exponent, const BigNum& modulus) const {
    try {
        MontgomeryContext mont(modulus);
        
        // Convert base to Montgomery form
        BigNum base_mont = mont.toMontgomery(*this % modulus);
        BigNum result_mont = mont.toMontgomery(BigNum(1LL));
        
        // Binary exponentiation in Montgomery form
        BigNum exp = exponent;
        BigNum current_base = base_mont;
        
        while (!exp.isZero()) {
            if (exp.isOdd()) {
                result_mont = mont.multiply(result_mont, current_base);
            }
            current_base = mont.multiply(current_base, current_base);
            exp >>= 1;
        }
        
        // Convert result back from Montgomery form
        return mont.fromMontgomery(result_mont);
        
    } catch (const std::exception&) {
        // Fall back to binary method if Montgomery setup fails
        return modPowBinary(exponent, modulus);
    }
}

BigNum BigNum::modPowBinary(const BigNum& exponent, const BigNum& modulus) const {
    BigNum result(1LL);
    BigNum base = *this % modulus;
    BigNum exp = exponent;
    
    // Use Barrett reduction for large moduli
    if (modulus.getDigits().size() >= BARRETT_THRESHOLD) {
        try {
            BarrettContext barrett(modulus);
            
            while (!exp.isZero()) {
                if (exp.isOdd()) {
                    result = barrett.reduce(result * base);
                }
                base = barrett.reduce(base * base);
                exp >>= 1;
            }
            
            return result;
        } catch (const std::exception&) {
            // Fall back to basic modular arithmetic
        }
    }
    
    // Basic binary exponentiation with simple modular reduction
    while (!exp.isZero()) {
        if (exp.isOdd()) {
            result = (result * base) % modulus;
        }
        base = (base * base) % modulus;
        exp >>= 1;
    }
    
    return result;
}

BigNum BigNum::gcd(const BigNum& other) const {
    BigNum a = *this;
    BigNum b = other;
    
    if (a.negative) a = -a;
    if (b.negative) b = -b;
    
    while (!b.isZero()) {
        BigNum temp = b;
        b = a % b;
        a = temp;
    }
    
    return a;
}

BigNum BigNum::modInverse(const BigNum& modulus) const {
    auto result = extendedGcd(modulus);
    if (!result.first.isOne()) {
        throw std::invalid_argument("Modular inverse does not exist");
    }
    
    BigNum inv = result.second.first;
    if (inv.isNegative()) {
        inv += modulus;
    }
    
    return inv % modulus;
}

std::pair<BigNum, std::pair<BigNum, BigNum>> BigNum::extendedGcd(const BigNum& other) const {
    BigNum old_r = *this;
    BigNum r = other;
    BigNum old_s = BigNum(1LL);
    BigNum s = BigNum(0LL);
    BigNum old_t = BigNum(0LL);
    BigNum t = BigNum(1LL);
    
    // Make sure we work with absolute values
    if (old_r.isNegative()) old_r = -old_r;
    if (r.isNegative()) r = -r;
    
    while (!r.isZero()) {
        BigNum quotient = old_r / r;
        
        BigNum temp_r = r;
        r = old_r - quotient * r;
        old_r = temp_r;
        
        BigNum temp_s = s;
        s = old_s - quotient * s;
        old_s = temp_s;
        
        BigNum temp_t = t;
        t = old_t - quotient * t;
        old_t = temp_t;
    }
    
    // Adjust signs if the original inputs were negative
    if (this->isNegative()) old_s = -old_s;
    if (other.isNegative()) old_t = -old_t;
    
    return {old_r, {old_s, old_t}};
}

// Utility functions
bool BigNum::isZero() const {
    return digits.size() == 1 && digits[0] == 0;
}

bool BigNum::isOne() const {
    return !negative && digits.size() == 1 && digits[0] == 1;
}

bool BigNum::isNegative() const {
    return negative && !isZero();
}

bool BigNum::isEven() const {
    return (digits[0] & 1) == 0;
}

bool BigNum::isOdd() const {
    return (digits[0] & 1) == 1;
}

size_t BigNum::bitLength() const {
    if (isZero()) return 0;
    
    size_t bits = (digits.size() - 1) * 64;
    uint64_t topDigit = digits.back();
    
    while (topDigit) {
        bits++;
        topDigit >>= 1;
    }
    
    return bits;
}

size_t BigNum::byteLength() const {
    return (bitLength() + 7) / 8;
}

// Conversion functions
std::string BigNum::toHexString() const {
    if (isZero()) return "0";
    
    std::stringstream ss;
    if (negative) ss << "-";
    
    ss << std::hex;
    bool first = true;
    for (int i = digits.size() - 1; i >= 0; --i) {
        if (first) {
            ss << digits[i];
            first = false;
        } else {
            ss << std::setfill('0') << std::setw(16) << digits[i];
        }
    }
    
    return ss.str();
}

std::vector<uint8_t> BigNum::toByteArray() const {
    size_t byteLen = byteLength();
    std::vector<uint8_t> result(byteLen);
    
    for (size_t i = 0; i < byteLen; ++i) {
        size_t digitIndex = i / 8;
        size_t byteIndex = i % 8;
        
        if (digitIndex < digits.size()) {
            result[byteLen - 1 - i] = static_cast<uint8_t>(
                (digits[digitIndex] >> (byteIndex * 8)) & 0xFF
            );
        }
    }
    
    return result;
}

int64_t BigNum::toInt64() const {
    // Check if the number is too large to fit in any 64-bit integer
    if (digits.size() > 1) {
        throw std::overflow_error("BigNum value is too large to fit in an int64_t");
    }

    // If it's a single digit, check its value against int64_t limits
    if (isZero()) {
        return 0;
    }

    uint64_t value = digits[0];

    if (!isNegative()) {
        // For positive numbers, the value must be <= INT64_MAX
        if (value > static_cast<uint64_t>(std::numeric_limits<int64_t>::max())) {
            throw std::overflow_error("BigNum value exceeds INT64_MAX");
        }
        return static_cast<int64_t>(value);
    } else {
        // For negative numbers, the value must be <= (INT64_MAX + 1)
        // Note: -INT64_MIN is INT64_MAX + 1
        uint64_t limit = static_cast<uint64_t>(std::numeric_limits<int64_t>::max()) + 1;
        if (value > limit) {
            throw std::overflow_error("BigNum value is less than INT64_MIN");
        }
        return -static_cast<int64_t>(value);
    }
}

// Factory methods
BigNum BigNum::fromByteArray(const std::vector<uint8_t>& bytes) {
    if (bytes.empty()) return BigNum::zero();
    
    // Calculate number of 64-bit digits needed
    size_t numDigits = (bytes.size() + 7) / 8;
    std::vector<uint64_t> digits(numDigits, 0);
    
    // Fill digits from big-endian byte array
    for (size_t i = 0; i < bytes.size(); ++i) {
        size_t digitIndex = (bytes.size() - 1 - i) / 8;
        size_t byteIndex = (bytes.size() - 1 - i) % 8;
        
        digits[digitIndex] |= static_cast<uint64_t>(bytes[i]) << (byteIndex * 8);
    }
    
    return BigNum(digits, false);
}

BigNum BigNum::fromHexString(const std::string& hexStr) {
    if (hexStr.empty()) return BigNum(0LL);
    
    std::string str = hexStr;
    bool neg = false;
    
    if (str[0] == '-') {
        neg = true;
        str = str.substr(1);
    }
    
    // Remove 0x prefix if present
    if (str.length() >= 2 && str.substr(0, 2) == "0x") {
        str = str.substr(2);
    }
    
    if (str.empty()) return BigNum(0LL);
    
    std::vector<uint64_t> result;
    
    // Process hex string in chunks of 16 characters (64 bits)
    for (int i = str.length(); i > 0; i -= 16) {
        int start = std::max(0, i - 16);
        std::string chunk = str.substr(start, i - start);
        
        uint64_t value = 0;
        for (char c : chunk) {
            value <<= 4;
            if (c >= '0' && c <= '9') {
                value |= c - '0';
            } else if (c >= 'a' && c <= 'f') {
                value |= c - 'a' + 10;
            } else if (c >= 'A' && c <= 'F') {
                value |= c - 'A' + 10;
            } else {
                throw std::invalid_argument("Invalid hex character");
            }
        }
        
        result.push_back(value);
    }
    
    return BigNum(result, neg);
}

BigNum BigNum::zero() {
    return BigNum(0LL);
}

BigNum BigNum::one() {
    return BigNum(1LL);
}

BigNum BigNum::two() {
    return BigNum(2LL);
}

// Prime testing (Miller-Rabin)
bool BigNum::isProbablePrime(int rounds) const {
    if (*this <= BigNum(1LL)) return false;
    if (*this == BigNum(2LL)) return true;
    if (isEven()) return false;
    
    // Write n-1 as d * 2^r
    BigNum n_minus_1 = *this - BigNum(1LL);
    BigNum d = n_minus_1;
    int r = 0;
    
    while (d.isEven()) {
        d >>= 1;
        r++;
    }
    
    std::random_device rd;
    std::mt19937 gen(rd());
    
    for (int i = 0; i < rounds; ++i) {
        BigNum a = BigNum::random(bitLength() - 1);
        if (a <= BigNum(1LL) || a >= *this - BigNum(1LL)) {
            continue;
        }
        
        BigNum x = a.modPow(d, *this);
        
        if (x == BigNum(1LL) || x == n_minus_1) {
            continue;
        }
        
        bool composite = true;
        for (int j = 0; j < r - 1; ++j) {
            x = x.modPow(BigNum(2LL), *this);
            if (x == n_minus_1) {
                composite = false;
                break;
            }
        }
        
        if (composite) return false;
    }
    
    return true;
}

BigNum BigNum::random(size_t bitLength) {
    if (bitLength == 0) return BigNum(0LL);
    
    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<uint64_t> dis(0, 0xFFFFFFFFFFFFFFFFULL);
    
    size_t numDigits = (bitLength + 63) / 64;
    std::vector<uint64_t> result(numDigits);
    
    for (size_t i = 0; i < numDigits; ++i) {
        result[i] = dis(gen);
    }
    
    // Mask the top digit to get exact bit length and ensure MSB is set
    size_t topBits = bitLength % 64;
    if (topBits == 0) {
        // Full 64-bit word: ensure MSB is set
        result.back() |= (1ULL << 63);
    } else {
        // Partial word: mask and set MSB
        uint64_t mask = (1ULL << topBits) - 1;
        result.back() &= mask;
        result.back() |= (1ULL << (topBits - 1));  // Set MSB
    }
    
    return BigNum(result, false);
}

BigNum BigNum::randomPrime(size_t bitLength) {
    if (bitLength < 2) {
        throw std::invalid_argument("Prime bit length must be at least 2");
    }
    
    // For small bit lengths, handle special cases
    if (bitLength == 2) {
        return BigNum(2LL);
    }
    if (bitLength == 3) {
        return BigNum(5LL); // or 7
    }
    
    BigNum candidate;
    int maxAttempts = bitLength * 50; // Reasonable limit
    
    for (int attempts = 0; attempts < maxAttempts; ++attempts) {
        // Generate random number of specified bit length
        candidate = BigNum::random(bitLength);
        
        // Ensure it's odd (required for primality)
        if (candidate.isEven()) {
            candidate += BigNum(1LL);
        }
        
        // Ensure it has the exact bit length by setting MSB if needed
        if (candidate.bitLength() < bitLength) {
            // Add the high bit to ensure exact bit length
            BigNum highBit = BigNum(1LL) << (bitLength - 1);
            candidate = candidate | highBit;
        }
        
        // Test for primality
        if (candidate.isProbablePrime(20)) {
            return candidate;
        }
        
        // Try the next odd number
        candidate += BigNum(2LL);
        if (candidate.isProbablePrime(20)) {
            return candidate;
        }
    }
    
    throw std::runtime_error("Failed to generate prime after maximum attempts");
}

// Stream operations
std::ostream& operator<<(std::ostream& os, const BigNum& num) {
    os << num.toHexString();
    return os;
}

std::istream& operator>>(std::istream& is, BigNum& num) {
    std::string str;
    is >> str;
    num = BigNum::fromHexString(str);
    return is;
}

//------------------------------------------------------------------------------
// MontgomeryContext implementation (at global/file scope)
//------------------------------------------------------------------------------

MontgomeryContext::MontgomeryContext(const BigNum& mod) : modulus(mod) {
    if (mod.isZero() || mod.isEven()) {
        throw std::invalid_argument("Montgomery form requires odd modulus");
    }
    
    k = mod.getDigits().size();
    
    // R = 2^(k*64)
    r = BigNum(1LL) << (k * 64);
    
    // Compute R^(-1) mod modulus using extended GCD
    auto gcd_result = r.extendedGcd(modulus);
    if (!gcd_result.first.isOne()) {
        throw std::invalid_argument("gcd(R, modulus) != 1");
    }
    r_inv = gcd_result.second.first;
    if (r_inv.isNegative()) {
        r_inv += modulus;
    }
    
    // Compute n' = -modulus^(-1) mod R
    auto mod_inv_result = modulus.extendedGcd(r);
    if (!mod_inv_result.first.isOne()) {
        throw std::invalid_argument("Modulus not invertible mod R");
    }
    n_prime = -mod_inv_result.second.first;
    if (n_prime.isNegative()) {
        n_prime += r;
    }
}

BigNum MontgomeryContext::reduce(const BigNum& a) const {
    // Optimized Montgomery reduction (CIOS method)
    std::vector<uint64_t> t(2 * k + 1, 0);
    
    // Copy input to working array
    const auto& a_digits = a.getDigits();
    for (size_t i = 0; i < std::min(a_digits.size(), 2 * k); ++i) {
        t[i] = a_digits[i];
    }
    
    // Montgomery reduction steps
    for (size_t i = 0; i < k; ++i) {
        // Calculate m = t[i] * n_prime[0] mod 2^64
        const auto& n_prime_digits = n_prime.getDigits();
        uint64_t m = t[i] * (n_prime_digits.empty() ? 0 : n_prime_digits[0]);
        
        // Add m * modulus to t, starting at position i
        __uint128_t carry = 0;
        const auto& mod_digits = modulus.getDigits();
        for (size_t j = 0; j < k; ++j) {
            __uint128_t prod = static_cast<__uint128_t>(t[i + j]) + 
                              static_cast<__uint128_t>(m) * mod_digits[j] + carry;
            t[i + j] = static_cast<uint64_t>(prod);
            carry = prod >> 64;
        }
        
        // Propagate carry
        for (size_t j = i + k; j < 2 * k + 1 && carry; ++j) {
            __uint128_t sum = static_cast<__uint128_t>(t[j]) + carry;
            t[j] = static_cast<uint64_t>(sum);
            carry = sum >> 64;
        }
    }
    
    // Extract result from positions k to 2k-1
    std::vector<uint64_t> result_digits(t.begin() + k, t.begin() + 2 * k);
    BigNum result(result_digits, false);
    
    // Final conditional subtraction
    if (result >= modulus) {
        result -= modulus;
    }
    
    return result;
}

BigNum MontgomeryContext::multiply(const BigNum& a, const BigNum& b) const {
    // Montgomery multiplication: (a * b) / R mod modulus
    BigNum product = a * b;
    return reduce(product);
}

BigNum MontgomeryContext::toMontgomery(const BigNum& a) const {
    // Convert to Montgomery form: a * R mod modulus
    return (a * r) % modulus;
}

BigNum MontgomeryContext::fromMontgomery(const BigNum& a) const {
    // Convert from Montgomery form: a / R mod modulus
    return reduce(a);
}

//------------------------------------------------------------------------------
// BarrettContext implementation (at global/file scope)
//------------------------------------------------------------------------------

BarrettContext::BarrettContext(const BigNum& mod) : modulus(mod) {
    if (mod.isZero()) {
        throw std::invalid_argument("Barrett reduction requires non-zero modulus");
    }
    
    k = mod.bitLength();
    
    // mu = floor(R^2 / modulus) where R = 2^k
    BigNum r_squared = BigNum(1LL) << (2 * k);
    mu = r_squared / modulus;
}

BigNum BarrettContext::reduce(const BigNum& a) const {
    // Barrett reduction algorithm
    if (a < modulus) {
        return a;
    }
    
    size_t a_bits = a.bitLength();
    if (a_bits <= k) {
        return a % modulus;
    }
    
    // q1 = floor(a / R^(k-1))
    BigNum q1 = a >> (k - 1);
    
    // q2 = q1 * mu
    BigNum q2 = q1 * mu;
    
    // q3 = floor(q2 / R^(k+1))
    BigNum q3 = q2 >> (k + 1);
    
    // r1 = a mod R^(k+1)
    BigNum r_mask = (BigNum(1LL) << (k + 1)) - BigNum(1LL);
    BigNum r1 = a & r_mask;
    
    // r2 = (q3 * modulus) mod R^(k+1)
    BigNum r2 = (q3 * modulus) & r_mask;
    
    // result = r1 - r2
    BigNum result = r1 - r2;
    if (result.isNegative()) {
        result += (BigNum(1LL) << (k + 1));
    }
    
    // Final reduction
    while (result >= modulus) {
        result -= modulus;
    }
    
    return result;
}