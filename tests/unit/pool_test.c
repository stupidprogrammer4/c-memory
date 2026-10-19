#include <stdint.h>
#include <string.h>
#include "../test.h"
#include "../support.h"
#include "c_memory/pool.h"

int main(void)
{
    failure_state state = {0};
    cm_pool pool = {0};
    CHECK(cm_pool_init(&pool, failure_allocator(&state), 13, 64, 17) == CM_OK);
    CHECK(state.live == 1 && pool.available == 17);
    void *items[17];
    for (size_t i = 0; i < 17; ++i) {
        CHECK(cm_pool_allocate(&pool, &items[i]) == CM_OK);
        CHECK((uintptr_t)items[i] % 64 == 0);
        memset(items[i], (int)i, 13);
    }
    void *out = NULL;
    CHECK(cm_pool_allocate(&pool, &out) == CM_CAPACITY_EXCEEDED && out == NULL);
    CHECK(cm_pool_deallocate(&pool, (unsigned char *)items[0] + 1) == CM_INVALID_ARGUMENT);
    CHECK(cm_pool_deallocate(&pool, &state) == CM_INVALID_ARGUMENT);
    for (size_t i = 0; i < 17; ++i) {
        for (size_t j = 0; j < 13; ++j) CHECK(((unsigned char *)items[i])[j] == i);
        CHECK(cm_pool_deallocate(&pool, items[i]) == CM_OK);
        CHECK(cm_pool_deallocate(&pool, items[i]) == CM_INVALID_ARGUMENT);
    }
    CHECK(pool.available == 17);
    cm_allocator adapter = cm_pool_allocator(&pool);
    out = &state;
    CHECK(adapter.allocate(adapter.context, 14, 1, &out) == CM_CAPACITY_EXCEEDED);
    CHECK(out == NULL);
    CHECK(cm_allocate(adapter, 14, 1, &out) == CM_CAPACITY_EXCEEDED);
    CHECK(cm_allocate(adapter, 1, 128, &out) == CM_CAPACITY_EXCEEDED);
    CHECK(cm_allocate(adapter, 13, 64, &out) == CM_OK);
    CHECK(cm_deallocate(adapter, out) == CM_OK);
    for (size_t i = 0; i < 10000; ++i) {
        CHECK(cm_pool_allocate(&pool, &out) == CM_OK);
        CHECK(cm_pool_deallocate(&pool, out) == CM_OK);
    }
    cm_pool_reset(&pool);
    CHECK(pool.available == 17);
    cm_pool_destroy(&pool);
    cm_pool_destroy(&pool);
    CHECK(state.live == 0);

    state.fail_at = state.attempts + 1;
    CHECK(cm_pool_init(&pool, failure_allocator(&state), 8, 0, 2) == CM_OUT_OF_MEMORY);
    CHECK(state.live == 0 && pool.storage == NULL);
    CHECK(cm_pool_init(&pool, failure_allocator(&state), SIZE_MAX, 64, 2) == CM_SIZE_OVERFLOW);
    CHECK(cm_pool_init(&pool, failure_allocator(&state), 8, 0, SIZE_MAX) == CM_SIZE_OVERFLOW);
    CHECK(cm_pool_init(&pool, failure_allocator(&state), sizeof(size_t), 1,
                        SIZE_MAX / sizeof(size_t)) == CM_SIZE_OVERFLOW);
    CHECK(cm_pool_init(&pool, failure_allocator(&state), 0, 0, 2) == CM_INVALID_ARGUMENT);
    CHECK(cm_pool_init(&pool, failure_allocator(&state), 8, 3, 2) == CM_INVALID_ARGUMENT);
    CHECK(cm_pool_init(&pool, failure_allocator(&state), 8, 0, 0) == CM_INVALID_ARGUMENT);
    CHECK(cm_pool_init(&pool, failure_allocator(&state), 1, 1, 1) == CM_OK);
    CHECK(cm_pool_allocate(&pool, &out) == CM_OK);
    CHECK(cm_pool_deallocate(&pool, out) == CM_OK);
    cm_pool_destroy(&pool);
    CHECK(state.live == 0);
    return 0;
}
