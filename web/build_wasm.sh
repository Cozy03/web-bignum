#!/bin/bash
set -e

echo "🚀 Building BigNum WebAssembly module..."

# Check if Emscripten is available
if ! command -v emcc &> /dev/null; then
    echo "❌ Emscripten not found!"
    echo "💡 Run: source /tmp/emsdk/emsdk_env.sh"
    exit 1
fi

# Ensure we're in the right directory
cd "$(dirname "$0")"

echo "📂 Current directory: $(pwd)"
echo "🔍 Checking source files..."

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
    --bind \
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
