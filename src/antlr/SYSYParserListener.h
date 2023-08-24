
// Generated from /home/carbon/code/compiler_guas/grammar/SYSYParser.g4 by ANTLR 4.12.0

#pragma once


#include "antlr4-runtime.h"
#include "SYSYParser.h"


/**
 * This interface defines an abstract listener for a parse tree produced by SYSYParser.
 */
class  SYSYParserListener : public antlr4::tree::ParseTreeListener {
public:

  virtual void enterCompilationUnit(SYSYParser::CompilationUnitContext *ctx) = 0;
  virtual void exitCompilationUnit(SYSYParser::CompilationUnitContext *ctx) = 0;

  virtual void enterDecl(SYSYParser::DeclContext *ctx) = 0;
  virtual void exitDecl(SYSYParser::DeclContext *ctx) = 0;

  virtual void enterConstdecl(SYSYParser::ConstdeclContext *ctx) = 0;
  virtual void exitConstdecl(SYSYParser::ConstdeclContext *ctx) = 0;

  virtual void enterConstdef(SYSYParser::ConstdefContext *ctx) = 0;
  virtual void exitConstdef(SYSYParser::ConstdefContext *ctx) = 0;

  virtual void enterInit(SYSYParser::InitContext *ctx) = 0;
  virtual void exitInit(SYSYParser::InitContext *ctx) = 0;

  virtual void enterVardecl(SYSYParser::VardeclContext *ctx) = 0;
  virtual void exitVardecl(SYSYParser::VardeclContext *ctx) = 0;

  virtual void enterVardef(SYSYParser::VardefContext *ctx) = 0;
  virtual void exitVardef(SYSYParser::VardefContext *ctx) = 0;

  virtual void enterFuncdef(SYSYParser::FuncdefContext *ctx) = 0;
  virtual void exitFuncdef(SYSYParser::FuncdefContext *ctx) = 0;

  virtual void enterFuncFparams(SYSYParser::FuncFparamsContext *ctx) = 0;
  virtual void exitFuncFparams(SYSYParser::FuncFparamsContext *ctx) = 0;

  virtual void enterFuncFparam(SYSYParser::FuncFparamContext *ctx) = 0;
  virtual void exitFuncFparam(SYSYParser::FuncFparamContext *ctx) = 0;

  virtual void enterFunctype(SYSYParser::FunctypeContext *ctx) = 0;
  virtual void exitFunctype(SYSYParser::FunctypeContext *ctx) = 0;

  virtual void enterTypeId(SYSYParser::TypeIdContext *ctx) = 0;
  virtual void exitTypeId(SYSYParser::TypeIdContext *ctx) = 0;

  virtual void enterBlock(SYSYParser::BlockContext *ctx) = 0;
  virtual void exitBlock(SYSYParser::BlockContext *ctx) = 0;

  virtual void enterStmt(SYSYParser::StmtContext *ctx) = 0;
  virtual void exitStmt(SYSYParser::StmtContext *ctx) = 0;

  virtual void enterLval(SYSYParser::LvalContext *ctx) = 0;
  virtual void exitLval(SYSYParser::LvalContext *ctx) = 0;

  virtual void enterCond(SYSYParser::CondContext *ctx) = 0;
  virtual void exitCond(SYSYParser::CondContext *ctx) = 0;

  virtual void enterExp(SYSYParser::ExpContext *ctx) = 0;
  virtual void exitExp(SYSYParser::ExpContext *ctx) = 0;

  virtual void enterNumber(SYSYParser::NumberContext *ctx) = 0;
  virtual void exitNumber(SYSYParser::NumberContext *ctx) = 0;


};

