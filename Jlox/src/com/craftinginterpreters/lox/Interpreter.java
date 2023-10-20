package com.craftinginterpreters.lox;

public class Interpreter implements Expr.Visitor<Object> {

    void interpret(Expr expr) {
        try {
            Object value = evaluate(expr);
            System.out.println(stringify(value));
        } catch (RuntimeError error) {
            Lox.runtimeError(error);
        }
    }

    private String stringify(Object object) {
        if (object == null) return "nil";

        if (object instanceof Double) {
            String text = object.toString();
            if (text.endsWith(".0")) {
                text = text.substring(0, text.length() - 2);
            }
            return text;
        }

        return object.toString();
    }

    @Override
    public Object visitLiteralExpr(Expr.Literal expr) {
        return expr.value;
    }

    @Override
    public Object visitUnaryExpr(Expr.Unary expr) {
        Object right = evaluate(expr.right);

        switch (expr.operator.type) {
            case BANG:
                return !isTruthy(right);
            case MINUS:
                checkNumberOperand(expr.operator, right);
                return -(double)right;
        }

        return null;
    }

    /**
     * nil or false is falsey, else is  truthy
     * */
    public boolean isTruthy(Object obj) {
        if (obj == null) return false;
        if (obj instanceof Boolean) return (boolean) obj;
        return true;
    }

    private void checkNumberOperand(Token operator, Object operand) {
        if (operand instanceof Double) return;
        throw new RuntimeError(operator, "operand must be an number");
    }

    private void checkNumberOperand(Token operator, Object left, Object right) {
        if (left instanceof Double && right instanceof Double) return;

        throw new RuntimeError(operator, "operand must be an number or string");
    }

    private void detectDivisorIsZero(Token operator, Object obj) {
        if ((double) obj == 0) {
            throw new RuntimeError(operator, "divisor could not be zero.");
        }
    }

    @Override
    public Object visitGroupingExpr(Expr.Grouping expr) {
        return evaluate(expr.expression);
    }

    private Object evaluate(Expr expr) {
        return expr.accept(this);
    }

    @Override
    public Object visitBinaryExpr(Expr.Binary expr) {
        Object left = evaluate(expr.left);
        Object right = evaluate(expr.right);

        switch (expr.operator.type) {
            case BANG_EQUAL:
                return !isEqual(left, right);
            case EQUAL_EQUAL:
                return isEqual(left, right);
            case GREATER:
                checkNumberOperand(expr.operator, left, right);
                return (double) left > (double) right;
            case GREATER_EQUAL:
                checkNumberOperand(expr.operator, left, right);
                return (double) left >= (double) right;
            case LESS:
                checkNumberOperand(expr.operator, left, right);
                return (double) left < (double) right;
            case LESS_EQUAL:
                checkNumberOperand(expr.operator, left, right);
                return (double) left <= (double) right;
            case MINUS:
                checkNumberOperand(expr.operator, left, right);
                return (double) left - (double) right;
            case STAR:
                checkNumberOperand(expr.operator, left, right);
                return (double) left * (double) right;
            case SLASH:
                checkNumberOperand(expr.operator, left, right);
                detectDivisorIsZero(expr.operator, right);
                return (double) left / (double) right;
            case PLUS:
                if (left instanceof Double && right instanceof Double) {
                    return (double) left + (double) right;
                }

                if ((left instanceof String || left instanceof Double) && (right instanceof String || right instanceof Double)) {
                    if (left instanceof Double) {
                        return left.toString().substring(0, left.toString().length() - 2) + right;
                    }

                    if (right instanceof Double) {
                        return left + right.toString().substring(0, right.toString().length() - 2);
                    }
                }

                throw new RuntimeError(expr.operator, "operand must be two number or two string.");
        }

        return null;
    }

    private boolean isEqual(Object left, Object right) {
        if (left == null && right == null) return true;
        if (left == null) return false;

        return left.equals(right);
    }

}
