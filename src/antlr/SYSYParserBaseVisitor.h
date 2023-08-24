
// Generated from /home/carbon/code/compiler_guas/grammar/SYSYParser.g4 by ANTLR 4.12.0

#pragma once


#include "antlr4-runtime.h"
#include "SYSYParserVisitor.h"


/**
 * This class provides an empty implementation of SYSYParserVisitor, which can be
 * extended to create a visitor which only needs to handle a subset of the available methods.
 */
class  SYSYParserBaseVisitor : public SYSYParserVisitor {
public:

  virtual std::any visitCompilationUnit(SYSYParser::CompilationUnitContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitDecl(SYSYParser::DeclContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitConstdecl(SYSYParser::ConstdeclContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitConstdef(SYSYParser::ConstdefContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitInit(SYSYParser::InitContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitVardecl(SYSYParser::VardeclContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitVardef(SYSYParser::VardefContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitFuncdef(SYSYParser::FuncdefContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitFuncFparams(SYSYParser::FuncFparamsContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitFuncFparam(SYSYParser::FuncFparamContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitFunctype(SYSYParser::FunctypeContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitTypeId(SYSYParser::TypeIdContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitBlock(SYSYParser::BlockContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitStmt(SYSYParser::StmtContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitLval(SYSYParser::LvalContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitCond(SYSYParser::CondContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitExp(SYSYParser::ExpContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitNumber(SYSYParser::NumberContext *ctx) override {
    return visitChildren(ctx);
  }


};

