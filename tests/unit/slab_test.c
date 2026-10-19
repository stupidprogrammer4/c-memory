#include <stdint.h>
#include <string.h>
#include "../test.h"
#include "../support.h"
#include "c_memory/slab.h"

int main(void)
{
    cm_slab_class classes[] = {{16, 16, 2}, {64, 64, 2}, {256, 64, 1}};
    failure_state state = {0};
    cm_slab slab = {0};
    CHECK(cm_slab_init(&slab, failure_allocator(&state), classes, 3) == CM_OK);
    CHECK(state.live == 4);
    void *items[5];
    for (size_t i = 0; i < 5; ++i) {
        CHECK(cm_slab_allocate(&slab, 8, 8, &items[i]) == CM_OK);
        memset(items[i], 0x5a, 8);
    }
    void *out = NULL;
    CHECK(cm_slab_allocate(&slab, 8, 8, &out) == CM_CAPACITY_EXCEEDED);
    CHECK(cm_slab_deallocate(&slab, &state) == CM_INVALID_ARGUMENT);
    for (size_t i = 0; i < 5; ++i) CHECK(cm_slab_deallocate(&slab, items[i]) == CM_OK);
    CHECK(cm_slab_allocate(&slab, 33, 64, &out) == CM_OK);
    CHECK((uintptr_t)out % 64 == 0);
    CHECK(cm_slab_deallocate(&slab, out) == CM_OK);
    CHECK(cm_slab_deallocate(&slab, out) == CM_INVALID_ARGUMENT);
    CHECK(cm_slab_allocate(&slab, 257, 1, &out) == CM_CAPACITY_EXCEEDED);
    CHECK(cm_slab_allocate(&slab, 1, 128, &out) == CM_CAPACITY_EXCEEDED);
    cm_allocator adapter = cm_slab_allocator(&slab);
    CHECK(cm_allocate(adapter, 17, 0, &out) == CM_OK);
    CHECK(cm_deallocate(adapter, out) == CM_OK);
    cm_slab_reset(&slab);
    cm_slab_destroy(&slab);
    cm_slab_destroy(&slab);
    CHECK(state.live == 0);

    for (size_t fail = 1; fail <= 4; ++fail) {
        state = (failure_state){0, 0, fail};
        CHECK(cm_slab_init(&slab, failure_allocator(&state), classes, 3) == CM_OUT_OF_MEMORY);
        CHECK(state.live == 0 && slab.pools == NULL);
    }
    cm_slab_class invalid[] = {{32, 0, 1}, {16, 0, 1}};
    CHECK(cm_slab_init(&slab, failure_allocator(&state), invalid, 2) == CM_INVALID_ARGUMENT);
    CHECK(cm_slab_init(&slab, failure_allocator(&state), classes, SIZE_MAX) == CM_SIZE_OVERFLOW);
    CHECK(cm_slab_init(&slab, failure_allocator(&state), NULL, 1) == CM_INVALID_ARGUMENT);
    return 0;
}
