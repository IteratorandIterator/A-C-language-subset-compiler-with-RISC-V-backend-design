
// Generated from /home/carbon/code/compiler_guas/grammar/SYSYParser.g4 by ANTLR 4.12.0

#pragma once


#include "antlr4-runtime.h"
#include "SYSYParserListener.h"


/**
 * This class provides an empty implementation of SYSYParserListener,
 * which can be extended to create a listener which only needs to handle a subset
 * of the available methods.
 */
class  SYSYParserBaseListener : public SYSYParserListener {
public:

  virtual void enterCompilationUnit(SYSYParser::CompilationUnitContext * /*ctx*/) override { }
  virtual void exitCompilationUnit(SYSYParser::CompilationUnitContext * /*ctx*/) override { }

  virtual void enterDecl(SYSYParser::DeclContext * /*ctx*/) override { }
  virtual void exitDecl(SYSYParser::DeclContext * /*ctx*/) override { }

  virtual void enterConstdecl(SYSYParser::ConstdeclContext * /*ctx*/) override { }
  virtual void exitConstdecl(SYSYParser::ConstdeclContext * /*ctx*/) override { }

  virtual void enterConstdef(SYSYParser::ConstdefContext * /*ctx*/) override { }
  virtual void exitConstdef(SYSYParser::ConstdefContext * /*ctx*/) override { }

  virtual void enterInit(SYSYParser::InitContext * /*ctx*/) override { }
  virtual void exitInit(SYSYParser::InitContext * /*ctx*/) override { }

  virtual void enterVardecl(SYSYParser::VardeclContext * /*ctx*/) override { }
  virtual void exitVardecl(SYSYParser::VardeclContext * /*ctx*/) override { }

  virtual void enterVardef(SYSYParser::VardefContext * /*ctx*/) override { }
  virtual void exitVardef(SYSYParser::VardefContext * /*ctx*/) override { }

  virtual void enterFuncdef(SYSYParser::FuncdefContext * /*ctx*/) override { }
  virtual void exitFuncdef(SYSYParser::FuncdefContext * /*ctx*/) override { }

  virtual void enterFuncFparams(SYSYParser::FuncFparamsContext * /*ctx*/) override { }
  virtual void exitFuncFparams(SYSYParser::FuncFparamsContext * /*ctx*/) override { }

  virtual void enterFuncFparam(SYSYParser::FuncFparamContext * /*ctx*/) override { }
  virtual void exitFuncFparam(SYSYParser::FuncFparamContext * /*ctx*/) override { }

  virtual void enterFunctype(SYSYParser::FunctypeContext * /*ctx*/) override { }
  virtual void exitFunctype(SYSYParser::FunctypeContext * /*ctx*/) override { }

  virtual void enterTypeId(SYSYParser::TypeIdContext * /*ctx*/) override { }
  virtual void exitTypeId(SYSYParser::TypeIdContext * /*ctx*/) override { }

  virtual void enterBlock(SYSYParser::BlockContext * /*ctx*/) override { }
  virtual void exitBlock(SYSYParser::BlockContext * /*ctx*/) override { }

  virtual void enterStmt(SYSYParser::StmtContext * /*ctx*/) override { }
  virtual void exitStmt(SYSYParser::StmtContext * /*ctx*/) override { }

  virtual void enterLval(SYSYParser::LvalContext * /*ctx*/) override { }
  virtual void exitLval(SYSYParser::LvalContext * /*ctx*/) override { }

  virtual void enterCond(SYSYParser::CondContext * /*ctx*/) override { }
  virtual void exitCond(SYSYParser::CondContext * /*ctx*/) override { }

  virtual void enterExp(SYSYParser::ExpContext * /*ctx*/) override { }
  virtual void exitExp(SYSYParser::ExpContext * /*ctx*/) override { }

  virtual void enterNumber(SYSYParser::NumberContext * /*ctx*/) override { }
  virtual void exitNumber(SYSYParser::NumberContext * /*ctx*/) override { }


  virtual void enterEveryRule(antlr4::ParserRuleContext * /*ctx*/) override { }
  virtual void exitEveryRule(antlr4::ParserRuleContext * /*ctx*/) override { }
  virtual void visitTerminal(antlr4::tree::TerminalNode * /*node*/) override { }
  virtual void visitErrorNode(antlr4::tree::ErrorNode * /*node*/) override { }

};

