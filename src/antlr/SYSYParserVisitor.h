
// Generated from /home/carbon/code/compiler_guas/grammar/SYSYParser.g4 by ANTLR 4.12.0

#pragma once


#include "antlr4-runtime.h"
#include "SYSYParser.h"
#include<any>


/**
 * This class defines an abstract visitor for a parse tree
 * produced by SYSYParser.
 */
class  SYSYParserVisitor : public antlr4::tree::AbstractParseTreeVisitor {
public:

  /**
   * Visit parse trees produced by SYSYParser.
   */
    virtual std::any visitCompilationUnit(SYSYParser::CompilationUnitContext *context) = 0;

    virtual std::any visitDecl(SYSYParser::DeclContext *context) = 0;

    virtual std::any visitConstdecl(SYSYParser::ConstdeclContext *context) = 0;

    virtual std::any visitConstdef(SYSYParser::ConstdefContext *context) = 0;

    virtual std::any visitInit(SYSYParser::InitContext *context) = 0;

    virtual std::any visitVardecl(SYSYParser::VardeclContext *context) = 0;

    virtual std::any visitVardef(SYSYParser::VardefContext *context) = 0;

    virtual std::any visitFuncdef(SYSYParser::FuncdefContext *context) = 0;

    virtual std::any visitFuncFparams(SYSYParser::FuncFparamsContext *context) = 0;

    virtual std::any visitFuncFparam(SYSYParser::FuncFparamContext *context) = 0;

    virtual std::any visitFunctype(SYSYParser::FunctypeContext *context) = 0;

    virtual std::any visitTypeId(SYSYParser::TypeIdContext *context) = 0;

    virtual std::any visitBlock(SYSYParser::BlockContext *context) = 0;

    virtual std::any visitStmt(SYSYParser::StmtContext *context) = 0;

    virtual std::any visitLval(SYSYParser::LvalContext *context) = 0;

    virtual std::any visitCond(SYSYParser::CondContext *context) = 0;

    virtual std::any visitExp(SYSYParser::ExpContext *context) = 0;

    virtual std::any visitNumber(SYSYParser::NumberContext *context) = 0;


};

