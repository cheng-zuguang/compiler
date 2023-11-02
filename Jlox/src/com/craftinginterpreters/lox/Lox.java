package com.craftinginterpreters.lox;
import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.nio.charset.Charset;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.util.List;


public class Lox {
    static boolean hadError = false;
    static boolean hadRuntimeError = false;

    private static final Interpreter interpreter = new Interpreter();

    public static void main(String[] args) throws IOException{
        if (args.length > 1) {
            System.out.println("Usage: Jlox [script]");
            System.exit(64);
        } else if (args.length == 1) {
            runFile(args[0]);
        } else {
            runPrompt();
        }
    }

    // 直接从源文件执行脚本语言
    private static void runFile(String path) throws IOException {
        byte[] bytes = Files.readAllBytes(Paths.get((path)));
        System.out.println(path);

        // 让已出现问题的代码不再执行
        if (hadError) System.exit(65);
        if (hadRuntimeError) System.exit(70);

        run(new String(bytes, Charset.defaultCharset()));
    }

    // 从命令行读取并执行
    private static void runPrompt() throws IOException {
        InputStreamReader input = new InputStreamReader(System.in);
        BufferedReader reader = new BufferedReader(input);

        for (;;) {
            System.out.print(">>");
            String line = reader.readLine();
            if (line == null) break;
            run(line);

            // 重置错误标识，报错但不影响下次命令的执行
            hadError = false;
        }
    }

    private static void run(String source) {
        Scanner scanner = new Scanner(source);
        List<Token> tokens = scanner.scanTokens();
        Parser parser = new Parser(tokens);
        List<Stmt> statements = parser.parse();
//        Expr expression = parser.parse();

        // 中止运行
        if (hadError) return;

        Resolver resolver = new Resolver(interpreter);
        resolver.resolve(statements);

        interpreter.interpret(statements);

//        System.out.println(new AstPrinter().print(expression));

//        for (Token token : tokens) {
//            System.out.println(token);
//        }
    }

    static void error(int line, String message) {
        report(line, "", message);
    }

    // error handling
    public static void error(Token token, String message) {
        if (token.type == TokenType.EOF) {
            report(token.line, " at end", message);
        } else {
            report(token.line, " at '" + token.lexeme + "'", message);
        }
    }

    public static void runtimeError(RuntimeError error) {
        System.out.println(error.getMessage() + "\n[line " + error.token.line + "]");
        hadRuntimeError = true;
    }

    private static void report(int line, String where, String message) {
        System.err.println("[line " + line + "] Error" + where + ": " + message);
        hadError = true;
    }
}
