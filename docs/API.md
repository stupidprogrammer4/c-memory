# Using the toolkit safely

Public headers contain the normative error and lifetime contracts. All outputs
that receive a new allocation are cleared on failure. Resize is different: it
keeps the original pointer and contents on failure. Error statuses distinguish
invalid input, arithmetic overflow, backing allocation failure and fixed-capacity
exhaustion. Zero-size allocations are rejected; resizing to zero frees.

## Allocation and alignment

Use `cm_system_allocator()` with `cm_allocate`, `cm_callocate`, `cm_resize` and
`cm_deallocate`. Alignment zero selects `_Alignof(max_align_t)`; explicit values
must be powers of two. Extended-alignment system allocations round the backing
size to the requested alignment after checking overflow. The caller owns a
successful allocation until deallocation through the same allocator.

Resize allocates a replacement before freeing the source. Supply the actual old
size; the library cannot infer it. A pool, slab, arena or tracker may therefore
need room for an extra allocation during resize. When resize fails, release the
original allocation normally. Never free a pointer twice or through a different
system allocator instance with a different mutable context.

## Arenas

Initialize an arena with caller-owned storage, then allocate through the direct
API or its adapter. Reset invalidates every outstanding pointer and keeps the
buffer. Individual frees do nothing. When storing typed objects, use backing
memory obtained from an allocation function (as in `examples/arena.c`) and
respect C's effective-type rules. A declared character array is suitable for
byte data but does not remove the language's object-type requirements.

## Pools and slabs

Initialize a pool with its backing allocator, payload size, alignment and block
count. It owns one backing allocation and never grows automatically. The stride
can exceed payload size to hold a free-list index and satisfy alignment. Direct
pool frees reject foreign, interior and duplicate pointers.

Slabs own an array of fixed-block pools. Supply strictly increasing size classes
with alignment and block count for each. Allocation tries eligible classes in
order and may spill into a larger class when a smaller class is full. An
oversized or unsupported-alignment request returns capacity exhaustion without
calling an unbounded fallback allocator. A failed init releases every successful
backing allocation made so far. An arena backing allocator still only reclaims
capacity on its own reset, because its deallocator is a no-op.

Destroy a pool or slab before its backing allocator context expires. Destruction
and reset invalidate outstanding payloads. Never copy an owning pool or slab
struct or initialize it again without destruction. Repeated destroy of a
zero-initialized or previously destroyed object is safe.

## Optional tracking

Provide a `cm_tracker_entry` array and wrap any backing allocator. The tracker
performs no hidden metadata allocations. Its records describe live pointers,
sizes and alignments. Inspect `live_count`, `live_bytes`, `peak_bytes`,
`total_allocations` and `invalid_frees` for diagnostics. Invalid frees are rejected
and counted; the direct tracker API also returns an error.

Inspect outstanding entries before calling `cm_tracker_release_all`, which
explicitly cleans up remaining allocations and returns their count. Losing the
tracker does not automatically free user allocations. All tracker storage and
backing contexts must outlive its adapters and allocations. Debug tracking is
optional; ordinary allocators have no dependency on it.

## Concurrency

There are no internal locks or worker threads. Separate independent arenas,
pools, slabs and trackers can be used concurrently. Access to one shared mutable
instance requires external synchronization. A system allocator adapter has no
mutable context; its concurrency guarantees are those of the C runtime.
