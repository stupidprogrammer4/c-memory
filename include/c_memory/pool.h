#ifndef CMEMORY_POOL_H
#define CMEMORY_POOL_H
#include "c_memory/allocator.h"
#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    cm_allocator backing;
    unsigned char *storage;
    unsigned char *occupied;
    size_t block_size;
    size_t stride;
    size_t alignment;
    size_t capacity;
    size_t available;
    size_t head;
} cm_pool;

/* Owns one backing allocation containing blocks, free-list links, and a bitmap.
 * Init is O(capacity), allocation/free/ownership checks are O(1). All sizes and
 * capacity must be positive. Failure leaves pool unchanged. Initialize only
 * an uninitialized or destroyed pool. A live pool must not be shallow-copied.
 * Observe fields but do not modify them. Synchronize shared pools externally. */
cm_status cm_pool_init(cm_pool *pool, cm_allocator backing, size_t block_size,
                       size_t alignment, size_t capacity);
cm_status cm_pool_allocate(cm_pool *pool, void **out);
/* Rejects foreign, interior, or already-freed pointers. NULL is a no-op. */
cm_status cm_pool_deallocate(cm_pool *pool, void *allocation);
/* Reports block boundaries, including currently free blocks. */
int cm_pool_owns(const cm_pool *pool, const void *allocation);
/* O(capacity); invalidates all outstanding allocations without zeroing. */
void cm_pool_reset(cm_pool *pool);
/* Releases backing storage, invalidating all allocations. Repeated destroy
 * is safe after a successful init or zero initialization. */
void cm_pool_destroy(cm_pool *pool);
/* Adapter accepts size <= block_size and alignment <= configured alignment.
 * Valid frees are a callback precondition; use the direct API for diagnostics. */
cm_allocator cm_pool_allocator(cm_pool *pool);

#ifdef __cplusplus
}
#endif
#endif
