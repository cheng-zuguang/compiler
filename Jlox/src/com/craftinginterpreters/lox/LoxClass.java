package com.craftinginterpreters.lox;

import java.util.List;

public class LoxClass implements LoxCallable {
    final String name;

    LoxClass(String name) {
        this.name = name;
    }

    @Override
    public String toString() {
        return "<class> " + name;
    }

    @Override
    public Object call(Interpreter interpreter, List<Object> arguments) {
        LoxInstance instance = new LoxInstance(this);
        return instance;
    }


    // when user user-defined constructor, we will revisit it.
    // But now you can not pass any, so is zero.
    @Override
    public int arity() {
        return 0;
    }
}
