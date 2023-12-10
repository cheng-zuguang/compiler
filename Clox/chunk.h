//
// Created by 42134 on 2023/11/8.
//

#ifndef CLOX_CHUNK_H
#define CLOX_CHUNK_H

#include "common.h"
#include "value.h"

typedef enum {
    OP_CONSTANT,
    OP_NIL,
    OP_TRUE,
    OP_FALSE,
    OP_POP,
    OP_GET_LOCAL,
    OP_SET_LOCAL,
    OP_GET_GLOBAL,
    OP_SET_GLOBAL,
    OP_DEFINE_GLOBAL,
    OP_EQUAL,
    // OP_BANG_EQUAL -> OP_NOT(OP_EQUAL) -> !(a == b)
    OP_GREATER,
    // OP_GREATER_EQUAL -> OP_NOT(OP_LESS) -> !(a < b)
    OP_LESS,
    // OP_LESS_EQUAL -> OP_NOT(OP_GREATER) -> !(a > b)
    OP_ADD,
    OP_SUBTRACT,
    OP_MULTIPLY,
    OP_DIVIDE,
    OP_NOT,
    // unary
    OP_NEGATE,
    OP_PRINT,
    OP_JUMP,
    OP_JUMP_IF_FALSE,
    OP_LOOP,
    OP_CALL,
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
