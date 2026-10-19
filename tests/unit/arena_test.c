#include <stdint.h>
#include <string.h>
#include "../test.h"
#include "c_memory/arena.h"

int main(void)
{
    unsigned char buffer[256];
    cm_arena arena;
    CHECK(cm_arena_init(&arena, buffer + 1, sizeof(buffer) - 1) == CM_OK);
    CHECK(arena.used == 0);
    void *first = NULL;
    CHECK(cm_arena_allocate(&arena, 7, 64, &first) == CM_OK);
    CHECK((uintptr_t)first % 64 == 0);
    memset(first, 0xa5, 7);
    void *second = NULL;
    CHECK(cm_arena_allocate(&arena, 11, 1, &second) == CM_OK);
    CHECK((unsigned char *)second >= (unsigned char *)first + 7);
    CHECK(((unsigned char *)first)[6] == 0xa5);

    size_t used = arena.used;
    CHECK(cm_arena_allocate(&arena, SIZE_MAX, 1, &second) == CM_CAPACITY_EXCEEDED);
    CHECK(second == NULL && arena.used == used);
    CHECK(cm_arena_allocate(&arena, 1, 3, &second) == CM_INVALID_ARGUMENT);
    CHECK(cm_arena_allocate(&arena, 0, 1, &second) == CM_INVALID_ARGUMENT);
    CHECK(arena.used == used);

    cm_allocator adapter = cm_arena_allocator(&arena);
    CHECK(cm_allocate(adapter, 8, 0, &second) == CM_OK);
    CHECK((uintptr_t)second % _Alignof(max_align_t) == 0);
    used = arena.used;
    CHECK(cm_deallocate(adapter, second) == CM_OK);
    CHECK(arena.used == used);
    cm_arena_reset(&arena);
    CHECK(arena.used == 0);
    CHECK(cm_arena_allocate(&arena, 7, 64, &second) == CM_OK && second == first);
    cm_arena_reset(&arena);
    cm_arena_reset(&arena);
    CHECK(arena.used == 0);

    cm_arena empty;
    CHECK(cm_arena_init(&empty, NULL, 0) == CM_OK);
    CHECK(cm_arena_allocate(&empty, 1, 1, &second) == CM_CAPACITY_EXCEEDED);
    CHECK(second == NULL);
    CHECK(cm_arena_init(&empty, NULL, 4) == CM_INVALID_ARGUMENT);
    CHECK(cm_arena_init(NULL, buffer, 1) == CM_INVALID_ARGUMENT);
    CHECK(cm_arena_allocate(NULL, 1, 1, &second) == CM_INVALID_ARGUMENT);
    CHECK(cm_arena_allocate(&arena, 1, 1, NULL) == CM_INVALID_ARGUMENT);
    cm_arena_reset(NULL);

    _Alignas(64) unsigned char exact[64];
    CHECK(cm_arena_init(&arena, exact + 1, 63) == CM_OK);
    CHECK(cm_arena_allocate(&arena, 1, 64, &second) == CM_CAPACITY_EXCEEDED);
    CHECK(arena.used == 0 && second == NULL);
    CHECK(cm_arena_allocate(&arena, 63, 1, &second) == CM_OK);
    CHECK(arena.used == 63);
    CHECK(cm_arena_allocate(&arena, 1, 1, &second) == CM_CAPACITY_EXCEEDED);
    CHECK(cm_arena_init(&arena, NULL, 1) == CM_INVALID_ARGUMENT);
    CHECK(arena.storage == exact + 1 && arena.used == 63);
    return 0;
}
