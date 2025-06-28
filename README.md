# Web-BigNum

**In-browser arbitrary-precision calculator** powered by a C++ BigNum library compiled to WebAssembly.

🔗 **Live demo:** [https://cozy03.github.io/web-bignum/](https://cozy03.github.io/web-bignum/)

## 🚀 Features

* **REPL terminal:** `123 + 456`, `2^512`, `gcd(48,18)`, `modpow(3,100,7)`
* **Cryptographic utilities:** prime generation, modular inverse, extended GCD, random big integers
* **Base conversion:** decimal ↔ hexadecimal ↔ binary
* **Performance dashboard:** latency, ops/sec, memory usage

## 🛠️ Prerequisites

* **Git & CMake**
* **Emscripten SDK:** [montudor/setup-emscripten](https://github.com/montudor/setup-emscripten)
* **Python 3** (for local HTTP server)

## 📥 Quickstart

```bash
git clone https://github.com/Cozy03/web-bignum.git
cd web-bignum
git submodule update --init --recursive
cd web
./build_wasm.sh
# Serve locally:
python3 -m http.server 8000
```

Open `http://localhost:8000` in your browser.

## 📦 CI/CD

* **Workflow:** `.github/workflows/gh-pages-mac.yml`
* **Deploys** the `web/` directory to GitHub Pages (`gh-pages` branch)

## 📄 License

MIT © Debrup
