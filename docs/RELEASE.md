# C Memory 0.1.0

The complete initial toolkit scope is available as an independent MIT-licensed
C17 library for Linux x86-64:

- Checked allocation, zeroing, aligned allocation and failure-preserving resize.
- Injectable allocator callbacks with documented ownership and error contracts.
- Bounded arenas with explicit bulk reset.
- Fixed-block pools with constant-time allocation/free and occupancy diagnostics.
- Size-class slabs with spill behavior and initialization rollback.
- Optional allocation tracking with caller-funded metadata, counters and cleanup.

The release includes CMake package exports, source installation, examples,
benchmarks and API documentation. It has no RepoPulse or sibling-library
dependency. Consumers use `find_package(CMemory 0.1 CONFIG REQUIRED)` and link
`CMemory::c_memory`.

Validation is local: GCC 15.2.0 and Clang 20.1.8, Debug/Release, static/shared
libraries, ASan/LSan/UBSan, both static analyzers, allocation-failure tests,
90,000 seeded mixed-operation stress steps, and clean-source/archive installation
checks. Hosted CI is temporarily manual and is not represented as passing.

Assets include Linux x86-64 static and shared library packages, a clean source
archive, build provenance and SHA-256 checksums. Follow `docs/INSTALL.md` to install from
an archive or run `./scripts/install.sh` from a source checkout.

Linux x86-64 is the validated platform. Mutable allocator contexts require
external synchronization. Arenas reclaim memory on reset; pool/slab/tracker
diagnostics do not make stale pointers valid after address reuse. This library
does not replace correct caller lifetimes or promise universal leak prevention.
The 0.1 API is the first public version; future incompatible changes require an
appropriate version/ABI change.
