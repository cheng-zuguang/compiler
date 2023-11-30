//
// Created by 42134 on 2023/11/14.
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
 *      block       -> "{" declaration* "}" ;
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

typedef struct {
    // variable name.
    Token name;
    // current scope depth of the block where the local variable was declared.
    int depth;
} Local;

typedef struct {
    Local locals[UINT8_COUNT];
    // track how many locals are in scope.
    int localCount;
    // the number of blocks surrounding the current bit of code
    int scopeDepth;
} Compiler;

static ParseRule *getRule(TokenType type);
static void parsePrecedence();
static void expression();
static void and_(bool canAssign);

static uint8_t identifierConstant(Token* name);
static int resolveLocal(Compiler* compiler, Token* name);

Parser parser;
Compiler* current = NULL;
Chunk *compilingChunk;

int innermostLoopStart = -1;
int innermostLoopScopeDepth = 0;

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

static void emitLoop(int loopStart) {
    emitByte(OP_LOOP);

    // the +2 is to take account the size of the op_loop instruction's own operand which we also need to jump over.
    int offset = currentChunk()->count - loopStart + 2;
    if (offset > UINT16_MAX) error("Loop body too large.");

    emitByte((offset >> 8) & 0xff);
    emitByte(offset & 0xff);
}

// back-patching(回填)
// emit the jump instruction first with placeholder offset operand.
// then we compile the then body, we know how far to jump.
// so we go back and replace that placeholder offset operand.
static int emitJump(uint8_t instruction) {
    emitByte(instruction);
    // two bits as jump offset, range(0 - 65,535)
    emitByte(0xff);
    emitByte(0xff);
    return currentChunk()->count - 2;
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

static void patchJump(int offset) {
    // -2 to adjust for the bytecode for the jump offset itself.
    // current - (beforeCurrent -2) - 2 = current - beforeCurrent
    // equal the distance of jump offset to code block.
    int jump = currentChunk()->count - offset - 2;

    if (jump > UINT16_MAX) {
        error("Too much code to jump.");
    }

    // store the distance of the jump separately
    currentChunk()->code[offset] = (jump >> 8) & 0xff;
    currentChunk()->code[offset + 1] = jump & 0xff;
}

static void initCompiler(Compiler* compiler) {
    compiler->localCount = 0;
    compiler->scopeDepth = 0;
    current = compiler;
}

static void endCompiler() {
    emitReturn();
#ifdef DEBUG_PRINT_CODE
    if (!parser.hadError) {
        disassembleChunk(currentChunk(), "code");
    }
#endif
}

static void beginScope() {
    current->scopeDepth++;
}

static void endScope() {
    current->scopeDepth--;

    // when we pop a scope, we walk through the local array looking for any variable declared at the scope depth
    // we just left.
    while (current->localCount > 0 &&
        current->locals[current->localCount - 1].depth > current->scopeDepth
    ) {
        // pop local variable.
        emitByte(OP_POP);
        current->localCount--;
    }
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

static void or_(bool canAssign) {
    int elseJump = emitJump(OP_JUMP_IF_FALSE);
    int endJump = emitJump(OP_JUMP);

    // when the left operand if falsey, then we unconditional jump over the code for the right operand.
    // otherwise skip the right operand.
    patchJump(elseJump);
    emitByte(OP_POP);

    parsePrecedence(PREC_OR);
    patchJump(endJump);
}

// case: str-variable.
static void string(bool canAssign) {
    // trim the leading and trailing quotation marks.
    emitConstant(OBJ_VAL(copyString(parser.previous.start + 1, parser.previous.length - 2)));
}

static void namedVariable(Token name, bool canAssign) {
//    uint8_t arg = identifierConstant(&name);
    uint8_t getOp, setOp;
    // try to find local variable with the give name.
    int arg = resolveLocal(current, &name);
    if (arg != -1) {
        getOp = OP_GET_LOCAL;
        setOp = OP_SET_LOCAL;
    } else {
        arg = identifierConstant(&name);
        getOp = OP_GET_GLOBAL;
        setOp = OP_SET_GLOBAL;
    }


    if (canAssign && match(TOKEN_EQUAL)) {
        // if we see the =, we compile it as an assignment or setter instead of variable access or getter.
        expression();
        emitBytes(setOp, (uint8_t) arg);
    } else {
        emitBytes(getOp, (uint8_t) arg);
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
        [TOKEN_AND]           = {NULL, and_, PREC_AND},
        [TOKEN_CLASS]         = {NULL, NULL, PREC_NONE},
        [TOKEN_ELSE]          = {NULL, NULL, PREC_NONE},
        [TOKEN_FALSE]         = {literal, NULL, PREC_NONE},
        [TOKEN_FOR]           = {NULL, NULL, PREC_NONE},
        [TOKEN_FUN]           = {NULL, NULL, PREC_NONE},
        [TOKEN_IF]            = {NULL, NULL, PREC_NONE},
        [TOKEN_NIL]           = {literal, NULL, PREC_NONE},
        [TOKEN_OR]            = {NULL, or_, PREC_OR},
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
    return makeConstant(OBJ_VAL(copyString(name->start, name->length)));
}

static bool identifiersEqual(Token* a, Token* b) {
    if (a->length != b->length) return false;
    return memcmp(a->start, b->start, a->length) == 0;
}

// we begin the current scope, find the local variable with the given name
// we walk the array backward that ensures the inner local variable correctly shadow locals with the same name in
// surrounding scopes.
static int resolveLocal(Compiler* compiler, Token* name) {
    for (int i = compiler->localCount - 1; i >= 0; i--) {
        Local* local = &compiler->locals[i];
        if (identifiersEqual(name, &local->name)) {
            if (local->depth == -1) {
                error("Can't read local variable in its own initializer.");
            }
            return i;
        }
    }

    return -1;
}

static void addLocal(Token name) {
    if (current->localCount == UINT8_COUNT) {
        error("Too many local variable in function.");
        return;
    }

    Local* local = &current->locals[current->localCount++];
    local->name = name;
//    local->depth = current->scopeDepth;
    // indicate the variable uninitialized
    local->depth = -1;
}

static void declareVariable() {
    // if we are in the top level global scope, exit.
    if (current->scopeDepth == 0) return;

    // add it to the compiler's list of variables in the current scope.
    Token* name = &parser.previous;
    // Local variables are appended to array when they are declared.
    // so current scope is always at the end of the array.
    for (int i = current->localCount - 1; i >= 0; i--) {
        Local* local = &current->locals[i];
        // for shadowing:
        // {
        //      var a = 1;
        //      {   var a = 2; }
        // }
        if (local->depth != -1 && local->depth < current->scopeDepth) {
            break;
        }

        // same the scope has same the variable.
        if (identifiersEqual(name, &local->name)) {
            error("Already a variable with this name in this scope.");
        }
    }

    addLocal(*name);
}

// consume current IDENTIFIER TOKEN, and add the token name in constant table.
static uint8_t parseVariable(const char* errorMessage) {
    consume(TOKEN_IDENTIFIER, errorMessage);

    // support local variables.
    declareVariable();
    //  so if the declaration is inside a local scope, we return a dummy table index instead.
    if (current->scopeDepth > 0) return 0;

    return identifierConstant(&parser.previous);
}

// the "declaring" is diff with "defining".
// "Declaring" is when the variable is added to the scope
// "Define" is when it becomes available for use.
static void markInitialized() {
    current->locals[current->localCount - 1].depth = current->scopeDepth;
}

// store in stack from constant table ix.
static void defineVariable(uint8_t global) {
    if (current->scopeDepth > 0) {
        markInitialized();
        return;
    }

    emitBytes(OP_DEFINE_GLOBAL, global);
}

static void and_(bool canAssign) {
    int endJump = emitJump(OP_JUMP_IF_FALSE);

    emitByte(OP_POP);
    parsePrecedence(PREC_AND);

    // if the left value is false, then will jump right operand.
    patchJump(endJump);
}

static ParseRule *getRule(TokenType type) {
    return &rules[type];
}

static void expression() {
    parsePrecedence(PREC_ASSIGNMENT);
}

// block       -> "{" declaration* "}" ;
static void block() {
    while (!check(TOKEN_RIGHT_BRACE) && !check(TOKEN_EOF)) {
        declaration();
    }

    consume(TOKEN_RIGHT_BRACE, "Expected '}' after block.");
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
// forStmt    →   "for" "("  ( varDecl | exprStmt | ";" ) expression? ";" expression? ")" statement ;
static void forStatement() {
    beginScope();
    consume(TOKEN_LEFT_PAREN, "Excepted '(' after the for.");

    // initializer
    if (match(TOKEN_SEMICOLON)) {
        // no initializer
    } else if (match(TOKEN_VAR)) {
        varDeclaration();
    } else {
        expressionStatement();
    }

//    int loopStart = currentChunk()->count;

    int surroundingLoopStart = innermostLoopStart;
    int surroundingLoopScopeDepth = innermostLoopScopeDepth;
    innermostLoopStart = currentChunk()->count;
    innermostLoopScopeDepth = current->scopeDepth;

    // exit the loop
    int exitJump = -1;
    if (!match(TOKEN_SEMICOLON)) {
        expression();
        consume(TOKEN_SEMICOLON, "Expect ';' after loop condition.");

        // jump out of the loop if the condition is false.
        exitJump = emitJump(OP_JUMP_IF_FALSE);
        emitByte(OP_POP);
    }

    if (!match(TOKEN_RIGHT_PAREN)) {
        int bodyJump = emitJump(OP_JUMP);
        int incrementStart = currentChunk()->count;
        expression();
        emitByte(OP_POP);
        consume(TOKEN_RIGHT_PAREN, "Excepted ')' after clauses.");

        // jump over the increment, run the body, and jump back up the increment, run it.
        emitLoop(innermostLoopStart);
        innermostLoopStart = incrementStart;
        patchJump(bodyJump);
    }

    statement();
    emitLoop(innermostLoopStart);

    if (exitJump != -1) {
        patchJump(exitJump);
        emitByte(OP_POP);
    }

    innermostLoopStart = surroundingLoopStart;
    innermostLoopScopeDepth = surroundingLoopScopeDepth;

    endScope();
}

static void continueStatement() {
    if (innermostLoopStart == -1) {
        error("Can't use 'continue' outside of loop.");
    }

    consume(TOKEN_SEMICOLON, "Expect ';' after continue.");

    // discard any locals created inside loop.
    for (
            int i = current->localCount - 1;
            i >= 0 && current->locals[i].depth > innermostLoopScopeDepth;
            i--
            ) {
        emitByte(OP_POP);
    }

    // jump to the top of current innermost loop.
    emitLoop(innermostLoopStart);
}

// ifStmt         → "if" "(" expression ")" statement
//                  ( "else" statement )? ;
static void ifStatement() {
    consume(TOKEN_LEFT_PAREN, "Excepted '(' after the if.");
    expression();
    consume(TOKEN_RIGHT_PAREN, "Excepted ')' after the condition.");

    // the operand for how much offset the ip will skip. (for then branch)
    int thenJump = emitJump(OP_JUMP_IF_FALSE);
    emitByte(OP_POP);
    statement();

    // (for else branch)
    int elseBranch = emitJump(OP_JUMP);

    patchJump(thenJump);
    emitByte(OP_POP);

    if (match(TOKEN_ELSE)) statement();

    patchJump(elseBranch);
}

static void printStatement() {
    expression();
    consume(TOKEN_SEMICOLON, "Expect ';' after value.");
    emitByte(OP_PRINT);
}

// whileStmt      → "while" "(" expression ")" statement ;
static void whileStatement() {
    int loopStart = currentChunk()->count;
    consume(TOKEN_LEFT_PAREN, "Excepted '(' after the while.");
    expression();
    consume(TOKEN_RIGHT_PAREN, "Excepted ')' after the condition.");

    int exitJump = emitJump(OP_JUMP_IF_FALSE);
    emitByte(OP_POP);
    statement();
    // jumping forward.
    emitLoop(loopStart);

    patchJump(exitJump);
    emitByte(OP_POP);
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
    } else if (match(TOKEN_IF)) {
        ifStatement();
    } else if (match(TOKEN_WHILE)) {
        whileStatement();
    } else if (match(TOKEN_FOR)) {
        forStatement();
    } else if (match(TOKEN_CONTINUE)) {
        continueStatement();
    } else if (match(TOKEN_LEFT_BRACE)) {
        beginScope();
        block();
        endScope();
    } else {
        expressionStatement();
    }
}


bool compile(const char *source, Chunk *chunk) {
    initScanner(source);

    Compiler compiler;
    initCompiler(&compiler);

    compilingChunk = chunk;

    parser.hadError = false;
    parser.panicMode = false;

    advance();
    while (!match(TOKEN_EOF)) {
        declaration();
    }
//    expression();
//    consume(TOKEN_EOF, "Expected end of expression.");

    endCompiler();
    return !parser.hadError;
}