#ifndef CMEMORY_INTERNAL_ALIGNMENT_H
#define CMEMORY_INTERNAL_ALIGNMENT_H
#include "c_memory/allocator.h"

static inline cm_status cm_normalize_alignment(size_t *alignment)
{
    if (*alignment == 0) {
        *alignment = _Alignof(max_align_t);
    }
    return (*alignment & (*alignment - 1)) == 0 ? CM_OK : CM_INVALID_ARGUMENT;
}

#endif
