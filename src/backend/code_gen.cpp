#include "code_gen.h"

#include <bitset>
#include <functional>
#include <map>
#include <memory>
#include <ostream>
#include <queue>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "arch.h"
#include "inst.h"
#include "register_allocator.h"
#include "pass.h"

using std::bitset;
using std::deque;
using std::make_unique;
using std::map;
using std::ostream;
using std::pair;
using std::set;
using std::string;
using std::unique_ptr;
using std::vector;

string generate_name(const string &s) { //命名
  auto legal_char = [&](char ch){
    return isalpha(ch) || isdigit(ch) || ch == '_';
  };
  assert(s.length() > 0);
  std::ostringstream ret;
  ret << "_m";
  int last = -1;
  for (int i = 0; i < static_cast<int>(s.length()); ++i)
    if (!legal_char(s[i])) { //不合法字符出现在里面
      if (i - last - 1 > 0) { 
        ret << std::to_string(i - last - 1) << s.substr(last + 1, i - last - 1);
      }
      unsigned int cur = s[i];
      ret << '0' << std::hex << cur / 16 << cur % 16 << std::dec;
      last = i;
    }
  if (last < static_cast<int>(s.length() - 1)) {
    ret << std::to_string(static_cast<int>(s.length() - 1) - last)
        << s.substr(last + 1);
  }
  return ret.str();
}

namespace Archriscv {

Medium2Backend::Medium2Backend() : reg_n(RegCount) {}

Reg Medium2Backend::new_backend_reg(bool is_float) { return Reg{reg_n++, is_float ? FP : INT}; }

Reg Medium2Backend::reg_medium2backend(Medium::Reg ir_reg) {
  auto it = reg_mapping.find(ir_reg.id);
  if (it != reg_mapping.end()) return it->second;
  Reg ret = new_backend_reg(!ir_reg.is_int);
  reg_mapping[ir_reg.id] = ret;
  return ret;
}

Block::Block(string _name) : prob(1), name(_name), label_used(false) {}

void Block::append(unique_ptr<Inst> inst) {
  insts.push_back(std::move(inst));
}

void Block::extend(std::list<unique_ptr<Inst>> inst_list) {
  for (auto &i : inst_list) insts.push_back(std::move(i));
}

void Block::insert_before_jump(unique_ptr<Inst> inst) {//******
  auto i = insts.end();
  while (i != insts.begin()) {
    auto prev_i = std::prev(i);
    if ((*prev_i)->as<Branch>() ||(*prev_i)->as<FBranch>()|| (*prev_i)->as<Jump>()) {
      i = prev_i;
    } else {
      break;
    }
  }
  insts.insert(i, std::move(inst));
}

void Block::gen_asm(ostream &out, AsmContext *ctx) {
  ctx->temp_sp_offset = 0;
  if (label_used) out << name << ":\n";
  for (auto &i : insts) i->gen_asm(out, ctx);
}

void Block::print(ostream &out) {
  out << '\n' << name << ":\n";
  for (auto &i : insts) i->print(out);
}

Function::Function(std::string _name): name(_name), entry(nullptr), reg_n(0) {}

void Function::update_reg()
{
  regs.clear();
  regs.reserve(reg_n);
  for (auto &block : blocks) {//遍历函数中的所有block
  decltype(block->insts.begin()) inst;
   for (auto &inst : block->insts){//将寄存器的信息从inst中抽取出来
        for (Reg r : inst->def_reg()) regs[r.id]=r;
        for (Reg r : inst->use_reg()) regs[r.id]=r;
    }
  }
}


void Function::erase_def_use(const OccurPoint &p, Inst *inst) {
  for (Reg r : inst->def_reg()) reg_def[r.id].erase(p);
  for (Reg r : inst->use_reg()) reg_use[r.id].erase(p);
}
//将OccurPoint和每个虚拟寄存器绑定在一起（一个虚拟寄存器可能会对应好几个OccurPoint）,这些信息保存在reg_def、reg_use中
void Function::add_def_use(const OccurPoint &p, Inst *inst) {
  for (Reg r : inst->def_reg()) reg_def[r.id].insert(p);
  for (Reg r : inst->use_reg()) reg_use[r.id].insert(p);
}
//构建一个函数中的所有OccurPoint
void Function::build_def_use() {
  reg_def.clear();
  reg_use.clear();
  reg_def.resize(reg_n);
  reg_use.resize(reg_n);
  OccurPoint p;
  for (auto &block : blocks) {
    p.b = block.get();
    p.pos = 0;
    for (p.it = block->insts.begin(); p.it != block->insts.end();
         ++p.it, ++p.pos) {
      add_def_use(p, p.it->get());
    }
  }
}

void Function::calc_live(RegFilter filter) {
  deque<pair<Block *, Reg>> update;//保存BB和reg之间的关系
  for (auto &block : blocks) {//这里的遍历和block的顺序无关
    //初始化def和use
    block->live_use.clear();
    block->def.clear();

    for (auto it = block->insts.rbegin(); it != block->insts.rend(); ++it) {//倒着遍历这个函数，然后按照书上那个规则确定每条指令中的变量的活跃状态
      for (Reg r : (*it)->def_reg())//遍历指令左边的变量
        if (filter(r) && (r.is_virtual() || integer_allocable(r.id))) {//如果寄存器已经分配完了，或者是普通寄存器（不是控制sp之类的）
          block->live_use.erase(r);//设置为不活跃
          block->def.insert(r);//注意这个def是set类型，不会出现多个相同的寄存器
        }
      for (Reg r : (*it)->use_reg())//遍历指令右边的变量
        if (filter(r) && (r.is_virtual() || integer_allocable(r.id))) {
          block->def.erase(r);
          block->live_use.insert(r);//设置为活跃
        }
    }
    //通过上面的操作，可以得到def之前的use（就是live_in），和use之前的def
    for (Reg r : block->live_use) update.emplace_back(block.get(), r);//将活跃的寄存器和对应的BB保存到update中
    block->live_in = block->live_use;//将live_in保存为live_use，清空live_out
    block->live_out.clear();
  }

  //上面已经完成了live_in的初步计算

  //完成live_out的计算和live_in的更新
  while (!update.empty()) {
    //不断弹出update中的元素
    pair<Block *, Reg> cur = update.front();
    update.pop_front();
    for (Block *prev : cur.first->in_edge)//入度，获取前驱的block
      if (prev->live_out.find(cur.second) == prev->live_out.end()) {//前驱的live_out里面没有当前的寄存器
        prev->live_out.insert(cur.second);//在live——out里面添加这个寄存器
        if (prev->def.find(cur.second) == prev->def.end() &&//如果这个添加的寄存器，不在block的def里面，而且live里面也没有，就将这个寄存器添加到live_in里面
            prev->live_in.find(cur.second) == prev->live_in.end()) {
          prev->live_in.insert(cur.second);
          //说明这个寄存器会带来新的更新（这个寄存器穿过了一个block，可能会传播到上层）
          update.emplace_back(prev, cur.second);
        }
      }
  }
}

map<Reg,int> Function::reg_allocate(bool *succeed) {
  // stat->spill_cnt = 0;
  bool is_float = false;
  
  std::map<Reg,int> color_assign;
  vector<int> ret1,ret2;
  SimpleColoringAllocator allocator(this);
  while (true) {
    ret2 = allocator.run(succeed,false);
    if (*succeed) break;
  }
  while (true) {
    ret1 = allocator.run(succeed,true);
    if (*succeed) break;
  }
  while (true) {
    ret2 = allocator.run(succeed,false);
    if (*succeed) break;
  }
  for (int i = 0; i < ret1.size(); i++) {
    if (ret1[i] != -1) 
    {
      
      if(ret1[i]>32||ret1[i]<-1)std::cout<<"float?"<<ret1[i]<<std::endl;
       color_assign[Reg{i,FP}] = ret1[i];
    }
    else if (ret2[i] != -1) 
    {
      if(ret2[i]>32||ret2[i]<-1)std::cout<<"int?"<<ret2[i]<<std::endl;
       color_assign[Reg{i,INT}] = ret2[i];
    }
  }
  return color_assign;
}

bool Function::check_store_stack() {
  bool ret = true;
  for (auto &block : blocks) {
    int32_t sp_offset = 0;
    for (auto i = block->insts.begin(); i != block->insts.end(); ++i) {
      (*i)->maintain_sp(sp_offset);
      if (auto store_stk = (*i)->as<StoreStack>()) {
        int32_t total_offset =
            store_stk->base->position + store_stk->offset - sp_offset;
        if (!is_imm12(total_offset)) {
          Reg addr{reg_n++};
          block->insts.insert(i, make_unique<LoadStackAddr>(
                                     store_stk->base, addr, store_stk->offset));
          *i = make_unique<Store>(store_stk->src, addr, 0);
          ret = false;
        }
      }
    }
  }
  return ret;
}

void Function::replace_with_reg_alloc(const map<Reg,int> &reg_alloc) {
  for (auto &block : blocks)
    for (auto &inst : block->insts)
      for (Reg *i : inst->regs())
        if (i->is_virtual()){
          i->id = reg_alloc.at(*i);
        }
}

void Function::replace_complex_inst() { //用简单指令代替复杂指令
  for (auto &block : blocks) {
    int32_t sp_offset = 0;
    for (auto i = block->insts.begin(); i != block->insts.end(); ++i) {
      (*i)->maintain_sp(sp_offset);
      if (auto load_stk = (*i)->as<LoadStack>()) {
        int32_t total_offset =
            load_stk->base->position + load_stk->offset - sp_offset;
        Reg dst = load_stk->dst;
        if (is_imm12(total_offset)) {
          *i = make_unique<Load>(dst, Reg{sp}, total_offset,dst.is_float()?Archriscv::Load::Flw:Archriscv::Load::Lw,load_stk->base->size != INT_SIZE);
        } else {
          block->insts.insert(i, make_unique<LoadImm>(dst, total_offset));
          block->insts.insert(
              i, make_unique<RegRegInst>(RegRegInst::Add, dst, Reg{sp}, dst));
          *i = make_unique<Load>(dst, dst, 0,dst.is_float()?Archriscv::Load::Flw:Archriscv::Load::Lw,load_stk->base->size != INT_SIZE);
        }
      } else if (auto store_stk = (*i)->as<StoreStack>()) {

        int32_t total_offset =
            store_stk->base->position + store_stk->offset- sp_offset;
    
        assert(is_imm12(total_offset));
      
        *i = make_unique<Store>(store_stk->src, Reg{sp}, total_offset,store_stk->src.is_float()?Archriscv::Store::Fsw:Archriscv::Store::Sw,store_stk->base->size!= INT_SIZE);
       
       

      } else if (auto load_stk_addr = (*i)->as<LoadStackAddr>()) {//将地址从内存加载到寄存器中
        
        int32_t total_offset =
            load_stk_addr->base->position + load_stk_addr->offset - sp_offset;
        Reg dst = load_stk_addr->dst;
        if (is_imm12(total_offset)) {
          *i = make_unique<RegImmInst>(RegImmInst::Addi, dst, Reg{sp},
                                       total_offset);
        } else {
          block->insts.insert(i, make_unique<LoadImm>(dst, total_offset));
          *i = make_unique<RegRegInst>(RegRegInst::Add, dst, Reg{sp}, dst);
        }
      }
    }
  }
}

void Function::reg_allocate(){
  bool stat;
  while (true) {
    reg_alloc = reg_allocate(&stat);//获得寄存器分配的结果
    stack_size = 0;
    for (auto i = stack_objects.rbegin(); i != stack_objects.rend(); ++i) {//计算栈的大小
    if(stack_size%8!=0&&(*i)->size!=INT_SIZE)
      stack_size+=4; 
    (*i)->position = stack_size;
    stack_size += (*i)->size;
    }
    //
    //获取预设的寄存器,比如ra和s0
    int tmp = reg_alloc.size();
    // std::cout<<tmp<<std::endl;
    save_regs.clear();
    bool used[RegCount] = {};
    bool used_float[RegCount]= {};
    for (auto [reg,i] : reg_alloc) {
      if (i >= 0&&!reg.is_float()) used[i] = true;
      if (i >= 0&&reg.is_float()) used_float[i] = true;
    }
      
    for (int i = 0; i < RegCount; ++i)
    {
      if (REGISTER_USAGE[i] == callee_save && used[i])
        save_regs.emplace_back(i);
      if (REGISTER_USAGE[i] == callee_save && used_float[i])
        save_regs.emplace_back(i,FP);
    }
    int extra = 0;
    if(stack_size%8!=0) extra=4;
    int32_t cur_pos = stack_size + save_regs.size() * INT_SIZE*2 + extra;
    for (auto &i : caller_stack_object) {
      i->position = cur_pos;
      cur_pos += i->size;
    }
    if (check_store_stack()) break;
  }
  replace_with_reg_alloc(this->reg_alloc);
  replace_complex_inst();
}

void Function::get_asm(ostream &out) {
  // optimize_after_reg_alloc(this);//寄存器分配后优化
  out << asm_code.str();
}



void Function::generate_asm_prologue(){
  if (stack_size == 0 && save_regs.size() == 0) return;
  int extra = 0;
  if(stack_size % 8 != 0) extra=4;
  if (-stack_size - static_cast<int32_t>(save_regs.size() * INT_SIZE*2) >=
      IMM12_L) {
    int offset = -stack_size - static_cast<int32_t>(save_regs.size() * INT_SIZE*2) - extra;
    // asm_code.emplace_back(std::move("addi sp, sp, " + std::to_string(offset) + '\n'));
    asm_code << "addi sp, sp, " << offset << '\n';
    int32_t cur_pos = stack_size + extra;
    for (Reg r : save_regs) {
      if(r.is_float())
        // asm_code.emplace_back("fsd")
        asm_code << "fsd " << r << ", " << cur_pos << "(sp)\n";
      else
        asm_code << "sd " << r << ", " << cur_pos << "(sp)\n";
      cur_pos += 8;
    }
  } else {
    if (save_regs.size()) {
      asm_code << "addi sp, sp, -" << save_regs.size() * INT_SIZE*2  << '\n';  
      int32_t cur_pos = 0;
      for (Reg r : save_regs) {
        if(r.is_float())
          asm_code << "fsd " << r << ", " << cur_pos << "(sp)\n";
        else
          asm_code << "sd " << r << ", " << cur_pos << "(sp)\n";
        cur_pos += 8;
      }
    }
    if (stack_size > 0) {
      if (-stack_size >= IMM12_L) {
        asm_code << "addi sp, sp, " << -stack_size << '\n';
      } else {//数组
        int extra=stack_size%8==0?0:4;
        asm_code << "li t0, " << stack_size+extra << '\n';
        asm_code << "sub sp, sp, t0\n";
      }
    }
  }
}

void Function::generate_asm_epilogue(){
  AsmContext ctx;
  ctx.epilogue = [this](std::ostream &asm_code) {
    if (stack_size == 0 && save_regs.size() == 0) return;
    //处理对齐
    int extra=0;
    if(stack_size%8!=0)extra=4;
    if (stack_size + static_cast<int32_t>(save_regs.size() * INT_SIZE*2+extra) <//使用用ra,s0
        IMM12_R) {
      int32_t cur_pos = stack_size +extra;
      for (Reg r : save_regs) {
        if(r.is_float())
          asm_code << "fld " << r << ", " << cur_pos << "(sp)\n";
        else
          asm_code << "ld " << r << ", " << cur_pos << "(sp)\n";
        
        cur_pos += 8;
      }
      asm_code << "addi sp, sp, "
          << stack_size + static_cast<int32_t>(save_regs.size() * INT_SIZE)*2+extra
          << '\n';
    } else {//
      if (stack_size > 0) {
        if (stack_size < IMM12_R) {
          asm_code << "addi sp, sp, " << stack_size << '\n';
        } else {
           int extra=stack_size%8==0?0:4;
          asm_code << "li t0, " << stack_size+extra << '\n';
          asm_code << "add sp, sp, t0\n";
        }
      }
      if (save_regs.size()) {
        int32_t cur_pos = 0;
        for (Reg r : save_regs) {
          if(r.is_float())
            asm_code << "fld " << r << ", " << cur_pos << "(sp)\n";
          else
            asm_code << "ld " << r << ", " << cur_pos << "(sp)\n";
          cur_pos += 8;
        }
        asm_code << "addi sp, sp, " << save_regs.size() * INT_SIZE*2 << '\n';
      }
    }
  };
  for (auto &block : blocks) block->gen_asm(asm_code, &ctx);
}

void Function::print(ostream &out) {
  out << '\n' << name << ":\n[prologue]\n";
  for (auto &block : blocks) block->print(out);
}

Program::Program() : block_n(0) {}

void Program::get_asm(ostream &out) {
  out << asm_code.str();
  for(auto & func : functions)
    func->get_asm(out);

}

void Program::build_code_before_func(){
  bool exist_bss = false, exist_data = false;
  for (auto &obj : global_objects) {
    if (obj->init)//有初始值
      exist_data = true;
    else
      exist_bss = true;
  }
  if (exist_data) {
    asm_code << ".data\n";
    for (auto &obj : global_objects)
      if (obj->init) {
          int32_t *init = reinterpret_cast<int32_t *>(obj->init);
          asm_code << ".align 2\n";
          asm_code << obj->name << ":\n";
          asm_code << "    .4byte ";
          for (int i = 0; i < obj->size / 4; ++i) {
            if (i > 0) asm_code << ", ";
            asm_code << init[i];
          }
          asm_code << '\n';
      }
  }
  if (exist_bss) {
    asm_code << ".bss\n";
    for (auto &obj : global_objects)
      if (!obj->init) {
        if (obj->is_int) asm_code << ".align 2\n";
        asm_code << obj->name << ":\n";
        asm_code << "    .space " << obj->size << '\n';
      }
  }

//sys_clone = 220
 

  asm_code << ".globl main\n";
  asm_code << ".text\n";
}

void Program::build_code_funcs(){

  for (auto &func : functions){
    func->asm_code << '\n' << func->name << ":\n";
    func->generate_asm_prologue();
    func->generate_asm_epilogue();
  }
}

void Program::reg_allocate(){
  for(auto & func : functions){
    func->reg_allocate();
  }
}

void Program::opt_after_reg_allocate(){
  for(auto & func : functions){
    // func->
  }
}

}  // namespace Archriscv