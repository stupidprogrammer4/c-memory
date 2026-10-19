# Allocation benchmark

Run a Release build without sanitizers:

```sh
cmake -S . -B build/release -DCMAKE_BUILD_TYPE=Release -DCMEMORY_BUILD_BENCHMARKS=ON
cmake --build build/release --parallel
./build/release/allocation_benchmark
```

Each measurement performs 100,000 batches of 64 allocations of 32 bytes. It
touches and reads each payload, then releases the batch. The arena uses an
explicit benchmark adapter that resets after the last release in a batch;
ordinary arena frees still do nothing. Pool/slab backing storage is allocated
before timing. The tracker includes its diagnostic bookkeeping cost. Results
use C process CPU time and report nanoseconds per allocate/free pair. Run
several times and report variation rather than treating one sample as a target.

## Initial baseline: October 19, 2026

Linux x86-64, AMD Ryzen 7 3750H, GCC 15.2.0 (Ubuntu 15.2.0-4ubuntu4), CMake
Release flags, no link-time optimization. Five sequential runs, with raw data in
[`benchmarks/linux-x86_64-gcc15.csv`](benchmarks/linux-x86_64-gcc15.csv).

| Allocator | Minimum ns/pair | Median ns/pair | Maximum ns/pair |
| --- | ---: | ---: | ---: |
| libc malloc/free | 15.92 | 16.47 | 18.61 |
| System adapter | 23.86 | 25.49 | 26.30 |
| Arena with batch reset | 14.76 | 16.00 | 16.92 |
| Fixed-block pool | 13.63 | 14.71 | 17.27 |
| Three-class slab | 20.93 | 22.59 | 23.11 |
| Tracker with 64 records | 44.14 | 49.14 | 51.29 |

This is one synthetic workload on one machine. Arena and pool results include
preallocation advantages, while the system and tracker paths call the runtime
allocator. The table does not establish superiority for arbitrary sizes,
allocation lifetimes, thread contention, fragmentation or real repositories.
Choose a custom allocator only after measuring the consuming application's
workload. Tracking is an optional diagnostic cost, not a release-mode default.
