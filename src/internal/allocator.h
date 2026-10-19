#ifndef CMEMORY_INTERNAL_ALLOCATOR_H
#define CMEMORY_INTERNAL_ALLOCATOR_H

#include <stdbool.h>
#include "c_memory/allocator.h"

static inline bool cm_allocator_is_valid(cm_allocator allocator)
{
    return allocator.allocate != NULL && allocator.deallocate != NULL;
}

#endif
