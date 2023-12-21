//
// Created by 42134 on 2023/11/14.
//
#include "vm.h"
#include "object.h"

#ifndef CLOX_COMPILER_H
#define CLOX_COMPILER_H

ObjFunction* compile(const char* source);
void markCompilerRoots();

static void expression();
static void declaration();
static void statement();


#endif //CLOX_COMPILER_H
