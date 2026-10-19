#ifndef CMEMORY_SLAB_H
#define CMEMORY_SLAB_H
#include "c_memory/pool.h"
#ifdef __cplusplus
extern "C" {
#endif

typedef struct { size_t block_size; size_t alignment; size_t capacity; } cm_slab_class;
typedef struct { cm_allocator backing; cm_pool *pools; size_t class_count; } cm_slab;

/* Owns a pool-descriptor array and one allocation per size class. Classes must
 * have strictly increasing block sizes. Init rolls back all allocations on
 * failure and leaves slab unchanged. Do not reinitialize or copy a live slab.
 * Allocation picks the smallest eligible class with room, spilling into larger
 * classes when necessary. Allocate/free are O(class_count), with no resizing or
 * unbounded fallback. Synchronize operations on a shared slab externally. */
cm_status cm_slab_init(cm_slab *slab, cm_allocator backing,
                       const cm_slab_class *classes, size_t class_count);
cm_status cm_slab_allocate(cm_slab *slab, size_t size, size_t alignment, void **out);
cm_status cm_slab_deallocate(cm_slab *slab, void *allocation);
/* Both invalidate all outstanding allocations; reset retains backing memory. */
void cm_slab_reset(cm_slab *slab);
void cm_slab_destroy(cm_slab *slab);
cm_allocator cm_slab_allocator(cm_slab *slab);

#ifdef __cplusplus
}
#endif
#endif
