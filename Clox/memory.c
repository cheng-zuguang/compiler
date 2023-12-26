//
// Created by 42134 on 2023/11/9.
//
#include <stdlib.h>
#include "memory.h"
#include "vm.h"
#include "compiler.h"

#ifdef DEBUG_LOG_GC
#include <stdio.h>
#include "debug.h"
#endif

#define GC_HEAP_GROW_FACTOR 2

// 通过传的size来确定要做的操作
/*
 *  oldSize     newSize                 Op
 *  0           None-zero               Allocate new Block
 *  None-zero   0                       free
 *  None-zero   smaller than oldSize    shrinking existing allocation
 *  None-zero   greater than oldSize    growing existing allocation
 */

void* reallocate(void* pointer, size_t oldSize, size_t newSize) {
    vm.bytesAllocated += newSize - oldSize;
    if (newSize > oldSize) {
#ifdef DEBUG_STRESS_GC
        collectGarbage();
#endif
        if (vm.bytesAllocated > vm.nextGC) {
            collectGarbage();
        }
    }

    if (newSize == 0) {
        free(pointer);
        return NULL;
    }

    void* result = realloc(pointer, newSize);

    if (result == NULL) exit(1);

    return result;
}

void markObject(Obj* object) {
    if (object == NULL) return;
    // promise GC doesn't get truck in an infinite loop
    // as continually re-adds the same series of objects to gray stack.
    if (object->isMarked) return;

#ifdef DEBUG_LOG_GC
    printf("%p mark \n", (void *) object);
    printValue(OBJ_VAL(object));
    printf("\n");
#endif

    object->isMarked = true;

    // tricolor abstraction: https://www.craftinginterpreters.com/garbage-collection.html#tracing-object-references
    // extra worklist
    if (vm.grayCapacity < vm.grayCount + 1) {
        vm.grayCapacity = GROW_CAPACITY(vm.grayCapacity);
        // call realloc() prevent from gc recursive
        vm.grayStack = (Obj**) realloc(vm.grayStack,
                                       sizeof(Obj*) * vm.grayCapacity);
        // allocation failure.
        if (vm.grayStack == NULL) exit(1);
    }
    vm.grayStack[vm.grayCount++] = object;
}

void markValue(Value value) {
    if (IS_OBJ(value)) markObject(AS_OBJ(value));
}

static void markArray(ValueArray* array) {
    for (int i = 0; i < array->count; i++) {
        markValue(array->values[i]);
    }
}

static void blackenObject(Obj* object) {
    // for seeing the tracing percolate through the object graph.
#ifdef DEBUG_LOG_GC
    printf("%p blacken ", (void *) object);
    printValue(OBJ_VAL(object));
    printf("\n");
#endif

    switch (object->type) {
        case OBJ_BOUND_METHOD: {
            ObjBoundMethod* bound = (ObjBoundMethod*) object;
            markValue(bound->receiver);
            markObject((Obj*)bound->method);
            break;
        }
        case OBJ_CLASS: {
            ObjClass* klass = (ObjClass*) object;
            markObject((Obj*)klass->name);
            markTable(&klass->methods);
            break;
        }
        case OBJ_CLOSURE: {
            ObjClosure* closure = (ObjClosure*) object;
            markObject((Obj*) closure->function);
            for (int i = 0; i < closure->upvalueCount; i++) {
                markObject((Obj*)closure->upvalues[i]);
            }
            break;
        }
        case OBJ_FUNCTION: {
            ObjFunction* function = (ObjFunction*) object;
            markObject((Obj*)function->name);
            markArray(&function->chunk.constants);
            break;
        }
        case OBJ_INSTANCE: {
            ObjInstance* instance = (ObjInstance*) object;
            markObject((Obj*)instance->klass);
            markTable(&instance->fields);
            break;
        }
        case OBJ_UPVALUE:
            markValue(((ObjUpvalue*)object)->closed);
            break;
        /* no reference, no process */
        case OBJ_NATIVE:
        case OBJ_STRING:
            break;
    }
}

static void freeObject(Obj* obj) {
#ifdef DEBUG_LOG_GC
    printf("%p free type %d\n", (void *) obj, obj->type);
#endif

    switch (obj->type) {
        case OBJ_BOUND_METHOD:
            FREE(ObjBoundMethod, obj);
            break;
        case OBJ_CLASS: {
            ObjClass* klass = (ObjClass*)obj;
            freeTable(&klass->methods);
            FREE(ObjClass, obj);
            break;
        }
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
        case OBJ_INSTANCE: {
            ObjInstance* instance = (ObjInstance*)obj;
            freeTable(&instance->fields);
            FREE(ObjInstance, obj);
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

static void markRoots() {
    // most roots are local variables or temporaries sitting right in th VM's stack.
    for (Value* slot = vm.stack; slot < vm.stackTop; slot++) {
        markValue(*slot);
    }

    /*
     * hide-holes of roots
     * 1. call frame
     * 2. upvalues
     * */
    for (int i = 0; i < vm.frameCount; i++) {
        markObject((Obj*)vm.frames[i].closure);
    }

    for (ObjUpvalue* upvalue = vm.openUpvalues; upvalue != NULL; upvalue = upvalue->next) {
        markObject((Obj*)upvalue);
    }

    // other main source code of roots are the global variables.
    markTable(&vm.globals);
    markCompilerRoots();
    markObject((Obj*)vm.initString);
}

static void traceReferences() {
    // pulling out the gray objects, traversing their reference, and mark them black.
    // all reachable variable marks gray that means is black.
    while (vm.grayCount > 0) {
        Obj* object = vm.grayStack[--vm.grayCount];
        blackenObject(object);
    }
}

static void sweep() {
    Obj* previous = NULL;
    Obj* object = vm.objects;
    while (object != NULL) {
        if (object->isMarked) {
            object->isMarked = false;
            previous = object;
            object = object->next;
        } else {
            Obj* unreached = object;
            object = object->next;
            if (previous != NULL) {
                previous->next = object;
            } else {
                vm.objects = object;
            }

            freeObject(unreached);
        }
    }
}

/*
 * adopt the Mark and Sweep(MS) algorithm
 * phase 1: mark roots and can reach the obj
 * phase 2: sweep the objs that can not reach
 * */
void collectGarbage() {
#ifdef DEBUG_LOG_GC
    printf("-- gc begin\n");
    size_t before = vm.bytesAllocated;
#endif

    markRoots();

    // weak reference
    tableRemoveWhite(&vm.strings);

    traceReferences();
    sweep();

    // dynamic adjust the threshold
    vm.nextGC = vm.bytesAllocated * GC_HEAP_GROW_FACTOR;

#ifdef DEBUG_LOG_GC
    printf("-- gc end\n");
    printf("    collected %zu bytes (from %zu to %zu) next at %zu\n",
           before - vm.bytesAllocated, before, vm.bytesAllocated
           );
#endif
}

void freeObjects() {
    Obj* object = vm.objects;
    while (object != NULL) {
       Obj* next = object->next;
       freeObject(object);
       object = next;
    }

    free(vm.grayStack);
}
