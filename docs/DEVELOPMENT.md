# Development and editor configuration

The project compiles as ISO C17. Project builds enable type, conversion,
prototype and format-string diagnostics; warnings are errors by default. This
policy applies to this project's code, not installed consumers. Override
`CMEMORY_WARNINGS_AS_ERRORS` only when integrating a new compiler, and resolve
diagnostics before submitting changes.

## Compiler information for editors

Configure the project once using its standard build directory:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

CMake exports `build/compile_commands.json` with the actual standard, include
paths and per-file options. The checked-in `.clangd` points to this directory;
it leaves diagnostics enabled. Each independent sibling project has its own
configuration and compilation database, so the parent RepoPulse build is not
used to guess this library's include paths.

For the VS Code C/C++ extension, open this repository as a folder (or add it as
a workspace folder), then select its CMake configuration provider or set
`compileCommands` to this repository's generated database. C files should be
parsed as C17. Do not fix C diagnostics by treating implementations as C++,
adding unchecked casts, or disabling error reporting.

Configuration references: [clangd compilation databases](https://clangd.llvm.org/config#compilationdatabase)
and [VS Code C/C++ settings](https://code.visualstudio.com/docs/cpp/customize-cpp-settings).

## Consistent source formatting

The checked-in `.clang-format` defines the style. Use clang-format 23.1.3,
including braces around control flow and designated initialization for structs
whose field names clarify ownership. Formatting is a development tool; consumers
do not need Python or clang-format to build or use the library.

Refactor against the existing behavioral tests. Add a failing regression test
before changing behavior, and rerun the sanitizer build after resource-handling
changes. Public ownership and error contracts remain in the installed headers.
