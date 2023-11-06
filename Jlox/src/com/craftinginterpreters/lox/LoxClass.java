package com.craftinginterpreters.lox;

import java.util.List;
import java.util.Map;

public class LoxClass extends LoxInstance implements LoxCallable {
    final String name;
    private final Map<String, LoxFunction> methods;

//    LoxClass(String name, Map<String, LoxFunction> methods) {
//        this.name = name;
//        this.methods = methods;
//    }
    LoxClass(LoxClass metaclass, String name, Map<String, LoxFunction> methods) {
        super(metaclass);
        this.name = name;
        this.methods = methods;
    }

    public LoxFunction findMethod(String name) {
        if (methods.containsKey(name)) {
            return methods.get(name);
        }

        return null;
    }

    @Override
    public String toString() {
        return "<class> " + name;
    }

    @Override
    public Object call(Interpreter interpreter, List<Object> arguments) {
        LoxInstance instance = new LoxInstance(this);

        LoxFunction initializer = findMethod("init");
        if (initializer != null) {
            initializer.bind(instance).call(interpreter, arguments);
        }

        return instance;
    }

    // when user user-defined constructor, we will revisit it.
    // But now you can not pass any, so is zero.
    @Override
    public int arity() {
        LoxFunction initializer = findMethod("init");

        if (initializer == null) return 0;

        return initializer.arity();
    }
}
