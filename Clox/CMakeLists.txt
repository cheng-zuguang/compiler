cmake_minimum_required(VERSION 3.23)
project(Clox C)

set(CMAKE_C_STANDARD 11)

add_library(cloxLibary STATIC chunk.c memory.c)

include_directories(.)

add_executable(Clox
        common.h
        chunk.h
        chunk.c
        memory.h
        memory.c
        value.h
        value.c
        object.h
        object.c
        vm.h
        vm.c
        compiler.h
        compiler.c
        scanner.h
        scanner.c
        debug.h
        debug.c
        main.c)
