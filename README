# Web-BigNum

**Author:** Debrup [debrupc3@gmail.com](mailto:debrupc3@gmail.com)

An **in-browser arbitrary-precision calculator** powered by a C++ BigNum library compiled to WebAssembly. This project integrates the `bignum-cpp` library as a Git submodule and provides a static UI to perform high-precision arithmetic and cryptographic operations directly in the browser.

---

## 🚀 Features

* **Interactive REPL** style terminal for expressions: `123 + 456`, `2^512`, `gcd(48,18)`, `modpow(3,100,7)`, etc.
* **Cryptographic utilities**: prime generation, modular inverse, extended GCD, random big integers.
* **Number base converter**: decimal, hex, binary, octal.
* **Performance dashboard**: last operation time, ops/sec, memory usage.
* **Demo sequences**: RSA key generation, primality testing, factorization workflows.
* **Zero-backend architecture**: 100% static, no server required once deployed.

---

## 🛠️ Prerequisites

* **Git** (>= 2.20)
* **CMake** (>= 3.16)
* **Emscripten SDK** (installed via [`montudor/setup-emscripten`](https://github.com/montudor/setup-emscripten))
* **Python 3** (for local HTTP server) or **Node.js** (optional)

---

## 📥 Getting Started

1. **Clone repository**

   ```bash
   git clone https://github.com/Debrup/web-bignum.git
   cd web-bignum
   ```
2. **Initialize submodules**

   ```bash
   git submodule update --init --recursive
   ```
3. **Build WebAssembly artifacts**

   ```bash
   cd web
   ./build_wasm.sh
   ```
4. **Serve the UI locally**

   ```bash
   # Option A: Python static server
   python3 -m http.server 8000

   # Option B: Node.js (if installed)
   npx serve . -l 8000
   ```
5. Open your browser at `http://localhost:8000` and start calculating!

---

## 🔧 CI/CD (GitHub Actions)

A macOS-based workflow is provided at `.github/workflows/gh-pages-mac.yml`:

1. **Checkout** with submodules
2. **Install** Emscripten
3. **Build** the WASM & JS with `web/build_wasm.sh`
4. **Publish** the `web/` directory to GitHub Pages

Once enabled under **Settings → Pages**, the site will be available at:

```
https://Debrup.github.io/web-bignum/
```

---

## 📝 Repository Structure

```
web-bignum/
├── .github/
│   └── workflows/gh-pages-mac.yml   # CI: macOS → build & deploy
├── bignum-cpp/                      # Submodule: C++ BigNum library
├── web/                             # Static UI + WASM artifacts
│   ├── build_wasm.sh                # Shell script to build WASM
│   ├── bignum.wasm                  # Compiled WebAssembly module
│   ├── bignum.js                    # Emscripten JS glue code
│   └── index.html                   # Interactive UI
└── README.md                        # Project overview & instructions
```

---

## 📈 Roadmap

* **Progressive Web App** support (offline caching via service worker).
* **Custom domain** and HTTPS via GitHub Pages or Cloudflare.
* **Extended API** integration for embedding in other web apps.
* **Analytics** to track real-user performance.

---

## 📄 License

MIT License © Debrup
