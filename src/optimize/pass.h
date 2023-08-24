#pragma once

#include <cstring>
#include"ir.h"
// #include "ir_priter.h"

using namespace Medium;

struct CFG{
  //cfg
  std::vector<IrBlock *> in, out;
};
struct DOM{
  //dominate tree
  std::vector<IrBlock *> dom_ch;   
  IrBlock *dom_fa = NULL;          //支配树中当前结点的father
 
  int df_start = 0; //深度优先遍历支配树时本结点开始访问的时间
  int df_finish = 0; //深度优先遍历支配树时本结点结束时的时间
};
struct LOOP{
 //loop tree
  std::vector<IrBlock *> loop_ch;
  std::unordered_set<IrBlock *> loop_exit;
  IrBlock *loop_fa = NULL,          //用于代表循环的结点，一般是while的条件判断所在的结点     
          *loop_pre = NULL,         //指向loop的第一个块，不在loop里面
          *loop_pre_exit = NULL,    //可能是循环条件判断的最后一个块，也可能是空
          *loop_last = NULL;        //循环体内部的一个block，直接通向循环的头
  bool loop_rt = 0;//当前结点是不是loop root
  bool loop_simple = 0;  //不是嵌套循环是1
  int loop_depth = 0;     //循环的嵌套层数
};
struct InfoNode {
  CFG cfg;
  DOM dom;
  LOOP loop;
  //else
  int tag = 0;//各种用途
  IrBlock *self = nullptr;//BB自己
  std::unordered_set<IrBlock *> DF;  //支配边界，用于ssa中的phi指令的插入

  //判断自身是否支配s
  bool is_dom(const InfoNode &s) const 
  { 
    return dom.df_start <= s.dom.df_start && s.dom.df_start <= dom.df_finish;
  }

  //判断自身是否严格支配s
  bool is_strict_dom(const InfoNode &s) const 
  {
    return dom.df_start < s.dom.df_start && s.dom.df_start <= dom.df_finish;
  }

  //返回当前结点所在的循环的loop root
  IrBlock *get_loop_rt() const { return loop.loop_rt ? self : loop.loop_fa; }
};


using Dom_Loop_CFG=std::unordered_map<IrBlock *, InfoNode>;



struct SimpleTypeInfo {//通过对应变量来记录寄存器的类型
  Variable *base = NULL;
};

struct ArrayRWInfo {
  std::unordered_set<Reg> r_addr, w_addr;
  bool r_any = 0, w_any = 0;
  void update(const ArrayRWInfo &v) {
    for (Reg x : v.r_addr) r_addr.insert(x);
    for (Reg x : v.w_addr) w_addr.insert(x);
    r_any |= v.r_any;
    w_any |= v.w_any;
  }
};

struct ReduceVar {
  Reg base, step;
  BinaryOp::Type op;
  bool ind_var = 0, reduce_var = 0;
  // variables of form: a <- a op b
  // b is const: ind var, value only depend on loop times
  // b is computed in loop: reduce var
};

struct SimpleCond {
  BinaryOp::Type op;  //<,<=
  bool rev;
  Reg ind, c;
};

struct LoopVarInfo {
  // std::unordered_set<Reg> dep; //TODO: effect of branches ?
  Variable *mem = NULL;  // this is a state of mem, or mem=NULL
  ReduceVar reduce;
};
//每个循环有一个
struct LoopInfo {
  std::unordered_map<Variable *, ArrayRWInfo> rw;

  bool simple = 0;
  // only for simple loop:
  std::optional<SimpleCond> simple_cond;
  std::unordered_map<Reg, LoopVarInfo> loop_var;
  bool parallel_for = 0;
};
//数组读写情况
struct ArrayRW {
  struct ArgInfo {//数组参数读写情况
    bool may_write = 0;//这个数组参数是否被写
    std::unordered_set<Variable *> maybe;
  };
  std::unordered_map<Variable *, bool>global_rw; // global array access not from arg，全局数组
  std::unordered_map<int, ArgInfo> arg_rw;  // arg array access，数组参数
  std::vector<CallInstr *> rec_calls, normal_calls;//调用自己，调用其他函数的call instruction
  std::unordered_map<Variable *, std::unordered_set<Variable *>> alias;//一个变量以及对应的同名变量
  std::unordered_map<Reg, SimpleTypeInfo> types;//寄存器的类型
};
// check whether a Reg is the address in a MemObject
std::unordered_map<Reg, SimpleTypeInfo> simple_type_check(UserFunction *f);

// NormalFunc side effect analysis
// arg array alias analysis
// std::unordered_map<IrFunction *, ArrayRW> get_array_rw(CompileUnit &c);

// check the input
void cfg_check(CompileUnit &c);


// remove RegWriteInstr s.t. d1 is not used and no other side effect
void remove_unused_def(UserFunction *f);

// remove unused NormalFunc
void remove_unused_func(CompileUnit &c);

// remove unused MemObject
void remove_unused_memobj(CompileUnit &c);

// remove pure loop
bool remove_unused_loop(UserFunction *f);

// remove unreachable IrBlocks, merge some IrBlocks
bool dce_BB(UserFunction *f);

// build dominator tree
// build loop tree
Dom_Loop_CFG build_info_node(UserFunction *f);

// construct ssa from tac
// or reconstruct ssa after some optimizations
// assume x is a well-formed def in ssa when check(x)=0
void ssa_construction(UserFunction *f, std::function<bool(Reg)> check = [](Reg r) { return 1; });
void ssa_construction_mem(UserFunction *f, std::function<bool(Reg)> check = [](Reg r) { return 1; });
// void array_ssa_passes(CompileUnit &c, int _check_loop = 0);


// compute def instr of Reg
std::unordered_map<Reg, RegWriteInstr *> map_defreg_inst(UserFunction *f);

// find constants
std::unordered_map<Reg, int> build_const_int(UserFunction *f);

// compute use count of Reg
std::unordered_map<Reg, int> build_use_count(UserFunction *f);

// compute each Instr's parent IrBlock
std::unordered_map<Instr *, IrBlock *> build_in2bb(UserFunction *f);

// compute def instr of Reg in a IrBlock
void get_defs(std::unordered_map<Reg, RegWriteInstr *> &ret, IrBlock *w);

//
std::unordered_map<Reg, RegWriteInstr *> build_defs(UserFunction *f);

// compute use count of Reg in a IrBlock
void get_use_count(std::unordered_map<Reg, int> &ret, IrBlock *w);

// get out edges of a IrBlock
std::vector<IrBlock *> get_BB_out(IrBlock *w);

// topology sort the functions, ret[i] can only call ret[0..i]
std::vector<UserFunction *> get_call_order(CompileUnit &c);

// dfs on dominator tree, pre-order
void dom_tree_dfs(Dom_Loop_CFG &S, std::function<void(IrBlock *)> F);

// dfs on dominator tree, in reversed order of dom_tree_dfs
void dom_tree_dfs_reverse(Dom_Loop_CFG &S, std::function<void(IrBlock *)> F);

// traverse loop tree from w
void loop_tree_for_each_bfs(Dom_Loop_CFG &S, IrBlock *w, std::function<void(IrBlock *)> F);

// check whether w is in loop
bool is_in_loop(Dom_Loop_CFG &S, IrBlock *w, IrBlock *loop);


// when bb_old is renamed to bb_cur, update phi uses second from bb_old to
// bb_cur
void phi_src_rewrite(IrBlock *bb_cur, IrBlock *bb_old);

bool pinned(RegWriteInstr *i);

void into_array_ssa(CompileUnit &c);
void exit_array_ssa(CompileUnit &c);
std::unordered_map<Reg, RegWriteInstr *> build_defs(UserFunction *f) ;
std::unordered_map<Reg, Variable *> infer_array_regs(UserFunction *f);
std::unordered_map<IrBlock *, LoopInfo> check_loop(UserFunction *f,int type);
bool simplify_load_store(UserFunction *f);
std::unordered_map<Reg, float> build_const_float(UserFunction *f);
namespace Pass {
class pass_base {
public:
  pass_base(CompileUnit* unit): c(unit) { };
  void set_name(std::string name) {this->name_ = name;}
  std::string get_name() {return name_;}
  virtual void run() = 0; 
  virtual void run(int n) {}
protected:
    CompileUnit* c;
    std::string name_;
};

class mem2reg_pass : public pass_base {
public:
  mem2reg_pass(CompileUnit* c) : pass_base(c) {};
  ~mem2reg_pass(){};
  virtual void run() override;
protected:
  std::unordered_map<Variable *, Variable *> S;
  void global_int_to_local();
  void mem2reg_local_int();
  void init_global_normal_var(std::unordered_map<Variable *, Variable *>& S);
  void copy_obj_to_local(std::unordered_map<Variable *, Variable *>& S);
  // void local_array_to_global();
};

class gvn_pass : public pass_base {
public:
  gvn_pass(CompileUnit* c) : pass_base(c) { };
  ~gvn_pass(){};
  virtual void run() override;
protected:
  int id;
  UserFunction* curf;//当前函数
  Dom_Loop_CFG bb_dlc;//BB和包含dom、loop、cfg信息的结点的对应关系

  std::unordered_map<Medium::Reg,Medium::RegWriteInstr*> defreg_inst;//def寄存器和对应的指令
  
  std::unordered_map<Variable *, int> vn_addr;//变量和地址

  std::unordered_map<int, int> vn_arg, //函数参数和gvn寄存器编号
                              intconst_vn, //整数常量和gvn寄存器编号
                              vn_intconst;//gvn寄存器编号和整数常数
  std::unordered_map<int,int> intconst_vn_sudden;
  std::unordered_map<int,int> vn_intconst_sudden;

  std::unordered_map<float, int> floatcosnt_vn;//浮点数常量和gvn寄存器编号的映射
  std::unordered_map<int, float> vn_floatconst;//gvn寄存器编号和常量值的映射

  std::unordered_map<std::pair<int, int>, int> vn_uop, vn_mem_read;
  std::unordered_map<std::tuple<int, int, int>, int> vn_op, vn_ai;
  std::unordered_map<Reg, int> reg_vn;//寄存器和gvn寄存器编号
  std::unordered_map<int, Reg> vn_reg;//gvn寄存器编号和寄存器对象
  std::unordered_map<int, std::pair<int, int>> radd_mul;
  std::unordered_map<int, std::pair<Reg, int>> radd_c;
  std::map<std::pair<IrFunction *, std::vector<int>>, int> vn_call;

  std::unordered_map<Reg, Reg> mp_reg;//值相同的两个寄存器，用后者替换前者

  bool sudden_inst;

  void init();
  void do_gvn(UserFunction* f);
  int get_vn_c(int c);
  int get_vn_c_f(float c);
  int get_id(int &x);
  int get_vn_uop(int s1, UnaryOp::Type type);
  int get_vn_op(int s1, int s2, BinaryOp::Type type);
  int get_vn(RegWriteInstr * x0);
  void reg2reg();
  void insert_loadconst(IrBlock* w);
};


class function_inline : public pass_base {
public:
  function_inline(CompileUnit* unit) : pass_base(unit) { };
  ~function_inline(){};
  void func_inline(CompileUnit &c);
  virtual void run() override;

};


// class gcm_schedule_early : public pass_base {
// public:
//   gcm_schedule_early(CompileUnit* unit) : pass_base(unit) { };
//   ~gcm_schedule_early(){};
//   void schedule_early(UserFunction* f);
//   virtual void run() override;

// };

class gcm_schedule_late : public pass_base {
public:
  gcm_schedule_late(CompileUnit* unit) : pass_base(unit) { };
  ~gcm_schedule_late(){};
  void schedule_late(UserFunction* f);
  virtual void run() override;

};

class move_condtion_out_of_loop : public pass_base {
public:
  move_condtion_out_of_loop(CompileUnit* unit) : pass_base(unit) { };
  ~move_condtion_out_of_loop(){};
  bool condtion_out_of_loop(UserFunction* f);
  virtual void run() override;

};


class inst_schedule : public pass_base {
public:
  inst_schedule(CompileUnit* unit) : pass_base(unit) { };
  ~inst_schedule(){};
  void _inst_schedule(UserFunction* f);
  virtual void run() override;

};


class loop_unroll : public pass_base {
public:
  loop_unroll(CompileUnit* unit) : pass_base(unit) { };
  ~loop_unroll(){};
  void _loop_unroll(const std::unordered_map<IrBlock *, LoopInfo> &LI, UserFunction *f);
  virtual void run() override;
};

class while_true_round : public pass_base {
public:
  while_true_round(CompileUnit* unit) : pass_base(unit) { };
  ~while_true_round(){};
  virtual void run() override;
protected:
  void loop_tree_dfs(UserFunction *f, std::function<void(IrBlock *)> F);
  void remove_dull_block(UserFunction* f);
  void convert_to_double_while(UserFunction* f);
  bool check_loop_condition(IrBlock* bb);
  Dom_Loop_CFG S;
};

class loop_parallel : public pass_base {
public:
  loop_parallel(CompileUnit* unit) : pass_base(unit) {};
  ~loop_parallel(){}
  void _loop_parallel(const std::unordered_map<IrBlock *, LoopInfo> &LI, CompileUnit &c,UserFunction *f);
  virtual void run() override;
protected:
  void compute_data_offset(CompileUnit &c);
  void code_reorder(UserFunction *f);
  void compute_thread_local(CompileUnit &c);
  void for_each_phi(IrBlock *w, std::function<void(PhiInstr *)> F);
  std::unordered_map<IrBlock *, double> estimate_BB_prob(UserFunction *f);
  void seperate_loops(UserFunction *f);
  void loop_tree_dfs(Dom_Loop_CFG &S, UserFunction *f, std::function<void(IrBlock *)> F);
};


class array2var : public pass_base {
public:
  array2var(CompileUnit* unit) : pass_base(unit) { };
  ~array2var(){};
  void _array2var(UserFunction *f);
  virtual void run() override;
protected:
  int id;
  UserFunction* curf;//当前函数
  Dom_Loop_CFG bb_dlc;//BB和包含dom、loop、cfg信息的结点的对应关系

  std::unordered_map<Medium::Reg,Medium::RegWriteInstr*> defreg_inst;//def寄存器和对应的指令
  
  std::unordered_map<Variable *, int> vn_addr;//变量和地址

  std::unordered_map<int, int> vn_arg, //函数参数和gvn寄存器编号
                              intconst_vn, //整数常量和gvn寄存器编号
                              vn_intconst;//gvn寄存器编号和整数常数
  std::unordered_map<int,int> intconst_vn_sudden;
  std::unordered_map<int,int> vn_intconst_sudden;

  std::unordered_map<float, int> floatcosnt_vn;//浮点数常量和gvn寄存器编号的映射
  std::unordered_map<int, float> vn_floatconst;//gvn寄存器编号和常量值的映射

  std::unordered_map<std::pair<int, int>, int> vn_uop, vn_mem_read;
  std::unordered_map<std::tuple<int, int, int>, int> vn_op, vn_ai;
  std::unordered_map<Reg, int> reg_vn;//寄存器和gvn寄存器编号
  std::unordered_map<int, Reg> vn_reg;//gvn寄存器编号和寄存器对象
  std::map<std::pair<IrFunction *, std::vector<int>>, int> vn_call;

  std::unordered_map<Reg, Reg> mp_reg;//值相同的两个寄存器，用后者替换前者

  std::unordered_map<std::pair<Reg, int>, int> arrayindex_vn;//已知地址和对应的gvn编号
  std::unordered_map<int, std::pair<Reg, int>> vn_arrayindex;//gvn编号和对应已知地址
  std::unordered_map<int, int> arrayindexval_int_vn;//已知地址最新的值和对应的
  std::unordered_map<int, int> vn_arrayindexval_int;//gvn编号和对应已知地址的值
  std::unordered_map<float, int> arrayindexval_float_vn;//已知地址最新的值和对应的
  std::unordered_map<int, float> vn_arrayindexval_float;//gvn编号和对应已知地址的值
  std::unordered_map<Reg,int> arrayaddr_vn;//保存有数组地址的寄存器和对应的<array,offet>的vn的对应关系
  std::unordered_map<int,Reg> vn_arrayaddr;//上面的反向对应关系
  std::list<Instr*> deletable_store;//可以删除的指令

  void init();
  int get_vn_c(int c);
  int get_vn_c_f(float c);
  int get_id(int &x);
  int get_vn_uop(int s1, UnaryOp::Type type);
  int get_vn_op(int s1, int s2, BinaryOp::Type type);
  int get_vn(RegWriteInstr * x0);
  void reg2reg();
  bool checkaddr(IrBlock* w);
  bool check_fun( UserFunction *f);
  void delete_memwrite(UserFunction *f);
  int getarrayindex(std::pair<Reg,int>);//如果有返回对应的函数的地址，如果没有创建一个新的vn
  bool succeed=true;
  bool can_delete=true;

  struct Load2LoadConst
  {
    bool available=false;
    bool is_int=true;
    int int_value;
    float float_value;
  };
  Load2LoadConst load2loadconst;
};


class recursive_cut_branch : public pass_base
{
public:
  recursive_cut_branch(CompileUnit* unit) : pass_base(unit) { };
  ~recursive_cut_branch() {};
  virtual void run() override;
protected:
  bool func_check(UserFunction* f);
  void insert_remember_inst(UserFunction*f);
  int type;
  Medium::Reg cur_index;
};

class PassManager {
private:
  CompileUnit * c;
  
  std::vector<pass_base*> mypass;
  std::vector<bool> print_ir_opt;
public:
  PassManager(CompileUnit * unit) : c(unit) {};
  void add_pass(std::string pass_name,bool is_print = false) {
    if (pass_name == "mem2reg") {
      mypass.emplace_back(std::move(new mem2reg_pass(c)));
      mypass.back()->set_name("mem2reg");
      print_ir_opt.emplace_back(is_print);
    }else if (pass_name == "gvn") {
      mypass.emplace_back(std::move(new gvn_pass(c)));
      mypass.back()->set_name("gvn");
      print_ir_opt.emplace_back(is_print);
    }
    else if (pass_name == "function_inline"){
      mypass.emplace_back(std::move(new function_inline(c)));
      mypass.back()->set_name("function_inline");
      print_ir_opt.emplace_back(is_print);
    }

    // else if (pass_name == "gcm_schedule_early"){
    //   mypass.emplace_back(std::move(new gcm_schedule_early(c)));
    //   mypass.back()->set_name("gcm_schedule_early");
    //   print_ir_opt.emplace_back(is_print);
    // }
    else if (pass_name == "gcm_schedule_late"){
      mypass.emplace_back(std::move(new gcm_schedule_late(c)));
      mypass.back()->set_name("gcm_schedule_late");
      print_ir_opt.emplace_back(is_print);
    }
    else if (pass_name == "move_condtion_out_of_loop"){
      mypass.emplace_back(std::move(new move_condtion_out_of_loop(c)));
      mypass.back()->set_name("move_condtion_out_of_loop");
      print_ir_opt.emplace_back(is_print);
    }
    else if (pass_name == "inst_schedule"){
      mypass.emplace_back(std::move(new inst_schedule(c)));
      mypass.back()->set_name("inst_schedule");
      print_ir_opt.emplace_back(is_print);
    }
    else if (pass_name == "while_true_round"){
      mypass.emplace_back(std::move(new while_true_round(c)));
      mypass.back()->set_name("while_true_round");
      print_ir_opt.emplace_back(is_print);
    }

    
     else if (pass_name == "loop_unroll"){
      mypass.emplace_back(std::move(new loop_unroll(c)));
      mypass.back()->set_name("loop_unroll");
      print_ir_opt.emplace_back(is_print);
    }


    else if (pass_name == "array2var"){
      mypass.emplace_back(std::move(new array2var(c)));
      mypass.back()->set_name("array2var");
      print_ir_opt.emplace_back(is_print);
    }
    else if(pass_name == "recursive_cut_branch"){
      mypass.emplace_back(std::move(new recursive_cut_branch(c)));
      mypass.back()->set_name("recursive_cut_branch");
      print_ir_opt.emplace_back(is_print);
    }
    else assert(0);
  }
  void run_pass() {
    for (int i = 0; i < mypass.size(); i++) {
      mypass[i]->run();
      // if(ir_print_ir && print_ir_opt[i])
      //   ir_print_ir->visit(*c);
    }
  }
  ~PassManager(){};
};

} // namespace Pass
namespace ExprUtil {

struct MulExpr  {
  std::multiset<Reg> terms;
  MulExpr(Reg r) { terms.insert(r); }
  // t1*t2*...*tn*c
  bool operator<(const MulExpr &w) const { return terms < w.terms; }
  MulExpr operator*(const MulExpr &w) const {
    MulExpr res = *this;
    res.terms.insert(w.terms.begin(), w.terms.end());
    return res;
  }
  size_t size() const { return terms.size(); }

};

struct AddExpr  {
  std::map<MulExpr, int64_t> terms;
  int64_t c = 0;
  AddExpr() {}
  AddExpr(Reg r) { terms[MulExpr(r)] = 1; }
  AddExpr(int64_t value) { c = value; }
  bool is_zero() const { return !c && terms.empty(); }
  bool is_const() const { return terms.empty(); }
  bool is_const(std::function<bool(Reg)> f) const {
    for (auto &kv : terms) {
      for (Reg r : kv.first.terms)
        if (!f(r)) return 0;
    }
    return 1;
  }
  // t1+t2+...+tn+c
  AddExpr operator+(const AddExpr &w) const { return add(w, 1); }
  AddExpr operator-(const AddExpr &w) const { return add(w, -1); }
  AddExpr operator-() const { return add(AddExpr(), -1); }
  AddExpr operator*(int64_t x) const { return *this * AddExpr(x); }
  AddExpr operator*(const AddExpr &w) const {
    AddExpr res;
    for (auto &kv1 : terms) {
      res.terms[kv1.first] += kv1.second * w.c;
      for (auto &kv2 : w.terms) {
        res.terms[kv1.first * kv2.first] += kv1.second * kv2.second;
      }
    }
    for (auto &kv2 : w.terms) {
      res.terms[kv2.first] += c * kv2.second;
    }
    for (auto it = res.terms.begin(); it != res.terms.end();) {
      if (it->second)
        ++it;
      else {
        auto del = it++;
        res.terms.erase(del);
      }
    }
    res.c += c * w.c;
    return res;
  }
  std::optional<std::pair<AddExpr, AddExpr>> as_linear(Reg r) const {
    std::pair<AddExpr, AddExpr> res;
    for (auto &kv : terms) {
      int x = kv.first.terms.count(r);
      if (x == 0)
        res.first.terms[kv.first] = kv.second;
      else if (x == 1) {
        auto key = kv.first;
        key.terms.erase(r);
        if (key.terms.empty())
          res.second.c = kv.second;
        else
          res.second.terms[key] = kv.second;
      } else
        return std::nullopt;
    }
    res.first.c = c;
    return res;
  }
  AddExpr add(const AddExpr &w, int64_t k) const {
    AddExpr res = *this;
    for (auto &kv : w.terms) {
      if (!(res.terms[kv.first] += kv.second * k)) {
        res.terms.erase(res.terms.find(kv.first));
      }
    }
    res.c += w.c * k;
    return res;
  }
  size_t size() const {
    size_t s = 0;
    for (auto &kv : terms) s += kv.first.size();
    return s;
  }
 
};

struct ArrayIndexExpr  {
  std::vector<std::optional<AddExpr>> terms;
  ArrayIndexExpr operator-(const ArrayIndexExpr &w) const {
    size_t n = terms.size();
    assert(n == w.terms.size());
    ArrayIndexExpr res;
    for (size_t i = 0; i < n; ++i) {
      if (terms[i] && w.terms[i])
        res.terms.emplace_back(*terms[i] - *w.terms[i]);
      else
        res.terms.emplace_back();
    }
    return res;
  }
  // mem[t1][t2]...[tn]
 
};

}  // namespace ExprUtil

