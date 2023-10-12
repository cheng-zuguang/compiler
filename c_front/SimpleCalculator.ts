import { ASTNode } from './ASTNode';
import { ASTNodeType, TokenType } from './global_var';
import { SimpleToken } from './parser';
import { TokenReader } from './simpleReader';
import { SimpleLexer } from './parser';

/**
* 一个简单的AST节点的实现。
* 属性包括：类型、文本值、父节点、子节点。
*/
// export class SimpleASTNode implements ASTNode {
//     children: ASTNode[] = [];
//     readonlyChildren: readonly ASTNode[] = this.children;
//     nodeType: ASTNodeType = null;
//     text: string = null;

//     constructor(nodeType: ASTNodeType, text: string) {
//         this.nodeType = nodeType;
//         this.text = text;
//     }

// }


class SimpleCalculator {

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
        const node = new ASTNode(ASTNodeType.Programm, "Calculator");

        const child = this.additive(tokens);

        if (child != null) {
            node.addChild(child);
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
                node = new ASTNode(ASTNodeType.Identifier, token.getText());
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
            while(true) {
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
}

// const calculator = new SimpleCalculator();

// let script = "int a = b + 3;";
// console.log("解析变量声明语句: " + script);

// const lexer = new SimpleLexer();
// const tokens = lexer.tokenize(script);
// try {
//     const node = calculator.intDeclare(tokens);
//     calculator.dumpAST(node, "");
// } catch (e) {
//     console.error(e);
// }

// let script = "2+3*5";
// console.log("\n计算: " + script + "，看上去一切正常。");
// calculator.evaluate(script);

//  //测试语法错误
//  script = "2+3+4";
//  console.log("\n: " + script + "，应该有语法错误。");
//  calculator.evaluate(script);

