# Architecture

This library is independent of RepoPulse and all sibling projects. It has no
mandatory dependency beyond a C17 standard library providing `aligned_alloc`.
Linux x86-64 is the currently validated platform.

```text
include/c_memory/    Public contracts, ownership and complexity
src/core/           Checked allocation wrappers and version
src/allocators/     System, arena, fixed-block pool and size-class slab
src/diagnostics/    Optional tracking adapter
src/internal/       Private helpers shared by implementations
tests/unit/         Behavior, boundaries, failures and seeded stress scenarios
tests/integration/  Fresh external consumer of the installed CMake package
examples/           Runnable API usage
benchmarks/         Allocation workload compared with libc
cmake/              Relocatable package configuration
scripts/            Checkout-based installation
```

The core depends on public contracts and private arithmetic helpers. Allocation
strategies depend on the core. Slabs compose pools inside the same library.
Tracking decorates any allocator through its public contract; other modules do
not depend on tracking. There is no global mutable library state. Consumers
choose individual headers or the umbrella `c_memory.h` header.

## Algorithms and cost

| Operation | Time excluding backing allocator | Storage |
| --- | --- | --- |
| Checked allocation | O(1) | No hidden metadata |
| Zeroed allocation and resize | O(bytes) | Requested allocation; resize temporarily keeps both blocks |
| Arena allocate/reset | O(1) | Caller buffer plus alignment padding |
| Pool init/reset | O(capacity) | Aligned block stride and one occupancy bit per block |
| Pool allocate/free/ownership | O(1) | Free-list index stored in free blocks |
| Slab allocate/free | O(class count) | One pool per configured class |
| Tracker allocate | O(1) | One caller-provided record per live allocation |
| Tracker free | O(live records) | Compact array with swap removal |

Pools use an occupancy bitmap to reject duplicate frees without a linear scan.
Free-list indices are copied with `memcpy` so metadata does not require pointer
aliasing or stricter payload alignment. Slabs deliberately scan a configured
small set of classes, preserving first-fit-with-spill behavior. Tracking is
optional diagnostic work, and its linear removal cost is explicit. No custom
strategy is claimed faster than libc for every workload.
