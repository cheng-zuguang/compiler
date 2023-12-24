//
// Created by 42134 on 2023/11/10.
//

#ifndef CLOX_VALUE_H
#define CLOX_VALUE_H

#include "common.h"

//typedef struct Obj Obj;
typedef struct sObj Obj;
typedef struct ObjString ObjString;

typedef enum {
    // for smaller, using stack, payload directly store struct.
    VAL_BOOL,
    VAL_NIL,
    VAL_NUMBER,
    // for larger, using heap. the inner of value is a pointer to that blob of memory.
    // call this, "Obj".
    VAL_OBJ
} ValueType;

typedef struct {
    ValueType type;
    union {
        bool boolean;
        double number;
        Obj* obj;
    } as;
} Value;

// determine type
#define IS_BOOL(value)      ((value).type == VAL_BOOL)
#define IS_NIL(value)       ((value).type == VAL_NIL)
#define IS_NUMBER(value)    ((value).type == VAL_NUMBER)
#define IS_OBJ(value)       ((value).type == VAL_OBJ)

// unpack underlying value from struct value.
#define AS_OBJ(value)       ((value).as.obj)
#define AS_BOOL(value)      ((value).as.boolean)
#define AS_NUMBER(value)    ((value).as.number)

// pack underlying value to struct value.
#define BOOL_VAL(value)             ((Value){VAL_BOOL, {.boolean = value}})
#define NIL_VAL                     ((Value){VAL_NIL, {.number = 0}})
#define NUMBER_VAL(value)           ((Value){VAL_NUMBER, {.number = value}})
#define OBJ_VAL(object)             ((Value){VAL_OBJ, {.obj = (Obj*)object}})

//typedef double Value;

typedef struct {
    int capacity;
    int count;
    Value *values;
} ValueArray;

bool valuesEqual(Value a, Value b);

void initValueArray(ValueArray *array);

void writeValueArray(ValueArray *array, Value val);

void freeValueArray(ValueArray *array);

void printValue(Value value);

#endif //CLOX_VALUE_H
