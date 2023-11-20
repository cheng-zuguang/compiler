//
// Created by 42134 on 2023/11/20.
//
#include <stdio.h>
#include <string.h>

#include "memory.h"
#include "object.h"
#include "value.h"
#include "vm.h"

// according to struct type's size to initialize heap size.
#define ALLOCATE_OBJ(type, objectType) \
    (type*)allocateObject(sizeof(type), objectType)

static Obj* allocateObject(size_t size, ObjType type) {
    Obj* obj = (Obj*) reallocate(NULL, 0, size);
    obj->type = type;

    obj->next = vm.objects;
    vm.objects = obj;

    return obj;
}

//static ObjString* allocateString(char* chars, int length) {
//    ObjString* string = ALLOCATE_OBJ(ObjString, OBJ_STRING);
//    string->length = length;
//    string->chars = chars;
//    return string;
//}

ObjString* makeString(int length) {
    ObjString* string = (ObjString*) allocateObject(
            sizeof(ObjString) + length + 1,
            OBJ_STRING
    );
    string->length = length;
    return string;
}

// ObjString* takeString(char* chars, int length) {
//     ObjString* string = makeString(length);
//     memcpy(string->chars, chars, length);
//     string->chars[length] ='\0';
//     return string;
// }
//
//ObjString* copyString(const char* chars, int length) {
//    // allocate heap for str.
//    char* heapChars = ALLOCATE(char, length + 1);
//    memcpy(heapChars, chars, length);
//    heapChars[length] ='\0';
//    return allocateString(heapChars, length);
//}


ObjString* copyString(const char* chars, int length) {
    // allocate heap for str.
    ObjString* string = makeString(length);
    memcpy(string->chars, chars, length);
    string->chars[length] ='\0';
    return string;
}

void printObject(Value value) {
    switch (OBJ_TYPE(value)) {
        case OBJ_STRING:
            printf("%s", AS_CSTRING(value));
            break;
    }
}