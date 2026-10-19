#include <string.h>
#include "c_memory/c_memory.h"
int main(void)
{
    return strcmp(c_memory_version(), "0.1.0") == 0 ? 0 : 1;
}
