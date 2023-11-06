package com.craftinginterpreters.lox;

import java.util.List;

public class LoxFunction implements LoxCallable {
//    private final String name;
    private final Stmt.Function declaration;
    private final Environment closure;

    private final boolean isInitializer;

    LoxFunction(Stmt.Function declaration, Environment closure, boolean isInitializer) {
//        this.name = name;

        this.declaration = declaration;
        this.closure = closure;
        this.isInitializer = isInitializer;
    }

    @Override
    public String toString() {
//        if (name == null) return "<fn>";
//        return "<fn " + name + ">";
        return "<fn " + declaration.name.lexeme + ">";
    }

    @Override
    public int arity() {
        return declaration.params.size();
    }

    @Override
    public Object call(Interpreter interpreter, List<Object> arguments) {
//        Environment environment = new Environment(interpreter.globals);
        Environment environment = new Environment(closure);

        for (int i = 0; i < declaration.params.size(); i++) {
            environment.define(declaration.params.get(i).lexeme, arguments.get(i));
        }
        // 使用异常来控制return返回， 如果到函数体结束都没有对应的return, 则隐式的返回nil.
        try {
            interpreter.executeBlock(declaration.body, environment);
        } catch (Return returnValue) {
            // return this instead of nil, corner case: return;
            if (isInitializer) return closure.getAt(0, "this");

            return returnValue.value;
        }

        if (isInitializer) return closure.getAt(0, "this");

        return null;
    }

    LoxFunction bind(LoxInstance instance) {
        Environment environment = new Environment(closure);
        environment.define("this", instance);
        return new LoxFunction(declaration, environment, isInitializer);
    }
}
