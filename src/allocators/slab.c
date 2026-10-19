#include <stdint.h>
#include "c_memory/slab.h"
#include "../internal/alignment.h"

cm_status cm_slab_init(cm_slab *slab, cm_allocator backing,
                       const cm_slab_class *classes, size_t class_count)
{
    if (slab == NULL || classes == NULL || class_count == 0) return CM_INVALID_ARGUMENT;
    if (class_count > SIZE_MAX / sizeof(cm_pool)) return CM_SIZE_OVERFLOW;
    for (size_t i = 0; i < class_count; ++i) {
        size_t alignment = classes[i].alignment;
        if (classes[i].block_size == 0 || classes[i].capacity == 0 ||
            (i != 0 && classes[i].block_size <= classes[i - 1].block_size) ||
            cm_normalize_alignment(&alignment) != CM_OK) return CM_INVALID_ARGUMENT;
    }
    void *descriptors = NULL;
    cm_status status = cm_callocate(backing, class_count, sizeof(cm_pool), 0, &descriptors);
    if (status != CM_OK) return status;
    cm_slab result = {backing, descriptors, 0};
    for (size_t i = 0; i < class_count; ++i) {
        status = cm_pool_init(&result.pools[i], backing, classes[i].block_size,
                              classes[i].alignment, classes[i].capacity);
        if (status != CM_OK) {
            cm_slab_destroy(&result);
            return status;
        }
        result.class_count++;
    }
    *slab = result;
    return CM_OK;
}

cm_status cm_slab_allocate(cm_slab *slab, size_t size, size_t alignment, void **out)
{
    if (out == NULL) return CM_INVALID_ARGUMENT;
    *out = NULL;
    if (slab == NULL || slab->pools == NULL || size == 0 ||
        cm_normalize_alignment(&alignment) != CM_OK) return CM_INVALID_ARGUMENT;
    for (size_t i = 0; i < slab->class_count; ++i) {
        cm_pool *pool = &slab->pools[i];
        if (size <= pool->block_size && alignment <= pool->alignment && pool->available != 0)
            return cm_pool_allocate(pool, out);
    }
    return CM_CAPACITY_EXCEEDED;
}

cm_status cm_slab_deallocate(cm_slab *slab, void *allocation)
{
    if (allocation == NULL) return CM_OK;
    if (slab == NULL || slab->pools == NULL) return CM_INVALID_ARGUMENT;
    for (size_t i = 0; i < slab->class_count; ++i)
        if (cm_pool_owns(&slab->pools[i], allocation))
            return cm_pool_deallocate(&slab->pools[i], allocation);
    return CM_INVALID_ARGUMENT;
}

void cm_slab_reset(cm_slab *slab)
{
    if (slab == NULL) return;
    for (size_t i = 0; i < slab->class_count; ++i) cm_pool_reset(&slab->pools[i]);
}

void cm_slab_destroy(cm_slab *slab)
{
    if (slab == NULL) return;
    for (size_t i = 0; i < slab->class_count; ++i) cm_pool_destroy(&slab->pools[i]);
    (void)cm_deallocate(slab->backing, slab->pools);
    *slab = (cm_slab){0};
}

static cm_status slab_allocate(void *context, size_t size, size_t alignment, void **out)
{
    return cm_slab_allocate(context, size, alignment, out);
}

static void slab_deallocate(void *context, void *allocation)
{
    (void)cm_slab_deallocate(context, allocation);
}

cm_allocator cm_slab_allocator(cm_slab *slab)
{
    cm_allocator allocator = {slab, slab_allocate, slab_deallocate};
    return allocator;
}
