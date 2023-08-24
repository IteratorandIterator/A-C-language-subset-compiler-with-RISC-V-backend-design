#pragma once
#include "ir.h"
#include <iostream>
#include <ostream>
namespace Medium {
class ir_serializer : public Medium::ir_visitor {
  public:  
    ir_serializer(ostream &_out) : out(_out) {}
    ~ir_serializer(){};
    std::ostream& out;
   //在这些函数中打印东西就行了
    virtual void visit(Reg &node) override;
    virtual void visit(Variable &node)override;
    virtual void visit(MediumScope &node)override;
    virtual void visit(IrBlock &node)override;
    virtual void visit(IrFunction &node)override;
    virtual void visit(LibFunction &node)override;
    virtual void visit(UserFunction &node)override;
    virtual void visit(CompileUnit &node)override;
    virtual void visit(UnaryOp &node)override;
    virtual void visit(BinaryOp &node)override;
    virtual void visit(ArrayIndex &node)override;
    virtual void visit(LoadAddr &node)override;
    virtual void visit(LoadConst &node)override;
    virtual void visit(LoadArg &node)override;
    virtual void visit(UnaryOpInstr &node)override;
    virtual void visit(BinaryOpInstr &node)override;
    virtual void visit(LoadInstr &node)override;
    virtual void visit(StoreInstr &node)override;
    virtual void visit(JumpInstr &node)override;
    virtual void visit(BranchInstr &node)override;
    virtual void visit(ReturnInstr &node)override;
    virtual void visit(CallInstr &node)override;
    virtual void visit(LocalVarDef &node)override;
    virtual void visit(PhiInstr &node)override;
    virtual void visit(Convert &node)override;

};
}