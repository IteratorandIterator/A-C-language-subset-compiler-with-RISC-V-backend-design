#include "ir.h"
#include "code_gen.h"
// using namespace Medium;
class program_visitor : public Medium::ir_visitor {    
public:
    program_visitor() = default;
    ~program_visitor() = default;

    virtual void visit(Medium::Reg &node) override;
    virtual void visit(Medium::Variable &node)override;
    virtual void visit(Medium::MediumScope &node)override;
    virtual void visit(Medium::IrBlock &node)override;
    virtual void visit(Medium::IrFunction &node)override;
    virtual void visit(Medium::LibFunction &node)override;
    virtual void visit(Medium::UserFunction &node)override;
    virtual void visit(Medium::CompileUnit &node)override;
    virtual void visit(Medium::UnaryOp &node)override;
    virtual void visit(Medium::BinaryOp &node)override;
    virtual void visit(Medium::ArrayIndex &node)override;
    virtual void visit(Medium::LoadAddr &node)override;
    virtual void visit(Medium::LoadConst &node)override;
    virtual void visit(Medium::LoadArg &node)override;
    virtual void visit(Medium::UnaryOpInstr &node)override;
    virtual void visit(Medium::BinaryOpInstr &node)override;
    virtual void visit(Medium::LoadInstr &node)override;
    virtual void visit(Medium::StoreInstr &node)override;
    virtual void visit(Medium::JumpInstr &node)override;
    virtual void visit(Medium::BranchInstr &node)override;
    virtual void visit(Medium::ReturnInstr &node)override;
    virtual void visit(Medium::CallInstr &node)override;
    virtual void visit(Medium::LocalVarDef &node)override;
    virtual void visit(Medium::PhiInstr &node)override;
    virtual void visit(Medium::MemDef &node)override;
    virtual void visit(Medium::MemUse &node)override;
    virtual void visit(Medium::Convert &node)override;
    virtual void visit(Medium::MemEffect &node)override;
    virtual void visit(Medium::MemRead &node)override;
    virtual void visit(Medium::MemWrite &node)override;
    Archriscv::Program* prog;
protected:
    Archriscv::Function* curf;
    Archriscv::GlobalObject* obj;
    Archriscv::Block* cur_block,* next_block;
    std::string func_name;
    Archriscv::Medium2Backend* inner_info;
    std::map<Archriscv::Reg, Archriscv::CmpInfo> cmp_info;
};
