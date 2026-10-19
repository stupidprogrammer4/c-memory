#include <stdint.h>
#include "c_memory/arena.h"
#include "../internal/alignment.h"

cm_status cm_arena_init(cm_arena *arena, void *storage, size_t size)
{
    if (arena == NULL || (storage == NULL && size != 0)) return CM_INVALID_ARGUMENT;
    arena->storage = storage;
    arena->capacity = size;
    arena->used = 0;
    return CM_OK;
}

cm_status cm_arena_allocate(cm_arena *arena, size_t size, size_t alignment, void **out)
{
    if (out == NULL) return CM_INVALID_ARGUMENT;
    *out = NULL;
    if (arena == NULL || size == 0 || arena->used > arena->capacity ||
        cm_normalize_alignment(&alignment) != CM_OK) return CM_INVALID_ARGUMENT;
    size_t available = arena->capacity - arena->used;
    if (size > available) return CM_CAPACITY_EXCEEDED;
    uintptr_t address = (uintptr_t)(arena->storage + arena->used);
    size_t remainder = (size_t)(address % alignment);
    size_t padding = remainder == 0 ? 0 : alignment - remainder;
    if (padding > available || size > available - padding) return CM_CAPACITY_EXCEEDED;
    *out = arena->storage + arena->used + padding;
    arena->used += padding + size;
    return CM_OK;
}

void cm_arena_reset(cm_arena *arena)
{
    if (arena != NULL) arena->used = 0;
}

static cm_status arena_allocate(void *context, size_t size, size_t alignment, void **out)
{
    return cm_arena_allocate(context, size, alignment, out);
}

static void arena_deallocate(void *context, void *allocation)
{
    (void)context;
    (void)allocation;
}

cm_allocator cm_arena_allocator(cm_arena *arena)
{
    cm_allocator allocator = {arena, arena_allocate, arena_deallocate};
    return allocator;
}
