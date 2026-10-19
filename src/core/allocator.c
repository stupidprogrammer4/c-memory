#include <stdint.h>
#include <string.h>
#include "c_memory/allocator.h"
#include "../internal/alignment.h"

static int allocator_valid(cm_allocator allocator)
{
    return allocator.allocate != NULL && allocator.deallocate != NULL;
}

cm_status cm_allocate(cm_allocator allocator, size_t size, size_t alignment, void **out)
{
    if (out == NULL) return CM_INVALID_ARGUMENT;
    *out = NULL;
    if (size == 0 || !allocator_valid(allocator) ||
        cm_normalize_alignment(&alignment) != CM_OK) return CM_INVALID_ARGUMENT;
    cm_status status = allocator.allocate(allocator.context, size, alignment, out);
    if (status != CM_OK) {
        *out = NULL;
        return status;
    }
    return *out != NULL ? CM_OK : CM_OUT_OF_MEMORY;
}

cm_status cm_callocate(cm_allocator allocator, size_t count, size_t size,
                      size_t alignment, void **out)
{
    if (out == NULL) return CM_INVALID_ARGUMENT;
    *out = NULL;
    if (count == 0 || size == 0) return CM_INVALID_ARGUMENT;
    if (size > SIZE_MAX / count) return CM_SIZE_OVERFLOW;
    cm_status status = cm_allocate(allocator, count * size, alignment, out);
    if (status == CM_OK) memset(*out, 0, count * size);
    return status;
}

cm_status cm_deallocate(cm_allocator allocator, void *allocation)
{
    if (allocation == NULL) return CM_OK;
    if (!allocator_valid(allocator)) return CM_INVALID_ARGUMENT;
    allocator.deallocate(allocator.context, allocation);
    return CM_OK;
}

cm_status cm_resize(cm_allocator allocator, void **allocation, size_t old_size,
                    size_t new_size, size_t alignment)
{
    if (allocation == NULL || !allocator_valid(allocator) ||
        cm_normalize_alignment(&alignment) != CM_OK) return CM_INVALID_ARGUMENT;
    if ((*allocation == NULL) != (old_size == 0)) return CM_INVALID_ARGUMENT;
    if (new_size == 0) {
        (void)cm_deallocate(allocator, *allocation);
        *allocation = NULL;
        return CM_OK;
    }
    void *replacement = NULL;
    cm_status status = cm_allocate(allocator, new_size, alignment, &replacement);
    if (status != CM_OK) return status;
    if (*allocation != NULL) {
        memcpy(replacement, *allocation, old_size < new_size ? old_size : new_size);
        (void)cm_deallocate(allocator, *allocation);
    }
    *allocation = replacement;
    return CM_OK;
}
