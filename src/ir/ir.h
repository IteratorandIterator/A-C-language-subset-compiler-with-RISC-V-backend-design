#pragma once

#include <algorithm>
#include <cassert>
#include <deque>
#include <optional>
#include <functional>
#include <unordered_map>
#include <iostream>
#include <set>
#include <list>
#include <map>
#include <vector>
#include <memory>
#include <queue>
#include <stack>
#include <unordered_set>

using MemSize = uint64_t;
constexpr MemSize INT_SIZE = 4;

namespace Medium {
using std::list;
using std::to_string;
using std::ostream;
using std::string;
using std::set; 
using std::function;

using std::unique_ptr;
using std::vector;

struct Variable;
struct IrFunction;
struct IrBlock;
struct UserFunction;
struct MediumScope;
struct LibFunction;
struct CompileUnit;
struct Instr;
struct Reg;
struct ir_visitor;

struct Ir_Node {
  virtual void accept(ir_visitor &visitor)  = 0;
  virtual ~Ir_Node() {}
};

// ostream &operator<<(ostream &os, const Ir_Node &r);

// template <class T>
// ostream &operator<<(ostream &os, const std::optional<T> &r) {
//   if (r)
//     os << r.value();
//   else
//     os << "(nullopt)";
//   return os;
// }

struct Reg : Ir_Node {//float类型应该要重新编号
  int id;
  bool is_int;
  int size;
  Reg(int id = 0,bool is_int=1,int _size=4) : id(id),is_int(is_int),size(_size) {}
  // id is unique in a NormalFunc

  bool operator==(const Reg &r) const { return id == r.id; }
  void accept(ir_visitor &visitor)  override;
  bool operator<(const Reg &r) const { return id < r.id; }
};
}  // namespace IR
namespace std {
template <>
struct hash<Medium::Reg> {
  size_t operator()(const Medium::Reg &r) const { return r.id; }
};
}  // namespace std

namespace Medium {
struct Variable : Ir_Node {
  // data stored in memory
  string name;
  int size = 0;    // number of bytes, size%4==0
  int offset = 0;  // offset from gp or sp, computed after machine irrelavant
                   // optimization
  bool global;
  bool arg = 0;
  // global=0 arg=0: local variable, but not arg
  // global=0 arg=1: arg of array type
  // global=1 arg=0: global variable
  // global=1 arg=1: stdin or stdout, for side effect analysis
  void *initial_value = NULL;
  bool is_int = 1;
  // only for global=1, NULL: zero initialization, non-NULL: init by these bytes

  bool is_const = 0;
  bool is_float =0;//
  // computed in optimize_passes

  std::vector<int> dims;
  // only for int array, array dim size

  void init(int32_t *x, int size) {
    initial_value = x;
    is_int = 1;
    this->size = size;
  }
  // void init(char *x, int size) {
  //   initial_value = x;
  //   is_int = 0;
  //   this->size = size;
  // }
  void init(float * x,int size){
    initial_value = x;
    is_int = 0;
    this->size = size;
  }
  bool is_normal_var() { return !arg && size == 4 && dims.empty(); }
  void accept(ir_visitor &visitor) override;
  int at(int x) {
    if (x % 4 != 0) return 0;                  // assert
    if (!(0 <= x && x + 4 <= size)) return 0;  // assert
    if (!initial_value) return 0;
    return ((int *)initial_value)[x / 4];
  }

 private:
 
  friend struct MediumScope;
  Variable(string name, bool global) : name(name), global(global) {}
  MediumScope *fa;
};

struct MediumScope : Ir_Node {
  string name;
  bool global;
  // global=0: stack frame
  // global=1: global variables
  std::unordered_map<int, Variable *> array_args;
  // map arg of array type to global=0,arg=1 MemObject
  std::unordered_map<Variable *, int> array_arg_id;
  // map global=0,arg=1 MemObject to arg id in 0..arg_cnt-1
  vector<unique_ptr<Variable>> objects;
  // list of MemObjects in this scope

  int size;
  // computed after machine irrelavant optimization
  Variable *new_MemObject(string _name) {
    Variable *m = new Variable(name + "::" + _name, global);
    objects.emplace_back(m);
    m->fa = this;
    return m;
  }
  void for_each(function<void(Variable *)> f) {
    for (auto &x : objects) f(x.get());
  }
  void for_each(function<void(Variable *, Variable *)> f) {
    for (auto &x : objects) {
      auto y = new Variable(*x);
      f(x.get(), y);
    }
  }
  void add(Variable *m) {
    assert(!m->global);
    m->name = name + "::" + m->name;
    objects.emplace_back(m);
    m->fa = this;
  }
  void set_arg(int id, Variable *m) {
    assert(has(m));
    assert(!array_args.count(id));
    assert(!array_arg_id.count(m));
    m->arg = 1;
    array_args[id] = m;
    array_arg_id[m] = id;
  }
  void accept(ir_visitor &visitor)  override;
  bool has(Variable *m) { return m && m->fa == this; }
 private:
  friend struct CompileUnit;
  friend struct UserFunction;
  MediumScope(string name, bool global) : name(name), global(global) {}
};

struct IrBlock;

struct Instr : Ir_Node {
  // IR instruction
  void map_use(function<void(Reg &)> f1);
  void map_BB(std::function<void(IrBlock *&)> f) {
    map([](auto &x) {}, f, [](auto &x) {}, 0);
  }
  Instr *copy() {
    return map([](Reg &) {}, [](IrBlock *&) {}, [](Variable *&) {}, 1);
  }
  
  Instr *map(function<void(Reg &,std::map<Medium::Reg, Medium::Reg>&)> f1, 
             function<void(IrBlock *&,std::map<IrBlock *, IrBlock *>&)> f2,
             function<void(Variable *&,std::map<Variable *, Variable *>&)> f3, bool copy = 1);
  Instr *map(function<void(Reg &)> f1, function<void(IrBlock *&)> f2,
             function<void(Variable *&)> f3, bool copy = 1);
  // copy this Instr, and map each Reg by f1, map each BB* by f2, map each
  // MemObject by f3
  // virtual void accept(ir_visitor &visitor) const override;
};
struct PhiInstr;
struct IrBlock : Ir_Node {
  // basic block
  string name;
  list<unique_ptr<Instr>> instrs;
  int id;
  bool disable_schedule_early = 0;
  // list of instructions in this basic block
  // the last one is ControlInstr, others are RegWriteInstr or StoreInstr
  //void print(ostream &os) const override;
  void accept(ir_visitor &visitor) override;
  void for_each(function<void(Instr *)> f) {
    for (auto &x : instrs) f(x.get());
  }
  //遍历每一条指令
  bool for_each_until(function<bool(Instr *)> f) {
    for (auto &x : instrs)
      if (f(x.get())) return 1;
    return 0;
  }
  void push_front(Instr *x) { instrs.emplace_front(x); }
  void push(Instr *x) { instrs.emplace_back(x); }
  void push1(Instr *x) { instrs.insert(--instrs.end(), unique_ptr<Instr>(x)); }
  void pop() { instrs.pop_back(); }
  Instr *back() { return instrs.back().get(); }
  void map_BB(std::function<void(IrBlock *&)> f) {
    for (auto &x : instrs) x->map_BB(f);
  }

 private:
  friend struct UserFunction;
  IrBlock(string name) : name(name) {}
};

struct IrFunction : Ir_Node {
  // function
  string name;
  bool ignore_return_value = 0;
  IrFunction(string name) : name(name) {}
};

struct LibFunction : IrFunction {
  // extern function
  //void print(ostream &os) const override { os << "LibFunc: " << name; }
  std::unordered_map<int, bool> array_args;
  // read/write args of array type
  // (arg_id,1): read and write
  // (arg_id,0): read only
  bool in = 0,
       out = 0;  // IO side effect, in: stdin changed, out: stdout changed

 void accept(ir_visitor &visitor)  override;
 private:
  friend struct CompileUnit;
  LibFunction(string name) : IrFunction(name) {}
};

struct UserFunction : IrFunction {
  //ccx adds function type
  bool is_int=0;
  bool is_float=0;
  // function defined in compile unit (.sy file)
  MediumScope scope;
  // local variables on stack, and args of array type
  IrBlock *entry = NULL;
  // first basic block to excute
  vector<unique_ptr<IrBlock>> bbs;
  // list of basic blocks in this function
  int max_reg_id = 0,max_reg_id_float = 0,max_reg_id_int=0, max_bb_id = 0;
  // for id allocation
  vector<std::pair<string,int>> reg_names;
  //function info
  vector<int> arg_info;//true表示float
  std::unordered_set<Reg> thread_local_regs;
  Reg new_Reg(bool is_int,int size=4) { return new_Reg("R" + string(is_int?"i":"f")+to_string(is_int?max_reg_id_int + 1:max_reg_id_float + 1),is_int,size); }//在浮点数寄存器的名字R后面加了一个f**********
  Reg new_Reg(string _name,bool is_int,int size=4) {
    reg_names.push_back({_name,size});
    // reg id : 1 .. max_reg_id
    // 1 .. param_cnt : arguments
    if(is_int)
    {
      max_reg_id_int++;
      return Reg(++max_reg_id,is_int,size);
    }
    else   {
      max_reg_id_float++;
      return Reg(++max_reg_id,is_int,size);
    }
  }
  IrBlock *new_BB(string _name = "BB") {
    IrBlock *bb = new IrBlock(name + "::" + _name + to_string(++max_bb_id));
    bbs.emplace_back(bb);
    return bb;
  }
  void for_each(function<void(IrBlock *)> f) {
    for (auto &bb : bbs) f(bb.get());
  }
  bool for_each_until(function<bool(IrBlock *)> f) {
    for (auto &bb : bbs)
      if (f(bb.get())) return 1;
    return 0;
  }
  void for_each(function<void(Instr *)> f) {
    for (auto &bb : bbs) bb->for_each(f);
  }
  void accept(ir_visitor &visitor) override;
  string get_name(Reg r) const { return reg_names.at(r.id).first; }

 private:
  friend struct CompileUnit;
  UserFunction(string name) : IrFunction(name), scope(name, 0) {
    reg_names.push_back({"?",4});
  }
};

struct CompileUnit : Ir_Node {
  // the whole program
  MediumScope scope;  // global arrays
  std::map<string, unique_ptr<UserFunction>> funcs;
  // functions defined in .sy file
  std::map<string, unique_ptr<LibFunction>> lib_funcs;
  // functions defined in library
  CompileUnit();
  UserFunction *new_NormalFunc(string _name) {
    UserFunction *f = new UserFunction(_name);
    funcs[_name] = unique_ptr<UserFunction>(f);
    return f;
  }
  void accept(ir_visitor &visitor)  override;
  void map(function<void(CompileUnit &)> f) { f(*this); }
  void for_each(function<void(UserFunction *)> f) {
    for (auto &kv : funcs) f(kv.second.get());
  }
  void for_each(function<void(MediumScope &)> f) {
    f(scope);
    for (auto &kv : funcs) f(kv.second->scope);
  }

 private:
  LibFunction *new_LibFunc(string _name, bool ignore_return_value) {
    LibFunction *f = new LibFunction(_name);
    f->ignore_return_value = ignore_return_value;
    lib_funcs[_name] = unique_ptr<LibFunction>(f);
    return f;
  }
};

struct UnaryOp : Ir_Node {
  enum Type {
    LNOT = 0,
    NEG = 1,
    ID = 2,
  } type;
  int compute(int x);
  float compute(float x);//ccx adds it********************
  UnaryOp(Type x) : type(x) {}
  const char *get_name() const {
    static const char *names[] = {"!", "-", "+"};
    return names[(int)type];
  }
 void accept(ir_visitor &visitor) override;
};

struct BinaryOp : Ir_Node {
  enum Type {
     ADD,
    SUB ,
    MUL ,
    DIV ,
    ADDW,
    SUBW ,
    MULW ,
    DIVW ,
    LESS ,
    LEQ ,
    EQ ,
    NEQ ,
    MOD ,
    MODW ,
    fADD ,
    fSUB ,
    fMUL ,
    fDIV ,
    fLESS,
    fLEQ ,
    fEQ ,
    fNEQ ,
    SRL,
    SRA
    
  } type;
  BinaryOp(Type x) : type(x) {}
  int compute(int x, int y);
  float compute(float x,float y);//ccx adds it *********
  float compute(int x,float y);
  float compute(float x,int y);
  bool comm() {
    return type == ADD ||type == ADDW|| type == MUL ||type == MULW|| type == EQ || type == NEQ;
  }
  const char *get_name() const {
    static const char *names[] = {"+",  "-",  "*",  "/",
                                  "+",  "-",  "*",  "/", "<",
                                  "<=", "==", "!=", "%","%",
                                  "+",  "-",  "*",  "/",
                                  "<","<=","==","!=","<<","<<"};
    return names[(int)type];
  }
 void accept(ir_visitor &visitor) override;
};


struct RegWriteInstr : Instr {
  // any instr that write a reg
  // no instr write multiple regs
  Reg d1;
  RegWriteInstr(Reg d1) : d1(d1) {}
};

struct LoadConst : RegWriteInstr {
  // load value to d1
  // d1 = value
  int value;
  float value_f;
  bool is_float;
  LoadConst(Reg d1, int value) : RegWriteInstr(d1), value(value), value_f(0.0) {is_float=false;}
  LoadConst(Reg d1, float value) : RegWriteInstr(d1), value_f(value), value(0) {is_float=true;}
  void set_float(float value, bool _is_float){
    value_f = value;
    is_float = _is_float;
  }
  void accept(ir_visitor &visitor) override;
};
struct LoadAddr : RegWriteInstr {
  // load address of offset to d1
  // d1 = addr
  Variable *offset;
  LoadAddr(Reg d1, Variable *offset) : RegWriteInstr(d1), offset(offset) {
    assert(!offset->arg);
  }
  void accept(ir_visitor &visitor)override;
};



struct UnaryOpInstr : RegWriteInstr {
  // d1 = op(s1)
  UnaryOpInstr(Reg d1, Reg s1, UnaryOp op)
      : RegWriteInstr(d1), s1(s1), op(op) {}
  Reg s1;
  UnaryOp op;
  int compute(int x);
  float compute(float x);
  void accept(ir_visitor &visitor)  override;
};
struct LoadArg : RegWriteInstr {
  // load arg with arg_id=id to d1
  // d1 = arg
  int id;
  LoadArg(Reg d1, int id) : RegWriteInstr(d1), id(id) {}
  void accept(ir_visitor &visitor)  override;
};


struct BinaryOpInstr : RegWriteInstr {
  // d1 = op(s1,s2)
  BinaryOpInstr(Reg d1, Reg s1, Reg s2, BinaryOp op)
      : RegWriteInstr(d1), s1(s1), s2(s2), op(op) {}
  Reg s1, s2;
  BinaryOp op;
  int calc(int x, int y);
  float calc(float x, float y);
  void accept(ir_visitor &visitor)  override;
};

struct LoadInstr : RegWriteInstr {
  // memory read, used in ssa, but not in array-ssa
  // d1 = M[addr]
  LoadInstr(Reg d1, Reg addr) : RegWriteInstr(d1), addr(addr) {}
  Reg addr;
   void accept(ir_visitor &visitor)  override;
};



struct ControlInstr : Instr {
  // any instr except call that change PC
};
struct StoreInstr : Instr {
  // memory write, used in ssa, but not in array-ssa
  // M[addr] = s1
  StoreInstr(Reg addr, Reg s1) : addr(addr), s1(s1) {}
  Reg addr;
  Reg s1;
   void accept(ir_visitor &visitor)  override;
};
struct JumpInstr : ControlInstr {
  // PC = target
  IrBlock *target;
  JumpInstr(IrBlock *target) : target(target) {}
  void accept(ir_visitor &visitor) override;
};
struct ReturnInstr : ControlInstr {
  // return s1
  Reg s1;
  bool ignore_return_value;
  ReturnInstr(Reg s1, bool ignore_return_value)
      : s1(s1), ignore_return_value(ignore_return_value) {}
  void accept(ir_visitor &visitor)  override;
};
struct BranchInstr : ControlInstr {
  // if (cond) then PC = value
  Reg cond;
  IrBlock *target1, *target0;
  BranchInstr(Reg cond, IrBlock *target1, IrBlock *target0)
      : cond(cond), target1(target1), target0(target0) {}
   void accept(ir_visitor &visitor)  override;
};



struct MemUse;
struct CallInstr : RegWriteInstr {
  // d1 = f(args[0],args[1],...)
  vector<std::pair<Reg,bool>> args;
  IrFunction *f;
  bool ignore_return_value, pure = 0;
  list<MemUse *> in;  // record mem-use and mem-effect in array ssa
  CallInstr(Reg d1, IrFunction *f, vector<std::pair<Reg,bool>> args, bool ignore_return_value)
      : RegWriteInstr(d1),
        args(args),
        f(f),
        ignore_return_value(ignore_return_value) {}
  void accept(ir_visitor &visitor) override;
};
struct ArrayIndex : RegWriteInstr {
  // d1 = s1+s2*size, 0 <= s2 < limit
  Reg s1, s2;
  int size, limit;
  ArrayIndex(Reg d1, Reg s1, Reg s2, int size, int limit)
      : RegWriteInstr(d1), s1(s1), s2(s2), size(size), limit(limit) {}
   void accept(ir_visitor &visitor)  override;
};
struct LocalVarDef : Instr {
  // define variable as uninitialized array of bytes
  Variable *data;
  // array from arg: data->global=0 data->size=0 data->arg=1
  // local variable: data->global=0
  LocalVarDef(Variable *data) : data(data) { assert(!data->global); }
   void accept(ir_visitor &visitor) override;
};



// for ssa

struct PhiInstr : RegWriteInstr {
  // only for ssa and array-ssa
  vector<std::pair<Reg, IrBlock *>> uses;
  PhiInstr(Reg d1) : RegWriteInstr(d1) {}
  void add_use(Reg r, IrBlock *pos) { uses.emplace_back(r, pos); }
  void accept(ir_visitor &visitor) override;
};

// for array-ssa



struct MemUse : Instr {
  // only for array-ssa
  // s1 is used
  // the concrete use is unknown
  // usually inserted before call
  Reg s1;
  Variable *data;
  MemUse(Reg s1, Variable *data) : s1(s1), data(data) {}
  void accept(ir_visitor &visitor)  override;
};




struct MemDef : RegWriteInstr {
  // only for array-ssa
  // d1:int
  Variable *data;
  // array from arg: data->global=0 data->size=0 data->arg=1 //TODO
  // local array def: data->global=0
  // global array intro: data->global=1
  MemDef(Reg d1, Variable *data) : RegWriteInstr(d1), data(data) {
    assert(data);
  }
  void accept(ir_visitor &visitor)  override;
};

struct Convert : RegWriteInstr {
  Reg src;
  Convert(Reg to, Reg from) : RegWriteInstr(to), src(from) {}
   void accept(ir_visitor &visitor)  override;
};





struct MemEffect : RegWriteInstr {
  // only for array-ssa
  // d1 is updated from s1
  // the concrete update is unknown
  // usually inserted after call
  Reg s1;
  Variable *data;
  MemEffect(Reg d1, Reg s1, Variable *data)
      : RegWriteInstr(d1), s1(s1), data(data) {}
  void accept(ir_visitor &visitor)  override;
};


struct MemWrite : RegWriteInstr {
  // only for array-ssa
  // d1:mem = write mem at addr with value s1
  Reg mem, addr, s1;
  Variable *data;
  MemWrite(Reg d1, Reg mem, Reg addr, Reg s1, Variable *data)
      : RegWriteInstr(d1), mem(mem), addr(addr), s1(s1), data(data) {}
  void accept(ir_visitor &visitor)  override;
};
struct MemRead : RegWriteInstr {
  // only for array-ssa
  // d1:int = read mem at addr
  Reg mem, addr;
  Variable *data;
  MemRead(Reg d1, Reg mem, Reg addr, Variable *data)
      : RegWriteInstr(d1), mem(mem), addr(addr), data(data) {}
  void accept(ir_visitor &visitor)  override;
};

// for each (R1,R2):mp_reg, change the usage of R1 to R2, but defs are not
// changed
void map_use(UserFunction *f, const std::unordered_map<Reg, Reg> &mp_reg);

template <class T>
std::function<void(T)> repeat(bool (*f)(T), int max = 1000000) {
  return [=](T x) {
    for (int i = 0; i < max && f(x); ++i)
      ;
  };
}


template <class T>
std::function<void(T &)> partial_map(std::unordered_map<T, T> &mp) {
  auto *p_mp = &mp;
  return [p_mp](T &x) {
    auto it = p_mp->find(x);
    if (it != p_mp->end()) x = it->second;
  };
}
template <class T>
std::function<void(T &)> partial_map(T from, T to) {
  return [from, to](T &x) {
    if (x == from) x = to;
  };
}
class ir_visitor {
  public://需要打印的似乎就只有这么一些，以后还有其他用途的话，直接在这里添加就可以了
    ir_visitor() = default;
    ~ir_visitor() = default;
    virtual void visit(Reg &node) = 0;
    virtual void visit(Variable &node) = 0;
    virtual void visit(MediumScope &node) = 0;
    virtual void visit(IrFunction &node) = 0;
    virtual void visit(LibFunction &node) = 0;
    virtual void visit(UserFunction &node) = 0;
    virtual void visit(CompileUnit &node) = 0;
    virtual void visit(UnaryOp &node) = 0;
    virtual void visit(BinaryOp &node) = 0;
    virtual void visit(ArrayIndex &node) = 0;
    virtual void visit(LoadAddr &node) = 0;
    virtual void visit(LoadConst &node) = 0;
    virtual void visit(LoadArg &node) = 0;
    virtual void visit(UnaryOpInstr &node) = 0;
    virtual void visit(BinaryOpInstr &node) = 0;
    virtual void visit(LoadInstr &node) = 0;
    virtual void visit(StoreInstr &node) = 0;
    virtual void visit(JumpInstr &node) = 0;
    virtual void visit(BranchInstr &node) = 0;
    virtual void visit(ReturnInstr &node) = 0;
    virtual void visit(CallInstr &node) = 0;
    virtual void visit(LocalVarDef &node) = 0;
    virtual void visit(PhiInstr &node) = 0;
    virtual void visit(MemDef &node) = 0;
    virtual void visit(MemUse &node) = 0;
    virtual void visit(IrBlock &node) = 0;
    virtual void visit(Convert &node) = 0;
    virtual void visit(MemEffect &node) = 0;
    virtual void visit(MemRead &node) = 0;
    virtual void visit(MemWrite &node) = 0;
};
}  // namespace IR

namespace std {

template <>
struct hash<tuple<int, int, int>> {
  size_t operator()(const tuple<int, int, int> &r) const {
    return get<0>(r) * 293999 + get<1>(r) * 1234577 + get<2>(r) * 29;
  }
};
template <class T1, class T2>
struct hash<pair<T1, T2>> {
  size_t operator()(const pair<T1, T2> &r) const {
    return hash<T1>()(r.first) * 1844677 + hash<T2>()(r.second) * 41;
  }
};
}  // namespace std

