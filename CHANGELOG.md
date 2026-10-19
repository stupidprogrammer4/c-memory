# Changelog

## 0.1.0

First usable toolkit release covering the declared initial scope:

- Checked and aligned allocation, zeroing, transactional resize and allocator injection.
- Bounded arenas, fixed-size pools with duplicate-free detection, and size-class slabs.
- Optional allocation tracking using caller-provided metadata.
- Explicit ownership, alignment, thread-safety, error and complexity contracts.
- CMake installation/export, standalone consumer validation, examples and benchmarks.

The API is an initial public version. Only Linux x86-64 has been validated.
No internal synchronization, automatic arena per-object reclamation, universal
leak prevention, or general-purpose malloc replacement is promised. Correct
caller ownership and object-lifetime handling remain required.
