#pragma once

#include <optional>
#include <string>
#include <utility>
#include <variant>
#include <vector>
#include <memory>
#include "syntax_tree.h"//my AST
#include "symbol_table.h"
#include "ir.h"
#include "SYSYParserBaseVisitor.h"

//乱包含的，先解决下报错
// #include "antlr-runtime"
template <typename T>
using ptr = std::shared_ptr<T>;
// List of reference of type
template <typename T>
using ptr_list = std::vector<ptr<T>>;

using namespace ast;//
class ASTVisitor : public syntax_tree::syntax_tree_visitor {
enum ValueMode { normal, compile_time, condition } mode;
enum RetType{VOID,INT,FLOAT} ;
  Medium::UserFunction *init_func;
  Medium::IrBlock *init_bb;

  FunctionTable functions;
  VariableTable global_var;
  
  std::string cur_func_name;
  Medium::UserFunction *cur_func;
  Medium::IrBlock *cur_block, *return_bb;
  std::vector<std::pair<Medium::Reg, Medium::IrBlock *>> return_value;
  std::vector<VariableTable *>
      local_var;  // local var table in current function, clear and destruct on
                  // finishing processing the function
  VariableTable *cur_local_table;
  // when cur_func == nullptr, cur_bb is nullptr, local_var is empty,
  // cur_local_table is nullptr
  bool in_init, found_main;
  std::vector<Medium::IrBlock *> break_target, continue_target;

  void register_lib_function(
      std::string name, RetType rettype,
      std::vector<std::variant<Type, StringType>> params);
  void register_lib_functions();
  Medium::Reg reg_changed(Medium::Reg src , bool is_int);
//   std::vector<MemSize> get_array_dims(
//       ptr_list<syntax_tree::expr_syntax *> dims);
      //TODO need to modify to our AST
  IRValue to_IRValue(antlrcpp::Any &value);  // check null, IRValue and CondJumpList
  CondJumpList to_CondJumpList(
      antlrcpp::Any &value);  // check null, IRValue and CondJumpList, after this
                             // call, cur_bb is nullptr
  Medium::Reg get_value(const IRValue &value);  // check array
  Medium::Reg new_medium_reg(bool is_int,int size=INT_SIZE);
  Medium::IrBlock *new_block();
  VariableTable *new_variable_table(VariableTable *parent);
  VariableTableEntry *resolve(const std::string &name);

 public:
    Medium::CompileUnit *ir;
    virtual void visit(syntax_tree::assembly &node) override final;
    virtual void visit(syntax_tree::func_def_syntax &node) override final;
    virtual void visit(syntax_tree::cond_syntax &node) override final;
    virtual void visit(syntax_tree::binop_expr_syntax &node) override final;
    virtual void visit(syntax_tree::unaryop_expr_syntax &node) override final;
    virtual void visit(syntax_tree::lval_syntax &node) override final;
    virtual void visit(syntax_tree::literal_syntax &node) override final;
    virtual void visit(syntax_tree::var_def_stmt_syntax &node) override final;
    virtual void visit(syntax_tree::assign_stmt_syntax &node) override final;
    virtual void visit(syntax_tree::func_call_stmt_syntax &node) override final;
    virtual void visit(syntax_tree::block_syntax &node) override final;
    virtual void visit(syntax_tree::if_stmt_syntax &node) override final;
    virtual void visit(syntax_tree::while_stmt_syntax &node) override final;
    virtual void visit(syntax_tree::empty_stmt_syntax &node) override final;
    virtual void visit(syntax_tree::func_param_syntax &node) override final;
    virtual void visit(syntax_tree::bcr_stmt_syntax & node) override final;
    virtual void visit(syntax_tree::func_call_syntax & node) override final;

};