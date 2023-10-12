export class ASTNode {
    // AST类型：也是非终结符的名称或Token类型
    private type: number = null;

    private text: string = null;

    private children: ASTNode[] = [];

    private parent: ASTNode = null;

    // protected static EpsilonNode: ASTNode = new ASTNode('Epsilon');

    constructor(type?: number, text?: string) {
        this.type = type;
        this.text = text;
    }

    public getType() {
        return this.type;
    }

    public getText() {
        return this.text;
    }

    public getChildren() {
        const arr: readonly ASTNode[] = [...this.children];
        return arr;
    }

    public getChildCount() {
        return this.children.length;
    }

    public getChild(pos: number) {
        return this.children[pos];
    }

    public getParent() {
        return this.parent;
    }

    public isTerminal() {
        return this.children.length === 0;
    }

    /**
     * 添加子节点的时候，如果子节点不是命名节点，直接把它的下级节点加进来。这样简化了AST。
     * @param child
     */
    public addChild(child: ASTNode): void {
        if (child.isNameNode()) {
            this.children.push(child);
            child.parent = this;
        } else {
            this.children.push(...child.children);
            for (const node of child.children) node.parent = this;
        }
    }

    public setText(text: string) {
        this.text = text;
    }

    /**
    * 是否是命名节点。
    * @return
    */
    protected isNameNode(): boolean {
        // if (this.type !== null && this.type.length > 1 && this.type.charAt(0) != '_') return true;
        if (this.type !== null) return true;


        return false;
    }

    /**
    * 树状结构打印自身以及下级节点。
    */
    protected dump() {
        ASTNode._dump(this, "");
    }

    /**
     * 树状结构打印AST
     * @param node
     * @param indent
     */
    private static _dump(node: ASTNode, indent: string) {
        let str = indent + node.type;
        if (node.text != null)
            str += "(" + node.text + ")";

        console.log(str);

        for (const child of node.children) {
            this._dump(child, indent + "\t");
        }
    }
}