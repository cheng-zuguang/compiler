//
// Created by 42134 on 2023/11/9.
//
#include <stdlib.h>
#include "memory.h"
#include "vm.h"

// 通过传的size来确定要做的操作
/*
 *  oldSize     newSize                 Op
 *  0           None-zero               Allocate new Block
 *  None-zero   0                       free
 *  None-zero   smaller than oldSize    shrinking existing allocation
 *  None-zero   greater than oldSize    growing existing allocation
 */

void* reallocate(void* pointer, size_t oldSize, size_t newSize) {
    if (newSize == 0) {
        free(pointer);
        return NULL;
    }

    void* result = realloc(pointer, newSize);

    if (result == NULL) exit(1);

    return result;
}

static void freeObject(Obj* obj) {
    switch (obj->type) {
        case OBJ_CLOSURE: {
            ObjClosure* closure = (ObjClosure*) obj;
            FREE_ARRAY(ObjClosure*, closure->upvalues, closure->upvalueCount);
            // just free the objClosure itself, there may be multiple closures that all reference the same function.
            FREE(ObjClosure, obj);
            break;
        }
        case OBJ_FUNCTION: {
            ObjFunction* function = (ObjFunction*)obj;
            // free chunk
            freeChunk(&function->chunk);
            // free obj.
            FREE(ObjFunction, obj);
            break;
        }
        case OBJ_NATIVE: {
            // free obj.
            FREE(ObjNative, obj);
            break;
        }
        case OBJ_STRING: {
            ObjString* string = (ObjString*)obj;
            // free chars array.
            FREE_ARRAY(char, string->chars, string->length + 1);
            // free obj.
            FREE(ObjString, obj);
            break;
        }
        case OBJ_UPVALUE: {
            FREE(ObjUpvalue , obj);
            break;
        }
    }
}

void freeObjects() {
    Obj* object = vm.objects;
    while (object != NULL) {
       Obj* next = object->next;
       freeObject(object);
       object = next;
    }
}
