import { SimpleToken } from "./parser";


export interface TokenReader {
    /**
     * 返回Token流中下一个Token，并从流中取出。 如果流已经为空，返回null;
     */
    read: () => SimpleToken;
    /**
     * 返回Token流中下一个Token，但不从流中取出。 如果流已经为空，返回null;
     */
    peek: () => SimpleToken;
     /**
     * Token流回退一步。恢复原来的Token。
     */
    unread: () => void;
    /**
     * 获取Token流当前的读取位置。
     * @return
     */
    getPosition: () => number;
    /**
     * 设置Token流当前的读取位置
     * @param position
     */
    setPosition: (position: number) => void;
}

export class SimpleTokenReader implements TokenReader {
    tokens: SimpleToken[] = null;
    pos: number = 0;

    constructor(tokens: SimpleToken[]) {
        this.tokens = tokens;
    }

    public read(): SimpleToken {
        if (this.pos < this.tokens.length) return this.tokens[this.pos++];

        return null;
    }

    public peek(): SimpleToken {
        if (this.pos < this.tokens.length) return this.tokens[this.pos];

        return null;
    }

    public unread() {
        if (this.pos > 0) this.pos = 0;
    }

    public getPosition(): number {
        return this.pos;
    }

    public setPosition(position: number) {
        if (position > 0 && position < this.tokens.length) {
            this.pos = position;
        }
    }
}
