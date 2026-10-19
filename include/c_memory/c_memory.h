#ifndef C_MEMORY_H
#define C_MEMORY_H

#include "c_memory/allocator.h"
#include "c_memory/arena.h"
#include "c_memory/pool.h"
#include "c_memory/slab.h"
#include "c_memory/tracker.h"

#ifdef __cplusplus
extern "C" {
#endif

const char *c_memory_version(void);

#ifdef __cplusplus
}
#endif
#endif
