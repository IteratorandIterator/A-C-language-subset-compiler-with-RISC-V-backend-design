#include "backend_pass.h"
using namespace Archriscv::backend_pass;


void merge_addi_lw_sw::merge_inst(Function *func) {
  func->build_def_use();//构建寄存器和指令，BB的对应关系，放在reg_def和reg_use里面
  func->update_reg();//更新寄存器集合
  for (int r = RegCount; r < func->reg_n; ++r) {//遍历所有指令
    if (func->reg_def[r].size() != 1) continue;//如果函数中没有这个def这个寄存器，直接下一个寄存器
    OccurPoint def = *func->reg_def[r].begin();//得到寄存器第一次def的地方
    Inst *def_inst = def.it->get();
    if (!check_def(def_inst)) continue;//没有通过check的条件就直接看下一个寄存器
    bool flag = true;
    int max_pos = -1;
    std::vector<OccurPoint> uses;
    std::vector<unique_ptr<Inst>> replace;
    for (const OccurPoint &use : func->reg_use[r]) {//遍历这个寄存器所有use的地方
      unique_ptr<Inst> cur_replace = check_use(use.it->get(), func->regs[r], def_inst);//这边需要传入寄存器的类型
      if (!cur_replace) {
        flag = false;
        break;
      }
      uses.push_back(use);
      replace.push_back(std::move(cur_replace));
      if (use.b == def.b) max_pos = std::max(max_pos, use.pos);
    }
    if (!flag) continue;
    for (Reg rely : (*def.it)->use_reg()) {
      for (const OccurPoint &rely_def : func->reg_def[rely.id])
        if (rely_def.b == def.b && rely_def.pos >= def.pos &&
            rely_def.pos <= max_pos) {
          flag = false;
          break;
        }
      if (!flag) break;
    }
    if (!flag) continue;
    for (size_t i = 0; i < uses.size(); ++i) {
      func->erase_def_use(uses[i], uses[i].it->get());
      *uses[i].it = std::move(replace[i]);
      func->add_def_use(uses[i], uses[i].it->get());
    }
  }
}

void merge_addi_lw_sw::run(){
    for (auto &f : program_->functions)
        this->merge_inst(f.get());    
}