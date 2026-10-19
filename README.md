# C Memory Toolkit

A standalone C17 memory-management toolkit focused on explicit lifetimes, safe allocation, and measurable performance.

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
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
ctest --test-dir build --output-on-failure
cmake --install build --prefix "$HOME/.local"
```

Or run `./scripts/install.sh "$HOME/.local"` to build, test and install.
Consumers use `find_package(CMemory 0.1 CONFIG REQUIRED)` and link
`CMemory::c_memory`. Development stays private until release validation is complete.
