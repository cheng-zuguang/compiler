//
// Created by 42134 on 2023/11/14.
//
#include <stdio.h>
#include <stdlib.h>
#include "common.h"
#include "compiler.h"
#include "scanner.h"
#include "debug.h"
#include "object.h"

/*
 * BNF:
 *      Declaration -> classDecl
 *                  |  functionDecl
 *                  |  varDecl
 *                  | statement ;
 *
 *      statement   -> exprStmt
 *                  | forStmt
 *                  | ifStmt
 *                  | printStmt
 *                  | returnStmt
 *                  | whileStmt
 *                  | block ;
 * */


typedef struct {
    Token current;
    Token previous;
    bool hadError;
    // avoid error cascades.
    bool panicMode;
} Parser;

typedef enum {
    PREC_NONE,
    PREC_ASSIGNMENT, // =
    PREC_OR, // or
    PREC_AND, // and
    PREC_EQUALITY, // == | !=
    PREC_COMPARISON, // < | > | >= | <=
    PREC_TERM, // + | -
    PREC_FACTOR, // * /
    PREC_UNARY,
    PREC_CALL,
    PREC_PRIMARY

} Precedence;

// 函数指针: 返回类型 + 指针名 + (入参类型, ...)
typedef void (*ParseFn)(bool canAssign);

typedef struct {
    ParseFn prefix;
    ParseFn infix;
    Precedence precedence;
} ParseRule;

static ParseRule *getRule(TokenType type);

static void parsePrecedence();

static void expression();

static uint8_t identifierConstant(Token* name);

Parser parser;
Chunk *compilingChunk;

// the key is strings, its value is the index of the identifier in the constant table.
Table stringConstants;


// for compiling user-defined func
// so encapsulate a func to return current chunk.
static Chunk *currentChunk() {
    return compilingChunk;
}

static void errorAt(Token *token, const char *message) {
    // 阻止其他的错误检测
    if (parser.panicMode) return;
    parser.panicMode = true;

    fprintf(stderr, "[line %d] Error", token->line);

    if (token->type == TOKEN_EOF) {
        fprintf(stderr, " at end.");
    } else if (token->type == TOKEN_ERROR) {
        //
    } else {
        fprintf(stderr, " at '%.*s'", token->length, token->start);
    }

    fprintf(stderr, ": %s\n", message);
    parser.hadError = true;
}

// report error when we consumed token.
static void error(const char *message) {
    errorAt(&parser.previous, message);
}

static void errorAtCurrent(const char *message) {
    errorAt(&parser.current, message);
}

static void advance() {
    parser.previous = parser.current;

    for (;;) {
        parser.current = scanToken();
        if (parser.current.type != TOKEN_ERROR) break;

        errorAtCurrent(parser.current.start);
    }
}

static void consume(TokenType type, const char *message) {
    if (parser.current.type == type) {
        advance();
        return;
    }

    errorAtCurrent(message);
}

static bool check(TokenType type) {
    return parser.current.type == type;
}

/**
  * @brief  if the type match current TokenType, consume it and return true, otherwise false.
  * @param  type: target type
  * @retval true or false.
  */
static bool match(TokenType type) {
    if (!check(type)) return false;
    advance();
    return true;
}

static void emitByte(uint8_t byte) {
    // send previous line to report accurately error.
    writeChunk(currentChunk(), byte, parser.previous.line);
}

// for constant, useful!!!
static void emitBytes(uint8_t byte1, uint8_t byte2) {
    emitByte(byte1);
    emitByte(byte2);
}

static void emitReturn() {
    // temporarily use OP_RETURN to print value.
    emitByte(OP_RETURN);
}

// insert constant and return ix.
// make sure limit 256 constants.
static uint8_t makeConstant(Value value) {
    int constant = addConstant(currentChunk(), value);
    if (constant > UINT8_MAX) {
        error("Too many constants in one chunk.");
        return 0;
    }

    return (uint8_t) constant;
}

static void emitConstant(Value value) {
    // 1.push opcode in stack
    // 2. insert value in constant table.
    emitBytes(OP_CONSTANT, makeConstant(value));
}

static void endCompiler() {
    emitReturn();
#ifdef DEBUG_PRINT_CODE
    if (!parser.hadError) {
        disassembleChunk(currentChunk(), "code");
    }
#endif
}


// case: parentheses for grouping
// BNF: '(' expression ')'
static void grouping(bool canAssign) {
    expression();
    consume(TOKEN_RIGHT_PAREN, "Excepted ')' after expression.");
}

// case: number literally
// assumes the token for the number literal has already been consumed
static void number(bool canAssign) {
    // 将字符串中的number literals 转为 double.
    double value = strtod(parser.previous.start, NULL);
    emitConstant(NUMBER_VAL(value));
}

// case: str-variable.
static void string(bool canAssign) {
    // trim the leading and trailing quotation marks.
    emitConstant(OBJ_VAL(copyString(parser.previous.start + 1, parser.previous.length - 2)));
}

static void namedVariable(Token name, bool canAssign) {
    uint8_t arg = identifierConstant(&name);

    if (canAssign && match(TOKEN_EQUAL)) {
        // if we see the =, we compile it as an assignment or setter instead of variable access or getter.
        expression();
        emitBytes(OP_GET_GLOBAL, arg);
    } else {
        emitBytes(OP_GET_GLOBAL, arg);
    }
}

// case: identifier
static void variable(bool canAssign) {
    namedVariable(parser.previous, canAssign);
}

// case: the four horseman of Arithmetic: +, -, /, *
// determining Binary Expression by the first two token,
// the left operand has evaluated, so the binary() is handling the rest of
// the arithmetic.
static void binary(bool canAssign) {
    // get infix token type.
    TokenType operatorType = parser.previous.type;
    // when we parse the right-hand operand, we need to worry about precedence.
    // like: 8 * 9 - 2.
    ParseRule *rule = getRule(operatorType);
    // Add one on the current operator's precedence, determining that only single
    // binary() function be call.
    // ensure left-associative.
    parsePrecedence((Precedence) (rule->precedence + 1));

    switch (operatorType) {
        case TOKEN_BANG_EQUAL:
            emitBytes(OP_EQUAL, OP_NOT);
            break;
        case TOKEN_EQUAL_EQUAL:
            emitByte(OP_EQUAL);
            break;
        case TOKEN_GREATER:
            emitByte(OP_GREATER);
            break;
        case TOKEN_GREATER_EQUAL:
            emitBytes(OP_LESS, OP_NOT);
            break;
        case TOKEN_LESS:
            emitByte(OP_LESS);
            break;
        case TOKEN_LESS_EQUAL:
            emitBytes(OP_GREATER, OP_NOT);
            break;
        case TOKEN_PLUS:
            emitByte(OP_ADD);
            break;
        case TOKEN_MINUS:
            emitByte(OP_SUBTRACT);
            break;
        case TOKEN_STAR:
            emitByte(OP_MULTIPLY);
            break;
        case TOKEN_SLASH:
            emitByte(OP_DIVIDE);
            break;
        default:
            return;
    }
}

// handle nil, false and true type.
static void literal(bool canAssign) {
    switch (parser.previous.type) {
        case TOKEN_FALSE:
            emitByte(OP_FALSE);
            break;
        case TOKEN_NIL:
            emitByte(OP_NIL);
            break;
        case TOKEN_TRUE:
            emitByte(OP_TRUE);
            break;
        default:
            return;
    }
}

// case: Unary negation -> -123
// stack order:
// 1. compile operand and push value on stack
// 2. pop value and negate it, and push the result.
static void unary(bool canAssign) {
    // pick the TokenType from previous item.
    TokenType operatorType = parser.previous.type;

    // compile the operand
    parsePrecedence(PREC_UNARY);

    // emit the operator instruction.
    // grab the token type to switch which opcode we're dealing with.
    switch (operatorType) {
        case TOKEN_BANG:
            emitByte(OP_NOT);
            break;
        case TOKEN_MINUS:
            emitByte(OP_NEGATE);
            break;
        default:
            return;
    }
}

ParseRule rules[] = {
        [TOKEN_LEFT_PAREN]    = {grouping, NULL, PREC_NONE},
        [TOKEN_RIGHT_PAREN]   = {NULL, NULL, PREC_NONE},
        [TOKEN_LEFT_BRACE]    = {NULL, NULL, PREC_NONE},
        [TOKEN_RIGHT_BRACE]   = {NULL, NULL, PREC_NONE},
        [TOKEN_COMMA]         = {NULL, NULL, PREC_NONE},
        [TOKEN_DOT]           = {NULL, NULL, PREC_NONE},
        [TOKEN_MINUS]         = {unary, binary, PREC_TERM},
        [TOKEN_PLUS]          = {NULL, binary, PREC_TERM},
        [TOKEN_SEMICOLON]     = {NULL, NULL, PREC_NONE},
        [TOKEN_SLASH]         = {NULL, binary, PREC_FACTOR},
        [TOKEN_STAR]          = {NULL, binary, PREC_FACTOR},
        [TOKEN_BANG]          = {unary, NULL, PREC_NONE},
        [TOKEN_BANG_EQUAL]    = {NULL, binary, PREC_EQUALITY},
        [TOKEN_EQUAL]         = {NULL, NULL, PREC_NONE},
        [TOKEN_EQUAL_EQUAL]   = {NULL, binary, PREC_EQUALITY},
        [TOKEN_GREATER]       = {NULL, binary, PREC_COMPARISON},
        [TOKEN_GREATER_EQUAL] = {NULL, binary, PREC_COMPARISON},
        [TOKEN_LESS]          = {NULL, binary, PREC_COMPARISON},
        [TOKEN_LESS_EQUAL]    = {NULL, binary, PREC_COMPARISON},
        [TOKEN_IDENTIFIER]    = {variable, NULL, PREC_NONE},
        [TOKEN_STRING]        = {string, NULL, PREC_NONE},
        [TOKEN_NUMBER]        = {number, NULL, PREC_NONE},
        [TOKEN_AND]           = {NULL, NULL, PREC_NONE},
        [TOKEN_CLASS]         = {NULL, NULL, PREC_NONE},
        [TOKEN_ELSE]          = {NULL, NULL, PREC_NONE},
        [TOKEN_FALSE]         = {literal, NULL, PREC_NONE},
        [TOKEN_FOR]           = {NULL, NULL, PREC_NONE},
        [TOKEN_FUN]           = {NULL, NULL, PREC_NONE},
        [TOKEN_IF]            = {NULL, NULL, PREC_NONE},
        [TOKEN_NIL]           = {literal, NULL, PREC_NONE},
        [TOKEN_OR]            = {NULL, NULL, PREC_NONE},
        [TOKEN_PRINT]         = {NULL, NULL, PREC_NONE},
        [TOKEN_RETURN]        = {NULL, NULL, PREC_NONE},
        [TOKEN_SUPER]         = {NULL, NULL, PREC_NONE},
        [TOKEN_THIS]          = {NULL, NULL, PREC_NONE},
        [TOKEN_TRUE]          = {literal, NULL, PREC_NONE},
        [TOKEN_VAR]           = {NULL, NULL, PREC_NONE},
        [TOKEN_WHILE]         = {NULL, NULL, PREC_NONE},
        [TOKEN_ERROR]         = {NULL, NULL, PREC_NONE},
        [TOKEN_EOF]           = {NULL, NULL, PREC_NONE},
};

// Pratt Parser
// in jlox, parsing lower-precedence expression include higher-precedence.
// In here, each only parse exactly one type of expression, and don't cascade to
// higher-precedence expression.
// hypothetical array is a table of function pointers.
// have two columns, One column associates prefix parser functions with token
// types. The second column associates infix parser functions with token types.
static void parsePrecedence(Precedence precedence) {
    // always prefix.
    // read next token and look up the corresponding ParseRule.
    advance();
    ParseFn prefixRule = getRule(parser.previous.type)->prefix;
    if (prefixRule == NULL) {
        error("Expect expression.");
        return;
    }

    // do something like: grouping()、unary()、number()
    // for identify the right precedence: a * b = c + d;
    bool canAssign = precedence < PREC_ASSIGNMENT;
    prefixRule(canAssign);

    // infix
    // 1 + 2 * 3
    while(precedence <= getRule(parser.current.type)->precedence) {
        advance();
        ParseFn infixRule = getRule(parser.previous.type)->infix;
        infixRule(canAssign);
    }

    // nothing to do, because of not consuming the token of equal.
    if (canAssign && match(TOKEN_EQUAL)) {
        error("Invalid assignment target.");
    }
}

// add its lexeme to constant table constant as string and return ix;
static uint8_t identifierConstant(Token* name) {
    ObjString* string = copyString(name->start, name->length);
    Value indexValue;
    if (tableGet(&stringConstants, string, &indexValue)) {
        return (uint8_t) AS_NUMBER(indexValue);
    }

    uint8_t ix = makeConstant(OBJ_VAL(string));
    tableSet(&stringConstants, string, NUMBER_VAL((double )ix));
    return ix;
}

// consume current IDENTIFIER TOKEN, and add the token name in constant table.
static uint8_t parseVariable(const char* errorMessage) {
    consume(TOKEN_IDENTIFIER, errorMessage);
    return identifierConstant(&parser.previous);
}

// store in stack from constant table ix.
static void defineVariable(uint8_t global) {
    emitBytes(OP_DEFINE_GLOBAL, global);
}

static ParseRule *getRule(TokenType type) {
    return &rules[type];
}

static void expression() {
    parsePrecedence(PREC_ASSIGNMENT);
}

// varDecl        → "var" IDENTIFIER ("=" expression ) ? ";" ;
static void varDeclaration() {
    uint8_t global = parseVariable("Expected variable name.");

    // if match '=', then call expression(), otherwise implicitly initializes it to nil.
    if (match(TOKEN_EQUAL)) {
        expression();
    } else {
        emitByte(OP_NIL);
    }

    consume(TOKEN_SEMICOLON, "Excepted ';' after variable expression.");
    defineVariable(global);
}

static void expressionStatement() {
    expression();
    consume(TOKEN_SEMICOLON, "Expected ';' after expression.");
    // since statement produces no values, so it ultimately leaves the stack unchanged,
    // evaluates the expression and discard the result. example: eat("apple");
    emitByte(OP_POP);
}

static void printStatement() {
    expression();
    consume(TOKEN_SEMICOLON, "Expect ';' after value.");
    emitByte(OP_PRINT);
}

/**
  * @brief  when we hit a compile error, enter the panic mode.
  * @note   None
  * @param  None
  * @retval None
  */
static void synchronize() {
    parser.panicMode = false;
    while (parser.current.type != TOKEN_EOF) {
        if (parser.previous.type == TOKEN_SEMICOLON) return;
        switch (parser.current.type) {
            case TOKEN_CLASS:
            case TOKEN_FUN:
            case TOKEN_VAR:
            case TOKEN_FOR:
            case TOKEN_IF:
            case TOKEN_WHILE:
            case TOKEN_PRINT:
            case TOKEN_RETURN:
                return;
            default:
                ;
        }
    }
    // recover the next normal statement or expression, avoid the cascaded compile errors.
    advance();
}

static void declaration() {
    if (match(TOKEN_VAR)) {
        varDeclaration();
    } else {
        statement();
    }


    if (parser.panicMode) synchronize();
}

// blocks can contain declaration,  control flow statements too. result in recursive.
static void statement() {
    if (match(TOKEN_PRINT)) {
        printStatement();
    } else {
        expressionStatement();
    }
}


bool compile(const char *source, Chunk *chunk) {
    initScanner(source);
    compilingChunk = chunk;

    parser.hadError = false;
    parser.panicMode = false;

    initTable(&stringConstants);

    advance();
    while (!match(TOKEN_EOF)) {
        declaration();
    }
//    expression();
//    consume(TOKEN_EOF, "Expected end of expression.");

    endCompiler();
    freeTable(&stringConstants);
    return !parser.hadError;
}