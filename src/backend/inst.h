#pragma once

#include <cassert>
#include <iostream>
#include <list>
#include <map>
#include <memory>
#include <set>
#include <sstream>
#include <vector>

#include "arch.h"
#include "def.h"
#include "code_gen.h"

namespace Archriscv {

inline std::ostream &operator<<(std::ostream &os, const Reg &reg) {
   if (reg.type == FP)
      os << 'f' << reg.id;
  else {
     if (reg.id == zero) os << "zero";
     else if (reg.id == ra) os << "ra";
     else if (reg.id == sp) os << "sp";
     else if (reg.id == gp) os << "gp";
     else if (reg.id == tp) os << "tp";
     else os << 'x' << reg.id;
  }
  return os;
}

struct Inst {
  virtual ~Inst() = default;

  virtual std::vector<Reg> def_reg() { return {}; }
  virtual std::vector<Reg> use_reg() { return {}; }
  virtual std::vector<Reg *> regs() { return {}; }
  virtual bool side_effect() {
    return false;
  }  // side effect apart from assigning value to def_reg() registers
  virtual void gen_asm(std::ostream &out, AsmContext *ctx) = 0;  //生成汇编
  virtual void print(std::ostream &out) { gen_asm(out, nullptr); } //打印
  virtual void maintain_sp(int32_t &sp_offset) {} 

  std::string to_string() {
    std::ostringstream buf;
    print(buf);
    return buf.str();
  }
  template <class T>
  T *as() {
    return dynamic_cast<T *>(this);
  }
  void update_live(std::set<Reg> &live) {
    for (Reg i : def_reg())
      if (i.is_virtual() || integer_allocable(i.id)) live.erase(i);
    for (Reg i : use_reg())
      if (i.is_virtual() || integer_allocable(i.id)) live.insert(i);
  }
  bool def(Reg reg) {
    for (Reg r : def_reg())
      if (r.id == reg.id) return true;
    return false;
  }
  bool use(Reg reg) {
    for (Reg r : use_reg())
      if (r.id == reg.id) return true;
    return false;
  }
  bool relate(Reg reg) { return def(reg) || use(reg); }
  void replace_reg(Reg before, Reg after) {
    for (Reg *i : regs())
      if ((*i).id == before.id) (*i) = after;
  }
};

//modified
struct RegRegInst : Inst {      //R-Type Instruction like add r1, r2 ——> r3
  enum Type {
    Add,  //addition
    Sub,  //substraction
    Mul,  //multiplication
    Div,  //division
    Rem,  //remain

     //支持溢出
    Addw,  //addition
    Subw,  //substraction
    Mulw,  //multiplication
    Divw,  //division
    Remw,  //remain

    Sll,  //left shift logic
    Srl,  //right shift logi
    Sra,  //right shift arithmatic
    And,  //and
    Or,   //or
    Xor,  //xor
    Slt,  //less than
    Sltu,  //less than unsinged
    Seqz,   //equal 0
    Seq,
    Sle,//two inst ,less or equal
    Sgt,
    Sne,
    Snez,

    Fadd,   //floating addition
    Fsub,   //floating substraction
    Fmul,   //floating multiplication
    Fdiv,   //floating division

    //****
    Fmin,   //less one of two floating number
    Fmax,   //larger one of two floating number
    Feq,    //floating equal ==
    Flt,    //floating less than <
    Fle,    //floating less than equal <=
    Fgt
  } op;
  Reg dst, lhs, rhs;
  RegRegInst(Type _op, Reg _dst, Reg _lhs, Reg _rhs)
      :  dst(_dst), lhs(_lhs), rhs(_rhs) {
        if(_op==Add)
          op=_dst.type==FP?Fadd:Add;
        else if(_op==Sub)
          op=_dst.type==FP?Fsub:Sub;
        else if(_op==Mul)
          op=_dst.type==FP?Fmul:Mul;
        else if(_op==Div)
          op=_dst.type==FP?Fdiv:Div;
        else
          op=_op;
      }

  virtual std::vector<Reg> def_reg() override { return {dst}; }
  virtual std::vector<Reg> use_reg() override { return {lhs, rhs}; }
  virtual std::vector<Reg *> regs() override { return {&dst, &lhs, &rhs}; }
  virtual void gen_asm(std::ostream &out, AsmContext *ctx) override {
    static const std::map<Type, std::string> asm_name{
        //Integer
        {Add, "add"}, {Sub, "sub"}, {Mul, "mul"},  {Div, "div"}, {Rem, "rem"},
        {Addw, "addw"}, {Subw, "subw"}, {Mulw, "mulw"},  {Divw, "divw"}, {Remw, "remw"},
        {Sll, "sll"}, {Srl, "srl"}, {Sra, "sra"},  {And, "and"}, {Or, "or"},
        {Xor, "xor"}, {Slt, "slt"}, {Sltu, "sltu"},{Seqz, "seqz"},{Sgt, "sgt"},{Snez, "snez"},
        //Floating
        {Fadd, "fadd.s"}, {Fsub, "fsub.s"}, {Fmul, "fmul.s"}, {Fdiv, "fdiv.s"}, {Fmin, "fmin.s"}, 
        {Fmax, "fmax.s"}, {Feq, "feq.s"}, {Flt, "flt.s"}, {Fle, "fle.s"},{Fgt,"fgt.s"}};
         out << asm_name.find(op)->second << ' ' << dst << ", " << lhs << ", " << rhs<< '\n';
  }

  static Type from_ir_binary_op(Medium::BinaryOp::Type t) {
    switch (t) {
      case Medium::BinaryOp::ADD:
        return Add;
      case Medium::BinaryOp::SUB:
        return Sub;
      case Medium::BinaryOp::MUL:
        return Mul;
      case Medium::BinaryOp::DIV:
        return Div;
      case Medium::BinaryOp::MOD:
        return Rem;

        //为了溢出
      case Medium::BinaryOp::ADDW:
        return Addw;
      case Medium::BinaryOp::SUBW:
        return Subw;
      case Medium::BinaryOp::MULW:
        return Mulw;
      case Medium::BinaryOp::DIVW:
        return Divw;
      case Medium::BinaryOp::MODW:
        return Remw;
      //位移运算
       case Medium::BinaryOp::SRA:
        return Sra; 
         case Medium::BinaryOp::SRL:
        return Srl; 
      //compare
      case Medium::BinaryOp::LEQ:
        return Sle;
       case Medium::BinaryOp::EQ:
        return Seq;
      case Medium::BinaryOp::LESS:
        return Slt;
      case Medium::BinaryOp::NEQ:
        return Sne; 
      default:
        assert(0);
    }
  }
};

struct RegImmInst : Inst {
  enum Type { Addi,Addiw, Slli, Srli, Srai, Andi, Ori, Xori, Slti, Sltiu ,Snez,Seqz} op;
  Reg dst, lhs;
  int32_t rhs;
  RegImmInst(Type _op, Reg _dst, Reg _lhs, int32_t _rhs)
      : op(_op), dst(_dst), lhs(_lhs), rhs(_rhs) {
    if (op == Slli || op == Srli || op == Srai) {
      assert(rhs >= 0 && rhs < 32);
    } else {
      assert(is_imm12(rhs));
    }
  }

  virtual std::vector<Reg> def_reg() override { return {dst}; }
  virtual std::vector<Reg> use_reg() override { return {lhs}; }
  virtual std::vector<Reg *> regs() override { return {&dst, &lhs}; }
  virtual void gen_asm(std::ostream &out, AsmContext *ctx) override {
    static const std::map<Type, std::string> asm_name{
        {Addi, "addi"},{Addiw, "addiw"}, {Slli, "slli"}, {Srli, "srli"},
        {Srai, "srai"}, {Andi, "andi"}, {Ori, "ori"},
        {Xori, "xori"}, {Slti, "slti"}, {Sltiu, "sltiu"},{Seqz, "seqz"},{Snez, "snez"}};
    if(op==Seqz||op==Seqz||op==Snez){
      out << asm_name.find(op)->second << ' ' << dst << ", " << lhs << '\n';
    } else {
      out << asm_name.find(op)->second << ' ' << dst << ", " << lhs << ", " << rhs<< '\n';
    }
      
  }
};

struct LoadImm : Inst { //用于将立即数加载到寄存器的汇编代码。
  Reg dst;
  int32_t value;
  LoadImm(Reg _dst, int32_t _value) : dst(_dst), value(_value) {}

  virtual std::vector<Reg> def_reg() override { return {dst}; }
  virtual std::vector<Reg *> regs() override { return {&dst}; }
  virtual void gen_asm(std::ostream &out, AsmContext *ctx) override {
    if (dst.type == INT)
      out << "li " << dst << ", " << value << '\n';
  }
};

struct Jump : Inst {  //用于生成无条件跳转指令的汇编代码。
  Block *target;
  Jump(Block *_target) : target(_target) { target->label_used = true; }

  virtual bool side_effect() override { return true; }
  virtual void gen_asm(std::ostream &out, AsmContext *ctx) override {
    out << "j " << target->name << '\n';
  }
};

struct Branch : Inst {  //用于生成条件跳转指令的汇编代码。
  Block *target;
  Reg lhs, rhs;
  Compare op;
  Branch(Block *_target, Reg _lhs, Reg _rhs, Compare _op)
      : target(_target), lhs(_lhs), rhs(_rhs), op(_op) {
    target->label_used = true;
  }

  virtual std::vector<Reg> use_reg() override { return {lhs, rhs}; }
  virtual std::vector<Reg *> regs() override { return {&lhs, &rhs}; }
  virtual bool side_effect() override { return true; }
  virtual void gen_asm(std::ostream &out, AsmContext *ctx) override {
      out << 'b' << op << ' ' << lhs << ", " << rhs << ", " << target->name
        << '\n';
  }
};

struct FBranch : Inst {
  Block *target;
  Reg res,lhs, rhs;
  Compare op;
  FBranch(Block *_target, Reg _res,Reg _lhs, Reg _rhs, Compare _op)
      : target(_target), res(_res),lhs(_lhs), rhs(_rhs), op(_op) {
    target->label_used = true;
  }

  virtual std::vector<Reg> use_reg() override { return {res,lhs, rhs}; }//TODO
  virtual std::vector<Reg> def_reg() override { return {res}; }
  virtual std::vector<Reg *> regs() override { return {&res,&lhs, &rhs}; }
  virtual bool side_effect() override { return true; }
  virtual void gen_asm(std::ostream &out, AsmContext *ctx) override {
    if(op==Ne)
    {
      out << 'f' << Eq <<".s"<< ' ' << res << ", " << lhs << ", "<< rhs 
        << '\n';
      out << "beq" <<  ' ' << res << ", " << "zero" << ", " << target->name
        << '\n';
    }
    else
    {
      out << 'f' << op <<".s"<< ' ' << res << ", " << lhs << ", "<< rhs 
        << '\n';
      out << "bne" <<  ' ' << res << ", " << "zero" << ", " << target->name
        << '\n';
    }
      
  }
};

struct FuncCall : Inst {  //用于生成函数调用指令的汇编代码。
  std::string name;
  int arg_cnt;
  FuncCall(std::string _name, int _arg_cnt)
      : name(std::move(_name)), arg_cnt(_arg_cnt) {}

  virtual std::vector<Reg> def_reg() override {
    std::vector<Reg> ret;
    for (int i = 0; i < RegCount; ++i)
    {
       if (REGISTER_USAGE[i] == caller_save) ret.emplace_back(i);
       if (REGISTER_USAGE[i] == caller_save) ret.emplace_back(Reg{i,Archriscv::FP});
    }
     
    ret.emplace_back(ra);
    return ret;
  }
  virtual std::vector<Reg> use_reg() override {
    std::vector<Reg> ret;
    for (int i = 0; i < std::min(arg_cnt, ARGUMENT_REGISTER_COUNT); ++i){
      ret.emplace_back(ARGUMENT_REGISTERS[i]);
      ret.emplace_back(Reg{ARGUMENT_REGISTERS[i],Archriscv::FP});
    }
    return ret;
  }
  virtual bool side_effect() override { return true; }
  virtual void gen_asm(std::ostream &out, AsmContext *ctx) override {
    out << "call ";
    // call is a pseudo instruction, let the linker decide to use jal or auipc +
    // jalr
    if (name == "putf")
      out << "printf";
    else if (name == "starttime")
      out << "_sysy_starttime";
    else if (name == "stoptime")
      out << "_sysy_stoptime";
    else
      out << name;
    out << '\n';
  }
};
struct Return : Inst { //用于生成函数返回指令的汇编代码。
  bool has_return_value;
  Return(bool _has_return_value) : has_return_value(_has_return_value) {}

  virtual std::vector<Reg> use_reg() override {
    if (has_return_value)
      return {Reg{ARGUMENT_REGISTERS[0],INT},Reg{ARGUMENT_REGISTERS[0],FP}};
    else
      return {};
  }
  virtual bool side_effect() override { return true; }
  virtual void gen_asm(std::ostream &out, AsmContext *ctx) override {
    ctx->epilogue(out);
    out << "ret\n";
  }
  virtual void print(std::ostream &out) override { out << "ret\n"; }
};


struct LuiInst : Inst {
  Reg base;
  std::string float_imm_name;
  LuiInst(Reg _base,std::string _float_imm_name)
      : base(_base) , float_imm_name(_float_imm_name) {}

  virtual std::vector<Reg> def_reg() override { return {base}; }
  virtual std::vector<Reg *> regs() override { return {&base}; }
  virtual void gen_asm(std::ostream &out, AsmContext *ctx) override {
      out << "lui " << base << "," << "%hi" << "(" << float_imm_name << ")" << "\n";
  }
};


struct FlwInst : Inst {
  Reg dst, base;
  std::string float_imm_name;
  FlwInst( Reg _dst, Reg _base , std::string _float_imm_name)
      : dst(_dst), base(_base), float_imm_name(_float_imm_name) {}

  virtual std::vector<Reg> def_reg() override { return {dst}; }
  virtual std::vector<Reg> use_reg() override { return {base}; }
  virtual std::vector<Reg *> regs() override { return {&dst, &base}; }
  virtual void gen_asm(std::ostream &out, AsmContext *ctx) override {
    out << "flw "<< dst << "," << "%lo" << "(" <<
        float_imm_name << ")" << "(" << base << ")" << "\n";
  }
};
struct Load : Inst {
  enum Type { 
   //integer
   Lw,  //load integer
   //floating
   Flw  //load floating
  } op;
  Reg dst, base;
  bool is_addr;
  int32_t offset;
  Load(Reg _dst, Reg _base, int32_t _offset,Type _op=Lw,bool _is_addr=false)
      : op(_op),dst(_dst), base(_base), offset(_offset),is_addr(_is_addr) {
    assert(is_imm12(offset));
  }

  virtual std::vector<Reg> def_reg() override { return {dst}; }
  virtual std::vector<Reg> use_reg() override { return {base}; }
  virtual std::vector<Reg *> regs() override { return {&dst, &base}; }
  virtual void gen_asm(std::ostream &out, AsmContext *ctx) override {
    static const std::map<Type, std::string> asm_name{
        {Lw, "lw"}, {Flw, "flw"}};
      if(is_addr&&!dst.is_float())//补丁中地补丁，因为我们在溢出地时候将变量类型的大小都搞成了64位，这样普通的float变量会被认为是addr，
        out << "ld " << dst << ", " << offset << "(" << base << ")\n";  
      else 
        out << asm_name.find(op)->second << ' ' << dst << ", "  << offset << '(' << base << ")\n";
  }
};

struct Store : Inst {
  enum Type { 
   //integer
   Sw,  //store integer
   //floating
   Fsw  //store floating
  } op;
  Reg src, base;
  int32_t offset;
  bool is_addr;
  Store(Reg _src, Reg _base, int32_t _offset,Type _op=Sw,bool _is_addr=false)
      : op(_op), src(_src), base(_base), offset(_offset),is_addr(_is_addr) {
    // assert(is_imm12(offset));
  }

  virtual std::vector<Reg> use_reg() override { return {src, base}; }
  virtual std::vector<Reg *> regs() override { return {&src, &base}; }
  virtual bool side_effect() override { return true; }
  virtual void gen_asm(std::ostream &out, AsmContext *ctx) override {
    static const std::map<Type, std::string> asm_name{
        {Sw, "sw"}, {Fsw, "fsw"}};
    if(is_addr&&!src.is_float())
        out << "sd " << src << ", " << offset << "(" << base << ")\n"; 
    else
      out << asm_name.find(op)->second << ' ' << src << ", "  << offset << '(' << base << ")\n";
  }
};



struct LoadStack : Inst {
  StackObject *base;
  Reg dst;
  int32_t offset;
  LoadStack(StackObject *_base, Reg _dst, int32_t _offset)
      : base(_base), dst(_dst), offset(_offset) {}

  virtual std::vector<Reg> def_reg() override { return {dst}; }
  virtual std::vector<Reg *> regs() override { return {&dst}; }
  virtual void gen_asm(std::ostream &out, AsmContext *ctx) override {}
  virtual void print(std::ostream &out) override {
    out << dst << " = LoadStack " << offset << '(' << base << ")\n";
  }
};

struct StoreStack : Inst {  //用于生成存储堆栈指令的汇编代码。
  StackObject *base;
  Reg src;
  int32_t offset;
  StoreStack(StackObject *_base, Reg _src, int32_t _offset)
      : base(_base), src(_src), offset(_offset) {}

  virtual std::vector<Reg> use_reg() override { return {src}; }
  virtual std::vector<Reg *> regs() override { return {&src}; }
  virtual bool side_effect() override { return true; }
  virtual void gen_asm(std::ostream &out, AsmContext *ctx) override {}
  virtual void print(std::ostream &out) override {
    out << "StoreStack " << src << ", " << offset << '(' << base << ")\n";
  }
};

struct LoadStackAddr : Inst {  //用于生成加载堆栈地址指令的汇编代码。
  StackObject *base;
  Reg dst;
  int32_t offset;
  LoadStackAddr(StackObject *_base, Reg _dst, int32_t _offset)
      : base(_base), dst(_dst), offset(_offset) {}

  virtual std::vector<Reg> def_reg() override { return {dst}; }
  virtual std::vector<Reg *> regs() override { return {&dst}; }
  virtual void gen_asm(std::ostream &out, AsmContext *ctx) override {}
  virtual void print(std::ostream &out) override {
    out << dst << " = LoadStackAddr " << offset << '(' << base << ")\n";
  }
};

struct Move : Inst {
  enum Type{INTMOV,FLOATMOV,FLOATMOVX}op;
  Reg dst, src;
  Move(Reg _dst, Reg _src,Type _op=INTMOV) : dst(_dst), src(_src),op(_op) {}

  virtual std::vector<Reg> def_reg() override { return {dst}; }
  virtual std::vector<Reg> use_reg() override { return {src}; }
  virtual std::vector<Reg *> regs() override { return {&dst, &src}; }
  virtual void gen_asm(std::ostream &out, AsmContext *ctx) override {
    if(op==INTMOV)
       out << "mv " << dst << ", " << src << '\n';
    else if(op==FLOATMOV)
      out << "fmv.s " << dst << ", " << src << '\n';
    else
      out << "fmv.s.x " << dst << ", " << src << '\n';
   
  }
};

struct MoveSP : Inst {  // for passing arguments 用于传递参数的堆栈指针移动指令的汇编代码。
  int32_t offset;       // imm12
  MoveSP(int32_t _offset) : offset(_offset) {
     assert(is_imm12(_offset)); 
     }

  virtual bool side_effect() override { return true; }
  virtual void gen_asm(std::ostream &out, AsmContext *ctx) override {
    out << "addi sp, sp, " << offset << '\n';
    ctx->temp_sp_offset += offset;
  }
  virtual void print(std::ostream &out) override {
    out << "MoveSP(" << offset << ")\n";
  }
  virtual void maintain_sp(int32_t &sp_offset) override { sp_offset += offset; }
};

struct LoadLabelAddr : Inst {  //将标签地址加载到寄存器中
  Reg dst;
  std::string label;
  LoadLabelAddr(Reg _dst, std::string _label)
      : dst(_dst), label(std::move(_label)) {}

  virtual std::vector<Reg> def_reg() override { return {dst}; }
  virtual std::vector<Reg *> regs() override { return {&dst}; }
  virtual void gen_asm(std::ostream &out, AsmContext *ctx) override {
    out << "la " << dst << ", " << label << '\n';
  }
};


struct Convert :Inst{
                               // for comparing operation
  enum Type {
    Fsqrt,  //square root
    Fcvtws, //convert to integer***
    Fcvtwus, //convert to unsigned integer
    Fcvtsw, //convert int to float***
    Fcvtswu, //convert unsigned to float
    Fmvxw,  //move to integer register
    Fmvwx   //move to f.p
  } op;
  Reg dst, lhs;
  Convert(Type _op, Reg _dst, Reg _lhs): 
    op(_op), dst(_dst), lhs(_lhs) {}

  virtual std::vector<Reg> def_reg() override { return {dst}; }
  virtual std::vector<Reg> use_reg() override { return {lhs}; }
  virtual std::vector<Reg *> regs() override { return {&dst,&lhs}; }
  virtual void gen_asm(std::ostream &out, AsmContext *ctx) override {
    static const std::map<Type, std::string> asm_name{
        {Fsqrt, "fsqrt"}, {Fcvtws, "fcvt.w.s"}, {Fcvtwus, "fcvt.wu.s"}, {Fcvtsw, "fcvt.s.w"}, 
        {Fcvtswu, "fcvt.s.wu"}, {Fmvxw, "fmv.x.w"}, {Fmvwx, "fmv.w.x"}};
    out << asm_name.find(op)->second << ' ' << dst << ", " << lhs ;
    if (op == Fcvtws)
      out << ", rtz";
    out << "\n";
    
  }
};

}  // namespace Archriscv
