#include "ir.h"

namespace Medium {


CompileUnit::CompileUnit() : scope("global", 1) {
  LibFunction *f;
  f = new_LibFunc("__create_threads", 0);
  f->in = 1;
  f->out = 1;
  f = new_LibFunc("__join_threads", 1);
  f->in = 1;
  f->out = 1;
  
  f = new_LibFunc("getint", 0);
  f->in = 1;
  f = new_LibFunc("getch", 0);
  f->in = 1;
  f = new_LibFunc("getarray", 0);
  f->array_args[0] = 1;  // write arg0
  f->in = 1;

  f = new_LibFunc("putint", 1);
  f->out = 1;
  f = new_LibFunc("putch", 1);
  f->out = 1;
  f = new_LibFunc("putarray", 1);
  f->array_args[1] = 0;  // read arg1
  f->out = 1;

  f = new_LibFunc("putf", 1);
  f->array_args[0] = 0;  // read arg0
  f->out = 1;

  f = new_LibFunc("starttime", 1);
  f->in = 1;
  f->out = 1;

  f = new_LibFunc("stoptime", 1);
  f->in = 1;
  f->out = 1;

  f = new_LibFunc("getfarray", 1);
  f->array_args[0] = 1;
  f->out = 1;

  f = new_LibFunc("getfloat", 1);
  f->in = 1;
  f->out = 1;

  f = new_LibFunc("putfloat", 1);
  f->in = 1;
  f->out = 1;

  f = new_LibFunc("putfarray", 1);
  f->array_args[1] = 0;
  f->out = 1;

  f = new_LibFunc("memset",1);
  f->in = 1;
  f->out = 1;

  auto _in = scope.new_MemObject("stdin");    // input
  auto _out = scope.new_MemObject("stdout");  // output
  scope.set_arg(0, _in);
  scope.set_arg(1, _out);
}



void Instr::map_use(function<void(Reg &)> f1) {
  auto f2 = [](IrBlock *&x) {};
  auto f3 = [](Variable *&x) {};
  if(auto  w = dynamic_cast<RegWriteInstr*>( this)) {
    map(
        [&](Reg &x) {
          if (&x != &w->d1) f1(x);
        },
        f2, f3, 0);
  }
  else {
    map(f1, f2, f3, 0);
  }
}

Instr *Instr::map(function<void(Reg &)> f1, function<void(IrBlock *&)> f2,
                  function<void(Variable *&)> f3, bool copy) {
  if(auto  w = dynamic_cast<LoadAddr*>( this)) {
    auto u = w;
    if (copy) u = new LoadAddr(*w);
    f1(u->d1);
    f3(u->offset);
    return u;
  }
  if(auto  w = dynamic_cast<LoadConst*>( this)) {
    auto u = w;
    if (copy) u = new LoadConst(*w);
    f1(u->d1);
    return u;
  }
  if(auto  w = dynamic_cast<LoadArg*>( this)) {
    auto u = w;
    if (copy) u = new LoadArg(*w);
    f1(u->d1);
    return u;
  }
  if(auto  w = dynamic_cast<LocalVarDef*>( this)) {
    auto u = w;
    if (copy) u = new LocalVarDef(*w);
    f3(u->data);
    return u;
  }
  if(auto  w = dynamic_cast<UnaryOpInstr*>( this)) {
    auto u = w;
    if (copy) u = new UnaryOpInstr(*w);
    f1(u->d1);
    f1(u->s1);
    return u;
  }
  if(auto  w = dynamic_cast<BinaryOpInstr*>( this)) {
    auto u = w;
    if (copy) u = new BinaryOpInstr(*w);
    f1(u->d1);
    f1(u->s1);
    f1(u->s2);
    return u;
  }
  if(auto  w = dynamic_cast<ArrayIndex*>( this)) {
    auto u = w;
    if (copy) u = new ArrayIndex(*w);
    f1(u->d1);
    f1(u->s1);
    f1(u->s2);
    return u;
  }
  if(auto  w = dynamic_cast<LoadInstr*>( this)) {
    auto u = w;
    if (copy) u = new LoadInstr(*w);
    f1(u->d1);
    f1(u->addr);
    return u;
  }
  if(auto  w = dynamic_cast<StoreInstr*>( this)) {
    auto u = w;
    if (copy) u = new StoreInstr(*w);
    f1(u->addr);
    f1(u->s1);
    return u;
  }
  if(auto  w = dynamic_cast<JumpInstr*>( this)) {
    auto u = w;
    if (copy) u = new JumpInstr(*w);
    f2(u->target);
    return u;
  }
  if(auto  w = dynamic_cast<BranchInstr*>( this)) {
    auto u = w;
    if (copy) u = new BranchInstr(*w);
    f1(u->cond);
    f2(u->target1);
    f2(u->target0);
    return u;
  }
  if(auto  w = dynamic_cast<ReturnInstr*>( this)) {
    auto u = w;
    if (copy) u = new ReturnInstr(*w);
    f1(u->s1);
    return u;
  }
  if(auto  w = dynamic_cast<CallInstr*>( this)) {
    auto u = w;
    if (copy) u = new CallInstr(*w);
    f1(u->d1);
    for (auto &[x,flag] : u->args) f1(x);
    return u;
  }
  if(auto  w = dynamic_cast<PhiInstr*>( this)) {
    auto u = w;
    if (copy) u = new PhiInstr(*w);
    f1(u->d1);
    for (auto &x : u->uses) f1(x.first), f2(x.second);
    return u;
  }
  if(auto  w = dynamic_cast<MemDef*>( this)) {
    auto u = w;
    if (copy) u = new MemDef(*w);
    f1(u->d1);
    f3(u->data);
    return u;
  }
  if(auto  w = dynamic_cast<MemUse*>( this)) {
    auto u = w;
    if (copy) u = new MemUse(*w);
    f1(u->s1);
    f3(u->data);
    return u;
  }
  if(auto  w = dynamic_cast<MemEffect*>( this)) {
    auto u = w;
    if (copy) u = new MemEffect(*w);
    f1(u->d1);
    f1(u->s1);
    f3(u->data);
    return u;
  }
  if(auto  w = dynamic_cast<MemRead*>( this)) {
    auto u = w;
    if (copy) u = new MemRead(*w);
    f1(u->d1);
    f1(u->mem);
    f1(u->addr);
    f3(u->data);
    return u;
  }
  if(auto  w = dynamic_cast<MemWrite*>( this)) {
    auto u = w;
    if (copy) u = new MemWrite(*w);
    f1(u->d1);
    f1(u->mem);
    f1(u->addr);
    f1(u->s1);
    f3(u->data);
    return u;
  }
  if(auto w = dynamic_cast<Convert*>(this)){
    auto u = w;
    if (copy) u = new Convert(*w);
    f1(u->d1);
    f1(u->src);
    // f1(u->d1);
    return u;
  }
  assert(0);
  return NULL;
}



int UnaryOpInstr::compute(int s1) { return op.compute(s1); }
float UnaryOpInstr::compute(float s1) { return op.compute(s1); }


int UnaryOp::compute(int s1)
{
  switch (type) {
    case UnaryOp::LNOT:
      return !s1;
    case UnaryOp::NEG:
      return -s1;
    case UnaryOp::ID:
      return s1;
    default:
      assert(0);
      return 0;
  }
}
float UnaryOp::compute(float s1) {//ccx adds it
  switch (type) {
    case UnaryOp::LNOT:
      return !s1;
    case UnaryOp::NEG:
      return -s1;
    case UnaryOp::ID:
      return s1;
    default:
      assert(0);
      return 0;
  }
}



int BinaryOpInstr::calc(int s1, int s2) { return op.compute(s1, s2); }
float BinaryOpInstr::calc(float s1, float s2) { return op.compute(s1, s2); }
int BinaryOp::compute(int s1, int s2)
{
  switch (type) {
    case BinaryOp::ADD: case BinaryOp::ADDW:
      return s1 + s2;
    case BinaryOp::SUB: case BinaryOp::SUBW:
      return s1 - s2;
    case BinaryOp::MUL: case BinaryOp::MULW:
      return s1 * s2;
    case BinaryOp::DIV: case BinaryOp::DIVW:
      return (s2 && !(s1 == -2147483648 && s2 == -1) ? s1 / s2 : 0);
    case BinaryOp::LESS:
      return (s1 < s2);
    case BinaryOp::LEQ:
      return (s1 <= s2);
    case BinaryOp::EQ:
      return (s1 == s2);
    case BinaryOp::NEQ:
      return (s1 != s2);
    case BinaryOp::MOD: case BinaryOp::MODW:
      return (s2 ? s1 % s2 : 0);
    default:
      assert(0);
      return 0;
  }
}
//ccx adds them
float BinaryOp::compute(float s1, int s2) {
  switch (type) {
     case BinaryOp::ADD: case BinaryOp::ADDW:
      return s1 + s2;
    case BinaryOp::SUB: case BinaryOp::SUBW:
      return s1 - s2; 
    case BinaryOp::MUL: case BinaryOp::MULW:
      return s1 * s2;
    case BinaryOp::DIV: case BinaryOp::DIVW:
      return (s2 && !(s1 == -2147483648 && s2 == -1) ? s1 / s2 : 0);
    case BinaryOp::LESS:
      return (s1 < s2);
    case BinaryOp::LEQ:
      return (s1 <= s2);
    case BinaryOp::EQ:
      return (s1 == s2);
    case BinaryOp::NEQ:
      return (s1 != s2);
    case BinaryOp::MOD:
      exit(EXIT_FAILURE);//error
    default:
      assert(0);
      return 0;
  }
}

float BinaryOp::compute(int s1, float s2)
{
  switch (type) {
    case BinaryOp::ADD: case BinaryOp::ADDW:
      return s1 + s2;
    case BinaryOp::SUB: case BinaryOp::SUBW:
      return s1 - s2; 
    case BinaryOp::MUL: case BinaryOp::MULW:
      return s1 * s2;
    case BinaryOp::DIV: case BinaryOp::DIVW:
      return (s2 && !(s1 == -2147483648 && s2 == -1) ? s1 / s2 : 0);
    case BinaryOp::LESS:
      return (s1 < s2);
    case BinaryOp::LEQ:
      return (s1 <= s2);
    case BinaryOp::EQ:
      return (s1 == s2);
    case BinaryOp::NEQ:
      return (s1 != s2);
    case BinaryOp::MOD:
      exit(EXIT_FAILURE);//error
    default:
      assert(0);
      return 0;
  }
}
float BinaryOp::compute(float s1, float s2) {
  switch (type) {
    case BinaryOp::ADD: case BinaryOp::ADDW:
      return s1 + s2;
    case BinaryOp::SUB: case BinaryOp::SUBW:
      return s1 - s2; 
    case BinaryOp::MUL: case BinaryOp::MULW:
      return s1 * s2;
    case BinaryOp::DIV: case BinaryOp::DIVW:
      return (s2 && !(s1 == -2147483648 && s2 == -1) ? s1 / s2 : 0);
    case BinaryOp::LESS:
      return (s1 < s2);
    case BinaryOp::LEQ:
      return (s1 <= s2);
    case BinaryOp::EQ:
      return (s1 == s2);
    case BinaryOp::NEQ:
      return (s1 != s2);
    case BinaryOp::MOD:
      exit(EXIT_FAILURE);//error
    default:
      assert(0);
      return 0;
  }
}


void map_use(UserFunction *f, const std::unordered_map<Reg, Reg> &mp_reg) {
  for (auto& blocks : f->bbs) {
    for (auto& x : blocks->instrs) {
      x->map_use([&](Reg &r) {
      auto it = mp_reg.find(r);
      if (it != mp_reg.end()) r = it->second;
    });
    }
  }
}

void ArrayIndex::accept(ir_visitor &visitor) {visitor.visit(*this);}
void Reg::accept(ir_visitor &visitor){visitor.visit(*this);}
void Variable::accept(ir_visitor &visitor){visitor.visit(*this);}
void MediumScope::accept(ir_visitor &visitor){visitor.visit(*this);}
void LibFunction::accept(ir_visitor &visitor){visitor.visit(*this);}
void UserFunction::accept(ir_visitor &visitor){visitor.visit(*this);}
void CompileUnit::accept(ir_visitor &visitor){visitor.visit(*this);}
void UnaryOp::accept(ir_visitor &visitor){visitor.visit(*this);}
void BinaryOp::accept(ir_visitor &visitor){visitor.visit(*this);}
void LoadAddr::accept(ir_visitor &visitor){visitor.visit(*this);}
void LoadConst::accept(ir_visitor &visitor){visitor.visit(*this);}
void LoadArg::accept(ir_visitor &visitor){visitor.visit(*this);}
void UnaryOpInstr::accept(ir_visitor &visitor){visitor.visit(*this);}
void BinaryOpInstr::accept(ir_visitor &visitor){visitor.visit(*this);}
void LoadInstr::accept(ir_visitor &visitor){visitor.visit(*this);}
void StoreInstr::accept(ir_visitor &visitor){visitor.visit(*this);}
void JumpInstr::accept(ir_visitor &visitor){visitor.visit(*this);}
void BranchInstr::accept(ir_visitor &visitor){visitor.visit(*this);}
void ReturnInstr::accept(ir_visitor &visitor){visitor.visit(*this);}
void CallInstr::accept(ir_visitor &visitor){visitor.visit(*this);}
void LocalVarDef::accept(ir_visitor &visitor){visitor.visit(*this);}
void PhiInstr::accept(ir_visitor &visitor){visitor.visit(*this);}
void MemDef::accept(ir_visitor &visitor){visitor.visit(*this);}
void MemUse::accept(ir_visitor &visitor){visitor.visit(*this);}
void IrBlock::accept(ir_visitor &visitor){visitor.visit(*this);}
void Convert::accept(ir_visitor &visitor){visitor.visit(*this);}
void MemEffect::accept(ir_visitor &visitor){visitor.visit(*this);}
void MemRead::accept(ir_visitor &visitor){visitor.visit(*this);}
void MemWrite::accept(ir_visitor &visitor){visitor.visit(*this);}
//草（一种植物），这么多，终于搞完了，麻了
} // namespace IR
