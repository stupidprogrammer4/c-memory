#include <stdint.h>
#include <stdlib.h>
#include "c_memory/allocator.h"

static cm_status system_allocate(void *context, size_t size, size_t alignment, void **out)
{
    (void)context;
    *out = NULL;
    if (alignment <= _Alignof(max_align_t)) {
        *out = malloc(size);
    } else {
        size_t mask = alignment - 1;
        if (size > SIZE_MAX - mask) {
            return CM_SIZE_OVERFLOW;
        }
        size_t rounded_size = (size + mask) & ~mask;
        *out = aligned_alloc(alignment, rounded_size);
    }
    return *out != NULL ? CM_OK : CM_OUT_OF_MEMORY;
}

static void system_deallocate(void *context, void *allocation)
{
    (void)context;
    free(allocation);
}

cm_allocator cm_system_allocator(void)
{
    cm_allocator allocator = {
        .allocate = system_allocate,
        .deallocate = system_deallocate,
    };
    return allocator;
}
