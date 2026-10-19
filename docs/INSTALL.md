# Installation

Requirements: CMake 3.20+, a C17 compiler, and a standard library with
`aligned_alloc`. Linux x86-64 is the tested platform. No sibling repository,
Python runtime or external service is needed.

## From a checkout

```sh
./scripts/install.sh "$HOME/.local"
```

The script configures Release, builds, runs CTest and installs only when checks
pass. Equivalently, run CMake configure/build, CTest, then `cmake --install` with
your selected prefix. Installing under your home directory needs no root access.
The public release checkout can be cloned without GitHub authentication.

## From a release archive

Use an archive matching your platform, verify its SHA-256 checksum, and extract
it to a chosen prefix. Do not use a development build with sanitizers as a release
library. The archive contains headers, the library, CMake package files, license
and documentation. Linux packages are built for the release runner's toolchain;
build from source when using an incompatible runtime or platform.

For the Linux x86-64 static-library archive:

```sh
curl -fLO https://github.com/stupidprogrammer4/c-memory/releases/download/v0.1.0/c-memory-0.1.0-Linux-x86_64.tar.gz
curl -fLO https://github.com/stupidprogrammer4/c-memory/releases/download/v0.1.0/SHA256SUMS
sha256sum --check --ignore-missing SHA256SUMS
mkdir -p "$HOME/.local"
tar -xzf c-memory-0.1.0-Linux-x86_64.tar.gz --strip-components=1 -C "$HOME/.local"
```

A separate `c-memory-0.1.0-Linux-x86_64-shared.tar.gz` asset supplies the shared
library; use it instead of the static archive when desired. For shared-library
deployment, ensure the chosen library directory is discoverable by the runtime
loader. CMake configures build-tree consumer runtime paths during validation.

## Consume the installed library

```cmake
find_package(CMemory 0.1 CONFIG REQUIRED)
target_link_libraries(my_application PRIVATE CMemory::c_memory)
```

Configure the application with `-DCMAKE_PREFIX_PATH="$HOME/.local"` or the prefix
you selected. Include only the required `c_memory/*.h` headers, or use
`c_memory/c_memory.h`. No source-tree paths are needed.

## Upgrade and uninstall

Keep the CMake install manifest or archive file list. Before an incompatible
upgrade, remove files from that manifest within the selected prefix, or install
the new version under a separate versioned prefix. Never delete a shared prefix
directory wholesale. The toolkit has no daemon, cache or global system state.

## Developer builds

```sh
cmake -S . -B build/sanitized -DCMAKE_BUILD_TYPE=Debug -DCMEMORY_ENABLE_SANITIZERS=ON
cmake --build build/sanitized --parallel
ASAN_OPTIONS=detect_leaks=1 UBSAN_OPTIONS=halt_on_error=1 ctest --test-dir build/sanitized --output-on-failure
```

The installed-consumer integration test runs in ordinary Debug/Release builds;
sanitized builds run behavioral tests directly. To build a release archive:

```sh
cmake -S . -B build/release -DCMAKE_BUILD_TYPE=Release
cmake --build build/release --parallel
ctest --test-dir build/release --output-on-failure
cmake --build build/release --target package
```

For the complete local compiler matrix, run `./scripts/validate.sh clang` (or
pass an absolute Clang executable path). It also uses `gcc`, which can be
selected through `CMEMORY_GCC`. GCC and Clang are development validation tools;
only one C17 compiler is required for ordinary source installation.
