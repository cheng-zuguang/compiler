// import reple from 'repl';
import * as process from 'process';
import * as readline from 'readline';

import { ASTNode } from './ASTNode';
import { ASTNodeType } from './global_var';
import { SimpleParser } from './05_SimpleParser';

export class SimpleScript {
    private variables = new Map<string, number>();
    private verbose = false;

    constructor(vb?: boolean) {
        this.verbose = !!vb;
    }

    /**
    * 遍历AST，计算值。
    * @param node
    * @param indent
    * @return
    * @throws Exception
    */
    public evalute(node: ASTNode, indent: string): number {
        let result: number = null;
        let child1: ASTNode = null;
        let value1: number;
        let child2: ASTNode = null;
        let value2: number;
        let varName: string;

        if (this.verbose) {
            console.log(indent + "Calculating: " + node.getType());
        }

        switch (node.getType()) {
            case ASTNodeType.Programm:
                for (const child of node.getChildren()) {
                    result = this.evalute(child, indent);
                }
                break;
            case ASTNodeType.Additive:
                child1 = node.getChildren()[0];
                value1 = this.evalute(child1, indent + '\t');
                child2 = node.getChildren()[1];
                value2 = this.evalute(child2, indent + '\t');
                if (node.getText() === '+') {
                    result = value1 + value2;
                } else {
                    result = value1 - value2;
                }
                break;
            case ASTNodeType.Multiplicative:
                child1 = node.getChildren()[0];
                value1 = this.evalute(child1, indent + '\t');
                child2 = node.getChildren()[1];
                value2 = this.evalute(child2, indent + '\t');
                if (node.getText() === '*') {
                    result = value1 * value2;
                } else {
                    result = value1 / value2;
                }
                break;
            case ASTNodeType.IntLiteral:
                result = parseInt(node.getText());
                break;
            case ASTNodeType.Identifier:
                varName = node.getText();
                if (this.variables.has(varName)) {
                    const value = this.variables.get(varName);
                    if (value !== null) {
                        result = value;
                    } else {
                        throw new Error("variable " + varName + " has not been set any value");
                    }
                } else {
                    throw new Error("unknow variable: " + varName);
                }
                break;
            case ASTNodeType.AssignmentStmt:
                varName = node.getText();
                if (!this.variables.has(varName)) {
                    throw new Error("unknown variable: " + varName);
                }
            case ASTNodeType.IntDeclaration:
                varName = node.getText();
                let varValue: number = null;
                if (node.getChildren().length > 0) {
                    let child = node.getChildren()[0];
                    result = this.evalute(child, indent + "\t");
                    varValue = result;
                }
                this.variables.set(varName, varValue);
                break;
            default:
        }

        if (this.verbose) {
            console.log(indent + "Result: " + result);
        } else if (indent === "") {
            if (node.getType() == ASTNodeType.IntDeclaration || node.getType() == ASTNodeType.AssignmentStmt) {
                console.log(node.getText() + ": " + result);
            } else if (node.getType() != ASTNodeType.Programm) {
                console.log(result);
            }
        }

        return result;
    }
}

const main = () => {
    let verbose = false;

    const args = process.argv;
    if (args.length > 2 && args[2] === "-v") {
        verbose = true;
        console.log("verbose mode");
    }

    console.log("Simple script language!");

    const parser = new SimpleParser();
    const script = new SimpleScript(verbose);

    // const tree = parser.parse("int age = 3;");
    // console.log(tree);
    // parser.dumpAST(tree, "");
    // script.evalute(tree, "");

    // return;

    let scriptText = "";
    console.log("\n");

    const rl = readline.createInterface({
        input: process.stdin,
        output: process.stdout,
        prompt: ">> "
    });

    rl.prompt();

    rl.on("line", (input) => {
        let line = input.trim();

        if (line === 'exit();') {
            console.log("see you!");
            // rl.close();
            process.exit(0);
        }

        scriptText += line + "\n";
        if (line.endsWith(";")) {
            let tree = parser.parse(scriptText);
            if (verbose) {
                parser.dumpAST(tree, "");
            }

            script.evalute(tree, "");
            console.log("\n");
            scriptText = "";
        }

        rl.prompt();
    });
}

main();
