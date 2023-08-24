#pragma once

#include <cstdint>
#include <map>
#include <string>
#include <variant>
#include <vector>

#include "ir.h"

struct Type {
  std::vector<MemSize> array_dims;
  bool is_const, omit_first_dim, is_int = 0,is_float = 0;//ccx adds it(is_float)*************88

  Type();                      // int
  bool is_array() const;       // array_dims not empty || omit_first_dim
  Type deref_one_dim() const;  // throw when not array. return the type where
                               // the first dimension is removed
  size_t count_array_dims() const;
  MemSize count_elements() const;
  MemSize size() const;
  bool check_assign(const Type &rhs) const;  // don't check is_const
  bool check_index(const std::vector<MemSize> &index);
  MemSize get_index(
      const std::vector<MemSize> &index);  // call when check_index is true
  static const Type UnknownLengthArray;
};

struct StringType {};

struct FunctionInterface {
  bool return_value_non_void, variadic;
  bool is_int=0;//ccx adds it *********
  bool is_float=0;
  std::vector<std::variant<Type, StringType>> args_type;

  FunctionInterface();
};

struct FunctionTableEntry {
  Medium::IrFunction *ir_func;
  FunctionInterface interface;
};

struct FunctionTable {
  std::map<std::string, std::unique_ptr<FunctionTableEntry>> mapping;

  FunctionTableEntry *resolve(const std::string &name);
  void register_func(const std::string &name, Medium::IrFunction *ir_func,
                     const FunctionInterface &interface);
};

struct VariableTableEntry {
  Medium::Variable *ir_obj;
  Type type;
  int arg_id;                       // -1 if not array parameter
  std::vector<int32_t> const_init;  // empty when !type.is_const
};

struct VariableTable {
  std::map<std::string, std::unique_ptr<VariableTableEntry>> mapping;
  VariableTable *parent;

  VariableTable(VariableTable *_parent);
  VariableTable(){}
  VariableTableEntry *resolve(const std::string &name);
  VariableTableEntry *recursively_resolve(const std::string &name);
  void register_var(const std::string &name, Medium::Variable *ir_obj,
                    const Type &type);
  void register_const(const std::string &name, Medium::Variable *ir_obj,
                      const Type &type, std::vector<int32_t> init);
};

struct IRValue {
  Type type;
  bool is_left_value;
  Medium::Reg reg;  // if is_left_value, it's the address instead of the value

  bool assignable() const;  // left value, not array and not constant
};


struct CondJumpList {
  std::vector<Medium::IrBlock**> true_list, false_list;
};

