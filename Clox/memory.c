//
// Created by 42134 on 2023/11/9.
//
#include <stdlib.h>
#include "memory.h"

// 通过传的size来确定要做的操作
/*
 *  oldSize     newSize                 Op
 *  0           None-zero               Allocate new Block
 *  None-zero   0                       free
 *  None-zero   smaller than oldSize    shrinking existing allocation
 *  None-zero   greater than oldSize    growing existing allocation
 */

void* reallocate(void* pointer, size_t oldSize, size_t newSize) {
    if (newSize == 0) {
        free(pointer);
        return NULL;
    }

    void* result = realloc(pointer, newSize);

    if (result == NULL) exit(1);

    return result;
}