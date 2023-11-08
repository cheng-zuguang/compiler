package com.craftinginterpreters.lox;

import java.util.List;

public class LoxArray extends LoxInstance {
    private final Object[] elements;

    LoxArray(int size) {
        super(null);
        elements = new Object[size];
    }

    @Override
    public Object get(Token name) {
        if (name.lexeme.equals("get")) {
            return new LoxCallable() {
                @Override
                public int arity() {
                    return 1;
                }

                @Override
                public Object call(Interpreter interpreter, List<Object> arguments) {
                    if (!(arguments.get(0) instanceof Double)) {
                        throw new RuntimeError(name, "array index must be integer.");
                    }
                    int ix = (int) (double) arguments.get(0);
                    return elements[ix];
                }
            };
        } else if (name.lexeme.equals("set")) {
            return new LoxCallable() {
                @Override
                public int arity() {
                    return 2;
                }

                @Override
                public Object call(Interpreter interpreter, List<Object> arguments) {
                    if (!(arguments.get(0) instanceof Double)) {
                        throw new RuntimeError(name, "array index must be integer.");
                    }

                    int ix = (int) (double) arguments.get(0);
                    Object value = arguments.get(1);
                    return elements[ix] = value;
                }
            };
        } else if (name.lexeme.equals("length")) {
            return (double) elements.length;
        }

        throw new RuntimeError(name, "Undefined property '" + name.lexeme + "'.");
    }

    @Override
    public void set(Token name, Object value) {
        throw new RuntimeError(name, "Can't add properties to arrays.");
    }

    @Override
    public String toString() {
        StringBuffer buffer = new StringBuffer();
        buffer.append("[");
        for (int i = 0; i < elements.length; i++) {
            if (i != 0) buffer.append(", ");
            buffer.append(elements[i]);
        }
        buffer.append("]");
        return buffer.toString();
    }
}
