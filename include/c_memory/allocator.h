#ifndef CMEMORY_ALLOCATOR_H
#define CMEMORY_ALLOCATOR_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    CM_OK = 0,
    CM_INVALID_ARGUMENT,
    CM_SIZE_OVERFLOW,
    CM_OUT_OF_MEMORY,
    CM_CAPACITY_EXCEEDED
} cm_status;

/* Callbacks receive a nonzero size and power-of-two alignment. Allocation
 * sets *out to NULL on failure and transfers ownership only on CM_OK.
 * Deallocation accepts only live allocations from this same allocator. */
typedef cm_status (*cm_allocate_fn)(void *context, size_t size, size_t alignment, void **out);
typedef void (*cm_deallocate_fn)(void *context, void *allocation);

typedef struct {
    void *context;
    cm_allocate_fn allocate;
    cm_deallocate_fn deallocate;
} cm_allocator;

/* Stateless malloc/aligned_alloc/free adapter, with no hidden library state. */
cm_allocator cm_system_allocator(void);

/* Alignment 0 requests _Alignof(max_align_t); others must be powers of two.
 * Size 0 is rejected. *out is NULL on failure when out is non-NULL.
 * Release successful allocations through the same allocator. Callback
 * contracts and valid frees are caller preconditions. Sharing a mutable
 * context requires caller synchronization. Deallocating NULL is a no-op. */
cm_status cm_allocate(cm_allocator allocator, size_t size, size_t alignment, void **out);
cm_status cm_callocate(cm_allocator allocator, size_t count, size_t size,
                      size_t alignment, void **out);
cm_status cm_deallocate(cm_allocator allocator, void *allocation);

/* O(bytes): preserves min(old_size, new_size) bytes by allocating/copying.
 * Supply the actual old size. Failure preserves the pointer and its contents.
 * new_size == 0 releases and clears the pointer. NULL with old_size == 0 is
 * allowed. Arenas retain old space until reset because their frees are no-ops. */
cm_status cm_resize(cm_allocator allocator, void **allocation, size_t old_size,
                    size_t new_size, size_t alignment);

#ifdef __cplusplus
}
#endif
#endif
