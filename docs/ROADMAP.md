# Scope and roadmap

This is an independently usable C17 library. It must have no dependency on RepoPulse or the data-structures repository.

## Planned modules

- Checked allocation helpers with explicit size-overflow and failure semantics
- Arena allocator for bounded, bulk-reset lifetimes
- Pool and slab allocators for measured fixed-size workloads
- Optional allocation tracking and leak diagnostics for development builds
- Allocator interface that lets other libraries inject their own allocation policy

## Acceptance requirements

Document alignment, ownership, thread-safety, reset and destruction semantics. Test zero sizes, overflow, exhaustion, invalid alignment, repeated reset, and cleanup. Debug tracking must not be required in release builds. Custom allocators are benchmarked against the system allocator and are never imposed on consumers.

## Delivery

Use CMake and CTest; install headers and library independently. Keep sanitizer-enabled development builds available.
