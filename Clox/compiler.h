//
// Created by 42134 on 2023/11/14.
//
#include "vm.h"

#ifndef CLOX_COMPILER_H
#define CLOX_COMPILER_H

bool compile(const char* source, Chunk* chunk);

static void expression();
static void declaration();
static void statement();

#endif //CLOX_COMPILER_H
