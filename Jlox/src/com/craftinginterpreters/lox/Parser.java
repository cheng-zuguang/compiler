package com.craftinginterpreters.lox;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.Map;

import static com.craftinginterpreters.lox.TokenType.*;
/*
    优先级从低到高
    program        → statement* EOF ;
    declaration    → varDecl
                     | statement ;
    statement      → exprStmt
                     | forStmt
                     | ifStmt
                     | printStmt
                     | whileStmt
                     | block ;
    forStmt        → "for" "("
                        ( varDecl | exprStmt | ";" ) expression? ";" expression?
                     ")" statement ;
    whileStmt      → "while" "(" expression ")" statement ;
    ifStmt         → "if" "(" expression ")" statement
                     ( "else" statement )? ;
    block          → "{" declaration* "}" ;
    exprStmt       → expression ";" ;
    printStmt      → "print" expression ";" ;
    varDecl        → "var" IDENTIFIER ("=" expression ) ? ";" ;
    expression     → assignment ;
    assignment     → IDENTIFIER "=" assignment | equality | logic_or ;
    logic_or       → login_and ( "or" login_and )* ;
    login_and      → equality ( "and" equality )* ;
    equality       → comparison ( ("!=" | "==") comparison )* ;
    comparison     → term ( (">" | ">=" | "<" | "<=" ) term )* ;
    term           → factor ( ( "-" | "+" ) factor )* ;
    factor         → unary ( ( "/" | "*" ) unary )* ;
    unary          → ( "-" | "!" ) unary | primary;
    primary        → NUMBER | STRING | "true" | "false" | "nil"
                   | "(" expression ")"
                   | IDENTIFIER
                   // error productions
                   | ("!=" | "==") equality
                   | (">" || ">=" || "<" || "<=") comparsion
                   | ("+") term
                   | ("/" | "*") factor ;


    About Syntax Errors:
    1. Detect and report the error
    2. Avoid crashing or hanging

    About Parser:
    1. Fast.
    2. Report as many distinct errors as there are. Report all the problem not just ones.
    3. Minimize cascaded errors. track the real problem's code, not ghost errors.
    * */
public class Parser {
//    Expr parse() {
//        try {
//            return expression();
//        } catch (ParseError error) {
//            return null;
//        }
//    }

    List<Stmt> parse() {
        List<Stmt> statements = new ArrayList<>();
        while (!isAtEnd()) {
            statements.add(declaration());
        }

        return statements;
    }

    private static class ParseError extends RuntimeException {}

    private final List<Token> tokens;
    private int current = 0;

    Parser(List<Token> tokens) {
        this.tokens = tokens;
    }

    // declaration    → varDecl | statement ;
    private Stmt declaration() {
        try {
            if (match(VAR)) {
                return varDeclaration();
            }
            return statement();
        } catch (ParseError error) {
            synchronize();
            return null;
        }
    }

    // statement      → exprStmt | forStmt | ifStmt | printStmt | whileStmt | block ;

    private Stmt statement() {
        if (match(FOR)) return forStatement();
        if (match(IF)) return ifStatement();
        if (match(PRINT)) return printStatement();
        if (match(WHILE)) return whileStatement();
        if (match(LEFT_BRACE)) return new Stmt.Block(block());

        return expressionStatement();
    }

    // forStmt        → "for" "("
    //                        ( varDecl | exprStmt | ";" ) expression? ";" expression?
    //                     ")" statement ;
    private Stmt forStatement() {
        consume(LEFT_PAREN, "expected '(' after for.");
        Stmt initializer;
        if (match(SEMICOLON)) {
            initializer = null;
        } else if (match(VAR)) {
            initializer = varDeclaration();
        } else  {
            initializer = expressionStatement();
        }

        Expr condition = null;
        if (!check(SEMICOLON)) {
            condition = expression();
        }
        consume(SEMICOLON, "Expect ';' after loop condition");

        Expr increment = null;
        if (!check(RIGHT_PAREN)) {
            increment = expression();
        }
        consume(RIGHT_PAREN, "Expect ')' after the class");

        Stmt body = statement();

        // Desugaring piece: for loop -> while loop
        if (increment != null) {
            body = new Stmt.Block(Arrays.asList(
                    body,
                    new Stmt.Expression(increment)
            ));
        }

        if (condition == null) condition = new Expr.Literal(true);
        body = new Stmt.While(condition, body);

        if (initializer != null) {
            body = new Stmt.Block(Arrays.asList(initializer, body));
        }

        return body;
    }

    // if     -> "if" "(" expression ")" statement ( "else" statement )? ;
    private Stmt ifStatement() {
        consume(LEFT_PAREN, "Expect 'after' 'if'.");
        Expr condition = expression();
        consume(RIGHT_PAREN, "Expect ')' after if conditions");

        Stmt thenBranch = statement();
        Stmt elseBranch = null;

        if (match(ELSE)) {
            elseBranch = statement();
        }

        return new Stmt.If(condition, thenBranch, elseBranch);
    }

    // printStmt      → "print" expression ";" ;
    private Stmt printStatement() {
        Expr expr = expression();
        consume(SEMICOLON, "Expected ':' after value");
        return new Stmt.Print(expr);
    }

    // exprStmt       → expression ";" ;
    private Stmt expressionStatement() {
        Expr expr = expression();
        consume(SEMICOLON, "Expected ':' after expression");
        return new Stmt.Expression(expr);
    }

    // block          → "{" declaration* "}" ;
    private List<Stmt> block() {
        List<Stmt> statements = new ArrayList<>();
        while (!check(RIGHT_BRACE) && !isAtEnd()) {
            statements.add(declaration());
        }

        consume(RIGHT_BRACE, "Excepted '}' after block");
        return statements;
    }

    // varDecl        → "var" IDENTIFIER ("=" expression ) ? ";" ;
    private Stmt varDeclaration() {
        Token name = consume(IDENTIFIER, "Excepted variable name");

        Expr initializer = null;
        if (match(EQUAL)) {
            initializer = expression();
        }

        consume(SEMICOLON, "Expected ';' after variable declaration");
        return new Stmt.Expression(initializer);
    }

    // whileStmt      → "while" "(" expression ")" statement ;
    private Stmt whileStatement() {
        consume(LEFT_PAREN, "Expect '(' after while.");
        Expr condition = expression();

        consume(RIGHT_PAREN, "Expect ')' after condition");
        Stmt body = statement();

        return new Stmt.While(condition, body);
    }

    // expression     → assignment ;
    // assignment     → IDENTIFIER "=" assignment | equality | logic_or ;
    private Expr expression() {
        return assignment();
    }

    private Expr assignment() {
//        Expr expr = equality();
        Expr expr = or();

        if (match(EQUAL)) {
            Token equals = previous();
            Expr value = assignment();

            if (expr instanceof Expr.Variable) {
                Token name = ((Expr.Variable) expr).name;
                return new Expr.Assign(name, value);
            }

            error(equals, "Invalid assignment target.");
        }

        return expr;
    }
    // logic_or       → login_and ( "or" login_and )* ;
    private Expr or() {
        Expr expr = and();

        while (match(OR)) {
            Token operator = previous();
            Expr right = and();
            expr = new Expr.Logical(expr, operator, right);
        }

        return expr;
    }
    // login_and      → equality ( "and" equality )* ;
    private Expr and() {
        Expr expr = equality();

        while (match(AND)) {
            Token operator = previous();
            Expr right = equality();
            expr = new Expr.Logical(expr, operator, right);
        }

        return expr;
    }

    /**
     * Challenge 6.1
     * expression     -> comma ;
     * comma          → equality ( "," equality )*;
     * other rules...
     * */
    private Expr comma() {
        Expr expr = equality();

        while (match(COMMA)) {
            Token operator = previous();
            Expr right = equality();

            expr = new Expr.Binary(expr, operator, right);
        }

        return expr;
    }
    /*
    * expression  -> conditional
    * conditional -> equality ( "?" expression ":" conditional )? ;
    * */
    private Expr conditional() {
        Expr expr = equality();

        if (match(QUESTION)) {
            Expr thenBranch = expression();
            consume(COLON, "Expect: after then branch of conditional expression.");
            Expr elseBranch = conditional();
//            expr = new Expr.Conditional(expr, thenBranch, elseBranch);
        }

        return expr;
    }

    // equality       → comparison ( ("!=" | "==") comparison )* ;
    private Expr equality() {
        Expr expr = comparison();

        while(match(BANG_EQUAL, EQUAL_EQUAL)) {
            Token operator = previous();
            Expr right = comparison();

            expr = new Expr.Binary(expr, operator, right);
        }

        return  expr;
    }

    // comparison     → term ( (">" | ">=" | "<" | "<=" ) term )* ;
    private Expr comparison() {
        Expr expr = term();

        while (match(GREATER, GREATER_EQUAL, LESS, LESS_EQUAL)) {
            Token operator = previous();
            Expr right = term();
            
            expr = new Expr.Binary(expr, operator, right);
        }
        
        return  expr;
    }

    // term           → factor ( ( "-" | "+" ) factor )* ;
    private Expr term() {
        Expr expr = factor();

        while (match(MINUS, PLUS)) {
            Token operator = previous();
            Expr right = factor();

            expr = new Expr.Binary(expr, operator, right);
        }

        return  expr;
    }

    // factor         → unary ( ( "/" | "*" ) unary )* ;
    private Expr factor() {
        Expr expr = unary();

        while (match(SLASH, STAR)) {
            Token operator = previous();
            Expr right = unary();

            expr = new Expr.Binary(expr, operator, right);
        }

        return  expr;
    }

    // unary          → ( "-" | "!" ) unary | primary;
    private Expr unary() {
        if (match(MINUS, BANG)) {
            Token operator = previous();
            Expr right = unary();
            return new Expr.Unary(operator, right);
        }

        return primary();
    }

    // primary        → NUMBER | STRING | "true" | "false" | "nil" | "(" expression ")" | IDENTIFIER ;
    private Expr primary() {
        if (match(FALSE)) return new Expr.Literal(false);
        if (match(TRUE)) return new Expr.Literal(true);
        if (match(NIL)) return new Expr.Literal(null);

        if (match(NUMBER, STRING)) {
            return new Expr.Literal(previous().literal);
        }

        if (match(IDENTIFIER)) {
            return new Expr.Variable(previous());
        }

        if (match(LEFT_PAREN)) {
            Expr expr = expression();
            consume(RIGHT_PAREN, "Except ')' after expression");
            return new Expr.Grouping(expr);
        }

        // error productions
        if (match(BANG_EQUAL, EQUAL_EQUAL)) {
            error(previous(), "Missing left-hand operand");
            equality();
            return null;
        }

        if (match(GREATER, GREATER_EQUAL, LESS, LESS_EQUAL)) {
            error(previous(), "Missing left-hand operand");
            comparison();
            return null;
        }

        if (match(PLUS)) {
            error(previous(), "Missing left-hand operand");
            term();
            return null;
        }

        if (match(SLASH, STAR)) {
            error(previous(), "Missing left-hand operand");
            factor();
            return null;
        }

        throw error(peek(), "Except expression.");
    }

    private Token consume(TokenType type, String message) {
        if (check(type)) return advance();
        throw error(peek(), message);
    }

    private boolean match(TokenType ...types) {
        for (TokenType type : types) {
            if (check(type)) {
                advance();
                return true;
            }
        }

        return false;
    }

    private boolean check(TokenType type) {
        if (isAtEnd()) return false;
        return peek().type == type;
    }

    private Token advance() {
        if (!isAtEnd()) current++;
        return previous();
    }

    private boolean isAtEnd() {
        return peek().type == EOF;
    }

    private Token peek() {
        return tokens.get(current);
    }

    private Token previous() {
        return tokens.get(current - 1);
    }

    private ParseError error(Token token, String message) {
        Lox.error(token, message);
        return new ParseError();
    }

    private void synchronize() {
        advance();

        while (!isAtEnd()) {
            if (previous().type == SEMICOLON) return;

            switch (peek().type) {
                case CLASS:
                case FUN:
                case VAR:
                case FOR:
                case WHILE:
                case IF:
                case PRINT:
                case RETURN:
                    return;
            }
            advance();
        }
    }

}
