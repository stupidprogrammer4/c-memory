#include <stdio.h>
#include "c_memory/c_memory.h"

int main(void)
{
    cm_allocator system = cm_system_allocator();
    void *storage = NULL;
    if (cm_allocate(system, 1024, 0, &storage) != CM_OK) return 1;
    cm_arena arena;
    if (cm_arena_init(&arena, storage, 1024) != CM_OK) {
        (void)cm_deallocate(system, storage);
        return 1;
    }
    cm_allocator allocator = cm_arena_allocator(&arena);
    void *allocation = NULL;
    if (cm_callocate(allocator, 8, sizeof(int), 0, &allocation) != CM_OK) {
        (void)cm_deallocate(system, storage);
        return 1;
    }
    int *values = allocation;
    for (int i = 0; i < 8; ++i) values[i] = i * i;
    printf("Final value: %d; arena bytes used: %zu\n", values[7], arena.used);
    cm_arena_reset(&arena);
    (void)cm_deallocate(system, storage);
    return 0;
}
