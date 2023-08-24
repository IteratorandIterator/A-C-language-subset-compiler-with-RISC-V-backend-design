#include "pass.h"
//这个函数大概就是将main函数中用到全局非数组变量的地方，将全局变量放到main函数的栈中
using namespace Pass;

//这个函数的作用是根据变量是否在寄存器或内存中，将ld和st指令优化成mv
//思路很简单，就是每个局部变量对应一个寄存器，然后将针对局部变量的操作转化为针对寄存器的mv指令，产生的冗余指令和不符合ssa在最后的construct ssa函数里面处理
void mem2reg_pass::mem2reg_local_int() {
  struct State {
    Variable *m = NULL;
    Reg newreg;
  };
  auto del_load_store=[&](UserFunction* f){
    
    std::unordered_map<Reg, State> reg_mem;        // map reg to addr of memobj
    std::unordered_map<Variable *, Reg> mem_reg;  // map memobj to reg
    std::unordered_set<Reg> new_reg;//用于保存需要加载为0的寄存器
    auto init_local_var=[&](Instr *x) {
      if(auto x0 = dynamic_cast<LoadAddr*>(x)) {
        if (!x0->offset->global && x0->offset->is_normal_var()) {  //只对局部变量进行考虑，注意，现在已经完成了main函数中的全局变为局部的操作
          //绑定loadaddr中的reg和memobj的对应关
          auto &rm = reg_mem[x0->d1];
          rm.m = x0->offset;
          auto &mr = mem_reg[x0->offset];
          mr.is_int=x0->offset->is_int;//*****
          //如果使用了0号寄存器，变成之间加载一个0到寄存器里面去
          if (!mr.id && mr.is_int) new_reg.insert(mr = f->new_Reg(x0->offset->name,true));//如果是零号寄存器，就需要创建一个新的寄存器
          if (!mr.id && !mr.is_int) new_reg.insert(mr = f->new_Reg(x0->offset->name,false));
          rm.newreg = mr;
        }
      }
    };
    for (auto &bb : f->bbs) 
    {
         for (auto &x : bb->instrs) init_local_var(x.get());
    }
   
    //完成加载一个0到寄存器里面的操作,这些似乎需要通过后面的ssa_construction进行调整
    for (Reg r : new_reg) {
      if (r.is_int) {
        f->entry->push_front(new LoadConst(r, static_cast<int>(0)));
      }else{
        f->entry->push_front(new LoadConst(r, static_cast<float>(0)));
      }
    }
    auto loadstore2mv=[&](IrBlock *bb) {
      for (auto it = bb->instrs.begin(); it != bb->instrs.end(); ++it) {
        if(auto x0 = dynamic_cast<LoadInstr*>(it->get())) {
          if (reg_mem.count(x0->addr)) {//如果变量的值已经在寄存器中了，就直接用mv就行了
            Reg d1 = x0->d1, s1 = reg_mem[x0->addr].newreg;
            *it = std::unique_ptr<Instr>(new UnaryOpInstr(d1, s1, UnaryOp::ID));
          }
        }
        else if(auto x0 = dynamic_cast<StoreInstr*>(it->get())){//如果变量的值已经在寄存器中了，就直接改该寄存器的值就行了
          if (reg_mem.count(x0->addr)) {
            Reg d1 = reg_mem[x0->addr].newreg, s1 = x0->s1;
            *it = std::unique_ptr<Instr>(new UnaryOpInstr(d1, s1, UnaryOp::ID));
          }
        }
      }
    };
    for (auto &bb : f->bbs) loadstore2mv(bb.get());

    ssa_construction_mem(f, [&](Reg r) { return new_reg.count(r); });//将新添加的虚拟寄存器进行构建SSA操作
  };

  for (auto &kv : c->funcs) del_load_store(kv.second.get());

}


 void  mem2reg_pass::init_global_normal_var(std::unordered_map<Variable *, Variable *>& VarMap)
 {
  auto init_global_var=[&](Variable *VarTmp) {
    if (VarTmp->is_normal_var()) {
      /*
        全局变量或者局部变量，并且不是函数参数，并且4字节，并且不是数组，即为single_var
      */
      VarMap[VarTmp] = NULL;
    }
  };
  for(auto &i : c->scope.objects)
  {
     init_global_var(i.get());
  }
  //如果在不是man函数里面的其他函数使用了全局变量，那么从S中删除这个变量

  auto erase_non_main_global_var=[&](UserFunction *f) {
    if (f->name != "main") {

      auto erase_inst=[&](Instr *i) {//遍历非main函数中的所有LoadAddr指令，如果发现加载的对象是全局变量，就将这个变量在S中删除，就是在S中删除非main函数中的全局变量
        if(auto i0 = dynamic_cast<LoadAddr*>(i)) {
          // loadaddr
          // 1. 如果是全局变量，在后端变为loadlabel，汇编代码变为la
          // 2. 如果是局部变量，根据立即数的长短变为 li 或者 addi sp
          auto VarTmp = i0->offset;
          if (VarTmp->global) {
            VarMap.erase(VarTmp);
          }
        }
      };
      for (auto &bb : f->bbs) 
      {
          // bb->for_each(erase_inst);
          for (auto &x : bb->instrs) 
            erase_inst(x.get());
      }
    }
  };
  for (auto &kv : c->funcs) erase_non_main_global_var(kv.second.get());
 }



void mem2reg_pass::copy_obj_to_local(std::unordered_map<Variable *, Variable *>& VarMap)
{
   //现在剩下只在main里面出现过的全局变量和main里面的局部变量
  UserFunction *main = c->funcs["main"].get();
//创建map中的second，和first保持一致，但是这边为什么要用main函数的scope创建呢？
  //给每一个ir_obj包括偏移量，创建一个新的对象
  for (auto &kv : VarMap) {
    kv.second = main->scope.new_MemObject(kv.first->name);

    kv.second->is_int = kv.first->is_int;
    kv.second->is_float = kv.first->is_float;
    kv.second->is_const = kv.first->is_const;

    kv.second->size = 4;


  }
//将main中所有loadaddr指令换成之前新产生的stackobj的偏移
  //把la指令的偏移量变成刚才那个对象
  auto replace_obj=[&](Instr *i) {
    if(auto i0 = dynamic_cast<LoadAddr*>(i)) {
      //找到addr指令的偏移量
      auto &m = i0->offset;
      //在S里面找偏移量
      auto it = VarMap.find(m);
      //把m换成刚才new出来的那个obj
      if (it != VarMap.end()) m = it->second;
    }
  };
for (auto &bb : main->bbs)
{
  for (auto &x : bb->instrs) replace_obj(x.get());
} 



  //把原来的东西取出来，放到另一个地方去
  for (auto &kv : VarMap) {
    auto &s = main->entry->instrs;
    Reg r1 = main->new_Reg(true);//地址
    Reg r2 = main->new_Reg(kv.first->is_int);//数值
    Reg r3 = main->new_Reg(true);//地址
    //我怎么感觉下面的顺序反了，应该是先将地址放到对应的寄存器中，然后再将具体的数值放到指定的地方
    //Cao,原来下面是往前面插的，所以顺序应该是反过来的。就是将first中的值先放到r2中，然后将r2中的值放到second指定的地方
    s.emplace_front(new StoreInstr(r3, r2));//将r2中的值保存到r3指向的地址
    s.emplace_front(new LoadAddr(r3, kv.second));//将新建的变量的地址放到r3中
    s.emplace_front(new LoadInstr(r2, r1));//将r1指向的内容放到r2中
    s.emplace_front(new LoadAddr(r1, kv.first));//将原来的变量的地址放到r1中
  }
}


void mem2reg_pass::global_int_to_local() {
  //将非参数、非数组的变量作为map的key
  S.clear();
  init_global_normal_var(S);
  copy_obj_to_local(S);
}


// void mem2reg_pass::local_array_to_global() {}

void mem2reg_pass::run() {
  global_int_to_local();
  mem2reg_local_int();
  remove_unused_memobj(*c);
}
