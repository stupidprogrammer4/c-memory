#include <stdint.h>
#include <string.h>
#include "../test.h"
#include "c_memory/allocator.h"

typedef struct {
    size_t allocations;
    size_t releases;
    size_t last_size;
    size_t last_alignment;
    int fail;
} observations;

static cm_status observed_allocate(void *context, size_t size, size_t alignment, void **out)
{
    observations *state = context;
    state->allocations++;
    state->last_size = size;
    state->last_alignment = alignment;
    if (state->fail) {
        *out = NULL;
        return CM_OUT_OF_MEMORY;
    }
    return cm_allocate(cm_system_allocator(), size, alignment, out);
}

static void observed_release(void *context, void *allocation)
{
    observations *state = context;
    state->releases++;
    (void)cm_deallocate(cm_system_allocator(), allocation);
}

int main(void)
{
    observations state = {0};
    cm_allocator allocator = {&state, observed_allocate, observed_release};
    void *allocation = NULL;
    CHECK(cm_allocate(allocator, 13, 64, &allocation) == CM_OK);
    CHECK(allocation != NULL && (uintptr_t)allocation % 64 == 0);
    CHECK(state.last_size == 13 && state.last_alignment == 64);
    memset(allocation, 0x5a, 13);

    void *original = allocation;
    state.fail = 1;
    CHECK(cm_resize(allocator, &allocation, 13, 37, 64) == CM_OUT_OF_MEMORY);
    CHECK(allocation == original && ((unsigned char *)allocation)[12] == 0x5a);
    CHECK(state.releases == 0);

    state.fail = 0;
    CHECK(cm_resize(allocator, &allocation, 13, 37, 64) == CM_OK);
    CHECK((uintptr_t)allocation % 64 == 0);
    for (size_t i = 0; i < 13; ++i) CHECK(((unsigned char *)allocation)[i] == 0x5a);
    CHECK(state.releases == 1);
    CHECK(cm_resize(allocator, &allocation, 37, 4, 64) == CM_OK);
    for (size_t i = 0; i < 4; ++i) CHECK(((unsigned char *)allocation)[i] == 0x5a);
    CHECK(cm_resize(allocator, &allocation, 4, 0, 0) == CM_OK);
    CHECK(allocation == NULL && state.releases == 3);

    CHECK(cm_callocate(allocator, 7, sizeof(unsigned), 0, &allocation) == CM_OK);
    CHECK(state.last_size == 7 * sizeof(unsigned));
    CHECK(state.last_alignment == _Alignof(max_align_t));
    for (size_t i = 0; i < 7 * sizeof(unsigned); ++i) CHECK(((unsigned char *)allocation)[i] == 0);
    CHECK(cm_deallocate(allocator, allocation) == CM_OK);

    size_t calls = state.allocations;
    allocation = &state;
    CHECK(cm_callocate(allocator, SIZE_MAX, 2, 0, &allocation) == CM_SIZE_OVERFLOW);
    CHECK(allocation == NULL && state.allocations == calls);
    CHECK(cm_allocate(allocator, 1, 3, &allocation) == CM_INVALID_ARGUMENT);
    CHECK(cm_allocate(allocator, 0, 0, &allocation) == CM_INVALID_ARGUMENT);
    CHECK(cm_callocate(allocator, 0, 1, 0, &allocation) == CM_INVALID_ARGUMENT);
    CHECK(cm_allocate(allocator, 1, 0, NULL) == CM_INVALID_ARGUMENT);
    CHECK(state.allocations == calls);
    CHECK(cm_deallocate(allocator, NULL) == CM_OK);
    CHECK(cm_allocate(cm_system_allocator(), SIZE_MAX, 64, &allocation) == CM_SIZE_OVERFLOW);
    CHECK(allocation == NULL);

    cm_allocator system = cm_system_allocator();
    allocation = &state;
    CHECK(system.allocate(system.context, SIZE_MAX, 64, &allocation) == CM_SIZE_OVERFLOW);
    CHECK(allocation == NULL);

    cm_allocator invalid = {0};
    CHECK(cm_allocate(invalid, 8, 0, &allocation) == CM_INVALID_ARGUMENT);
    CHECK(allocation == NULL);
    CHECK(cm_resize(allocator, NULL, 0, 1, 0) == CM_INVALID_ARGUMENT);
    CHECK(cm_resize(allocator, &allocation, 1, 8, 0) == CM_INVALID_ARGUMENT);
    CHECK(cm_resize(allocator, &allocation, 0, 8, 0) == CM_OK);
    original = allocation;
    CHECK(cm_resize(allocator, &allocation, 0, 16, 0) == CM_INVALID_ARGUMENT);
    CHECK(allocation == original);
    CHECK(cm_resize(allocator, &allocation, 8, 16, 3) == CM_INVALID_ARGUMENT);
    CHECK(allocation == original);
    CHECK(cm_deallocate(allocator, allocation) == CM_OK);
    return 0;
}
