parser grammar SYSYParser;
options { tokenVocab = SYSYLexer; }

compilationUnit:  (decl | funcdef)+ ;
decl: constdecl | vardecl;
constdecl: Const (Int|Float) constdef (Comma constdef)* SemiColon;
constdef:
    Identifier Assign init
    | Identifier (LeftBracket exp RightBracket)+ Assign init
;
init: 
    LeftBrace (init (Comma init)*)? RightBrace
    | exp
;
vardecl: (Int|Float) vardef (Comma vardef)* SemiColon;
vardef: 
    Identifier
    | Identifier (LeftBracket exp RightBracket)+
    | Identifier Assign init
    | Identifier (LeftBracket exp RightBracket)+ Assign init
;

funcdef: 
    functype Identifier LeftParen ( funcFparams )? RightParen block;
funcFparams:
    funcFparam ( Comma funcFparam )*;
funcFparam:
    typeId Identifier (LeftBracket RightBracket ( LeftBracket exp RightBracket)*)?;
functype: 
    Void | Int | Float;
typeId:
    Int | Float;

block: LeftBrace (decl | stmt)* RightBrace;
stmt: 
    lval Assign exp SemiColon
    | Identifier LeftParen exp ( Comma exp )* RightParen SemiColon
    | Identifier LeftParen  RightParen SemiColon
    | block
    | If LeftParen cond RightParen stmt (Else stmt)?
    | While LeftParen cond RightParen stmt
    | Continue SemiColon
    | Break SemiColon
    | Return (exp)? SemiColon
    | (exp)? SemiColon
;
lval: 
    Identifier
    | Identifier (LeftBracket exp RightBracket)+
;
cond:
    cond (Less | Greater | LessEqual | GreaterEqual) cond
    |cond (Equal | NonEqual) cond
    | exp
    | (Plus | Minus | Not) cond
    | cond And cond
    | cond Or cond
;
exp:
    (Plus | Minus) exp
    | exp (Multiply | Divide | Modulo) exp
    | exp (Plus | Minus) exp
    | LeftParen exp RightParen
    | number
    | lval
    | Identifier LeftParen  exp ( Comma exp )* RightParen
    | Identifier LeftParen  RightParen
;
number:
    (IntConst|FloatConst)
;
