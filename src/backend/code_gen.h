#pragma once

#include <functional>
#include <map>
#include <sstream>

#include "arch.h"
#include "def.h"
#include "ir.h"



std::string generate_name(const std::string &s);

namespace Archriscv {

struct Inst;
using RegFilter = std::function<bool(const Reg &)>;
// struct RegAllocStat {
//   // int spill_cnt, move_eliminated, callee_save_used;//这三个变量分别为溢出的寄存器数量，消除的复制语句（就是直接替换掉），被调用者保存寄存器数量？
//   bool succeed;
// };

struct AsmContext {
  //汇编上下文，包含临时堆栈偏移量（temp_sp_offset）和在生成汇编代码时调用的后续函数（epilogue）。
  int32_t temp_sp_offset;
  std::function<void(std::ostream &)> epilogue;
};

struct CmpInfo {
  //用于保存比较操作的信息，包括比较类型（type）、左操作数（lhs）和右操作数（rhs）所对应的寄存器。
  Compare type;
  Reg lhs, rhs;
};
struct GlobalObject;
struct Block {
  double prob;
  std::string name;
  bool label_used;
  std::list<std::unique_ptr<Inst>> insts;
  std::vector<Block *> in_edge, out_edge;
  std::set<Reg> live_use, def, live_in, live_out;
  std::vector<std::unique_ptr<GlobalObject>> global_objects;


  Block(std::string _name);  //初始化
  void append(std::unique_ptr<Inst> inst); //向基本块中添加一条指令？
  void extend(std::list<std::unique_ptr<Inst>> inst_list); //添加一堆指令？
  void insert_before_jump(std::unique_ptr<Inst> inst);  //在跳转指令之前插入指令
  void gen_asm(std::ostream &out, AsmContext *ctx); //生成汇编
  void print(std::ostream &out); //打印
};

struct Program;

struct OccurPoint {
  Block *b;//所属的BB
  std::list<std::unique_ptr<Inst>>::iterator it;//指向自己指令的一个迭代器
  int pos;//在块中的位置
  //用于比较两条指令的位置
  bool operator<(const OccurPoint &y) const {
    if (b != y.b)
      return std::less<Block *>{}(b, y.b);
    else
      return pos < y.pos;
  }
};

struct Function {
  std::string name;  //函数名称
  std::vector<std::unique_ptr<Block>> blocks;  //存储函数的基本块列表。
  std::vector<std::unique_ptr<StackObject>> stack_objects,  //存储函数的栈对象列表，表示在函数中分配的栈对象。
      caller_stack_object;  // caller_stack_object is for argument
  std::vector<Reg> arg_reg;
  Block *entry;
  std::vector<Reg> regs;//ccx adds it,表示所有寄存器
  //下面这些寄存器是预留的寄存器吗？
  std::set<Reg> spilling_reg;
  std::map<Reg,  int32_t> constant_reg;//保存了常量的寄存器，用于保存int
  std::map<Reg,  float> constant_reg_float;
  std::map<Reg, std::string> symbol_reg;//保存了跳转地址的寄存器
  std::map<Reg, std::pair<StackObject *, int32_t>> stack_addr_reg;
  int reg_n;
  std::vector<int> all_size;
  std::stringstream asm_code;
  std::map<Reg,int> reg_alloc;

  std::vector<std::set<OccurPoint>> reg_def, reg_use;  //记录reg定义和reg use

  Function(std::string _name);
  void update_reg();//根据指令，将寄存器放到regs这个数据结构中
  void erase_def_use(const OccurPoint &p, Inst *inst);
  void add_def_use(const OccurPoint &p, Inst *inst);
  void build_def_use();
  void calc_live(RegFilter filter = [](Reg reg){
    return true;
  });
  
  void get_asm(std::ostream &out);
  std::vector<Reg> save_regs;
  int stack_size;
  void reg_allocate();
  void generate_asm_prologue();
  void generate_asm_epilogue();
  void print(std::ostream &out);

 private:
  std::map<Reg,int> reg_allocate(bool *succeed);
  bool check_store_stack();  // if a StoreStack instruction immediate offset is
                             // out of range, replace with
                             // LoadImm + Add + Store
                             // and return false, else return true
  void replace_with_reg_alloc(const std::map<Reg,int> &reg_alloc);
  void replace_complex_inst();  // replace LoadStack, StoreStack and LoadStackAddr 替换复杂指令
};


struct GlobalObject {
  std::string name;
  int size;  // only available when is_int
  void *init;
  bool is_int, is_const , is_float; //xts
};

struct Program {
  std::vector<std::unique_ptr<Function>> functions;
  std::vector<std::unique_ptr<GlobalObject>> global_objects;
  int block_n;
  std::stringstream asm_code;
  void build_code_before_func();
  void build_code_funcs();
  void reg_allocate();
  void opt_after_reg_allocate();
  Program();
  void get_asm(std::ostream &out);
};

}  // namespace Archriscv