//
// Created by 42134 on 2023/11/8.
//

#ifndef CLOX_COMMON_H
#define CLOX_COMMON_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#define NAN_BOXING
// diagnostic logging flag
#define DEBUG_TRACE_EXECUTION
#define DEBUG_PRINT_CODE

// diagnostic the gc
#define DEBUG_STRESS_GC
#define DEBUG_LOG_GC

#define UINT8_COUNT (UINT8_MAX + 1)

#endif //CLOX_COMMON_H

#undef DEBUG_STRESS_GC
#undef DEBUG_LOG_GC
#undef DEBUG_TRACE_EXECUTION
#undef DEBUG_PRINT_CODE
#undef NAN_BOXING
