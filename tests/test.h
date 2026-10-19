#ifndef CMEMORY_TEST_H
#define CMEMORY_TEST_H

#include <stdio.h>

/* Unlike assert(), CHECK remains active in Release builds. */
#define CHECK(expression)                                                                          \
    do {                                                                                           \
        if (!(expression)) {                                                                       \
            fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #expression);                       \
            return 1;                                                                              \
        }                                                                                          \
    } while (0)

#endif
