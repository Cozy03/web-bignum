#!/bin/bash
set -e

echo "🚀 Building BigNum WebAssembly module..."

# Check if Emscripten is available
if ! command -v emcc &> /dev/null; then
    echo "❌ Emscripten not found!"
    
    # Try different common locations
    if [ -n "$EMSDK" ] && [ -f "$EMSDK/emsdk_env.sh" ]; then
        echo "💡 Sourcing from EMSDK environment: $EMSDK"
        source "$EMSDK/emsdk_env.sh"
    elif [ -f "/tmp/emsdk/emsdk_env.sh" ]; then
        echo "💡 Run: source /tmp/emsdk/emsdk_env.sh"
    else
        echo "💡 Install Emscripten SDK and ensure it's in your PATH"
    fi
    
    # Check again after potential sourcing
    if ! command -v emcc &> /dev/null; then
        exit 1
    fi
fi

# Ensure we're in the right directory
cd "$(dirname "$0")"

echo "📂 Current directory: $(pwd)"
echo "🔍 Checking source files..."

# Debug: Show what's actually in the repository
echo "🐛 DEBUG: Repository structure:"
ls -la ../
echo "🐛 DEBUG: Looking for bignum-cpp directory:"
ls -la ../bignum-cpp/ 2>/dev/null || echo "bignum-cpp directory not found"
echo "🐛 DEBUG: Looking for source files:"
ls -la ../bignum-cpp/src/ 2>/dev/null || echo "src directory not found"
ls -la ../bignum-cpp/include/ 2>/dev/null || echo "include directory not found"

if [ ! -f "bignum_bindings.cpp" ]; then
    echo "❌ bignum_bindings.cpp not found!"
    exit 1
fi

if [ ! -f "../bignum-cpp/src/bignum.cpp" ]; then
    echo "❌ bignum.cpp not found!"
    exit 1
fi

if [ ! -f "../bignum-cpp/include/bignum.h" ]; then
    echo "❌ bignum.h not found!"
    exit 1
fi

echo "✅ All source files found"

# Clean previous builds
echo "🧹 Cleaning previous builds..."
rm -f bignum.js bignum.wasm bignum.d.ts

# Compile to WebAssembly
echo "🔨 Compiling to WebAssembly..."

emcc ../bignum-cpp/src/bignum.cpp bignum_bindings.cpp \
    -I../bignum-cpp/include \
    -O3 \
    -s WASM=1 \
    -s MODULARIZE=1 \
    -s EXPORT_NAME="BigNumModule" \
    -s ALLOW_MEMORY_GROWTH=1 \
    -s MAXIMUM_MEMORY=268435456 \
    -s NO_EXIT_RUNTIME=1 \
    -s ASSERTIONS=0 \
    -s DISABLE_EXCEPTION_CATCHING=0 \
    -s FILESYSTEM=0 \
    -s ENVIRONMENT='web,worker' \
    -s EXPORTED_RUNTIME_METHODS='["ccall", "cwrap"]' \
    --bind \
    -std=c++17 \
    -o bignum.js

if [ $? -eq 0 ]; then
    echo "✅ WebAssembly compilation successful!"
    echo ""
    echo "📄 Generated files:"
    echo "   - bignum.js (JavaScript wrapper)"
    echo "   - bignum.wasm (WebAssembly binary)"
    echo ""
    echo "📊 File sizes:"
    ls -lh bignum.js bignum.wasm 2>/dev/null || echo "Files generated successfully"
    echo ""
    echo "🌐 Ready to deploy! Your BigNum calculator is ready to use."
else
    echo "❌ Compilation failed!"
    exit 1
fi