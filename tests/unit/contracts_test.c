#include <stdint.h>
#include <string.h>
#include "../test.h"
#include "../support.h"
#include "c_memory/c_memory.h"

static cm_status empty_success(void *context, size_t size, size_t alignment, void **out)
{
    (void)context;
    (void)size;
    (void)alignment;
    *out = NULL;
    return CM_OK;
}

static void no_release(void *context, void *allocation)
{
    (void)context;
    (void)allocation;
}

static int allocation_contracts(void)
{
    cm_allocator invalid = {0};
    cm_allocator empty = {.allocate = empty_success, .deallocate = no_release};
    void *out = &invalid;
    CHECK(cm_allocate(empty, 8, 0, &out) == CM_OUT_OF_MEMORY && out == NULL);
    CHECK(cm_callocate(empty, 2, 8, 0, &out) == CM_OUT_OF_MEMORY && out == NULL);
    CHECK(cm_deallocate(invalid, &out) == CM_INVALID_ARGUMENT);
    CHECK(cm_deallocate(invalid, NULL) == CM_OK);
    CHECK(cm_allocate(empty, 8, SIZE_MAX, &out) == CM_INVALID_ARGUMENT);
    CHECK(cm_resize(empty, &out, 0, 0, 0) == CM_OK && out == NULL);
    CHECK(cm_callocate(empty, 2, 0, 0, &out) == CM_INVALID_ARGUMENT);

    cm_allocator system = cm_system_allocator();
    for (size_t alignment = 1; alignment <= 4096; alignment *= 2) {
        CHECK(cm_allocate(system, 37, alignment, &out) == CM_OK);
        CHECK((uintptr_t)out % alignment == 0);
        memset(out, 0x5a, 37);
        CHECK(cm_deallocate(system, out) == CM_OK);
    }
    return 0;
}

static int pool_lifecycle(void)
{
    failure_state state = {0};
    cm_allocator backing = failure_allocator(&state);
    cm_pool pool = {0};
    void *out = &state;
    CHECK(cm_pool_allocate(&pool, &out) == CM_INVALID_ARGUMENT && out == NULL);
    CHECK(!cm_pool_owns(&pool, &state));
    CHECK(cm_pool_deallocate(NULL, NULL) == CM_OK);
    cm_pool_reset(NULL);
    cm_pool_destroy(NULL);
    CHECK(cm_pool_init(&pool, backing, 17, 64, 4) == CM_OK);
    void *items[4];
    for (size_t i = 0; i < 4; ++i) {
        CHECK(cm_pool_allocate(&pool, &items[i]) == CM_OK);
    }
    CHECK(pool.available == 0);
    CHECK(!cm_pool_owns(&pool, pool.occupied));
    CHECK(cm_pool_deallocate(&pool, pool.occupied) == CM_INVALID_ARGUMENT);
    cm_pool_reset(&pool);
    CHECK(pool.available == pool.capacity);
    for (size_t i = 0; i < 4; ++i) {
        CHECK(cm_pool_deallocate(&pool, items[i]) == CM_INVALID_ARGUMENT);
    }
    for (size_t i = 0; i < 4; ++i) {
        CHECK(cm_pool_allocate(&pool, &out) == CM_OK && out == items[i]);
        memset(out, 0x33, 17);
    }
    /* Destruction releases the owned buffer even with outstanding payloads. */
    cm_pool_destroy(&pool);
    CHECK(state.live == 0 && pool.storage == NULL);

    pool.block_size = 71;
    state.fail_at = state.attempts + 1;
    CHECK(cm_pool_init(&pool, backing, 8, 0, 1) == CM_OUT_OF_MEMORY);
    CHECK(pool.block_size == 71 && pool.storage == NULL && state.live == 0);
    return 0;
}

static int slab_rollback(void)
{
    failure_state state = {0};
    cm_slab slab = {0};
    cm_slab_class overflow[] = {{8, 0, 1}, {SIZE_MAX, 64, 1}};
    CHECK(cm_slab_init(&slab, failure_allocator(&state), overflow, 2) == CM_SIZE_OVERFLOW);
    CHECK(state.live == 0 && slab.pools == NULL);
    cm_slab_class duplicate[] = {{8, 0, 1}, {8, 0, 1}};
    size_t attempts = state.attempts;
    CHECK(cm_slab_init(&slab, failure_allocator(&state), duplicate, 2) == CM_INVALID_ARGUMENT);
    CHECK(state.attempts == attempts);
    void *out = &state;
    CHECK(cm_slab_allocate(&slab, 8, 0, &out) == CM_INVALID_ARGUMENT && out == NULL);
    cm_slab_class classes[] = {{16, 16, 2}, {64, 64, 2}};
    CHECK(cm_slab_init(&slab, failure_allocator(&state), classes, 2) == CM_OK);
    CHECK(cm_slab_allocate(&slab, 33, 64, &out) == CM_OK);
    cm_slab_reset(&slab);
    CHECK(cm_slab_deallocate(&slab, out) == CM_INVALID_ARGUMENT);
    for (size_t i = 0; i < slab.class_count; ++i) {
        CHECK(slab.pools[i].available == slab.pools[i].capacity);
    }
    cm_slab_destroy(&slab);
    CHECK(state.live == 0);
    cm_slab_reset(NULL);
    cm_slab_destroy(NULL);
    CHECK(cm_slab_deallocate(NULL, NULL) == CM_OK);
    return 0;
}

static int tracker_capacity(void)
{
    failure_state state = {0};
    cm_tracker tracker;
    cm_tracker_entry records[2];
    CHECK(cm_tracker_init(&tracker, failure_allocator(&state), records, 2) == CM_OK);
    cm_allocator allocator = cm_tracker_allocator(&tracker);
    void *first = NULL;
    void *second = NULL;
    CHECK(cm_allocate(allocator, 8, 0, &first) == CM_OK);
    CHECK(cm_allocate(allocator, 16, 0, &second) == CM_OK);
    memset(first, 0xa5, 8);
    void *original = first;
    CHECK(cm_resize(allocator, &first, 8, 32, 0) == CM_CAPACITY_EXCEEDED);
    CHECK(first == original && ((unsigned char *)first)[7] == 0xa5);
    CHECK(tracker.live_count == 2 && tracker.live_bytes == 24 && state.live == 2);
    CHECK(cm_tracker_deallocate(&tracker, second) == CM_OK);
    CHECK(cm_resize(allocator, &first, 8, 32, 64) == CM_OK);
    CHECK((uintptr_t)first % 64 == 0 && ((unsigned char *)first)[7] == 0xa5);
    CHECK(tracker.live_count == 1 && tracker.live_bytes == 32 && tracker.peak_bytes == 40);
    CHECK(tracker.total_allocations == 3);
    CHECK(cm_tracker_release_all(&tracker) == 1 && state.live == 0);
    CHECK(cm_allocate(allocator, 5, 0, &first) == CM_OK);
    CHECK(cm_allocate(allocator, 7, 0, &second) == CM_OK);
    CHECK(cm_tracker_release_all(&tracker) == 2 && state.live == 0);
    CHECK(tracker.live_bytes == 0 && tracker.peak_bytes == 40);
    CHECK(cm_tracker_release_all(NULL) == 0);
    CHECK(cm_tracker_deallocate(NULL, NULL) == CM_OK);
    return 0;
}

int main(void)
{
    CHECK(allocation_contracts() == 0);
    CHECK(pool_lifecycle() == 0);
    CHECK(slab_rollback() == 0);
    CHECK(tracker_capacity() == 0);
    return 0;
}
