#include <stdint.h>
#include <string.h>
#include "../test.h"
#include "../support.h"
#include "c_memory/c_memory.h"

typedef struct { void *pointer; size_t size; unsigned char pattern; } live_slot;

static uint32_t next_random(uint32_t *state)
{
    *state ^= *state << 13;
    *state ^= *state >> 17;
    *state ^= *state << 5;
    return *state;
}

static int exercise(cm_allocator allocator, size_t max_size, uint32_t seed)
{
    live_slot slots[128] = {{0}};
    for (size_t step = 0; step < 30000; ++step) {
        size_t index = next_random(&seed) % 128;
        live_slot *slot = &slots[index];
        if (slot->pointer != NULL) {
            for (size_t j = 0; j < slot->size; ++j)
                CHECK(((unsigned char *)slot->pointer)[j] == slot->pattern);
            CHECK(cm_deallocate(allocator, slot->pointer) == CM_OK);
            slot->pointer = NULL;
        } else {
            size_t size = next_random(&seed) % max_size + 1;
            size_t alignment = (size_t)1 << (next_random(&seed) % 7);
            cm_status status = cm_allocate(allocator, size, alignment, &slot->pointer);
            CHECK(status == CM_OK || status == CM_CAPACITY_EXCEEDED);
            if (status == CM_OK) {
                CHECK((uintptr_t)slot->pointer % alignment == 0);
                for (size_t j = 0; j < 128; ++j)
                    CHECK(j == index || slots[j].pointer != slot->pointer);
                slot->size = size;
                slot->pattern = (unsigned char)next_random(&seed);
                memset(slot->pointer, slot->pattern, size);
            } else {
                CHECK(slot->pointer == NULL);
            }
        }
    }
    for (size_t i = 0; i < 128; ++i) {
        if (slots[i].pointer == NULL) continue;
        for (size_t j = 0; j < slots[i].size; ++j)
            CHECK(((unsigned char *)slots[i].pointer)[j] == slots[i].pattern);
        CHECK(cm_deallocate(allocator, slots[i].pointer) == CM_OK);
    }
    return 0;
}

int main(void)
{
    failure_state state = {0};
    cm_allocator backing = failure_allocator(&state);
    cm_pool pool = {0};
    CHECK(cm_pool_init(&pool, backing, 256, 64, 65) == CM_OK);
    CHECK(exercise(cm_pool_allocator(&pool), 256, UINT32_C(0x12345678)) == 0);
    CHECK(pool.available == pool.capacity);
    cm_pool_destroy(&pool);

    cm_slab_class classes[] = {{16, 16, 17}, {64, 64, 23}, {256, 64, 31}};
    cm_slab slab = {0};
    CHECK(cm_slab_init(&slab, backing, classes, 3) == CM_OK);
    CHECK(exercise(cm_slab_allocator(&slab), 256, UINT32_C(0x87654321)) == 0);
    for (size_t i = 0; i < slab.class_count; ++i)
        CHECK(slab.pools[i].available == slab.pools[i].capacity);
    cm_slab_destroy(&slab);

    cm_tracker tracker;
    cm_tracker_entry records[65];
    CHECK(cm_tracker_init(&tracker, backing, records, 65) == CM_OK);
    CHECK(exercise(cm_tracker_allocator(&tracker), 256, UINT32_C(0xabcddcba)) == 0);
    CHECK(tracker.live_count == 0 && tracker.live_bytes == 0 && tracker.invalid_frees == 0);
    CHECK(state.live == 0);
    return 0;
}
