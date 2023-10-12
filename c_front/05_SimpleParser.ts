import { ASTNode } from './ASTNode';
import { ASTNodeType, TokenType } from './global_var';
import { SimpleToken } from './parser';
import { TokenReader } from './simpleReader';
import { SimpleLexer } from './parser';

export class SimpleParser {

    public evaluate(script: string) {
        try {
            const tree = this.parse(script);
            this.dumpAST(tree, "");
            this._evaluate(tree, "");
        } catch (err) {
            console.log(err);
        }
    }

    /**
     * 打印输出AST的树状结构
     * @param node
     * @param indent 缩进字符，由tab组成，每一级多一个tab
     */
    public dumpAST(node: ASTNode, indent: string) {
        console.log(indent + node.getType() + " " + node.getText());
        for (const child of node.getChildren()) {
            this.dumpAST(child, indent + "\t");
        }
    }

    /**
     * 解析脚本，并返回根节点
     * @param code
     * @return
     * @throws Exception
     */
    public parse(code: string): ASTNode {
        const lexer = new SimpleLexer();
        const tokens = lexer.tokenize(code);
        const rootNode = this.prog(tokens);

        return rootNode;
    }

    /**
   * 语法解析：根节点
   * @return
   * @throws Exception
   */
    private prog(tokens: TokenReader): ASTNode {
        const node = new ASTNode(ASTNodeType.Programm, "pwc");

        while (tokens.peek() != null) {
            let child = this.intDeclare(tokens);

            if (child === null) {
                child = this.expressionStatement(tokens);
            }

            if (child === null) {
                child = this.assignmentStatement(tokens);
            }

            if (child !== null) {
                node.addChild(child);
            } else {
                throw new Error("unknow statement");
            }
        }


        return node;
    }

    /**
  * 对某个AST节点求值，并打印求值过程。
  * @param node
  * @param indent  打印输出时的缩进量，用tab控制
  * @return
  */
    private _evaluate(node: ASTNode, indent: string): number {
        let result = 0;
        console.log(indent + "Calculating: " + node.getType());
        switch (node.getType()) {
            case ASTNodeType.Programm:
                for (const child of node.getChildren()) {
                    result = this._evaluate(child, indent + "\t");
                }
                break;
            case ASTNodeType.Additive:
                let child1: ASTNode = node.getChildren()[0];
                let val1 = this._evaluate(child1, indent + "\t");
                let child2: ASTNode = node.getChildren()[1];
                let val2 = this._evaluate(child2, indent + "\t");
                if (node.getText() === '+') {
                    result = val1 + val2;
                } else {
                    result = val1 - val2;
                }
                break;
            case ASTNodeType.Multiplicative:
                child1 = node.getChildren()[0];
                val1 = this._evaluate(child1, indent + "\t");
                child2 = node.getChildren()[1];
                val2 = this._evaluate(child2, indent + "\t");
                if (node.getText() === '*') {
                    result = val1 * val2;
                } else {
                    result = val1 / val2;
                }
                break;
            case ASTNodeType.IntLiteral:
                result = parseInt(node.getText());
                break;
            default:
        }
        console.log(indent + "Result: " + result);
        return result;
    }

    public intDeclare(tokens: TokenReader): ASTNode {
        let node: ASTNode = null;
        let token: SimpleToken = tokens.peek();

        if (token !== null && token.getType() == TokenType.Int) {
            token = tokens.read();
            if (tokens.peek().getType() === TokenType.Identifier) {
                token = tokens.read();
                node = new ASTNode(ASTNodeType.IntDeclaration, token.getText());
                token = tokens.peek();
                if (token !== null && token.getType() === TokenType.Assignment) {
                    tokens.read();
                    const child: ASTNode = this.additive(tokens);
                    if (child == null) {
                        throw new Error("invalide variable initialization, expecting an expression");
                    } else {
                        node.addChild(child);
                    }
                }
            } else {
                throw new Error("variable name expected");
            }

            if (node !== null) {
                token = tokens.peek();
                if (token !== null && token.getType() === TokenType.SemiColon) {
                    tokens.read();
                } else {
                    throw new Error("invalid statement, expecting semicolon");
                }
            }

        }

        return node;

    }

    private additive(tokens: TokenReader): ASTNode {
        let child1: ASTNode = this.multiplicative(tokens);
        let node: ASTNode = child1;

        // let token: SimpleToken = tokens.peek();
        // if (child1 !== null && token !== null) {
        //     if (token.getType() === TokenType.Plus || token.getType() === TokenType.Minus) {
        //         token = tokens.read();
        //         let child2: ASTNode = this.additive(tokens);
        //         console.log(tokens, child2);

        //         if (child2 !== null) {
        //             node = new ASTNode(ASTNodeType.Additive, token.getText());
        //             node.addChild(child1);
        //             node.addChild(child2);
        //         } else {
        //             throw new Error("invalid additive expression, expecting the right part.");
        //         }
        //     }
        // }
        if (child1 !== null) {
            while (true) {
                let token = tokens.peek();
                if (token !== null && (token.getType() === TokenType.Plus || token.getType() === TokenType.Minus)) {
                    token = tokens.read();
                    const child2 = this.multiplicative(tokens);
                    node = new ASTNode(ASTNodeType.Additive, token.getText());
                    node.addChild(child1);
                    node.addChild(child2);
                    child1 = node;
                } else {
                    break;
                }
            }
        }

        return node;
    }

    /**
    * 语法解析：乘法表达式
    * @return
    * @throws Exception
    */
    private multiplicative(tokens: TokenReader): ASTNode {
        let child1: ASTNode = this.primary(tokens);
        let node: ASTNode = child1;

        let token: SimpleToken = tokens.peek();
        if (child1 !== null && token !== null) {
            if (token.getType() === TokenType.Star || token.getType() === TokenType.Slash) {
                token = tokens.read();
                let child2: ASTNode = this.multiplicative(tokens);
                if (child2 !== null) {
                    node = new ASTNode(ASTNodeType.Multiplicative, token.getText());
                    node.addChild(child1);
                    node.addChild(child2);
                } else {
                    throw new Error("invalid multiplicative expression, expecting the right part.");
                }
            }
        }
        return node;
    }

    /**
     * 语法解析：基础表达式
     * @return
     * @throws Exception
     */
    private primary(tokens: TokenReader): ASTNode {
        let node: ASTNode = null;
        let token: SimpleToken = tokens.peek();

        if (token !== null) {
            if (token.getType() === TokenType.IntLiteral) {
                token = tokens.read();
                node = new ASTNode(ASTNodeType.IntLiteral, token.getText());
            } else if (token.getType() === TokenType.Identifier) {
                token = tokens.read();
                node = new ASTNode(ASTNodeType.Identifier, token.getText());
            } else if (token.getType() === TokenType.LeftParen) {
                token = tokens.read();
                node = this.additive(tokens);
                if (node !== null) {
                    token = tokens.peek();
                    if (token !== null && token.getType() === TokenType.RightParen) {
                        tokens.read();
                    } else {
                        throw new Error("expecting right parenthesis");
                    }
                } else {
                    throw new Error("expecting an additive expression inside parenthesis");
                }
            }
        }
        return node;

    }

    /**
     * 表达式语句，即表达式后面跟个分号。
     * @return
     * @throws Exception
     */
    private expressionStatement(tokens: TokenReader): ASTNode {
        let pos = tokens.getPosition();
        let node = this.additive(tokens);

        if (node !== null) {
            const token = tokens.peek();
            if (token !== null && token.getType() === TokenType.SemiColon) {
                tokens.read();
            } else {
                node = null;
                tokens.setPosition(pos);
            }
        }

        return node;
    }

    /**
    * 赋值语句，如age = 10*2;
    * @return
    * @throws Exception
    */
    private assignmentStatement(tokens: TokenReader): ASTNode {
        let node: ASTNode = null;
        let token = tokens.peek();
        if (token !== null && token.getType() === TokenType.Identifier) {
            token = tokens.read();
            node = new ASTNode(ASTNodeType.AssignmentStmt, token.getText());
            token = tokens.peek();

            if (token !== null && token.getType() === TokenType.Assignment) {
                tokens.read();
                let child = this.additive(tokens);
                if (child === null) {
                    throw new Error("invalid assignment, expecting an expression");
                } else {
                    node.addChild(child);
                    token = tokens.peek();
                    if (token !== null && token.getType() === TokenType.SemiColon) {
                        tokens.read();
                    } else {
                        throw new Error("少个分号");
                    }
                }
            } else {
                tokens.unread();
                node = null;
            }
        }

        return node;
    }
}
