#include <string.h>
#include "c_memory/c_memory.h"

int main(void)
{
    void *memory = NULL;
    cm_allocator allocator = cm_system_allocator();
    if (cm_callocate(allocator, 4, 8, 0, &memory) != CM_OK) {
        return 1;
    }
    memset(memory, 0x31, 32);
    return cm_deallocate(allocator, memory) == CM_OK ? 0 : 1;
}
