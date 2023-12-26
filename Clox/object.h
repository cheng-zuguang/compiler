//
// Created by 42134 on 2023/11/20.
//

#ifndef CLOX_OBJECT_H
#define CLOX_OBJECT_H

#include "common.h"
#include "value.h"
#include "chunk.h"
#include "table.h"

#define OBJ_TYPE(value)     (AS_OBJ(value)->type)

#define IS_BOUND_METHOD(value)      isObjType(value, OBJ_BOUND_METHOD)
#define IS_CLASS(value)             isObjType(value, OBJ_CLASS)
#define IS_CLOSURE(value)           isObjType(value, OBJ_CLOSURE)
#define IS_FUNCTION(value)          isObjType(value, OBJ_FUNCTION)
#define IS_INSTANCE(value)          isObjType(value, OBJ_INSTANCE)
#define IS_NATIVE(value)            isObjType(value, OBJ_NATIVE)
#define IS_STRING(value)            isObjType(value, OBJ_STRING)

// unpack underlying value
#define AS_BOUND_METHOD(value)  ((ObjBoundMethod*)AS_OBJ(value))
#define AS_CLASS(value)         ((ObjClass*)AS_OBJ(value))
#define AS_CLOSURE(value)       ((ObjClosure*)AS_OBJ(value))
#define AS_FUNCTION(value)      ((ObjFunction*)AS_OBJ(value))
#define AS_INSTANCE(value)      ((ObjInstance*)AS_OBJ(value))
#define AS_NATIVE(value)        (((ObjNative*)AS_OBJ(value))->function)
#define AS_STRING(value)        ((ObjString*)AS_OBJ(value))
// get str array from unpack underlying value.
#define AS_CSTRING(value)       (((ObjString*)AS_OBJ(value))->chars)

// TODO: add more type.
typedef enum {
    OBJ_BOUND_METHOD,
    OBJ_CLASS,
    OBJ_CLOSURE,
    OBJ_FUNCTION,
    OBJ_INSTANCE,
    OBJ_NATIVE,
    OBJ_STRING,
    OBJ_UPVALUE
} ObjType;

struct Obj {
    ObjType type;
    bool isMarked;
    struct Obj* next;
};

typedef struct {
    Obj obj;
    // store the number of parameters the function expects
    int arity;
    // the count of upvalue list
    int upvalueCount;
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

// runtime representation for upvalues.
typedef struct ObjUpvalue {
    Obj obj;

    // point the close-over variable.
    Value* location;

    // close over the value.
    Value closed;
    // the list of open upvalues.
    struct ObjUpvalue* next;
} ObjUpvalue;

typedef struct {
    Obj obj;
    // reference to the underlying bare function
    ObjFunction* function;
    // a pointer to a dynamically allocated array of pointers to upvalues.
    ObjUpvalue** upvalues;
    // for gc
    int upvalueCount;
} ObjClosure;

typedef struct {
    Obj obj;
    // class name.
    ObjString* name;
    Value initializer;
    // methods
    Table methods;
} ObjClass;

typedef struct {
    Obj obj;
    ObjClass* klass;
    // freely add field to the objects
    Table fields;
} ObjInstance;

typedef struct {
    Obj obj;
    // don't have to keep converting the pointer back to value when it gets passed to general functions.
    Value receiver;
    ObjClosure* method;
} ObjBoundMethod;

ObjBoundMethod* newBoundMethod(Value receiver, ObjClosure* method);
ObjClass* newClass(ObjString* name);
ObjClosure* newClosure(ObjFunction* function);
ObjFunction* newFunction();
ObjInstance* newInstance(ObjClass* klass);
ObjNative* newNative(NativeFn function);
ObjString* takeString(char* chars, int length);
ObjString* copyString(const char* chars, int length);
ObjUpvalue* newUpvalue(Value* slot);
void printObject(Value value);

// A macro is expanded by inserting the argument expression every place the parameter name appears in the body.
// like IS_STRING(POP()), it will execute twice.
static inline bool isObjType(Value value, ObjType type) {
    return IS_OBJ(value) && AS_OBJ(value)->type == type;
}

#endif //CLOX_OBJECT_H
