#ifndef CMEMORY_TRACKER_H
#define CMEMORY_TRACKER_H
#include "c_memory/allocator.h"
#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    void *allocation;
    size_t size;
    size_t alignment;
} cm_tracker_entry;
typedef struct {
    cm_allocator backing;
    cm_tracker_entry *entries;
    size_t capacity;
    size_t live_count;
    size_t live_bytes;
    size_t peak_bytes;
    size_t total_allocations;
    size_t invalid_frees;
} cm_tracker;

/* Borrows a caller-provided metadata array and allocates no internal metadata.
 * Entries [0, live_count) describe outstanding allocations for diagnostics.
 * Allocation is O(1) plus backing cost; deallocation is O(live_count).
 * Failure leaves tracker unchanged. Do not copy/reinitialize an active tracker.
 * The tracker, records and backing context must outlive its allocations and
 * adapters. Shared use requires caller synchronization. Counters saturate where
 * appropriate; live-byte overflow and metadata exhaustion reject allocation. */
cm_status cm_tracker_init(cm_tracker *tracker, cm_allocator backing, cm_tracker_entry *entries,
                          size_t capacity);
cm_allocator cm_tracker_allocator(cm_tracker *tracker);
/* Invalid frees are rejected and counted; NULL is a no-op. The adapter also
 * records invalid frees, but its void deallocator cannot return the diagnostic. */
cm_status cm_tracker_deallocate(cm_tracker *tracker, void *allocation);
/* Explicit shutdown cleanup, returning the number of outstanding allocations
 * that were released. Inspect entries/stats first to report leaks. The tracker
 * remains reusable and cumulative counters are retained. */
size_t cm_tracker_release_all(cm_tracker *tracker);

#ifdef __cplusplus
}
#endif
#endif
