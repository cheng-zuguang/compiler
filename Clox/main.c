//
// Created by 42134 on 2023/11/8.
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "chunk.h"
#include "debug.h"
#include "vm.h"

static void repl() {
    char line[1024];
    for (;;) {
        printf("> ");

        if (!fgets(line, sizeof(line), stdin)) {
            printf("\n");
            break;
        }

        interpret(line);
    }
}

static char* readFile(const char* path) {
    FILE* file = fopen(path, "rb");
    if (file == NULL) {
        fprintf(stderr, "Could not open file \"%s\".\n", path);
        exit(74);
    }

    // 定位到文件末尾
    fseek(file, 0L, SEEK_END);
    // 返回文件开始到结束的字节数
    size_t fileSize = ftell(file);
    // 回到文件的开始处， 等价于 fseek(file, 0L, SEEK_START);
    rewind(file);

    // 将文件内容一次性读入到buffer内
    char* buffer = (char*) malloc(fileSize + 1);
    if (buffer == NULL) {
        fprintf(stderr, "Not enough memory to read \"%s\".\n", path);
        exit(74);
    }

    size_t bytesRead = fread(buffer, sizeof(char), fileSize, file);
    if (bytesRead < fileSize) {
        fprintf(stderr, "Could not open file \"%s\".\n", path);
        exit(74);
    }
    buffer[bytesRead] = '\0';

    fclose(file);
    return buffer;
}

static void runFile(const char* path) {
    char* source = readFile(path);
    InterpretResult result = interpret(source);
    free(source);

    if (result == INTERPRET_COMPILE_ERROR) exit(65);
    if (result == INTERPRET_RUNTIME_ERROR) exit(70);
}

int main(int argc, const char *argv[]) {
    initVM();

//    if (argc == 1) {
//        repl();
//    } else if (argc == 2) {
//        runFile(argv[1]);
//    } else {
//        fprintf(stderr, "Usage clox [path]\n");
//        exit(64);
//    }

    runFile("D:\\PROJECTS\\complier\\Clox\\cmake-build-debug\\script.txt");

    freeVM();
    return 0;
}