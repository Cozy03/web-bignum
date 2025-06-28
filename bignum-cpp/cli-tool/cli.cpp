/**
 * @file interactive_cli_demo.cpp
 * @brief A stylish, interactive CLI demonstration of the BigNum library.
 * @author Debrup Chatterjee
 * @date June 27, 2025
 *
 * This example provides an interactive command-line interface to demonstrate
 * the core functionality of the BigNum library. It emphasizes a clean user
 * experience with styled menus and clear instructions.
 *
 * All numerical inputs for BigNum objects must be provided in
 * hexadecimal format (e.g., "ff", "0x1A2B", "-DEADBEEF").
 */

#include "bignum.h"
#include <iostream>
#include <string>
#include <vector>
#include <limits>
#include <iomanip>
#include <sstream>

// --- UI Styling & Colors ---
constexpr const char* RESET   = "\033[0m";
constexpr const char* RED     = "\033[31;1m"; // Bold Red
constexpr const char* GREEN   = "\033[32;1m"; // Bold Green
constexpr const char* YELLOW  = "\033[33;1m"; // Bold Yellow
constexpr const char* BLUE    = "\033[34;1m"; // Bold Blue
constexpr const char* MAGENTA = "\033[35;1m"; // Bold Magenta
constexpr const char* CYAN    = "\033[36;1m"; // Bold Cyan
constexpr const char* WHITE   = "\033[37;1m"; // Bold White

// --- Helper Functions for UI ---

/**
 * @brief Clears the input buffer to handle invalid inputs gracefully.
 */
void clearInputBuffer() {
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

/**
 * @brief Pauses execution and waits for the user to press ENTER.
 * This function is designed to work correctly even if there are leftover
 * characters in the input stream from previous operations.
 */
void pressEnterToContinue() {
    std::cout << "\n" << YELLOW << "└─ Press ENTER to continue..." << RESET;
    // If the last operation was not line-based (e.g., cin >> choice),
    // there might be a newline character left in the buffer.
    // The first call to cin.get() would consume it immediately.
    // To ensure we always wait for a new keypress, we check if the
    // next character is a newline and consume it if so.
    if (std::cin.peek() == '\n') {
        std::cin.ignore();
    }
    std::cin.get();
}

/**
 * @brief Displays a styled header with a title.
 * @param title The text to display in the header.
 */
void printHeader(const std::string& title) {
    std::cout << "\n" << WHITE << "╔══════════════════════════════════════════════════╗" << std::endl;
    std::cout << "║ " << std::left << std::setw(50) << title << " ║" << std::endl;
    std::cout << "╚══════════════════════════════════════════════════╝" << RESET << std::endl;
}

/**
 * @brief Prompts the user to enter a BigNum in hexadecimal format and handles invalid input.
 * @param prompt The message to display to the user.
 * @return The BigNum object created from the user's input.
 */
BigNum readBigNumFromHex(const std::string& prompt) {
    std::string inputStr;
    while (true) {
        std::cout << CYAN << "  > " << std::setw(18) << std::left << prompt << RESET;
        std::cin >> inputStr;
        if (std::cin.fail()) {
            clearInputBuffer();
            continue;
        }
        try {
            return BigNum::fromHexString(inputStr);
        } catch (const std::exception& e) {
            std::cout << RED << "  [!] Invalid hexadecimal input. Please try again." << RESET << std::endl;
        }
    }
}

// --- Operation Handlers ---

void handleAddition() {
    printHeader("1. Addition (A + B)");
    BigNum a = readBigNumFromHex("Enter A (hex):");
    BigNum b = readBigNumFromHex("Enter B (hex):");
    BigNum result = a + b;
    std::cout << GREEN << "  > Result:        " << RESET << result.toHexString() << std::endl;
}

void handleSubtraction() {
    printHeader("2. Subtraction (A - B)");
    BigNum a = readBigNumFromHex("Enter A (hex):");
    BigNum b = readBigNumFromHex("Enter B (hex):");
    BigNum result = a - b;
    std::cout << GREEN << "  > Result:        " << RESET << result.toHexString() << std::endl;
}

void handleMultiplication() {
    printHeader("3. Multiplication (A * B)");
    BigNum a = readBigNumFromHex("Enter A (hex):");
    BigNum b = readBigNumFromHex("Enter B (hex):");
    BigNum result = a * b;
    std::cout << GREEN << "  > Result:        " << RESET << result.toHexString() << std::endl;
}

void handleDivision() {
    printHeader("4. Division & Modulo");
    BigNum a = readBigNumFromHex("Enter Dividend (hex):");
    BigNum b = readBigNumFromHex("Enter Divisor (hex):");
    try {
        if (b.isZero()) {
            throw std::invalid_argument("Division by zero is not allowed.");
        }
        BigNum quotient = a / b;
        BigNum remainder = a % b;
        std::cout << GREEN << "  > Quotient:      " << RESET << quotient.toHexString() << std::endl;
        std::cout << GREEN << "  > Remainder:     " << RESET << remainder.toHexString() << std::endl;
    } catch (const std::exception& e) {
        std::cout << RED << "  [!] Error: " << e.what() << RESET << std::endl;
    }
}

void handleGcd() {
    printHeader("5. Greatest Common Divisor (GCD)");
    BigNum a = readBigNumFromHex("Enter A (hex):");
    BigNum b = readBigNumFromHex("Enter B (hex):");
    BigNum result = a.gcd(b);
    std::cout << GREEN << "  > GCD(A, B):     " << RESET << result.toHexString() << std::endl;
}

void handleExtendedGcd() {
    printHeader("6. Extended Euclidean Algorithm");
    BigNum a = readBigNumFromHex("Enter A (hex):");
    BigNum b = readBigNumFromHex("Enter B (hex):");
    auto result = a.extendedGcd(b);
    BigNum g = result.first;
    BigNum s = result.second.first;
    BigNum t = result.second.second;
    std::cout << "  ──────────────────────────────────────────────────" << std::endl;
    std::cout << GREEN << "  > GCD (g): " << RESET << g.toHexString() << std::endl;
    std::cout << GREEN << "  > s:       " << RESET << s.toHexString() << std::endl;
    std::cout << GREEN << "  > t:       " << RESET << t.toHexString() << std::endl;
    std::cout << YELLOW << "  > Verification: g = (A * s) + (B * t)" << RESET << std::endl;
}


void handleModPow() {
    printHeader("7. Modular Exponentiation (Base^Exp % Mod)");
    BigNum base = readBigNumFromHex("Enter Base (hex):");
    BigNum exp  = readBigNumFromHex("Enter Exponent (hex):");
    BigNum mod  = readBigNumFromHex("Enter Modulus (hex):");
    try {
        if (mod.isZero()) {
            throw std::invalid_argument("Modulus cannot be zero.");
        }
        BigNum result = base.modPow(exp, mod);
        std::cout << GREEN << "  > Result:        " << RESET << result.toHexString() << std::endl;
    } catch (const std::exception& e) {
        std::cout << RED << "  [!] Error: " << e.what() << RESET << std::endl;
    }
}

void handleModInverse() {
    printHeader("8. Modular Inverse (A^-1 % Mod)");
    BigNum a = readBigNumFromHex("Enter A (hex):");
    BigNum mod = readBigNumFromHex("Enter Modulus (hex):");
    try {
        BigNum result = a.modInverse(mod);
        std::cout << GREEN << "  > Inverse:       " << RESET << result.toHexString() << std::endl;
    } catch(const std::exception& e) {
        std::cout << RED << "  [!] Error: " << e.what() << RESET << std::endl;
    }
}

void handleComparison() {
    printHeader("9. Comparison");
    BigNum a = readBigNumFromHex("Enter A (hex):");
    BigNum b = readBigNumFromHex("Enter B (hex):");
    std::cout << "  ──────────────────────────────────────────────────" << std::endl;
    if (a == b) std::cout << GREEN << "  > Result: A is equal to B (A == B)" << RESET << std::endl;
    if (a != b) std::cout << GREEN << "  > Result: A is not equal to B (A != B)" << RESET << std::endl;
    if (a < b)  std::cout << GREEN << "  > Result: A is less than B (A < B)" << RESET << std::endl;
    if (a > b)  std::cout << GREEN << "  > Result: A is greater than B (A > B)" << RESET << std::endl;
}

void handleBitwise() {
    printHeader("10. Bitwise Operations");
    BigNum a = readBigNumFromHex("Enter A (hex):");
    BigNum b = readBigNumFromHex("Enter B (hex):");
    int shift;
    std::cout << CYAN << "  > Enter shift amount (e.g., 8): " << RESET;
    std::cin >> shift;

    std::cout << "  ──────────────────────────────────────────────────" << std::endl;
    std::cout << GREEN << "  > A & B:   " << RESET << (a & b).toHexString() << std::endl;
    std::cout << GREEN << "  > A | B:   " << RESET << (a | b).toHexString() << std::endl;
    std::cout << GREEN << "  > A ^ B:   " << RESET << (a ^ b).toHexString() << std::endl;
    std::cout << GREEN << "  > A << " << shift << ": " << RESET << (a << shift).toHexString() << std::endl;
    std::cout << GREEN << "  > A >> " << shift << ": " << RESET << (a >> shift).toHexString() << std::endl;
}

void handleProperties() {
    printHeader("11. Number Properties");
    BigNum a = readBigNumFromHex("Enter Value (hex):");
    std::cout << "  ──────────────────────────────────────────────────" << std::endl;
    std::cout << GREEN << "  > Is Negative: " << RESET << (a.isNegative() ? "Yes" : "No") << std::endl;
    std::cout << GREEN << "  > Is Zero:     " << RESET << (a.isZero() ? "Yes" : "No") << std::endl;
    std::cout << GREEN << "  > Is Even:     " << RESET << (a.isEven() ? "Yes" : "No") << std::endl;
    std::cout << GREEN << "  > Bit Length:  " << RESET << a.bitLength() << std::endl;
    std::cout << GREEN << "  > Byte Length: " << RESET << a.byteLength() << std::endl;

    try {
        int64_t val = a.toInt64();
        std::cout << GREEN << "  > As int64_t:  " << RESET << val << std::endl;
    } catch(const std::overflow_error& e) {
        std::cout << GREEN << "  > As int64_t:  " << RED << e.what() << RESET << std::endl;
    }
}

void handlePrimalityTest() {
    printHeader("12. Primality Test (Miller-Rabin)");
    BigNum n = readBigNumFromHex("Enter number (hex):");
    int rounds = 20; // A reasonable default
    std::cout << "  ──────────────────────────────────────────────────" << std::endl;
    if (n.isProbablePrime(rounds)) {
        std::cout << GREEN << "  > Result: " << n.toHexString() << " is probably prime." << RESET << std::endl;
    } else {
        std::cout << RED << "  > Result: " << n.toHexString() << " is composite." << RESET << std::endl;
    }
}

void handleConversion() {
    printHeader("13. To/From Byte Array");
    std::cout << YELLOW << "Converting a BigNum to a byte array:" << RESET << std::endl;
    BigNum a = readBigNumFromHex("Enter Value (hex):");
    std::vector<uint8_t> bytes = a.toByteArray();
    std::cout << GREEN << "  > Byte Array (BE): [ " << RESET;
    std::string byteStr;
    for (size_t i = 0; i < bytes.size(); ++i) {
        std::stringstream ss;
        ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(bytes[i]);
        byteStr += ss.str();
        std::cout << YELLOW << ss.str() << RESET;
        if (i + 1 < bytes.size()) std::cout << ", ";
    }
    std::cout << std::dec << GREEN << " ]" << RESET << std::endl;

    std::cout << "\n" << YELLOW << "Creating a BigNum from a byte array (represented as a hex string):" << RESET << std::endl;
    std::cout << CYAN << "  > Enter bytes as a single hex string (e.g., 0123ab): " << RESET;
    std::string hex_bytes_str;
    std::cin >> hex_bytes_str;
    try {
        BigNum from_bytes = BigNum::fromHexString(hex_bytes_str);
        std::cout << GREEN << "  > Resulting BigNum: " << from_bytes.toHexString() << RESET << std::endl;
    } catch (const std::exception& e) {
        std::cout << RED << "  [!] Invalid hex string." << RESET << std::endl;
    }
}

void handleGeneration() {
    printHeader("14. Number Generation");
    size_t bits;
    std::cout << CYAN << "  > Enter bit length (e.g., 256): " << RESET;
    std::cin >> bits;

    std::cout << "  ──────────────────────────────────────────────────" << std::endl;
    BigNum r = BigNum::random(bits);
    std::cout << GREEN << "  > Random (" << bits << "-bit): " << RESET << r.toHexString() << std::endl;
    std::cout << YELLOW << "  > Generating a " << bits << "-bit prime (may take a moment)..." << RESET << std::endl;
    try {
        BigNum p = BigNum::randomPrime(bits);
        std::cout << GREEN << "  > Prime (" << bits << "-bit):  " << RESET << p.toHexString() << std::endl;
    } catch (const std::exception& e) {
        std::cout << RED << "  [!] Error: " << e.what() << RESET << std::endl;
    }
}

void printMenu() {
    printHeader("Main Menu");
    std::cout << "  " << MAGENTA << "--- Basic Arithmetic ---" << RESET << std::endl;
    std::cout << "  " << GREEN << "1." << RESET << " Addition             (A + B)" << std::endl;
    std::cout << "  " << GREEN << "2." << RESET << " Subtraction          (A - B)" << std::endl;
    std::cout << "  " << GREEN << "3." << RESET << " Multiplication       (A * B)" << std::endl;
    std::cout << "  " << GREEN << "4." << RESET << " Division & Modulo    (A / B, A % B)" << std::endl;
    std::cout << "  " << MAGENTA << "--- Crypto & Math ---" << RESET << std::endl;
    std::cout << "  " << GREEN << "5." << RESET << " Greatest Common Divisor (GCD)" << std::endl;
    std::cout << "  " << GREEN << "6." << RESET << " Extended Euclidean Algorithm" << std::endl;
    std::cout << "  " << GREEN << "7." << RESET << " Modular Exponentiation" << std::endl;
    std::cout << "  " << GREEN << "8." << RESET << " Modular Inverse" << std::endl;
    std::cout << "  " << MAGENTA << "--- Utilities & Conversions ---" << RESET << std::endl;
    std::cout << "  " << GREEN << "9." << RESET << " Comparison             (A vs B)" << std::endl;
    std::cout << "  " << GREEN << "10."<< RESET << " Bitwise Operations" << std::endl;
    std::cout << "  " << GREEN << "11."<< RESET << " Number Properties" << std::endl;
    std::cout << "  " << GREEN << "12."<< RESET << " Primality Test (Miller-Rabin)" << std::endl;
    std::cout << "  " << GREEN << "13."<< RESET << " To/From Byte Array" << std::endl;
    std::cout << "  " << MAGENTA << "--- Generation ---" << RESET << std::endl;
    std::cout << "  " << GREEN << "14."<< RESET << " Random Number & Prime Generation" << std::endl;
    std::cout << "  ──────────────────────────────────────────────────" << std::endl;
    std::cout << "  " << RED   << "0." << RESET << " Exit" << std::endl;
    std::cout << CYAN << "\n  Enter your choice: " << RESET;
}


int main() {
    printHeader("BigNum Interactive CLI Calculator");
    std::cout << "Welcome! This program provides an interactive shell to use the" << std::endl;
    std::cout << "BigNum library, much like tools such as PARI/GP." << std::endl;
    std::cout << YELLOW << "\nIMPORTANT: All number inputs must be in hexadecimal format." << RESET << std::endl;
    std::cout << "(e.g., " << GREEN << "ff" << RESET << ", " << GREEN << "0x1A2B" << RESET << ", or " << GREEN << "-DEADBEEF" << RESET << ")" << std::endl;
    
    // The very first pause just needs to wait for a single newline.
    std::cout << YELLOW << "\nPress ENTER to begin..." << RESET;
    std::cin.get();

    bool running = true;
    while (running) {
        printMenu();
        int choice;
        // Read the user's menu choice, and handle non-integer input
        if (!(std::cin >> choice)) {
            std::cout << RED << "\n  [!] Invalid input. Please enter a number." << RESET << std::endl;
            clearInputBuffer();
            pressEnterToContinue();
            continue;
        }

        switch (choice) {
            case 1: handleAddition();       break;
            case 2: handleSubtraction();    break;
            case 3: handleMultiplication(); break;
            case 4: handleDivision();       break;
            case 5: handleGcd();            break;
            case 6: handleExtendedGcd();    break;
            case 7: handleModPow();         break;
            case 8: handleModInverse();     break;
            case 9: handleComparison();     break;
            case 10: handleBitwise();       break;
            case 11: handleProperties();    break;
            case 12: handlePrimalityTest(); break;
            case 13: handleConversion();    break;
            case 14: handleGeneration();    break;
            case 0: running = false;        continue; // Skip the final pause
            default:
                std::cout << RED << "\n  [!] Unknown option. Please try again." << RESET << std::endl;
                break;
        }
        pressEnterToContinue();
    }

    std::cout << "\n" << MAGENTA << "Thank you for trying the BigNum CLI demo!" << RESET << "\n" << std::endl;
    return 0;
}
