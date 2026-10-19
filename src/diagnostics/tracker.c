#include <stdint.h>
#include "c_memory/tracker.h"

cm_status cm_tracker_init(cm_tracker *tracker, cm_allocator backing,
                          cm_tracker_entry *entries, size_t capacity)
{
    if (tracker == NULL || entries == NULL || capacity == 0 ||
        backing.allocate == NULL || backing.deallocate == NULL) return CM_INVALID_ARGUMENT;
    if (capacity > SIZE_MAX / sizeof(cm_tracker_entry)) return CM_SIZE_OVERFLOW;
    cm_tracker result = {backing, entries, capacity, 0, 0, 0, 0, 0};
    *tracker = result;
    return CM_OK;
}

static cm_status tracker_allocate(void *context, size_t size, size_t alignment, void **out)
{
    *out = NULL;
    cm_tracker *tracker = context;
    if (tracker == NULL) return CM_INVALID_ARGUMENT;
    if (tracker->live_count == tracker->capacity) return CM_CAPACITY_EXCEEDED;
    if (size > SIZE_MAX - tracker->live_bytes) return CM_SIZE_OVERFLOW;
    cm_status status = cm_allocate(tracker->backing, size, alignment, out);
    if (status != CM_OK) return status;
    tracker->entries[tracker->live_count++] = (cm_tracker_entry){*out, size, alignment};
    tracker->live_bytes += size;
    if (tracker->live_bytes > tracker->peak_bytes) tracker->peak_bytes = tracker->live_bytes;
    if (tracker->total_allocations != SIZE_MAX) tracker->total_allocations++;
    return CM_OK;
}

cm_status cm_tracker_deallocate(cm_tracker *tracker, void *allocation)
{
    if (allocation == NULL) return CM_OK;
    if (tracker == NULL) return CM_INVALID_ARGUMENT;
    for (size_t i = 0; i < tracker->live_count; ++i) {
        if (tracker->entries[i].allocation == allocation) {
            tracker->live_bytes -= tracker->entries[i].size;
            tracker->backing.deallocate(tracker->backing.context, allocation);
            tracker->entries[i] = tracker->entries[--tracker->live_count];
            tracker->entries[tracker->live_count] = (cm_tracker_entry){0};
            return CM_OK;
        }
    }
    if (tracker->invalid_frees != SIZE_MAX) tracker->invalid_frees++;
    return CM_INVALID_ARGUMENT;
}

static void tracker_deallocate(void *context, void *allocation)
{
    (void)cm_tracker_deallocate(context, allocation);
}

cm_allocator cm_tracker_allocator(cm_tracker *tracker)
{
    cm_allocator allocator = {tracker, tracker_allocate, tracker_deallocate};
    return allocator;
}

size_t cm_tracker_release_all(cm_tracker *tracker)
{
    if (tracker == NULL) return 0;
    size_t released = tracker->live_count;
    for (size_t i = 0; i < released; ++i) {
        tracker->backing.deallocate(tracker->backing.context, tracker->entries[i].allocation);
        tracker->entries[i] = (cm_tracker_entry){0};
    }
    tracker->live_count = 0;
    tracker->live_bytes = 0;
    return released;
}
