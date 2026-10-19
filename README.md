# C Memory Toolkit

A standalone C17 memory-management toolkit focused on explicit lifetimes, safe allocation, and measurable performance.

See [the project scope and roadmap](docs/ROADMAP.md) for module plans and acceptance requirements. This repository is independently buildable and installable. Using it from RepoPulse is optional.

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

The repository currently contains the initial project scaffold. Feature-complete releases will be published after the roadmap milestones and tests are implemented.
