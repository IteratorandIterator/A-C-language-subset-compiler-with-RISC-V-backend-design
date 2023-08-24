#pragma once

#include <array>
#include <cstdint>
#include <ostream>

namespace Archriscv {

constexpr int RegCount = 32; //记录寄存器的数量
constexpr int zero = 0, ra = 1, sp = 2, gp = 3, tp = 4, fp = 8;  //记录不同寄存器的编号

enum RegisterUsage { caller_save, callee_save, special };

constexpr RegisterUsage REGISTER_USAGE[RegCount] = {//预设的寄存器使用方式
    special,     callee_save, special,     special,
    special,                                // zero, ra, sp, gp, tp

    caller_save, caller_save, caller_save,  // t0-t2
    callee_save, callee_save,               // s0-s1

    caller_save, caller_save, caller_save, caller_save,
    caller_save, caller_save, caller_save, caller_save,  // a0-a7

    callee_save, callee_save, callee_save, callee_save,
    callee_save, callee_save, callee_save, callee_save,
    callee_save, callee_save,                           // s2-s11
    caller_save, caller_save, caller_save, caller_save  // t3-t6
};


constexpr RegisterUsage REGISTER_USAGE_FLOAT[RegCount] = {//预设的寄存器使用方式

    caller_save, caller_save, caller_save, caller_save,
    caller_save, caller_save, caller_save, caller_save,  // ft0-ft7
    callee_save, callee_save,               // fs0-fs1

    caller_save, caller_save,               //fa0-fa1

    caller_save, caller_save,
    caller_save, caller_save, caller_save, caller_save,  // fa2-a7

    callee_save, callee_save, callee_save, callee_save,
    callee_save, callee_save, callee_save, callee_save,
    callee_save, callee_save,                           // fs2-fs11

    caller_save, caller_save, caller_save, caller_save  // t3-t6
};



constexpr bool integer_allocable(int reg_id) { //可以分配的寄存器是caller_save和callee_save类型的寄存器
  return REGISTER_USAGE[reg_id] == caller_save ||
         REGISTER_USAGE[reg_id] == callee_save;
}

constexpr bool float_allocable(int reg_id) { //可以分配的寄存器是caller_save和callee_save类型的寄存器
  return REGISTER_USAGE_FLOAT[reg_id] == caller_save ||
         REGISTER_USAGE_FLOAT[reg_id] == callee_save;
}

constexpr int ALLOCABLE_REGISTER_COUNT = []() constexpr { //计算可以分配的寄存器的个数
  int cnt = 0;
  for (int i = 0; i < RegCount; ++i)
    if (integer_allocable(i)) ++cnt;
  return cnt;
}
();

constexpr int ALLOCABLE_REGISTER_COUNT_FLOAT = []() constexpr { //计算可以分配的寄存器的个数
  int cnt = 0;
  for (int i = 0; i < RegCount; ++i)
    if (float_allocable(i)) ++cnt;
  return cnt;
}
();

constexpr std::array<int, ALLOCABLE_REGISTER_COUNT> ALLOCABLE_REGISTERS =  //将能分配的寄存器保存在一个数组里面
    []() constexpr {
  std::array<int, ALLOCABLE_REGISTER_COUNT> ret{};
  int cnt = 0;
  for (int i = 0; i < RegCount; ++i)
    if (integer_allocable(i)) ret[cnt++] = i;
  return ret;
}
();

constexpr std::array<int, ALLOCABLE_REGISTER_COUNT_FLOAT> ALLOCABLE_REGISTERS_FLOAT =  //将能分配的寄存器保存在一个数组里面
    []() constexpr {
  std::array<int, ALLOCABLE_REGISTER_COUNT_FLOAT> ret{};
  int cnt = 0;
  for (int i = 0; i < RegCount; ++i)
    if (float_allocable(i)) ret[cnt++] = i;
  return ret;
}
();

constexpr int ARGUMENT_REGISTER_COUNT = 8;  //用于参数传递寄存器的数量

constexpr int ARGUMENT_REGISTERS[ARGUMENT_REGISTER_COUNT] = {10, 11, 12, 13,
                                                             14, 15, 16, 17};  //保存寄存器编号


constexpr int32_t IMM12_L = -2048, IMM12_R = 2047;  //12位立即数的取值范围

constexpr bool is_imm12(int32_t value) {  //判断是否在范围之内
  return value >= IMM12_L && value <= IMM12_R;
}

enum Rtype {INT,FP};

struct Reg {
  int id;
  Rtype type;

  Reg(int _id = -1,Rtype _type = INT) : id(_id),type(_type) {}
  bool is_machine() const { return id < RegCount; }
  bool is_virtual() const { return id >= RegCount; }//这个就是编号大于可用的物理寄存器的东西
  bool is_float() const {return this->type == FP;}
  bool operator<(const Reg &rhs) const { return id < rhs.id||(id == rhs.id&&type<rhs.type); }
  bool operator==(const Reg &rhs) const { return (id == rhs.id)&&(is_float()==rhs.is_float()); }
  bool operator>(const Reg &rhs) const { return id > rhs.id||(id == rhs.id&&type>rhs.type); }
  bool operator!=(const Reg &rhs) const { return (id != rhs.id)||(is_float()==rhs.is_float()); }
  //判断寄存器之间的编号关系
  // bool operator<(const Reg &rhs) const { return id < rhs.id; }
  // bool operator==(const Reg &rhs) const { return id == rhs.id; }
  // bool operator>(const Reg &rhs) const { return id > rhs.id; }
  // bool operator!=(const Reg &rhs) const { return id != rhs.id; }
};

enum Compare { Eq, Ne, Lt, Le, Gt, Ge };

constexpr Compare logical_not(Compare c) {  //逻辑运算
  switch (c) {
    case Eq:
      return Ne;
    case Ne:
      return Eq;
    case Lt:
      return Ge;
    case Le:
      return Gt;
    case Gt:
      return Le;
    case Ge:
      return Lt;
  }
}

inline std::ostream &operator<<(std::ostream &os, Compare c) {  //输出运算符
  switch (c) {
    case Eq:
      os << "eq";
      break;
    case Ne:
      os << "ne";
      break;
    case Lt:
      os << "lt";
      break;
    case Le:
      os << "le";
      break;
    case Gt:
      os << "gt";
      break;
    case Ge:
      os << "ge";
      break;
  }
  return os;
}

}  // namespace Archriscv