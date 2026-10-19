# C and Assembly Engineering Rules

- Keep committed source comments and documentation in English.
- Use C17 and CMake. Register automated tests with CTest.
- Follow test-driven development: write a failing test before implementing each behavior, then refactor with tests passing.
- Document ownership, lifetime, mutability, thread-safety, complexity, and error semantics for public APIs.
- Check allocation, parsing, system-call, and I/O failures; clean up on every failure path.
- Consult *The Art of Computer Programming* where relevant; validate algorithm choices with correctness tests and benchmarks.
- Use `static inline` selectively; compiler inlining is not guaranteed by the C keyword.
- Add parallelism only when measurements justify it; use OpenMP when appropriate and preserve deterministic behavior.
- Use sanitizers and static analysis during development. Do not ignore test or sanitizer failures.
- Keep modules focused on one responsibility. Expose supported interfaces in public headers and keep implementation details in private headers. Enforce module dependencies with CMake targets where useful.
- Follow the checked-in `.clang-format` style, brace control flow, and use designated initializers when field names clarify resource ownership.
- Keep type and conversion diagnostics enabled. Fix their causes; do not hide errors with diagnostic suppression or unchecked casts.
