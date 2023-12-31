##### chapter 5
- **postfix**
  - `*`: repeated zero or more times
  - `+`: at least once, certain or more times
  - `?`: zero or one time
##### chapter 6
- **Rules**
  - precedence(lower -> higher)
  - expression     → equality ;
  - equality       → comparison ( ("!=" | "==") comparison )* ;
  - comparison     → term ( (">" | ">=" | "<" | "<=" ) term )*;
  - term           → factor ( ( "-" | "+" ) factor )* ;
  - factor         → unary ( ( "/" | "*" ) unary )* ;
  - unary          → ( "-" | "!" ) unary | primary;
  - primary        → NUMBER | STRING | "true" | "false" | "nil" | "(" expression ")" ;
  
- [About Syntax Errors](http://www.craftinginterpreters.com/parsing-expressions.html#syntax-errors)
  - Detect and report the error
  - Avoid crashing or hanging
  
- **About Parser**:
  - Fast.
  - Report as many distinct errors as there are. Report all the problem not just ones.
  - Minimize cascaded errors. track the real problem's code, not ghost errors.
  
- **CHALLENGES**

  - C1
    - ```
      expression -> comma ;
      comma      -> equality ( "," equality )* ;
      other rules...
      ```

    - ```java
      private Expr comma() {
      Expr expr = equality();
      
      while (match(COMMA)) {
          Token operator = previous();
          Expr right = equality();
      
          expr = new Expr.Binary(expr, operator, right);
      }
      
      return expr;
      }
      ```

  - C2

      - ```
          expression  -> conditional;
          conditional -> equality ( "?" expression ":" conditional )? ;
          ```

      - ```java
          private Expr expression() {
            return conditional();
          }
          
          private Expr conditional() {
            Expr expr = equality();
          
            if (match(QUESTION)) {
              Expr thenBranch = expression();
              consume(COLON,
                  "Expect ':' after then branch of conditional expression.");
              Expr elseBranch = conditional();
              expr = new Expr.Conditional(expr, thenBranch, elseBranch);
            }
          
            return expr;
          }
          ```

  - C3

      - ```
          ...other rules
          priamry -> NUMBER | STRING | "true" | "false" | "nil"
                     | "(" expression ")"
          		    // error productions
          		   | ("!=" | "==") equality
          		   | (">" || ">=" || "<" || "<=") comparsion
          		   | ("+") term
          		   | ("/" | "*") factor ;
          ```

      - ```java
          private Expr primary() {
                  // ...
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
          ```


  ##### chapter 7

  - **CHALLENGES**

    - c1

      - ```java
        // suport string comapre
        public Object visitBinaryExpr(Expr.Binary expr) {
            Object left = evaluate(expr.left);
            Object right = evaluate(expr.right);
            
            switch (expr.operator.type) {
                    ...
                    case GREATER:
                   		checkNumberOperand(expr.operator, left, right);
                        if (left instanceof String && right instanceof String && (left != null)) {
                        	return ((String) left).compareTo((String) right) > 0;
                        }
                       	return (double) left > (double) right;
                    ...
            }
        }
        
        private void checkNumberOperand(Token operator, Object left, Object right) {
                if ((left instanceof Double && right instanceof Double) || (left instanceof String && right instanceof String)) return;
        
                throw new RuntimeError(operator, "operand must be an number or string");
        }
        ```

      - 

    - c2

      - ```java
        // suport string or number concat
        public Object visitBinaryExpr(Expr.Binary expr) {
            Object left = evaluate(expr.left);
            Object right = evaluate(expr.right);
            
            switch (expr.operator.type) {
                    ...
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
                    ...
            }
        }
        
        ```

    - c3

      - ```java
        public Object visitBinaryExpr(Expr.Binary expr) {
            Object left = evaluate(expr.left);
            Object right = evaluate(expr.right);
            
            switch (expr.operator.type) {
                    ...
                    case SLASH:
                   		checkNumberOperand(expr.operator, left, right);
                        detectDivisorIsZero(expr.operator, right);
                        return (double) left / (double) right;
                    ...
            }
        }
        
        private void detectDivisorIsZero(Token operator, Object obj) {
        	if ((double) obj == 0) {
            	throw new RuntimeError(operator, "divisor could not be zero.");
        	}
        }
        ```

##### chapter 8

- [challenges](https://github.com/munificent/craftinginterpreters/blob/master/note/answers/chapter08_statements.md)

##### chapter 9

- c3

  - ```java
    statement      → exprStmt
        		   | breakStmt
                   | forStmt
                   | ifStmt
                   | printStmt
                   | whileStmt
                   | block ;
    ```

  - in GenerateAST.java

    - ```java
      // update defineAst in main
      defineAst(outputDir, "Stmt", Arrays.asList(
                      "Block      : List<Stmt> statements",
         				"Break		:",
                      "Expression : Expr expression",
                      "If         : Expr condition,Stmt thenBranch,Stmt elseBranch",
                      "Print      : Expr expression",
                      "Var        : Token name,Expr initialize",
                      "While      : Expr condition,Stmt body"
              ));
      
      // update defineType func
      private static void defineType(PrintWriter writer, String baseName, String className, String fieldsList) {
          // ...
          // replace 'String[] fields = fieldList.split(", ");'
          String[] fields;
          if (fieldList.isEmpty()) {
            fields = new String[0];
          } else {
            fields = fieldList.split(", ");
          }
      	// ...    
      }
      ```

  - In TokenType.java

    - ```java
      // insert type: BREAK
      public enum TokenType {
      	// ...
          // KEYWORDS
          AND, BREAK, CLASS, ELSE, FALSE, FUN, FOR, IF, NIL, OR, PRINT, RETURN, SUPER, THIS, VAR, WHILE,TRUE,
      	// ...    
      }
      ```

  - In Scanner.java

    - ```java
      public class Scanner{
          // ...
          static {
              keywords = new HashMap<>();
              // ...
              keywords.put("break", BREAK);
          }
      }
      ```

  - In Paser.java

    - ```java
      public class Parser {
          // 记录当前循环的嵌套深度
          private int loopDepth = 0; 
          
          statement() {
              // ...
              if (match(BREAK)) return breakStatement();
              
              // ...
          }
          private Stmt breakStatement() {
              if (loopDepth == 0) {
                  error(previous(), "Must be inside a loop to use 'break'.");
              }
              
              consume(SEMICOLON, "except ';' after break.");
              return new Stmt.Break();
          }
          
          private Stmt forStatement() {
              try {
              	loopDepth++;
                  // ...
              } finally {
                  loopDepth--;
              }
          }
          
          private Stmt whileStatement() {
              try {
              	loopDepth++;
                  // ...
              } finally {
                  loopDepth--;
              }
          }
      }
      ```

  - In interpreter.java

    - ```java
      public class Interpreter implements Expr.Visitor<Object>, Stmt.Visitor<Void> {
      	// add BreakException
          private static class BreakException extends RuntimeException {}
          // ...
          public Void visitBreakStmt(Stmt.Break stmt){
              throw new BreakException();
          }
          
          // update visitWhileStmt()
          public Void visitWhileStmt(Stmt.While stmt) {
              try {
                  while (isTruthy(evaluate(stmt.condition))) {
                  	execute(stmt.body);
              	}
              } catch(BreakException e) {
                  System.out.println("break error");
              }
              
              return null;
          }
      }
      ```

  
  ##### chapter 10
  
  - [Challenge](https://github.com/munificent/craftinginterpreters/blob/master/note/answers/chapter10_functions.md)


