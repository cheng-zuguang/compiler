//
// Created by 42134 on 2023/11/10.
//

#ifndef CLOX_VALUE_H
#define CLOX_VALUE_H

typedef double Value;

typedef struct {
    int capacity;
    int count;
    Value* values;
} ValueArray;

void initValueArray(ValueArray* array);
void writeValueArray(ValueArray* array, Value val);
void freeValueArray(ValueArray* array);

void printValue(Value value);

#endif //CLOX_VALUE_H
