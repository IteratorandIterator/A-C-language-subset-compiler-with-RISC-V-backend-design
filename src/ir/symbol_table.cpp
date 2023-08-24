#include "symbol_table.h"


using std::string;
using std::unique_ptr;
using std::vector;


void VariableTable::register_const(const string &varnm, Medium::Variable *ojir,
                                   const Type &t,
                                   std::vector<int32_t> init) {
  VariableTableEntry *vartmp = new VariableTableEntry();
  vartmp->ir_obj = ojir;
  vartmp->type = t;
  vartmp->arg_id = -1;
  vartmp->const_init = std::move(init);
  mapping[varnm] = unique_ptr<VariableTableEntry>{vartmp};
}


VariableTableEntry *VariableTable::resolve(const string &varnm) {
  auto it = mapping.find(varnm);
  if (it != mapping.end())
    return it->second.get();
  else
    return nullptr;
}

VariableTable::VariableTable(VariableTable *_parent) {
  this->parent = _parent;
}

void VariableTable::register_var(const string &varnm, Medium::Variable *ojir,
                                 const Type &t) {
  VariableTableEntry *vartmp = new VariableTableEntry();
  vartmp->ir_obj = ojir;
  vartmp->type = t;
  vartmp->arg_id = -1;
  mapping[varnm] = unique_ptr<VariableTableEntry>{vartmp};
}


void FunctionTable::register_func(const string &FNAME, Medium::IrFunction *ir_func,
                                  const FunctionInterface &interface) {
  FunctionTableEntry *FTemp = new FunctionTableEntry();
  FTemp->ir_func = ir_func;
  FTemp->interface = interface;
  mapping[FNAME] = unique_ptr<FunctionTableEntry>{FTemp};
}

Type::Type() {
  this->is_const = 0;
  this->omit_first_dim = 0;
}

bool Type::is_array() const { 
  return array_dims.size() > 0 || omit_first_dim; 
}

Type Type::deref_one_dim() const {
  Type ret = *this;
  if (ret.omit_first_dim) {
    ret.omit_first_dim = false;
  } else {
    ret.array_dims.erase(ret.array_dims.begin());
  }
  return ret;
}

size_t Type::count_array_dims() const {
  return omit_first_dim ? array_dims.size() + 1 : array_dims.size();
}

MemSize Type::count_elements() const {
  MemSize ret = 1;
  for (MemSize i : array_dims) ret *= i;
  return ret;
}

MemSize Type::size() const { 
  return 4/*INT_SIZE*/ * count_elements(); 
}

MemSize Type::get_index(const vector<MemSize> &index) {
  MemSize step = 1, ret = 0;
  size_t next = array_dims.size() - 1;
  for (size_t i = index.size() - 1; i < index.size(); --i) {
    ret += index[i] * step;
    if (next < array_dims.size()) {
      step *= array_dims[next];
      --next;
    }
  }
  return ret;
}

VariableTableEntry *VariableTable::recursively_resolve(const string &varnm) {
  auto find = mapping.find(varnm);
  if (find != mapping.end()) 
    return find->second.get();
  else if (parent) 
    return parent->recursively_resolve(varnm);
  else 
    return nullptr;
}

bool IRValue::assignable() const {
  return (is_left_value) && ((!type.is_const) && (!type.is_array()));
}


bool Type::check_assign(const Type &rhs) const {
  if (omit_first_dim) {
    if (rhs.omit_first_dim) 
      return array_dims == rhs.array_dims;
    if (rhs.array_dims.size() != array_dims.size() + 1) 
      return 0;
    for (size_t i = 1; i < rhs.array_dims.size(); ++i)
      if (rhs.array_dims[i] != array_dims[i - 1]) return 0;
    return 
      1;
  } else {
    return (!rhs.omit_first_dim) && array_dims == rhs.array_dims;
  }
}

bool Type::check_index(const vector<MemSize> &index) {
  if (index.size() != count_array_dims()) return 0;
  if (omit_first_dim) {
    for (size_t i = 1; i < index.size(); ++i)
      if (index[i] >= array_dims[i - 1]) return 0;
  } else {
    for (size_t i = 0; i < index.size(); ++i)
      if (index[i] >= array_dims[i]) return 0;
  }
  return 1;
}

const Type Type::UnknownLengthArray = []() {
  Type t;
  t.omit_first_dim = 1;
  return t;
}();


FunctionInterface::FunctionInterface() : variadic(0) {}

FunctionTableEntry *FunctionTable::resolve(const string &FNAME) {
  auto find = mapping.find(FNAME);
  if (!(find == mapping.end()))
    return find->second.get();
  return nullptr;
}


