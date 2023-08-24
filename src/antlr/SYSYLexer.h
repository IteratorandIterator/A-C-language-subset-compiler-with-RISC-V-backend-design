
// Generated from /home/carbon/code/compiler_guas/grammar/SYSYLexer.g4 by ANTLR 4.12.0

#pragma once


#include "antlr4-runtime.h"




class  SYSYLexer : public antlr4::Lexer {
public:
  enum {
    Comma = 1, SemiColon = 2, Assign = 3, LeftBracket = 4, RightBracket = 5, 
    LeftBrace = 6, RightBrace = 7, LeftParen = 8, RightParen = 9, If = 10, 
    Else = 11, While = 12, Const = 13, Equal = 14, NonEqual = 15, Less = 16, 
    Greater = 17, LessEqual = 18, GreaterEqual = 19, Plus = 20, Minus = 21, 
    Multiply = 22, Divide = 23, Modulo = 24, Not = 25, And = 26, Or = 27, 
    Int = 28, Float = 29, Void = 30, Identifier = 31, FloatConst = 32, IntConst = 33, 
    Break = 34, Continue = 35, Return = 36, LineComment = 37, BlockComment = 38, 
    WhiteSpace = 39
  };

  explicit SYSYLexer(antlr4::CharStream *input);

  ~SYSYLexer() override;


  std::string getGrammarFileName() const override;

  const std::vector<std::string>& getRuleNames() const override;

  const std::vector<std::string>& getChannelNames() const override;

  const std::vector<std::string>& getModeNames() const override;

  const antlr4::dfa::Vocabulary& getVocabulary() const override;

  antlr4::atn::SerializedATNView getSerializedATN() const override;

  const antlr4::atn::ATN& getATN() const override;

  // By default the static state used to implement the lexer is lazily initialized during the first
  // call to the constructor. You can call this function if you wish to initialize the static state
  // ahead of time.
  static void initialize();

private:

  // Individual action functions triggered by action() above.

  // Individual semantic predicate functions triggered by sempred() above.

};

