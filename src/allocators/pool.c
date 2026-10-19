#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "c_memory/pool.h"
#include "../internal/alignment.h"

static size_t bitmap_bytes(size_t capacity)
{
    return capacity / CHAR_BIT + (capacity % CHAR_BIT != 0);
}

static unsigned char block_mask(size_t index)
{
    return (unsigned char)(1u << (index % CHAR_BIT));
}

cm_status cm_pool_init(cm_pool *pool, cm_allocator backing, size_t block_size, size_t alignment,
                       size_t capacity)
{
    if (pool == NULL || block_size == 0 || capacity == 0 ||
        cm_normalize_alignment(&alignment) != CM_OK) {
        return CM_INVALID_ARGUMENT;
    }
    size_t minimum_block_size = block_size < sizeof(size_t) ? sizeof(size_t) : block_size;
    size_t mask = alignment - 1;
    if (minimum_block_size > SIZE_MAX - mask) {
        return CM_SIZE_OVERFLOW;
    }
    size_t stride = (minimum_block_size + mask) & ~mask;
    if (capacity > SIZE_MAX / stride) {
        return CM_SIZE_OVERFLOW;
    }
    size_t block_bytes = capacity * stride;
    size_t bitmap_size = bitmap_bytes(capacity);
    if (bitmap_size > SIZE_MAX - block_bytes) {
        return CM_SIZE_OVERFLOW;
    }
    void *storage = NULL;
    cm_status status = cm_allocate(backing, block_bytes + bitmap_size, alignment, &storage);
    if (status != CM_OK) {
        return status;
    }
    cm_pool result = {
        .backing = backing,
        .storage = storage,
        .occupied = (unsigned char *)storage + block_bytes,
        .block_size = block_size,
        .stride = stride,
        .alignment = alignment,
        .capacity = capacity,
    };
    cm_pool_reset(&result);
    *pool = result;
    return CM_OK;
}

void cm_pool_reset(cm_pool *pool)
{
    if (pool == NULL || pool->storage == NULL) {
        return;
    }
    memset(pool->occupied, 0, bitmap_bytes(pool->capacity));
    for (size_t i = 0; i < pool->capacity; ++i) {
        size_t next = i + 1 == pool->capacity ? SIZE_MAX : i + 1;
        memcpy(pool->storage + i * pool->stride, &next, sizeof(next));
    }
    pool->available = pool->capacity;
    pool->head = 0;
}

cm_status cm_pool_allocate(cm_pool *pool, void **out)
{
    if (out == NULL) {
        return CM_INVALID_ARGUMENT;
    }
    *out = NULL;
    if (pool == NULL || pool->storage == NULL) {
        return CM_INVALID_ARGUMENT;
    }
    if (pool->available == 0) {
        return CM_CAPACITY_EXCEEDED;
    }
    size_t index = pool->head;
    unsigned char *block = pool->storage + index * pool->stride;
    memcpy(&pool->head, block, sizeof(pool->head));
    pool->occupied[index / CHAR_BIT] |= block_mask(index);
    pool->available--;
    *out = block;
    return CM_OK;
}

static bool pool_find_block(const cm_pool *pool, const void *allocation, size_t *index)
{
    if (pool == NULL || pool->storage == NULL || allocation == NULL) {
        return false;
    }
    uintptr_t base = (uintptr_t)pool->storage;
    uintptr_t address = (uintptr_t)allocation;
    if (address < base) {
        return false;
    }
    uintptr_t offset = address - base;
    if (offset >= pool->capacity * pool->stride || offset % pool->stride != 0) {
        return false;
    }
    if (index != NULL) {
        /* The validated byte range fits size_t, so the block index does too. */
        *index = (size_t)(offset / pool->stride);
    }
    return true;
}

int cm_pool_owns(const cm_pool *pool, const void *allocation)
{
    return pool_find_block(pool, allocation, NULL);
}

cm_status cm_pool_deallocate(cm_pool *pool, void *allocation)
{
    if (allocation == NULL) {
        return CM_OK;
    }
    size_t index;
    if (!pool_find_block(pool, allocation, &index)) {
        return CM_INVALID_ARGUMENT;
    }
    unsigned char mask = block_mask(index);
    if ((pool->occupied[index / CHAR_BIT] & mask) == 0) {
        return CM_INVALID_ARGUMENT;
    }
    pool->occupied[index / CHAR_BIT] &= (unsigned char)~mask;
    memcpy(allocation, &pool->head, sizeof(pool->head));
    pool->head = index;
    pool->available++;
    return CM_OK;
}

void cm_pool_destroy(cm_pool *pool)
{
    if (pool == NULL) {
        return;
    }
    (void)cm_deallocate(pool->backing, pool->storage);
    *pool = (cm_pool){0};
}

static cm_status pool_allocate(void *context, size_t size, size_t alignment, void **out)
{
    *out = NULL;
    cm_pool *pool = context;
    if (pool == NULL) {
        return CM_INVALID_ARGUMENT;
    }
    if (size > pool->block_size || alignment > pool->alignment) {
        return CM_CAPACITY_EXCEEDED;
    }
    return cm_pool_allocate(pool, out);
}

static void pool_deallocate(void *context, void *allocation)
{
    (void)cm_pool_deallocate(context, allocation);
}

cm_allocator cm_pool_allocator(cm_pool *pool)
{
    cm_allocator allocator = {
        .context = pool,
        .allocate = pool_allocate,
        .deallocate = pool_deallocate,
    };
    return allocator;
}
