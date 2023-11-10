//
// Created by 42134 on 2023/11/9.
//
#include <stdlib.h>
#include "chunk.h"
#include "memory.h"

void initChunk(Chunk* chunk) {
    chunk->count = 0;
    chunk->capacity = 0;
    chunk->code = NULL;

    chunk->lineCount = 0;
    chunk->lineCapacity = 0;
    chunk->lines = NULL;

    initValueArray(&chunk->constants);
}

void writeChunk(Chunk* chunk, uint8_t byte, int line) {
    // 判断当前还有没有容量
    if (chunk->capacity < chunk->count + 1) {
        int oldCapacity = chunk->capacity;
        chunk->capacity = GROW_CAPACITY(oldCapacity);
        chunk->code = GROW_ARRAY(uint8_t, chunk->code, oldCapacity, chunk->capacity);
//        chunk->lines = GROW_ARRAY(int, chunk->lines, oldCapacity, chunk->capacity);
    }

    // 添加新元素
    chunk->code[chunk->count] = byte;
//    chunk->lines[chunk->count] = line;
    chunk->count++;

    // See if we're still on the same line.
    if (chunk->lineCount > 0 && chunk->lines[chunk->lineCount-1].line == line) {
        return;
    }

    if (chunk->lineCapacity < chunk->lineCapacity + 1) {
        int oldCapacity = chunk->lineCapacity;
        chunk->lineCapacity = GROW_CAPACITY(oldCapacity);
        chunk->lines = GROW_ARRAY(LineStart, chunk->lines, oldCapacity, chunk->lineCapacity);
    }

    LineStart* lineStart = &chunk->lines[chunk->lineCount++];
    lineStart->offset = chunk->count - 1;
    lineStart->line = line;
}

// return new add constant ix
int addConstant(Chunk* chunk, Value value) {
    writeValueArray(&chunk->constants, value);
    return chunk->constants.count - 1;
}

/*
 * @param chunk
 * @param instructionOffset instruction的偏移量
 * */
int getLine(Chunk* chunk, int instructionOffset) {
    int start = 0;
    int end = chunk->lineCount - 1;

    for (;;) {
        // 防止溢出
        int mid = (end - start) / 2 + start;
        LineStart* line = &chunk->lines[mid];

        if (instructionOffset < line->offset) {
            end = mid - 1;

        // case 1: 遍历到最后一个了，直接返回
        // case 2: 判断目标行号是否小于下一个偏移位置，如果是，则当前为目标的offset
        } else if (mid == chunk->lineCount - 1 || instructionOffset < chunk->lines[mid + 1].offset) {
            return line->line;
        } else {
            start = mid + 1;
        }

    }
}


void freeChunk(Chunk* chunk) {
    FREE_ARRAY(uint8_t, chunk->code, chunk->capacity);

    FREE_ARRAY(LineStart, chunk->lines, chunk->lineCapacity);

    freeValueArray(&chunk->constants);

    initChunk(chunk);
}