#include "backend_pass.h"
#include "ir.h"
using namespace Archriscv;
using namespace Archriscv::backend_pass;
using std::make_unique;
void inline_constant::run(){
    for(auto & f : program_->functions)
    {
       func = f.get();
        for (auto &block : func->blocks)
            for (auto &inst : block->insts)
                if (RegRegInst *rr = inst->as<RegRegInst>()) {
                    auto replace = opt_regreg(rr);
                    if (replace) inst = std::move(replace);
                }
    }
       
}

unique_ptr<Inst> inline_constant::opt_regreg(RegRegInst * rr){
    if (rr->op == RegRegInst::Add ||rr->op == RegRegInst::Addw || rr->op == RegRegInst::And ||
      rr->op == RegRegInst::Or || rr->op == RegRegInst::Xor) {
    RegImmInst::Type new_op;//指令对应的立即数版本
    switch (rr->op) {
      case RegRegInst::Add:
        new_op = RegImmInst::Addi;
        break;
      case RegRegInst::Addw:
        new_op = RegImmInst::Addiw;
        break;
      case RegRegInst::And:
        new_op = RegImmInst::Andi;
        break;
      case RegRegInst::Or:
        new_op = RegImmInst::Ori;
        break;
      case RegRegInst::Xor:
        new_op = RegImmInst::Xori;
        break;
      default:
        assert(0);
    }
    if (func->constant_reg.find(rr->lhs) != func->constant_reg.end()) {//左操作数的寄存器保存了常数
      int32_t lv = func->constant_reg[rr->lhs];
      if (is_imm12(lv)) {
        return make_unique<RegImmInst>(new_op, rr->dst, rr->rhs, lv);
      }
    }
    if (func->constant_reg.find(rr->rhs) != func->constant_reg.end()) {//同理
      int32_t rv = func->constant_reg[rr->rhs];
      if (is_imm12(rv)) {
        return make_unique<RegImmInst>(new_op, rr->dst, rr->lhs, rv);
      }
    }
  } else if (rr->op == RegRegInst::Sll || rr->op == RegRegInst::Srl ||
             rr->op == RegRegInst::Sra || rr->op == RegRegInst::Slt ||
             rr->op == RegRegInst::Sltu) {
    RegImmInst::Type new_op;
    switch (rr->op) {
      case RegRegInst::Sll:
        new_op = RegImmInst::Slli;
        break;
      case RegRegInst::Srl:
        new_op = RegImmInst::Srli;
        break;
      case RegRegInst::Sra:
        new_op = RegImmInst::Srai;
        break;
      case RegRegInst::Slt:
        new_op = RegImmInst::Slti;
        break;
      case RegRegInst::Sltu:
        new_op = RegImmInst::Sltiu;
        break;
      default:
        assert(0);
    }
    if (func->constant_reg.find(rr->rhs) != func->constant_reg.end()) {
      int32_t rv = func->constant_reg[rr->rhs];
      if (is_imm12(rv)) {
        return make_unique<RegImmInst>(new_op, rr->dst, rr->lhs, rv);
      }
    }
  } else if (rr->op == RegRegInst::Subw) {
    if (func->constant_reg.find(rr->rhs) != func->constant_reg.end()) {
      int32_t rv = func->constant_reg[rr->rhs];
      if (is_imm12(-rv)) {
        return make_unique<RegImmInst>(RegImmInst::Addi, rr->dst, rr->lhs, -rv);
      }
    }
  }
  
 else if (rr->op == RegRegInst::Sub) {
    if (func->constant_reg.find(rr->rhs) != func->constant_reg.end()) {
      int32_t rv = func->constant_reg[rr->rhs];
      if (is_imm12(-rv)) {
        return make_unique<RegImmInst>(RegImmInst::Addi, rr->dst, rr->lhs, -rv);
      }
    }
  } 
  else if (rr->op == RegRegInst::Mulw) {//将一些乘法转化为位移操作
    if (func->constant_reg.find(rr->lhs) != func->constant_reg.end()) {
      int32_t lv = func->constant_reg[rr->lhs];
      if (log2_map.find(lv) != log2_map.end()) {
        return make_unique<RegImmInst>(RegImmInst::Slli, rr->dst, rr->rhs,
                                       log2_map[lv]);
      }
    }
    if (func->constant_reg.find(rr->rhs) != func->constant_reg.end()) {
      int32_t rv = func->constant_reg[rr->rhs];
      if (log2_map.find(rv) != log2_map.end()) {
        return make_unique<RegImmInst>(RegImmInst::Slli, rr->dst, rr->lhs,
                                       log2_map[rv]);
      }
    }
  }
  else if (rr->op == RegRegInst::Mul) {//将一些乘法转化为位移操作
    if (func->constant_reg.find(rr->lhs) != func->constant_reg.end()) {
      int32_t lv = func->constant_reg[rr->lhs];
      if (log2_map.find(lv) != log2_map.end()) {
        return make_unique<RegImmInst>(RegImmInst::Slli, rr->dst, rr->rhs,
                                       log2_map[lv]);
      }
    }
    if (func->constant_reg.find(rr->rhs) != func->constant_reg.end()) {
      int32_t rv = func->constant_reg[rr->rhs];
      if (log2_map.find(rv) != log2_map.end()) {
        return make_unique<RegImmInst>(RegImmInst::Slli, rr->dst, rr->lhs,
                                       log2_map[rv]);
      }
    }
  }
  else if (rr->op == RegRegInst::Div) {
    if (func->constant_reg.find(rr->lhs) != func->constant_reg.end()) {
      int32_t lv = func->constant_reg[rr->lhs];
      if (is_imm12(lv)&&log2_map.find(lv) != log2_map.end()) {
        return make_unique<RegImmInst>(RegImmInst::Srai, rr->dst, rr->rhs,
                                       log2_map[lv]);
      }
    }

    if (func->constant_reg.find(rr->rhs) != func->constant_reg.end()) {
      int32_t rv = func->constant_reg[rr->rhs];
      if (is_imm12(rv)&&log2_map.find(rv) != log2_map.end()) {
        return make_unique<RegImmInst>(RegImmInst::Srai, rr->dst, rr->lhs,
                                       log2_map[rv]);
      }
    }
  }
  else if (rr->op == RegRegInst::Divw) {
    if (func->constant_reg.find(rr->lhs) != func->constant_reg.end()) {
      int32_t lv = func->constant_reg[rr->lhs];
      if (is_imm12(lv)&&log2_map.find(lv) != log2_map.end()) {
        return make_unique<RegImmInst>(RegImmInst::Srai, rr->dst, rr->rhs,
                                       log2_map[lv]);
      }
    }

    if (func->constant_reg.find(rr->rhs) != func->constant_reg.end()) {
      int32_t rv = func->constant_reg[rr->rhs];
      if (is_imm12(rv)&&log2_map.find(rv) != log2_map.end()) {
        return make_unique<RegImmInst>(RegImmInst::Srai, rr->dst, rr->lhs,
                                       log2_map[rv]);
      }
    }
  }
  return nullptr;
}

