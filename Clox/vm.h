//
// Created by 42134 on 2023/11/13.
//

#ifndef CLOX_VM_H
#define CLOX_VM_H

#include "chunk.h"
#include "value.h"
#include "table.h"

#define STACK_MAX 256

typedef struct {
    Chunk* chunk;
    // instruction pointer: the location of instruction is currently being executed.
    // like Program counter, ip++ pointing next instruction.
    uint8_t* ip;
    /*
     * Compared with Jlox, it will not reuse variable when traverse the AST node. This creates additional overhead.
     */
    Value stack[STACK_MAX];
    // As with the top of ip,using pointer since it is faster to dereference the pointer than calculate the offset from the ix.
    // point past the element containing the top value on the stack.
    Value* stackTop;

    // all intern strings(symbol).
    Table strings;

    // obj reference.
    Obj* objects;
} VM;

// For exiting the process by this(static error, runtime error).
typedef enum {
    INTERPRET_OK,
    INTERPRET_COMPILE_ERROR,
    INTERPRET_RETURNING_ERROR
} InterpretResult;

// for free the obj memory.
extern VM vm;

void initVM();
void freeVM();
InterpretResult interpret(const char* source);
void push(Value value);
Value pop();


#endif //CLOX_VM_H
