#pragma once

#include "syntax_tree.h"
#include "SYSYParserBaseVisitor.h"
#include<any>
namespace ast
{
namespace syntax_tree
{
class Scope {
public:
  // enter a new scope，一般在进入函数的时候操作
  void enter() {
    inner.push_back({});
    array_param_cum.push_back({});
    array_val.push_back({});
    array_val_index.push_back({});
  }

  // exit a scope，出了一个函数的范围
  void exit() {
    inner.pop_back();
    array_param_cum.pop_back();
    array_val.pop_back();
    array_val_index.pop_back();
  }

  bool in_global() { return inner.size() == 1; }//判断当前是不是在全局的地方（作用域只有一层就是）

  // push a name to scope
  // return true if successful
  // return false if this name already exits
  bool push(std::string name, expr_syntax *val) {//往当前层塞入一个变量，参数是变量名和变量的值
    auto result = inner[inner.size() - 1].insert({name, val});
    return result.second;
  }

  bool push_params(std::string name,  std::vector<expr_syntax *> params) {//insert array cum dim
    auto result = array_param_cum[array_param_cum.size() - 1].insert({name, params});
    return result.second;
  }
  bool push_arrary_val(std::string name,  std::vector<expr_syntax *> params,std::vector<std::shared_ptr<int>> val_index) {//insert arrary value
    auto result = array_val[array_param_cum.size() - 1].insert({name, params});
    std::vector<int> indices;
    for(auto i :val_index)
        indices.push_back(*(i.get()));
    array_val_index[array_param_cum.size() - 1].insert({name, indices});
    return result.second;
  }

  expr_syntax *& find(std::string name) {//根据名字找到某个变量，返回他的值
    for (auto s = inner.rbegin(); s != inner.rend(); s++) {
      auto iter = s->find(name);
      if (iter != s->end()) {//it will return end if not find
        return iter->second;
      }
    }

    return val_not_find;
  }
  void set(std::string name,expr_syntax * input)
  {
    for (auto s = inner.rbegin(); s != inner.rend(); s++) {
      auto iter = s->find(name);
      if (iter != s->end()) {//it will return end if not find
        iter->second=input;
        return;
      }
    }
  }

  std::vector<expr_syntax *>& find_arrary_params_cum(std::string name) {//根据名字找到参数（这里同时考虑int参数和数组参数，所以需要有一个输出参数）
    // std::cout<<"find_params"<<std::endl;
    for (auto s = array_param_cum.rbegin(); s != array_param_cum.rend(); s++) {
      // std::cout<<"find_params1"<<std::endl;
      auto iter = s->find(name);
      if (iter != s->end()) {
        // std::cout<<"find_params2"<<std::endl;
       // params.assign(iter->second.begin(), iter->second.end());//ccx deletes it
        return iter->second;
      }
    }
    // std::cout<<"find_params3"<<std::endl;
    return val_list_not_find;
  }

  expr_syntax *& find_arrary_val(std::string name,int coord) {//根据名字找到参数（这里同时考虑int参数和数组参数，所以需要有一个输出参数）
    // std::cout<<"find_params"<<std::endl;
    // std::vector<expr_syntax *> vals;
    int index=-1;

    for (auto s = array_val_index.rbegin(); s != array_val_index.rend(); s++) {
      // std::cout<<"find_params1"<<std::endl;
      auto iter = s->find(name);
      if (iter != s->end()) {
        // std::cout<<"find_params2"<<std::endl;
       // params.assign(iter->second.begin(), iter->second.end());//ccx deletes it
       auto indices=iter->second;
  
       for(int j=0;j<indices.size();j++)
       {
        if(indices[j]==coord)
        {
          index=j;
          break;
        }
       }
       
      }
    }
    if(index!=-1)
    {
       for (auto s = array_val.rbegin(); s != array_val.rend(); s++) {
      // std::cout<<"find_params1"<<std::endl;
      auto iter = s->find(name);
      if (iter != s->end()) {
        // std::cout<<"find_params2"<<std::endl;
       // params.assign(iter->second.begin(), iter->second.end());//ccx deletes it
        return iter->second[index];
      }
      
    }
    }
   

        auto zero_exp = new literal_syntax;
        zero_exp->is_int = true;//**********8888
        zero_exp->intConst = 0;//decimal
        zero_exp->floatConst=0;//这边类型
     
      //放入数据
    for (auto s = array_val_index.rbegin(); s != array_val_index.rend(); s++) {
      // std::cout<<"find_params1"<<std::endl;
      auto iter = s->find(name);
      if (iter != s->end()) {
            iter->second.push_back(coord);
       
      }
    }

       //插入信息并返回
        for (auto s = array_val.rbegin(); s != array_val.rend(); s++) {
          // std::cout<<"find_params1"<<std::endl;
          auto iter = s->find(name);
          if (iter != s->end()) {
            // std::cout<<"find_params2"<<std::endl;
          // params.assign(iter->second.begin(), iter->second.end());//ccx deletes it
            iter->second.push_back(zero_exp);
            return iter->second.back();
          }
          
        }
       

    // std::cout<<"find_params3"<<std::endl;
    // return val_not_find;
  }
  static expr_syntax *val_not_find;
  static std::vector<expr_syntax *>val_list_not_find;
private:
  std::vector<std::map<std::string, expr_syntax *>> inner;//名字，值
  std::vector<std::map<std::string, std::vector<expr_syntax *>>> array_param_cum;//数组的名字和各个累积维度
  std::vector<std::map<std::string, std::vector<expr_syntax *>>> array_val;//数组的名字和初始化的值
  std::vector<std::map<std::string, std::vector<int>>> array_val_index;//数组的名字和初始化的值
  

};
class syntax_tree_builder : public SYSYParserBaseVisitor
{
  public:
    syntax_tree_builder();

    virtual antlrcpp::Any visitCompilationUnit(SYSYParser::CompilationUnitContext *ctx) override;
    virtual antlrcpp::Any visitDecl(SYSYParser::DeclContext *ctx) override;
    virtual antlrcpp::Any visitConstdecl(SYSYParser::ConstdeclContext *ctx) override;
    virtual antlrcpp::Any visitConstdef(SYSYParser::ConstdefContext *ctx) override;
    virtual antlrcpp::Any visitVardecl(SYSYParser::VardeclContext *ctx) override;
    virtual antlrcpp::Any visitVardef(SYSYParser::VardefContext *ctx) override;
    virtual antlrcpp::Any visitFuncdef(SYSYParser::FuncdefContext *ctx) override;
    virtual antlrcpp::Any visitFuncFparam(SYSYParser::FuncFparamContext *ctx) override;
    virtual antlrcpp::Any visitBlock(SYSYParser::BlockContext *ctx) override;
    virtual antlrcpp::Any visitStmt(SYSYParser::StmtContext *ctx) override;
    virtual antlrcpp::Any visitLval(SYSYParser::LvalContext *ctx) override;
    virtual antlrcpp::Any visitCond(SYSYParser::CondContext *ctx) override;
    virtual antlrcpp::Any visitExp(SYSYParser::ExpContext *ctx) override;
    virtual antlrcpp::Any visitNumber(SYSYParser::NumberContext *ctx) override;
    void initelement(ptr_list<expr_syntax> &initializers,SYSYParser::InitContext *ctx,std::vector<int> &dem_list,ptr_list<int>& initializers_index,int coord,int dem);
    
    ptr<syntax_tree_node> operator()(antlr4::tree::ParseTree *ctx);
    Scope scope;
};
}
}


