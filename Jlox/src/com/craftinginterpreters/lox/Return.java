package com.craftinginterpreters.lox;

// control flow
public class Return extends RuntimeException {
    final Object value;
    Return(Object value) {
        // decrease JVM error stack overhead
        super(null, null, false, false);
        this.value = value;
    }
}
