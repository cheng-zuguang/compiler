//
// Created by 42134 on 2023/11/20.
//

#ifndef CLOX_OBJECT_H
#define CLOX_OBJECT_H

#include "common.h"
#include "value.h"
#include "chunk.h"

#define OBJ_TYPE(value)     (AS_OBJ(value)->type)
#define IS_FUNCTION(value)  isObjType(value, OBJ_FUNCTION)
#define IS_NATIVE(value)  isObjType(value, OBJ_NATIVE)
#define IS_STRING(value)    isObjType(value, OBJ_STRING)

// unpack underlying value
#define AS_FUNCTION(value)  ((ObjFunction*)AS_OBJ(value))
#define AS_NATIVE(value)    (((ObjNative*)AS_OBJ(value))->function)
#define AS_STRING(value)    ((ObjString*)AS_OBJ(value))
// get str array from unpack underlying value.
#define AS_CSTRING(value)    (((ObjString*)AS_OBJ(value))->chars)

// TODO: add more type.
typedef enum {
    OBJ_FUNCTION,
    OBJ_NATIVE,
    OBJ_STRING,
} ObjType;

struct Obj {
    ObjType type;
    struct Obj* next;
};

typedef struct {
    Obj obj;
    // store the number of parameters the function expects
    int arity;
    // func body all of bits
    Chunk chunk;
    // func name
    ObjString* name;
} ObjFunction;

// native function.
// argCount: arguments count, args: point the first argument.
typedef Value (*NativeFn) (int argCount, Value* args);
typedef struct {
    // function header
    Obj obj;
    // a pointer to the C function that implements the native behavior.
    NativeFn function;
} ObjNative;

struct ObjString {
    // type
    Obj obj;
    int length;
    // heap-allocated array.
    char* chars;
    // to small, end up colliding. Hash the all entries and cache it.
    uint32_t hash;
};

ObjFunction* newFunction();
ObjNative* newNative(NativeFn function);
ObjString* takeString(char* chars, int length);
ObjString* copyString(const char* chars, int length);
void printObject(Value value);

// A macro is expanded by inserting the argument expression every place the parameter name appears in the body.
// like IS_STRING(POP()), it will execute twice.
static inline bool isObjType(Value value, ObjType type) {
    return IS_OBJ(value) && AS_OBJ(value)->type == type;
}

#endif //CLOX_OBJECT_H
