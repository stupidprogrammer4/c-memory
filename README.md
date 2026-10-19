# C Memory Toolkit

An independent C17 toolkit with checked allocation, injectable allocators,
bounded arenas, fixed-size pools, size-class slabs, and optional allocation
tracking. No RepoPulse or sibling-library dependency is required.

See [API and ownership](docs/API.md), [architecture](docs/ARCHITECTURE.md),
[installation](docs/INSTALL.md), and [scope and completion status](docs/ROADMAP.md).
Editor setup and formatting are described in [development](docs/DEVELOPMENT.md).

## Requirements

- CMake 3.20 or newer
- A C17 compiler

## Build, test, and install

```sh
git clone --branch v0.1.0 https://github.com/stupidprogrammer4/c-memory.git
cd c-memory
./scripts/install.sh "$HOME/.local"
```

The installer builds and tests before installing. For individual build steps:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
ctest --test-dir build --output-on-failure
cmake --install build --prefix "$HOME/.local"
```

Or run `./scripts/install.sh "$HOME/.local"` to build, test and install.
Consumers use `find_package(CMemory 0.1 CONFIG REQUIRED)` and link
`CMemory::c_memory`. The initial declared toolkit scope is complete for version
0.1.0 on Linux x86-64. [Release archives and checksums](https://github.com/stupidprogrammer4/c-memory/releases/tag/v0.1.0)
are available for installation without building. Hosted CI is temporarily manual;
the release is validated locally with GCC and Clang.
