import { DfaState, TokenType } from './global_var'
import { SimpleTokenReader } from './simpleReader';


export class SimpleLexer {
    private tokenText: string = null;
    private tokens: SimpleToken[] = null;
    private token: SimpleToken = null;

    //是否是字母
    private isAlpha(ch: string) {
        return ch >= 'a' && ch <= 'z' || ch >= 'A' && ch <= 'Z';
    }

    //是否是数字
    private isDigit(ch: string) {
        return ch >= '0' && ch <= '9';
    }

    //是否是空白字符
    private isBlank(ch: string) {
        return ch == ' ' || ch == '\t' || ch == '\n';
    }

    private initToken(ch: string): DfaState {
        if (this.tokenText.length > 0) {
            this.token.text = this.tokenText;
            this.tokens.push(this.token);

            this.tokenText = '';
            this.token = new SimpleToken();
        }

        let newState: DfaState = DfaState.Initial;

        if (this.isAlpha(ch)) {
            if (ch === 'i') {
                newState = DfaState.Id_int1;
            } else {
                newState = DfaState.Id;
            }

            this.token.type = TokenType.Identifier;
            this.tokenText += ch;
        } else if (this.isDigit(ch)) {
            newState = DfaState.IntLiteral;
            this.token.type = TokenType.IntLiteral;
            this.tokenText += ch;
        } else if (ch === '>') {
            newState = DfaState.GT;
            this.token.type = TokenType.GT;
            this.tokenText += ch;
        } else if (ch == '+') {
            newState = DfaState.Plus;
            this.token.type = TokenType.Plus;
            this.tokenText += ch;
        } else if (ch == '-') {
            newState = DfaState.Minus;
            this.token.type = TokenType.Minus;
            this.tokenText += ch;
        } else if (ch == '*') {
            newState = DfaState.Star;
            this.token.type = TokenType.Star;
            this.tokenText += ch;
        } else if (ch == '/') {
            newState = DfaState.Slash;
            this.token.type = TokenType.Slash;
            this.tokenText += ch;
        } else if (ch == ';') {
            newState = DfaState.SemiColon;
            this.token.type = TokenType.SemiColon;
            this.tokenText += ch;
        } else if (ch == '(') {
            newState = DfaState.LeftParen;
            this.token.type = TokenType.LeftParen;
            this.tokenText += ch;
        } else if (ch == ')') {
            newState = DfaState.RightParen;
            this.token.type = TokenType.RightParen;
            this.tokenText += ch;
        } else if (ch == '=') {
            newState = DfaState.Assignment;
            this.token.type = TokenType.Assignment;
            this.tokenText += ch;
        } else {
            newState = DfaState.Initial; // skip all unknown patterns
        }

        return newState;
    }

    public tokenize(code: string): SimpleTokenReader {
        this.tokens = [];
        this.tokenText = "";
        this.token = new SimpleToken();

        const charArr = code.split("");

        let ich = 0;
        let ch = '';

        let state = DfaState.Initial;

        let i = 0;

        try {
            while (charArr.length !== 0) {
                ch = charArr.shift();
                switch (state) {
                    case DfaState.Initial:
                        state = this.initToken(ch);
                        break;
                    case DfaState.Id:
                        if (this.isAlpha(ch) || this.isDigit(ch)) {
                            this.tokenText += ch;
                        } else {
                            state = this.initToken(ch);
                        }

                        break;
                    case DfaState.GT:
                        if (ch === '=') {
                            this.token.type = TokenType.GE;
                            state = DfaState.GE;
                            this.tokenText += ch;
                        } else {
                            state = this.initToken(ch);
                        }
                        break;
                    case DfaState.GE:
                    case DfaState.Assignment:
                    case DfaState.Plus:
                    case DfaState.Minus:
                    case DfaState.Star:
                    case DfaState.Slash:
                    case DfaState.SemiColon:
                    case DfaState.LeftParen:
                    case DfaState.RightParen:
                        state = this.initToken(ch);
                        break;
                    case DfaState.IntLiteral:
                        if (this.isDigit(ch)) {
                            this.tokenText += ch;
                        } else {
                            state = this.initToken(ch);
                        }
                        break
                    case DfaState.Id_int1:
                        if (ch === 'n') {
                            state = DfaState.Id_int2;
                            this.tokenText += ch;
                        } else if (this.isDigit(ch) || this.isAlpha(ch)) {
                            state = DfaState.Id;
                            this.tokenText += ch;
                        } else {
                            state = this.initToken(ch);
                        }
                        break;
                    case DfaState.Id_int2:
                        if (ch === 't') {
                            state = DfaState.Id_int3;
                            this.tokenText += ch;
                        } else if (this.isDigit(ch) || this.isAlpha(ch)) {
                            state = DfaState.Id;
                            this.tokenText += ch;
                        } else {
                            state = this.initToken(ch);
                        }
                        break;
                    case DfaState.Id_int3:
                        if (this.isBlank(ch)) {
                            this.token.type = TokenType.Int;
                            state = this.initToken(ch);
                        } else {
                            state = DfaState.Id;
                            this.tokenText += ch;
                        }
                        break;
                    default:


                }
            }

            // 把最后一个token送进去
            if (this.tokenText.length > 0) {
                this.initToken(ch);
            }
        } catch (err) {
            console.error(err);
        }

        return new SimpleTokenReader(this.tokens);
    }
}


function dump(tokenReader: SimpleTokenReader) {
    let token: SimpleToken = null;
    while ((token = tokenReader.read()) != null) {
        console.log(token.getText() + "\t\t" + token.getType());
    }
}

export class SimpleToken {
    public type: TokenType = null;

    public text: string = null;

    public getType(): TokenType {
        return this.type;
    }


    public getText(): string {
        return this.text;
    }
}

// const instance = new SimpleLexer();

// const scriptStr1 = "int inta = 45";
// console.log(scriptStr1);

// const tokenReader1 = instance.tokenize(scriptStr1);
// dump(tokenReader1);

// const scriptStr2 = "age >= 45";
// console.log(scriptStr2);

// const tokenReader2 = instance.tokenize(scriptStr2);
// dump(tokenReader2);