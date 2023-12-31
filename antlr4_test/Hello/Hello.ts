// Generated from Hello.g4 by ANTLR 4.13.1
// noinspection ES6UnusedImports,JSUnusedGlobalSymbols,JSUnusedLocalSymbols
import {
	ATN,
	ATNDeserializer,
	CharStream,
	DecisionState, DFA,
	Lexer,
	LexerATNSimulator,
	RuleContext,
	PredictionContextCache,
	Token
} from "antlr4";
export default class Hello extends Lexer {
	public static readonly If = 1;
	public static readonly Int = 2;
	public static readonly IntLiteral = 3;
	public static readonly StringLiteral = 4;
	public static readonly AssignmentOP = 5;
	public static readonly RelationalOP = 6;
	public static readonly Star = 7;
	public static readonly Plus = 8;
	public static readonly Sharp = 9;
	public static readonly SemiColon = 10;
	public static readonly Dot = 11;
	public static readonly Comm = 12;
	public static readonly LeftBracket = 13;
	public static readonly RightBracket = 14;
	public static readonly LeftBrace = 15;
	public static readonly RightBrace = 16;
	public static readonly LeftParen = 17;
	public static readonly RightParen = 18;
	public static readonly Id = 19;
	public static readonly Whitespace = 20;
	public static readonly Newline = 21;
	public static readonly EOF = Token.EOF;

	public static readonly channelNames: string[] = [ "DEFAULT_TOKEN_CHANNEL", "HIDDEN" ];
	public static readonly literalNames: (string | null)[] = [ null, "'if'", 
                                                            "'int'", null, 
                                                            null, "'='", 
                                                            null, "'*'", 
                                                            "'+'", "'#'", 
                                                            "';'", "'.'", 
                                                            "','", "'['", 
                                                            "']'", "'{'", 
                                                            "'}'", "'('", 
                                                            "')'" ];
	public static readonly symbolicNames: (string | null)[] = [ null, "If", 
                                                             "Int", "IntLiteral", 
                                                             "StringLiteral", 
                                                             "AssignmentOP", 
                                                             "RelationalOP", 
                                                             "Star", "Plus", 
                                                             "Sharp", "SemiColon", 
                                                             "Dot", "Comm", 
                                                             "LeftBracket", 
                                                             "RightBracket", 
                                                             "LeftBrace", 
                                                             "RightBrace", 
                                                             "LeftParen", 
                                                             "RightParen", 
                                                             "Id", "Whitespace", 
                                                             "Newline" ];
	public static readonly modeNames: string[] = [ "DEFAULT_MODE", ];

	public static readonly ruleNames: string[] = [
		"If", "Int", "IntLiteral", "StringLiteral", "AssignmentOP", "RelationalOP", 
		"Star", "Plus", "Sharp", "SemiColon", "Dot", "Comm", "LeftBracket", "RightBracket", 
		"LeftBrace", "RightBrace", "LeftParen", "RightParen", "Id", "Whitespace", 
		"Newline",
	];


	constructor(input: CharStream) {
		super(input);
		this._interp = new LexerATNSimulator(this, Hello._ATN, Hello.DecisionsToDFA, new PredictionContextCache());
	}

	public get grammarFileName(): string { return "Hello.g4"; }

	public get literalNames(): (string | null)[] { return Hello.literalNames; }
	public get symbolicNames(): (string | null)[] { return Hello.symbolicNames; }
	public get ruleNames(): string[] { return Hello.ruleNames; }

	public get serializedATN(): number[] { return Hello._serializedATN; }

	public get channelNames(): string[] { return Hello.channelNames; }

	public get modeNames(): string[] { return Hello.modeNames; }

	public static readonly _serializedATN: number[] = [4,0,21,121,6,-1,2,0,
	7,0,2,1,7,1,2,2,7,2,2,3,7,3,2,4,7,4,2,5,7,5,2,6,7,6,2,7,7,7,2,8,7,8,2,9,
	7,9,2,10,7,10,2,11,7,11,2,12,7,12,2,13,7,13,2,14,7,14,2,15,7,15,2,16,7,
	16,2,17,7,17,2,18,7,18,2,19,7,19,2,20,7,20,1,0,1,0,1,0,1,1,1,1,1,1,1,1,
	1,2,4,2,52,8,2,11,2,12,2,53,1,3,1,3,5,3,58,8,3,10,3,12,3,61,9,3,1,3,1,3,
	1,4,1,4,1,5,1,5,1,5,1,5,1,5,1,5,3,5,73,8,5,1,6,1,6,1,7,1,7,1,8,1,8,1,9,
	1,9,1,10,1,10,1,11,1,11,1,12,1,12,1,13,1,13,1,14,1,14,1,15,1,15,1,16,1,
	16,1,17,1,17,1,18,1,18,5,18,101,8,18,10,18,12,18,104,9,18,1,19,4,19,107,
	8,19,11,19,12,19,108,1,19,1,19,1,20,1,20,3,20,115,8,20,1,20,3,20,118,8,
	20,1,20,1,20,1,59,0,21,1,1,3,2,5,3,7,4,9,5,11,6,13,7,15,8,17,9,19,10,21,
	11,23,12,25,13,27,14,29,15,31,16,33,17,35,18,37,19,39,20,41,21,1,0,4,1,
	0,48,57,3,0,65,90,95,95,97,122,4,0,48,57,65,90,95,95,97,122,2,0,9,9,32,
	32,129,0,1,1,0,0,0,0,3,1,0,0,0,0,5,1,0,0,0,0,7,1,0,0,0,0,9,1,0,0,0,0,11,
	1,0,0,0,0,13,1,0,0,0,0,15,1,0,0,0,0,17,1,0,0,0,0,19,1,0,0,0,0,21,1,0,0,
	0,0,23,1,0,0,0,0,25,1,0,0,0,0,27,1,0,0,0,0,29,1,0,0,0,0,31,1,0,0,0,0,33,
	1,0,0,0,0,35,1,0,0,0,0,37,1,0,0,0,0,39,1,0,0,0,0,41,1,0,0,0,1,43,1,0,0,
	0,3,46,1,0,0,0,5,51,1,0,0,0,7,55,1,0,0,0,9,64,1,0,0,0,11,72,1,0,0,0,13,
	74,1,0,0,0,15,76,1,0,0,0,17,78,1,0,0,0,19,80,1,0,0,0,21,82,1,0,0,0,23,84,
	1,0,0,0,25,86,1,0,0,0,27,88,1,0,0,0,29,90,1,0,0,0,31,92,1,0,0,0,33,94,1,
	0,0,0,35,96,1,0,0,0,37,98,1,0,0,0,39,106,1,0,0,0,41,117,1,0,0,0,43,44,5,
	105,0,0,44,45,5,102,0,0,45,2,1,0,0,0,46,47,5,105,0,0,47,48,5,110,0,0,48,
	49,5,116,0,0,49,4,1,0,0,0,50,52,7,0,0,0,51,50,1,0,0,0,52,53,1,0,0,0,53,
	51,1,0,0,0,53,54,1,0,0,0,54,6,1,0,0,0,55,59,5,34,0,0,56,58,9,0,0,0,57,56,
	1,0,0,0,58,61,1,0,0,0,59,60,1,0,0,0,59,57,1,0,0,0,60,62,1,0,0,0,61,59,1,
	0,0,0,62,63,5,34,0,0,63,8,1,0,0,0,64,65,5,61,0,0,65,10,1,0,0,0,66,73,5,
	62,0,0,67,68,5,62,0,0,68,73,5,61,0,0,69,73,5,60,0,0,70,71,5,60,0,0,71,73,
	5,61,0,0,72,66,1,0,0,0,72,67,1,0,0,0,72,69,1,0,0,0,72,70,1,0,0,0,73,12,
	1,0,0,0,74,75,5,42,0,0,75,14,1,0,0,0,76,77,5,43,0,0,77,16,1,0,0,0,78,79,
	5,35,0,0,79,18,1,0,0,0,80,81,5,59,0,0,81,20,1,0,0,0,82,83,5,46,0,0,83,22,
	1,0,0,0,84,85,5,44,0,0,85,24,1,0,0,0,86,87,5,91,0,0,87,26,1,0,0,0,88,89,
	5,93,0,0,89,28,1,0,0,0,90,91,5,123,0,0,91,30,1,0,0,0,92,93,5,125,0,0,93,
	32,1,0,0,0,94,95,5,40,0,0,95,34,1,0,0,0,96,97,5,41,0,0,97,36,1,0,0,0,98,
	102,7,1,0,0,99,101,7,2,0,0,100,99,1,0,0,0,101,104,1,0,0,0,102,100,1,0,0,
	0,102,103,1,0,0,0,103,38,1,0,0,0,104,102,1,0,0,0,105,107,7,3,0,0,106,105,
	1,0,0,0,107,108,1,0,0,0,108,106,1,0,0,0,108,109,1,0,0,0,109,110,1,0,0,0,
	110,111,6,19,0,0,111,40,1,0,0,0,112,114,5,13,0,0,113,115,5,10,0,0,114,113,
	1,0,0,0,114,115,1,0,0,0,115,118,1,0,0,0,116,118,5,10,0,0,117,112,1,0,0,
	0,117,116,1,0,0,0,118,119,1,0,0,0,119,120,6,20,0,0,120,42,1,0,0,0,9,0,53,
	59,72,100,102,108,114,117,1,6,0,0];

	private static __ATN: ATN;
	public static get _ATN(): ATN {
		if (!Hello.__ATN) {
			Hello.__ATN = new ATNDeserializer().deserialize(Hello._serializedATN);
		}

		return Hello.__ATN;
	}


	static DecisionsToDFA = Hello._ATN.decisionToState.map( (ds: DecisionState, index: number) => new DFA(ds, index) );
}