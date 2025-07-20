 # 🚀 jing-lib

A wrapper dynamic library for the jing project, providing essential utilities for:
- Memory access optimization
- Data compression/decompression
- File I/O operations
- Network communication
- Encryption/decryption services

## ⚙️ Prerequisites

### Platform-specific requirements
- **Windows**: Visual Studio 2022, NASM, Perl
- **Linux**: GCC compiler
- **macOS**: Clang compiler

### Universal requirements
- Git
- CMake

> **Note**: Currently supporting win-x64, linux-x64, and clang-aarch64 architectures only

## 🛠️ Build Instructions

To build jing-lib, run the following commands:

```bash
cmake -E remove_directory build
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release --verbose
```

## 📚 Dependencies

jing-lib builds upon these excellent libraries:

| Library | Purpose |
|---------|---------|
| [wepoll](https://github.com/piscisaureus/wepoll) | Event polling |
| [rpmalloc](https://github.com/mjansson/rpmalloc) | Memory allocation |
| [mimalloc](https://github.com/microsoft/mimalloc) | Memory allocation |
| [fast_float](https://github.com/fastfloat/fast_float) | Fast floating-point parsing |
| [libdeflate](https://github.com/ebiggers/libdeflate) | Compression algorithms |
| [Brotli](https://github.com/google/brotli) | Compression algorithms |
| [zstd](https://github.com/facebook/zstd) | Compression algorithms |
| [OpenSSL](https://github.com/openssl/openssl) | Cryptography |

---

For questions or contributions, please open an issue or submit a pull request.

