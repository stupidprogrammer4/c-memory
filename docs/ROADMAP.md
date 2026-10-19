# Scope and roadmap

This is an independently usable C17 library. It must have no dependency on RepoPulse or the data-structures repository.

Status: implementation and local validation are complete; external CI and
release publication checks are required before publication. Keep this repository
private until those checks complete.

## Implemented modules

- Checked allocation helpers with explicit size-overflow and failure semantics
- Arena allocator for bounded, bulk-reset lifetimes
- Pool and slab allocators for measured fixed-size workloads
- Optional allocation tracking and leak diagnostics for development builds
- Allocator interface that lets other libraries inject their own allocation policy

## Acceptance requirements

Document alignment, ownership, thread-safety, reset and destruction semantics. Test zero sizes, overflow, exhaustion, invalid alignment, repeated reset, and cleanup. Debug tracking must not be required in release builds. Custom allocators are benchmarked against the system allocator and are never imposed on consumers.

## Delivery

Use CMake and CTest; install headers and library independently. Keep sanitizer-enabled development builds available.

## Completion evidence

- Checked allocation, zeroing, resize, and injectable allocator callbacks.
- Bounded arena and adapter with explicit bulk-reset ownership.
- Constant-time pool operations with a free list and occupancy bitmap.
- Size-class slabs with overflow checks and complete initialization rollback.
- Optional caller-funded tracking with leak records, counters and cleanup.
- Unit tests, allocation-failure injection at every slab backing allocation,
  and 90,000 deterministic mixed-allocation stress steps.
- Local GCC Debug, Release, static/shared builds, ASan/LSan/UBSan and GCC analyzer
  checks; no failures reported.
- Independent installed-package and relocated-archive consumers, runnable example, source installer,
  CPack archive generation, API/lifetime documentation and measured benchmarks.

Remaining release gates: GCC/Clang CI, release archive and checksum publication.
These are release work, not deferred modules.

See `VALIDATION.md` for the recorded checks and remaining publication gates.
