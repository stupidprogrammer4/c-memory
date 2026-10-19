#ifndef CMEMORY_ARENA_H
#define CMEMORY_ARENA_H

#include "c_memory/allocator.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    unsigned char *storage;
    size_t capacity;
    size_t used;
} cm_arena;

/* Borrows storage; never allocates or frees its backing buffer. NULL storage
 * requires size 0. Failed initialization leaves arena unchanged. Storage must
 * remain valid and exclusively managed until the arena ends. Do not mutate
 * these observable fields after initialization. */
cm_status cm_arena_init(cm_arena *arena, void *storage, size_t size);

/* O(1). Failure leaves used unchanged and *out NULL. Alignment follows
 * cm_allocate; alignment padding consumes capacity. */
cm_status cm_arena_allocate(cm_arena *arena, size_t size, size_t alignment, void **out);

/* O(1); invalidates allocations without zeroing. NULL is a no-op.
 * Calls sharing an arena require external synchronization. */
void cm_arena_reset(cm_arena *arena);

/* Borrows arena. Individual frees are no-ops; only reset reclaims space.
 * The arena and backing storage must outlive the adapter. */
cm_allocator cm_arena_allocator(cm_arena *arena);

#ifdef __cplusplus
}
#endif
#endif
