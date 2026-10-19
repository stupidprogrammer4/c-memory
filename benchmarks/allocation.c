#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "c_memory/c_memory.h"

enum { BATCH_SIZE = 64, ITERATIONS = 100000, ALLOCATION_SIZE = 32 };
static volatile unsigned long checksum;

static int measure(const char *name, const cm_allocator *allocator)
{
    void *allocations[BATCH_SIZE];
    clock_t start = clock();
    if (start == (clock_t)-1) {
        return 1;
    }
    for (int batch = 0; batch < ITERATIONS; ++batch) {
        size_t allocated = 0;
        for (; allocated < BATCH_SIZE; ++allocated) {
            cm_status status = CM_OK;
            if (allocator == NULL) {
                allocations[allocated] = malloc(ALLOCATION_SIZE);
                if (allocations[allocated] == NULL) {
                    status = CM_OUT_OF_MEMORY;
                }
            } else {
                status = cm_allocate(*allocator, ALLOCATION_SIZE, 0, &allocations[allocated]);
            }
            if (status != CM_OK) {
                break;
            }
            ((unsigned char *)allocations[allocated])[0] = (unsigned char)allocated;
        }
        unsigned long sum = 0;
        for (size_t i = 0; i < allocated; ++i) {
            sum += ((unsigned char *)allocations[i])[0];
            if (allocator == NULL) {
                free(allocations[i]);
            } else {
                (void)cm_deallocate(*allocator, allocations[i]);
            }
        }
        checksum += sum;
        if (allocated != BATCH_SIZE) {
            return 1;
        }
    }
    clock_t end = clock();
    if (end == (clock_t)-1) {
        return 1;
    }
    double seconds = (double)(end - start) / CLOCKS_PER_SEC;
    printf("%s,%d,%d,%.6f,%.2f\n", name, BATCH_SIZE, ITERATIONS, seconds,
           seconds * 1e9 / ((double)BATCH_SIZE * ITERATIONS));
    return 0;
}

/* Arena frees do not reclaim memory: reset after the last free in each batch. */
typedef struct {
    cm_arena *arena;
    size_t outstanding;
} arena_batch;
static cm_status batch_allocate(void *context, size_t size, size_t alignment, void **out)
{
    arena_batch *batch = context;
    cm_status status = cm_arena_allocate(batch->arena, size, alignment, out);
    if (status == CM_OK) {
        batch->outstanding++;
    }
    return status;
}
static void batch_deallocate(void *context, void *allocation)
{
    arena_batch *batch = context;
    (void)allocation;
    if (--batch->outstanding == 0) {
        cm_arena_reset(batch->arena);
    }
}

int main(void)
{
    cm_allocator system = cm_system_allocator();
    void *storage = NULL;
    cm_pool pool = {0};
    cm_slab slab = {0};
    int result = 1;
    if (cm_allocate(system, BATCH_SIZE * ALLOCATION_SIZE, 0, &storage) != CM_OK) {
        goto cleanup;
    }
    cm_arena arena;
    if (cm_arena_init(&arena, storage, BATCH_SIZE * ALLOCATION_SIZE) != CM_OK) {
        goto cleanup;
    }
    arena_batch batch = {&arena, 0};
    cm_allocator arena_allocator = {&batch, batch_allocate, batch_deallocate};
    if (cm_pool_init(&pool, system, ALLOCATION_SIZE, 0, BATCH_SIZE) != CM_OK) {
        goto cleanup;
    }
    cm_slab_class classes[] = {{32, 0, BATCH_SIZE}, {128, 0, BATCH_SIZE}, {512, 0, BATCH_SIZE}};
    if (cm_slab_init(&slab, system, classes, 3) != CM_OK) {
        goto cleanup;
    }
    cm_tracker tracker;
    cm_tracker_entry records[BATCH_SIZE];
    if (cm_tracker_init(&tracker, system, records, BATCH_SIZE) != CM_OK) {
        goto cleanup;
    }
    cm_allocator pool_allocator = cm_pool_allocator(&pool);
    cm_allocator slab_allocator = cm_slab_allocator(&slab);
    cm_allocator tracked = cm_tracker_allocator(&tracker);
    puts("allocator,batch_size,iterations,cpu_seconds,ns_per_allocate_free_pair");
    result = measure("malloc_free", NULL) || measure("system_adapter", &system) ||
             measure("arena_batch_reset", &arena_allocator) || measure("pool", &pool_allocator) ||
             measure("slab_three_classes", &slab_allocator) ||
             measure("tracker_64_records", &tracked);
    if (cm_tracker_release_all(&tracker) != 0) {
        result = 1;
    }
cleanup:
    cm_slab_destroy(&slab);
    cm_pool_destroy(&pool);
    (void)cm_deallocate(system, storage);
    return result;
}
