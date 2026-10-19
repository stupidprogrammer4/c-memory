#include <string.h>
#include <stdint.h>
#include "../test.h"
#include "../support.h"
#include "c_memory/tracker.h"

int main(void)
{
    failure_state state = {0};
    cm_tracker tracker;
    cm_tracker_entry entries[2];
    CHECK(cm_tracker_init(&tracker, failure_allocator(&state), entries, 2) == CM_OK);
    cm_allocator adapter = cm_tracker_allocator(&tracker);
    void *first = NULL;
    void *second = NULL;
    void *extra = NULL;
    CHECK(cm_allocate(adapter, 13, 64, &first) == CM_OK);
    CHECK(cm_allocate(adapter, 29, 0, &second) == CM_OK);
    CHECK(tracker.live_count == 2 && tracker.live_bytes == 42 && tracker.peak_bytes == 42);
    CHECK(cm_allocate(adapter, 1, 0, &extra) == CM_CAPACITY_EXCEEDED);
    CHECK(state.live == 2 && extra == NULL);
    extra = &state;
    CHECK(adapter.allocate(adapter.context, 1, 1, &extra) == CM_CAPACITY_EXCEEDED);
    CHECK(extra == NULL);
    CHECK(cm_tracker_deallocate(&tracker, &state) == CM_INVALID_ARGUMENT);
    CHECK(tracker.invalid_frees == 1 && state.live == 2);
    CHECK(cm_deallocate(adapter, first) == CM_OK);
    CHECK(tracker.live_count == 1 && tracker.live_bytes == 29);
    CHECK(cm_tracker_deallocate(&tracker, first) == CM_INVALID_ARGUMENT);
    CHECK(tracker.invalid_frees == 2 && state.live == 1);
    memset(second, 0x31, 29);
    state.fail_at = state.attempts + 1;
    CHECK(cm_resize(adapter, &second, 29, 37, 0) == CM_OUT_OF_MEMORY);
    CHECK(tracker.live_count == 1 && tracker.live_bytes == 29);
    CHECK(((unsigned char *)second)[28] == 0x31);
    CHECK(cm_tracker_release_all(&tracker) == 1);
    CHECK(tracker.live_count == 0 && tracker.live_bytes == 0 && state.live == 0);
    CHECK(cm_tracker_release_all(&tracker) == 0);
    CHECK(cm_tracker_init(NULL, failure_allocator(&state), entries, 2) == CM_INVALID_ARGUMENT);
    CHECK(cm_tracker_init(&tracker, failure_allocator(&state), entries, 0) == CM_INVALID_ARGUMENT);
    CHECK(cm_tracker_init(&tracker, failure_allocator(&state), entries, SIZE_MAX) == CM_SIZE_OVERFLOW);
    return 0;
}
