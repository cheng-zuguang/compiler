//
// Created by 42134 on 2023/11/13.
//

#ifndef CLOX_VM_H
#define CLOX_VM_H

#include "chunk.h"
#include "value.h"
#include "table.h"
#include "object.h"

#define FRAMES_MAX 64
#define STACK_MAX (FRAMES_MAX * UINT8_COUNT)

typedef struct {
//    ObjFunction* function;
//    ObjClosure* closure;
// --> challenge 25.1
    Obj* function;
// --> challenge 25.1
    uint8_t* ip;
    // point the VM's value stack at the first slot that this function can use.
    Value* slots;
} CallFrame;

typedef struct {
    // every CallFrames has its own ip and its own pointer to the ObjFunction that it's executing.
    CallFrame frames[FRAMES_MAX];
    // stores the current height of the CallFrame stack——the number of ongoing function calls.
    int frameCount;

//    Chunk* chunk;
    // instruction pointer: the location of instruction is currently being executed.
    // like Program counter, ip++ pointing next instruction.
//    uint8_t* ip;
    /*
     * Compared with Jlox, it will not reuse variable when traverse the AST node. This creates additional overhead.
     */
    Value stack[STACK_MAX];
    // As with the top of ip,using pointer since it is faster to dereference the pointer than calculate the offset from the ix.
    // point past the element containing the top value on the stack.
    Value* stackTop;

    // all global variable name.
    Table globals;

    // all intern strings(symbol).
    Table strings;

    // the list of open upvalues.
    ObjUpvalue* openUpvalues;

    // obj reference.
    Obj* objects;
} VM;

// For exiting the process by this(static error, runtime error).
typedef enum {
    INTERPRET_OK,
    INTERPRET_COMPILE_ERROR,
    INTERPRET_RUNTIME_ERROR
} InterpretResult;

// for free the obj memory.
extern VM vm;

void initVM();
void freeVM();
InterpretResult interpret(const char* source);
void push(Value value);
Value pop();


#endif //CLOX_VM_H
