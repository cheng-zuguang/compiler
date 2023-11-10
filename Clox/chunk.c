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
    chunk->lines = NULL;
    initValueArray(&chunk->constants);
}

void writeChunk(Chunk* chunk, uint8_t byte, int line) {
    // 判断当前还有没有容量
    if (chunk->capacity < chunk->count + 1) {
        int oldCapacity = chunk->capacity;
        chunk->capacity = GROW_CAPACITY(oldCapacity);
        chunk->code = GROW_ARRAY(uint8_t, chunk->code, oldCapacity, chunk->capacity);
        chunk->lines = GROW_ARRAY(int, chunk->lines, oldCapacity, chunk->capacity);
    }

    // 添加新元素
    chunk->code[chunk->count] = byte;
    chunk->lines[chunk->count] = line;
    chunk->count++;
}

// return new add constant ix
int addConstant(Chunk* chunk, Value value) {
    writeValueArray(&chunk->constants, value);
    return chunk->constants.count - 1;
}

void writeConstant(Chunk* chunk, Value value, int line) {
    int ix = addConstant(chunk, value);
    if (ix < 256) {
        writeChunk(chunk, OP_CONSTANT, line);
        writeChunk(chunk, (uint8_t) ix, line);
    } else {
        writeChunk(chunk, OP_CONSTANT_LONG, line);
        // 截取超过255部分
        writeChunk(chunk, (uint8_t) (ix & 0xff), line);
        // 除2^8次方再截取超过255部分
        writeChunk(chunk, (uint8_t) ((ix >> 8) & 0xff), line);
        // 除2^16次方再截取超过255部分
        writeChunk(chunk, (uint8_t) ((ix >> 16) & 0xff), line);
    }
}



void freeChunk(Chunk* chunk) {
    FREE_ARRAY(uint8_t, chunk->code, chunk->capacity);
    FREE_ARRAY(int, chunk->lines, chunk->capacity);
    freeValueArray(&chunk->constants);

    initChunk(chunk);
}