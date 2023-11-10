//
// Created by 42134 on 2023/11/8.
//

#ifndef CLOX_CHUNK_H
#define CLOX_CHUNK_H

#include "common.h"
#include "value.h"

typedef enum {
    OP_CONSTANT,
    OP_RETURN
} OpCode;

typedef struct {
    // 当前已使用的空间
    int count;
    // 数组的容量
    int capacity;
    // opcode
    uint8_t* code;
    // 行信息
    int* lines;
    // 值
    ValueArray constants;
} Chunk;

void initChunk(Chunk* chunk);
void writeChunk(Chunk* chunk, uint8_t byte, int line);
void freeChunk(Chunk* chunk);
int addConstant(Chunk* chunk, Value value);

#endif //CLOX_CHUNK_H
