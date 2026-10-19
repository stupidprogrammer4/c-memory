#ifndef CMEMORY_TEST_SUPPORT_H
#define CMEMORY_TEST_SUPPORT_H
#include "c_memory/allocator.h"

typedef struct {
    size_t attempts;
    size_t live;
    size_t fail_at;
} failure_state;

static inline cm_status failing_allocate(void *context, size_t size, size_t alignment, void **out)
{
    failure_state *state = context;
    if (++state->attempts == state->fail_at) {
        *out = NULL;
        return CM_OUT_OF_MEMORY;
    }
    cm_status status = cm_allocate(cm_system_allocator(), size, alignment, out);
    if (status == CM_OK) {
        state->live++;
    }
    return status;
}

static inline void counted_release(void *context, void *allocation)
{
    failure_state *state = context;
    state->live--;
    (void)cm_deallocate(cm_system_allocator(), allocation);
}

static inline cm_allocator failure_allocator(failure_state *state)
{
    cm_allocator allocator = {state, failing_allocate, counted_release};
    return allocator;
}
#endif
