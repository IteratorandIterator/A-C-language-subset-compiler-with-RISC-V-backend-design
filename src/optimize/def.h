#pragma once

#include <cstdint>
#include <map>
#include <utility>

#include "arch.h"
#include "ir.h"

namespace Archriscv {

struct StackObject { //栈对象
  int32_t size, position;   //大小和位置
  StackObject(int32_t _size, int32_t _pos = -1) : size(_size), position(_pos) {}
};

struct StackObject;//栈对象
struct Block;//基本块
struct Function;//函数

struct Medium2Backend {
  std::map<Medium::Variable *, StackObject *> obj_mapping;  //将一个memobj映射成一个stackobj
  std::map<int, Reg> reg_mapping;  //将一个int映射成一个reg
  std::map<Medium::IrBlock *, Block *> block_mapping; //将一个BB，映射成一个block
  std::map<Block *, Medium::IrBlock *> rev_block_mapping; //将一个block映射成一个BB
  std::map<int, int32_t> constant_value; //将一个int映射成一个int32
  std::map<int, std::pair<StackObject *, int32_t>> constant_addr; //将一个int映射成一个常量地址的映射表
  int reg_n;  //寄存器数量

  Medium2Backend(); //初始化

  Reg new_backend_reg(bool is_float=false);
  Reg reg_medium2backend(Medium::Reg ir_reg);
};

}  // namespace Archriscv