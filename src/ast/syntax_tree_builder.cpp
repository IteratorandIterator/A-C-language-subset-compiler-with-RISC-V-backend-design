
#include <algorithm>
#include <syntax_tree_builder.h>
#include <iostream>
#include <memory>
#include <vector>

using namespace ast;
using namespace ast::syntax_tree;
int literal_type=0;//0：unknow ,1:int ,2:float
std::vector<expr_syntax*> nothing_list;
expr_syntax* nothing;
expr_syntax* Scope::val_not_find;
std::vector<expr_syntax*> Scope::val_list_not_find;
syntax_tree_builder::syntax_tree_builder(){}
bool is_variable_int;
static bool pre_enter_scope;
bool control_flow = false;
bool usepropagation=true;
int getdemension(SYSYParser::InitContext* ctx, int depth) {
  if (ctx->exp())return depth;
  if (ctx->LeftBrace()) {
    depth += 1;
    int max = depth;
    int tmp;
    for (auto son : ctx->init()) {
      tmp = getdemension(son, depth);
      if (tmp > depth)max = tmp;
    }
    return max;
  }
}
void syntax_tree_builder::initelement(ptr_list<expr_syntax>& initializers, SYSYParser::InitContext* ctx, std::vector<int>& dem_list,ptr_list<int>& initializers_index, int coord, int dem) {
  if (ctx->exp()) {
    //initializers[coord].reset(visit(ctx->exp()).as<expr_syntax*>());//assign it with the exp in ctx 
    initializers.emplace_back(std::any_cast<expr_syntax*>(visit(ctx->exp())));
    initializers_index.push_back(std::make_shared<int>(coord));
  }
  if (ctx->LeftBrace()) {
    int tmp = coord;
    for (auto son : ctx->init()) {
      initelement(initializers, son, dem_list,initializers_index, tmp, dem + 1);
      if (son->LeftBrace()) { //something like {1},add the coresponding size
        int mul = 1;
        for (int i = dem + 1;i < dem_list.size();i++)
          mul *= dem_list[i];//get the size in this dimension
        tmp += mul;
      }
      else tmp += 1;//exp +1
    }
  }
}
antlrcpp::Any syntax_tree_builder::visitCompilationUnit(SYSYParser::CompilationUnitContext* ctx) {
  this->scope.enter();
  auto result = new assembly;
  auto var_defs = new assembly;
  auto fun_defs = new assembly;
  result->line = ctx->getStart()->getLine();
  result->pos = ctx->getStart()->getCharPositionInLine();
  auto decls = ctx->decl();
  for (int i = 0;i < decls.size();++i)//variable def node
  {
    if (decls[i]->vardecl()) {
      auto vardefs = decls[i]->vardecl()->vardef();
      is_variable_int = decls[i]->vardecl()->Int();//pass type to the defs
      for (int j = 0;j < vardefs.size();++j)
        var_defs->global_defs.emplace_back(std::any_cast<var_def_stmt_syntax*>(visit(vardefs[j])));
    }
    if (decls[i]->constdecl()) {
      auto constdefs = decls[i]->constdecl()->constdef();
      is_variable_int = decls[i]->constdecl()->Int();
      for (int j = 0;j < constdefs.size();++j)
        var_defs->global_defs.emplace_back(std::any_cast<var_def_stmt_syntax*>(visit(constdefs[j])));
    }
  }
  auto funcdefs = ctx->funcdef();
  for (int i = 0;i < funcdefs.size();++i)//function def node
    fun_defs->global_defs.emplace_back(std::any_cast<func_def_syntax*>(visit(funcdefs[i])));
  //sort the decl and function def
  auto v = var_defs->global_defs.begin();
  auto f = fun_defs->global_defs.begin();
  while (v != var_defs->global_defs.end() || f != fun_defs->global_defs.end())//fill the global_def attribute of the block node according to the place of each def node
  {
    if (v != var_defs->global_defs.end() && (f == fun_defs->global_defs.end() || (*v)->line < (*f)->line || (*v)->line == (*f)->line && (*v)->pos < (*f)->line)) {
      result->global_defs.push_back(*v);
      ++v;
    }
    else {
      result->global_defs.push_back(*f);
      ++f;
    }
  }
  this->scope.exit();
  return static_cast<assembly*>(result);
}
antlrcpp::Any syntax_tree_builder::visitDecl(SYSYParser::DeclContext* ctx) {//don't need it any more

}
antlrcpp::Any syntax_tree_builder::visitConstdecl(SYSYParser::ConstdeclContext* ctx) {//don't need it anymore


}
antlrcpp::Any syntax_tree_builder::visitConstdef(SYSYParser::ConstdefContext* ctx) {//almost the same as the non-const case ,just respecify the is_constant and delete the non-assignment case
  auto result = new var_def_stmt_syntax;//variable defination statement node
  result->line = ctx->getStart()->getLine();
  result->pos = ctx->getStart()->getCharPositionInLine();
  result->is_constant = true;//the const defination and non-const defination share the same syntax node,this attribute is used to seperate them
  result->is_int = is_variable_int;

  
 


  result->name = ctx->Identifier()->getSymbol()->getText();
  if (ctx->LeftBracket().size())//array
  {
    int count = 0;
    std::vector<int> dem_list;
    ptr_list<int> initializers_index;
    for (;count < ctx->exp().size();++count) {
      result->array_length.emplace_back(std::any_cast<expr_syntax*>(visit(ctx->exp()[count])));//this exp is the length of the array
    }
    for (int i = 0;i < ctx->exp().size();i++) {
      auto literal = dynamic_cast<literal_syntax*>(result->array_length.at(i).get());
      if (literal != NULL) {
        if (!literal->is_int)throw "number in dim must be an int!";
        dem_list.push_back(literal->intConst);
      }
    }
    //1.calc the value of demension
    int num = 1;
    for (int i : dem_list) num *= i;
    //1.2get the value of each dimension
    //2.assert dimension
    int demension = result->array_length.size();
    int depth = 0;
    depth = getdemension(ctx->init(), 0);
    //2.1check dimension
    //3.init value
    //3.1 init all to 0,for some reason ,comment it
    // for (int i = 0;i < num;i++) {
    //   auto zero_exp = new literal_syntax;
    //   zero_exp->is_int = is_variable_int==true?true:false;
    //   zero_exp->line = ctx->getStart()->getLine();
    //   zero_exp->pos = ctx->getStart()->getCharPositionInLine();//this position is given by mistake,but I mean it.
    //   zero_exp->intConst = 0;//decimal
    //   result->initializers.emplace_back(static_cast<expr_syntax*>(zero_exp));
    // }
    //3.2 init the given elements
     int literal_type_tmp=literal_type;
  literal_type=result->is_int?1:2;


    initelement(result->initializers, ctx->init(), dem_list,result->initializers_index, 0, 0);

    //初始化数据后根据变量的类型纠正解析的常数的类型
    literal_syntax * tmp;
    for (auto &val_node : result->initializers) 
    {
      if((tmp=dynamic_cast<literal_syntax*>(val_node.get()))&&tmp->is_int!=result->is_int)//如果是常数类型，就对类型进行转化的
      {
        static_cast<literal_syntax*>(val_node.get())->is_int=result->is_int;
        if(result->is_int)
          static_cast<literal_syntax*>(val_node.get())->intConst=static_cast<literal_syntax*>(val_node.get())->floatConst;
        else
          static_cast<literal_syntax*>(val_node.get())->floatConst=static_cast<literal_syntax*>(val_node.get())->intConst;
      }
    }

    literal_type=literal_type_tmp;

    // result->initializers_index=initializers_index;
    //get the cum para
    std::vector<expr_syntax*> arrary_para;
    std::vector<expr_syntax*> arrary_val;
    int dim_cum = 1;
    for (auto dim_node = result->array_length.end() - 1;dim_node >= result->array_length.begin();dim_node--) {
      int dim = static_cast<literal_syntax*>(dim_node->get())->intConst;//assume its type is int
      auto tmp = new literal_syntax;
      tmp->is_int = true;
      tmp->intConst = dim_cum;
      arrary_para.insert(arrary_para.begin(), static_cast<expr_syntax*>(tmp));
      dim_cum *= dim;
    }

    for (auto val_node : result->initializers) {
    
      if(this->scope.in_global())val_node->is_global=true;
      arrary_val.push_back(val_node.get());//这里后续可以优化
    }
    this->scope.push_arrary_val(result->name, arrary_val,result->initializers_index);//assign value
    this->scope.push_params(result->name, arrary_para);






    //补丁，用于区分int a[4];和int a[4]={};
    if(result->initializers.size()==0&&ctx->Assign())
    {
      result->initializers_index.push_back(std::make_shared<int>(-1));
    }
  }
  else//normal variable,initialize it with the only exp
  {
        int literal_type_tmp=literal_type;
  literal_type=result->is_int?1:2;


    result->initializers.emplace_back(std::any_cast<expr_syntax*>(visit(ctx->init()->exp())));
    //对数据进行纠正
       literal_syntax * tmp;
    for (auto &val_node : result->initializers) 
    {
      if((tmp=dynamic_cast<literal_syntax*>(val_node.get()))&&tmp->is_int!=result->is_int)//如果是常数类型，就对类型进行转化的
      {
        static_cast<literal_syntax*>(val_node.get())->is_int=result->is_int;
        if(result->is_int)
          static_cast<literal_syntax*>(val_node.get())->intConst=static_cast<literal_syntax*>(val_node.get())->floatConst;
        else
          static_cast<literal_syntax*>(val_node.get())->floatConst=static_cast<literal_syntax*>(val_node.get())->intConst;
      }
    }


    if(this->scope.in_global())result->initializers[0]->is_global=true;
    this->scope.push(result->name, result->initializers[0].get());

    literal_type=literal_type_tmp;
  }

  

  return static_cast<var_def_stmt_syntax*>(result);
}
antlrcpp::Any syntax_tree_builder::visitVardecl(SYSYParser::VardeclContext* ctx) { //don't need it anymore

}
antlrcpp::Any syntax_tree_builder::visitVardef(SYSYParser::VardefContext* ctx) {
  auto result = new var_def_stmt_syntax;//variable defination statement node
  result->line = ctx->getStart()->getLine();
  result->pos = ctx->getStart()->getCharPositionInLine();
  result->is_constant = false;//the const defination and non-const defination share the same syntax node,this attribute is used to seperate them
  result->is_int = is_variable_int;





  result->name = ctx->Identifier()->getSymbol()->getText();
  if (ctx->Assign())//defination with assignment
  {
    if (ctx->LeftBracket().size())//array
    {
      int count = 0;
      std::vector<int> dem_list;
        ptr_list<int> initializers_index;
      for (; count < ctx->exp().size(); ++count)
        result->array_length.emplace_back(std::any_cast<expr_syntax*>(visit(ctx->exp()[count])));//this exp is the length of the array
      for (int i = 0; i < ctx->exp().size(); i++) {
        auto literal = dynamic_cast<literal_syntax*>(result->array_length.at(i).get());

        if (literal != NULL) {
          if (!literal->is_int)throw "number in dim must be an int!";
          dem_list.push_back(literal->intConst);
        }
      }
      //1.calc the value of demension
      int num = 1;
      for (int i : dem_list) num *= i;
      //1.2get the value of each dimension
      //2.assert dimension
      int demension = result->array_length.size();
      int depth = 0;
      depth = getdemension(ctx->init(), 0);
      //3.init value
      //3.1 init all to 0
      // for (int i = 0;i < num;i++) {
      //   auto zero_exp = new literal_syntax;
      //   zero_exp->is_int = is_variable_int==true?true:false;//**********8888
      //   zero_exp->line = ctx->getStart()->getLine();
      //   zero_exp->pos = ctx->getStart()->getCharPositionInLine();//this position is given by mistake,but I mean it.
      //   zero_exp->intConst = 0;//decimal
      //   result->initializers.emplace_back(static_cast<expr_syntax*>(zero_exp));
      // }
      //3.2 init the given elements

        int literal_type_tmp=literal_type;
          literal_type=result->is_int?1:2;

      initelement(result->initializers, ctx->init(), dem_list,result->initializers_index, 0, 0);
      //对常数类型就行纠正
          literal_syntax * tmp;
    for (auto &val_node : result->initializers) 
    {
      if((tmp=dynamic_cast<literal_syntax*>(val_node.get()))&&tmp->is_int!=result->is_int)//如果是常数类型，就对类型进行转化的
      {
        static_cast<literal_syntax*>(val_node.get())->is_int=result->is_int;
        if(result->is_int)
          static_cast<literal_syntax*>(val_node.get())->intConst=static_cast<literal_syntax*>(val_node.get())->floatConst;
        else
          static_cast<literal_syntax*>(val_node.get())->floatConst=static_cast<literal_syntax*>(val_node.get())->intConst;
      }
    }





       literal_type=literal_type_tmp;

      //普通全局变量设置为不可折叠
      //get the cum para
      if (!this->scope.in_global()) {
        std::vector<expr_syntax*> arrary_para;
        std::vector<expr_syntax*> arrary_val;
        int dim_cum = 1;
        for (auto dim_node = result->array_length.end() - 1;dim_node >= result->array_length.begin();dim_node--) {
          int dim = static_cast<literal_syntax*>(dim_node->get())->intConst;//assume its type is int
          auto tmp = new literal_syntax;
          tmp->is_int = true;
          tmp->intConst = dim_cum;
          arrary_para.insert(arrary_para.begin(), static_cast<expr_syntax*>(tmp));
          dim_cum *= dim;
        }

        for (auto val_node : result->initializers) {
          arrary_val.push_back(val_node.get());//这里后续可以优化
        }
        this->scope.push_arrary_val(result->name, arrary_val,result->initializers_index);//assign value
        this->scope.push_params(result->name, arrary_para);
      }




      //补丁，用于区分int a[4];和int a[4]={};
    if(result->initializers.size()==0&&ctx->Assign())
    {
      result->initializers_index.push_back(std::make_shared<int>(-1));
    }

    }
    else//normal variable,initialize it with the only exp
    {

        int literal_type_tmp=literal_type;
          literal_type=result->is_int?1:2;

      result->initializers.emplace_back(std::any_cast<expr_syntax*>(visit(ctx->init()->exp())));

      //对常数类型进行纠正
          literal_syntax * tmp;
    for (auto &val_node : result->initializers) 
    {
      if((tmp=dynamic_cast<literal_syntax*>(val_node.get()))&&tmp->is_int!=result->is_int)//如果是常数类型，就对类型进行转化的
      {
        static_cast<literal_syntax*>(val_node.get())->is_int=result->is_int;
        if(result->is_int)
          static_cast<literal_syntax*>(val_node.get())->intConst=static_cast<literal_syntax*>(val_node.get())->floatConst;
        else
          static_cast<literal_syntax*>(val_node.get())->floatConst=static_cast<literal_syntax*>(val_node.get())->intConst;
      }
    }


      if (!this->scope.in_global())
        this->scope.push(result->name, result->initializers[0].get());

          literal_type=literal_type_tmp;


    }
  }
  else//defination without assignment
  {
    if (ctx->LeftBracket().size())//array
    {

      int count = 0;
      std::vector<int> dem_list;
      ptr_list<int> initializers_index;
      for (; count < ctx->exp().size(); ++count)
        result->array_length.emplace_back(std::any_cast<expr_syntax*>(visit(ctx->exp()[count])));//this exp is the length of the array
      for (int i = 0; i < ctx->exp().size(); i++) {
        auto literal = dynamic_cast<literal_syntax*>(result->array_length.at(i).get());

        if (literal != NULL) {
          if (!literal->is_int)throw "number in dim must be an int!";
          dem_list.push_back(literal->intConst);
        }
      }
      //1.calc the value of demension
      int num = 1;
      for (int i : dem_list) num *= i;
      //1.2get the value of each dimension
      //2.assert dimension
      int demension = result->array_length.size();
      //3.init value
      //3.1 init all to 0 ,for some reason ,I comment it
      // for (int i = 0;i < num;i++) {
      //   auto zero_exp = new literal_syntax;
      //   zero_exp->is_int =is_variable_int==true?true:false;
      //   zero_exp->line = ctx->getStart()->getLine();
      //   zero_exp->pos = ctx->getStart()->getCharPositionInLine();//this position is given by mistake,but I mean it.
      //   zero_exp->intConst = 0;//decimal
      //   result->initializers.emplace_back(static_cast<expr_syntax*>(zero_exp));
      // }


      if (!this->scope.in_global()) {
        std::vector<expr_syntax*> arrary_para;
        std::vector<expr_syntax*> arrary_val;
        int dim_cum = 1;
        for (auto dim_node = result->array_length.end() - 1;dim_node >= result->array_length.begin();dim_node--) {
          int dim = static_cast<literal_syntax*>(dim_node->get())->intConst;//assume its type is int
          auto tmp = new literal_syntax;
          tmp->is_int = true;
          tmp->intConst = dim_cum;
          arrary_para.insert(arrary_para.begin(), static_cast<expr_syntax*>(tmp));
          dim_cum *= dim;
        }

        for (auto val_node : result->initializers) {
          arrary_val.push_back(val_node.get());//这里后续可以优化
        }
        this->scope.push_arrary_val(result->name, arrary_val,initializers_index);//assign value
        this->scope.push_params(result->name, arrary_para);
      }


    }
    else {//normal variable
      if (!this->scope.in_global())
        this->scope.push(result->name, nothing);
    }
  }

 

  return static_cast<var_def_stmt_syntax*>(result);
}
antlrcpp::Any syntax_tree_builder::visitFuncdef(SYSYParser::FuncdefContext* ctx) {
  this->scope.enter();
  pre_enter_scope = true;
  auto func = new func_def_syntax;
  auto name = ctx->Identifier()->getText();
  auto pos = ctx->getStart()->getCharPositionInLine();
  auto line = ctx->getStart()->getLine();
  func->line = line;
  func->pos = pos;
  func->name = name;
  if (ctx->funcFparams()) {
    auto params = ctx->funcFparams()->funcFparam();
    for (int i = 0; i < params.size(); i++) {
      func->params.emplace_back(std::any_cast<func_param_syntax*>(visit(params[i])));
    }
  }
  //assign type 
  if (ctx->functype()->Int()) func->ret_type = "int";
  else if (ctx->functype()->Float())func->ret_type = "float";
  else func->ret_type = "void";
  func->body.reset(std::any_cast<block_syntax*>(visit(ctx->block())));



  this->scope.exit();
  return static_cast<func_def_syntax*>(func);
}
antlrcpp::Any syntax_tree_builder::visitFuncFparam(SYSYParser::FuncFparamContext* ctx) {

  auto res = new func_param_syntax;

  auto expressions = ctx->exp();

  res->ident = ctx->Identifier()->getText();
  if (ctx->typeId()->Float()) {
    res->type = ctx->typeId()->Float()->getText();
  }
  else {
    res->type = ctx->typeId()->Int()->getText();
  }

  res->pos = ctx->getStart()->getCharPositionInLine();
  res->line = ctx->getStart()->getLine();

  if (ctx->LeftBracket().size() != 0) {//?why is 1 ,not  0? ccx modifies it
    res->is_array = 1;
  }
  else {
    res->is_array = 0;
  }
  //add to scope
  if (res->is_array)//array
  {
    this->scope.push_params(res->ident, nothing_list);

  }
  else {//normal variable
    this->scope.push(res->ident, nothing);
  }


  for (int i = 0; i < expressions.size(); i++) {
    res->dimension_of_array.emplace_back(
      std::any_cast<expr_syntax*>(visit(expressions[i])));
  }

  return antlrcpp::Any(res);
}
antlrcpp::Any syntax_tree_builder::visitBlock(SYSYParser::BlockContext* ctx) {
  bool need_exit_scope = pre_enter_scope;
  if (!pre_enter_scope) {
    scope.enter();
  }
  pre_enter_scope = false;
  auto res = new block_syntax;
  auto decls = ctx->decl();
  auto stmts = ctx->stmt();
  std::vector<std::pair<int, antlr4::ParserRuleContext*>> context;
  for (auto& decl : decls) context.push_back({ 0,decl });
  for (auto& stmt : stmts) context.push_back({ 1,stmt });

  sort(context.begin(), context.end(), [](std::pair<int, antlr4::ParserRuleContext*> c1, std::pair<int, antlr4::ParserRuleContext*> c2) {
    auto row1 = c1.second->getStart()->getLine();
    auto col1 = c1.second->getStart()->getCharPositionInLine();
    auto row2 = c2.second->getStart()->getLine();
    auto col2 = c2.second->getStart()->getCharPositionInLine();
    if (row1 != row2)
      return row1 < row2;
    else
      return col1 < col2;
  });

  res->line = ctx->getStart()->getLine();
  res->pos = ctx->getStart()->getCharPositionInLine();

  for (int i = 0; i < context.size(); i++) {
    int type = context[i].first;
    auto context_son = context[i].second;
    if (!type) {
      if (auto decl = static_cast<SYSYParser::DeclContext*>(context_son)->vardecl()) {
        auto vardefs = decl->vardef();
        is_variable_int = decl->Int();//pass type to the defs
        for (int j = 0;j < vardefs.size();++j)
          res->body.emplace_back(std::any_cast<var_def_stmt_syntax*>(visit(vardefs[j])));
      }
      if (auto decl = static_cast<SYSYParser::DeclContext*>(context_son)->constdecl()) {
        auto constdefs = decl->constdef();
        is_variable_int = decl->Int();//pass type to the defs
        for (int j = 0;j < constdefs.size();++j)
          res->body.emplace_back(std::any_cast<var_def_stmt_syntax*>(visit(constdefs[j])));
      }
    }
    else {
      auto stmt = static_cast<SYSYParser::StmtContext*>(context_son);

      if (!stmt->Assign() && !stmt->Return() && !stmt->LeftParen() && stmt->exp().size())//get k+1;
      {

        std::deque<SYSYParser::ExpContext*> tool;
        for (auto exp : stmt->exp()[0]->exp()) {
          tool.push_back(exp);
        }
        while (tool.size() != 0) {
          auto exp = tool.at(0);
          tool.pop_front();

          if (exp->Identifier())//functioncall
          {
            // std::cout<<"find function call"<<std::endl;
            auto fun = new func_call_stmt_syntax;
            fun->name = exp->Identifier()->getText();
            fun->line = exp->getStart()->getLine();
            fun->pos = exp->getStart()->getCharPositionInLine();
            if (exp->exp().size())//if the  function call has paras
            {
              for (int i = 0; i < exp->exp().size(); ++i)
                fun->array_index.emplace_back(std::any_cast<expr_syntax*>(visit(exp->exp()[i])));
            }
            res->body.emplace_back(static_cast<stmt_syntax*>(fun));
          }
          else if (exp->exp().size()) {
            for (int i = exp->exp().size() - 1;i >= 0;i--) {
              tool.push_front(exp->exp()[i]);
            }
          }

        }

      }
      else
        res->body.emplace_back(std::any_cast<stmt_syntax*>(visit(stmt)));
      //}
    }
  }
  pre_enter_scope = need_exit_scope;
  if (!pre_enter_scope) {
    this->scope.exit();
  }
  return antlrcpp::Any(res);
}
antlrcpp::Any syntax_tree_builder::visitStmt(SYSYParser::StmtContext* ctx) {
  auto pos = ctx->getStart()->getCharPositionInLine();
  auto line = ctx->getStart()->getLine();
  if (ctx->If()) {
    bool tmp=control_flow;
    control_flow = true;
    auto res = new if_stmt_syntax;
    res->pred.reset(std::any_cast<cond_syntax*>(visit(ctx->cond())));
    res->then_body.reset(std::any_cast<stmt_syntax*>(visit(ctx->stmt()[0])));
    if (ctx->Else()) {
      res->else_body.reset(std::any_cast<stmt_syntax*>(visit(ctx->stmt()[1])));
    }
    else {
      res->else_body = nullptr;
    }
    res->line = line;
    res->pos = pos;
    return static_cast<stmt_syntax*>(res);
    control_flow = tmp;
  }
  else if (ctx->While()) {
    bool tmp=control_flow;
    control_flow = true;
    auto res = new while_stmt_syntax;
    usepropagation=false;
    res->pred.reset(std::any_cast<cond_syntax*>(visit(ctx->cond())));
    res->body.reset(std::any_cast<stmt_syntax*>(visit(ctx->stmt()[0])));
    usepropagation=true;
    res->line = line;
    res->pos = pos;
    return static_cast<stmt_syntax*>(res);
    control_flow =  tmp;
  }
  else if (ctx->block()) {
    return static_cast<stmt_syntax*>(std::any_cast<block_syntax*>(visit(ctx->block())));
  }
  else if (ctx->Assign()) {
    auto res = new assign_stmt_syntax;
    res->target.reset(std::any_cast<lval_syntax*>(visit(ctx->lval())));
    res->value.reset(std::any_cast<expr_syntax*>(visit(ctx->exp()[0])));
    res->line = line;
    res->pos = pos;
    auto lvalue = res->target;
    if (lvalue->array_index.size())//arrary
    {
      //calc coordinate
      auto arrary_dims = this->scope.find_arrary_params_cum(lvalue->name);
      int coordinate = 0;
      bool available = true;
      if (arrary_dims != nothing_list) {
        for (int i = 0; i < lvalue->array_index.size();i++) {
          //dim is unknow!
          auto literal = dynamic_cast<literal_syntax*>(lvalue->array_index[i].get());
          if (!literal) {

            available = false;
            continue;
          }
          coordinate += (static_cast<literal_syntax*>(lvalue->array_index[i].get())->intConst) * (static_cast<literal_syntax*>(arrary_dims[i])->intConst);
        }
        if (available)//assign the corresponding value
        {
          if (control_flow&&this->scope.find_arrary_val(lvalue->name,coordinate)&&!this->scope.find_arrary_val(lvalue->name,coordinate)->is_global) {//如果是在控制流中，就直接赋值为空，虽然本层中可以进行折叠，但是这个后面再优化
            this->scope.find_arrary_params_cum(lvalue->name) = nothing_list;
          }
          else
            this->scope.find_arrary_val(lvalue->name,coordinate) = res->value.get();

        }
        else {
          //set the arrary to not available  by clear its dims!!! good idea!
          this->scope.find_arrary_params_cum(lvalue->name) = nothing_list;

        }
      }

    }
    else//normal variable
    {
      auto tmp=this->scope.find(lvalue->name);
      if (control_flow&&this->scope.find(lvalue->name)&&!this->scope.find(lvalue->name)->is_global) {//如果是在控制流中，就直接赋值为空，虽然本层中可以进行折叠，但是这个后面再优化
        this->scope.find(lvalue->name) = nothing;
      }
      else {
        auto& val = this->scope.find(lvalue->name);//=res->value.get()
        if (val != Scope::val_not_find)val = res->value.get();
      }
      //this->scope.set(lvalue->name,res->value.get());
      //这边需要判断这个变量是否存在，一般情况下变量是一定可以找到的，但是这边我们对全局的普通变量不进行折叠。。。防止数据流的紊乱
    }
    return static_cast<stmt_syntax*>(res);
  }
  else if (ctx->LeftParen()) {
    auto res = new func_call_stmt_syntax;
    res->name = ctx->Identifier()->getText();
    res->line = line;
    res->pos = pos;
    if (ctx->exp().size())//if the  function call has paras
    {
      for (int i = 0; i < ctx->exp().size(); ++i) {
        res->array_index.emplace_back(std::any_cast<expr_syntax*>(visit(ctx->exp()[i])));
      }
      //将传入的数组无效化TODO*****exp那边也要搞
      for (auto para : res->array_index) {
        auto val = dynamic_cast<lval_syntax*>(para.get());//只判断变量
        if (val && this->scope.find_arrary_params_cum(val->name) != Scope::val_list_not_find)//发现传入的是数组变量,如果找到的是const的数组就完了TODO******
        {
          this->scope.find_arrary_params_cum(val->name) = nothing_list;
        }
      }

    }
    return static_cast<stmt_syntax*>(res);
  }
  else if (ctx->Continue()) {
    auto res = new bcr_stmt_syntax;
    res->type = 0;
    res->line = line;
    res->pos = pos;
    return static_cast<stmt_syntax*>(res);
  }
  else if (ctx->Break()) {
    auto res = new bcr_stmt_syntax;
    res->type = 1;
    res->line = line;
    res->pos = pos;
    return static_cast<stmt_syntax*>(res);
  }
  else if (ctx->Return()) {
    auto res = new bcr_stmt_syntax;
    res->type = 2;
    res->line = line;
    res->pos = pos;
    if (ctx->exp().size()) {
      res->exp.reset(std::any_cast<expr_syntax*>(visit(ctx->exp()[0])));
    }
    else {
      res->exp = nullptr;
    }
    return static_cast<stmt_syntax*>(res);
  }
  else {
    auto res = new empty_stmt_syntax;
    res->line = line;
    res->pos = pos;
    return static_cast<stmt_syntax*>(res);
  }
}
antlrcpp::Any syntax_tree_builder::visitLval(SYSYParser::LvalContext* ctx) {
  auto res = new lval_syntax;
  res->line = ctx->getStart()->getLine();
  res->pos = ctx->getStart()->getCharPositionInLine();
  res->name = ctx->Identifier()->getSymbol()->getText();
  if (ctx->LeftBracket().size()) {
    for (int i = 0; i < ctx->exp().size(); ++i)
      res->array_index.emplace_back(std::any_cast<expr_syntax*>(visit(ctx->exp()[i])));
  }
  return static_cast<lval_syntax*>(res);
}
antlrcpp::Any syntax_tree_builder::visitCond(SYSYParser::CondContext* ctx) {
  auto result = new cond_syntax;//the condition node
  result->line = ctx->getStart()->getLine();
  result->pos = ctx->getStart()->getCharPositionInLine();
  if (ctx->exp()) {
    result->op = relop::op_none;
    result->rhs.reset(std::any_cast<expr_syntax*>(visit(ctx->exp())));
    return static_cast<cond_syntax*>(result);
  }
  //cond op cond
  if (ctx->cond().size() == 2) {
    result->lhsc.reset(std::any_cast<cond_syntax*>(visit(ctx->cond()[0])));
    if (ctx->And()) {//determinate the operation
      result->op = relop::op_and;
    }
    if (ctx->Or()) {
      result->op = relop::op_or;
    }
    if (ctx->Equal())//determinate the operation
      result->op = relop::equal;
    if (ctx->NonEqual())
      result->op = relop::non_equal;
    if (ctx->Less())
      result->op = relop::less;
    if (ctx->Greater())
      result->op = relop::greater;
    if (ctx->LessEqual())
      result->op = relop::less_equal;
    if (ctx->GreaterEqual())
      result->op = relop::greater_equal;
    result->rhsc.reset(std::any_cast<cond_syntax*>(visit(ctx->cond()[1])));
    return static_cast<cond_syntax*>(result);
  }
  //op cond
  if (ctx->cond().size() == 1) {
    if (ctx->Not()) {
      result->op = relop::op_not;
      result->rhsc.reset(std::any_cast<cond_syntax*>(visit(ctx->cond()[0])));
      return static_cast<cond_syntax*>(result);
    }
    if (ctx->Plus()) {
      result->op = relop::op_plus;
      result->rhsc.reset(std::any_cast<cond_syntax*>(visit(ctx->cond()[0])));
      return static_cast<cond_syntax*>(result);
    }
    if (ctx->Minus()) {
      result->op = relop::op_minus;
      result->rhsc.reset(std::any_cast<cond_syntax*>(visit(ctx->cond()[0])));
      return static_cast<cond_syntax*>(result);
    }
  }
}

// Returns antlrcpp::Any, which is constructable from any type.
// However, you should be sure you use the same type for packing and depacking
// the `Any` object. Or a std::bad_cast exception will rise. This function
// always returns an `Any` object containing a `expr_syntax *`.
antlrcpp::Any syntax_tree_builder::visitExp(SYSYParser::ExpContext* ctx) {
  // Get all sub-contexts of type `exp`.
  auto expressions = ctx->exp();
  int left_int, right_int;
  float left_float, right_float;
  bool is_left_int = true;//本次计算的结果是不是int类型的
  bool is_right_int = true;//本次计算的结果是不是int类型的
  bool is_precalced_l = false;//本次计算能否进行
  bool is_precalced_r = false;//本次计算能否进行
  // Two sub-expressions presented: this indicates it's a expression of binary
  // operator, aka `binop`.
  if (expressions.size() == 2 && !ctx->Identifier()) {
    auto result = new binop_expr_syntax;
    // Set line and pos.
    result->line = ctx->getStart()->getLine();
    result->pos = ctx->getStart()->getCharPositionInLine();
    // visit(some context) is equivalent to calling corresponding visit method;
    // dispatching is done automatically by ANTLR4 runtime. For this case, it's
    // equivalent to visitExp(expressions[0]). Use reset to set a new pointer to
    // a std::shared_ptr object. DO NOT use assignment; it won't work. Use
    // `.as<Type>()' to get value from antlrcpp::Any object; notice that this
    // Type must match the type used in constructing the Any object, which is
    // constructed from (usually pointer to some derived class of syntax_node,
    // in this case) returning value of the visit call.
    result->lhs.reset(std::any_cast<expr_syntax*>(visit(expressions[0])));
    result->rhs.reset(std::any_cast<expr_syntax*>(visit(expressions[1])));
    // Check if each token exists.
    // Returnd value of the calling will be nullptr (aka NULL in C) if it isn't
    // there; otherwise non-null pointer.

    auto literal_l = dynamic_cast<literal_syntax*>(result->lhs.get());

    if (literal_l != nullptr) {
      if (literal_l->is_int) {
        left_int = literal_l->intConst;
        is_left_int = true;
      }
      else {
        left_float = literal_l->floatConst;
        is_left_int = false;
      }
      is_precalced_l = true;
    }


    auto literal_r = dynamic_cast<literal_syntax*>(result->rhs.get());

    if (literal_r != nullptr) {
      if (literal_r->is_int) {
        right_int = literal_r->intConst;
        is_right_int = true;
      }

      else {
        right_float = literal_r->floatConst;
        is_right_int = false;
      }
      is_precalced_r = true;
    }


    if (ctx->Plus()) {
      result->op = binop::plus;
      if (is_precalced_l && is_precalced_r)//如果两边都可以直接算，那么输出literal类型
      {
        auto result_p = new literal_syntax;
        result_p->line = result->line;
        result_p->pos = result->pos;
        if (is_right_int && is_left_int) {
          result_p->is_int = true;
          result_p->intConst = left_int + right_int;
        }

        else {
          result_p->is_int = false;
          result_p->floatConst = (is_left_int ? left_int : left_float) + (is_right_int ? right_int : right_float);
        }
        return static_cast<expr_syntax*>(result_p);
      }

    }
    if (ctx->Minus()) {
      result->op = binop::minus;
      if (is_precalced_l && is_precalced_r)//如果两边都可以直接算，那么输出literal类型
      {
        auto result_p = new literal_syntax;
        result_p->line = result->line;
        result_p->pos = result->pos;
        if (is_right_int && is_left_int) {
          result_p->is_int = true;
          result_p->intConst = left_int - right_int;
        }

        else {
          result_p->is_int = false;
          result_p->floatConst = (is_left_int ? left_int : left_float) - (is_right_int ? right_int : right_float);
        }
        return static_cast<expr_syntax*>(result_p);
      }
    }
    if (ctx->Multiply()) {
      result->op = binop::multiply;
      if (is_precalced_l && is_precalced_r)//如果两边都可以直接算，那么输出literal类型
      {
        auto result_p = new literal_syntax;
        result_p->line = result->line;
        result_p->pos = result->pos;
        if (is_right_int && is_left_int) {
          result_p->is_int = true;
          result_p->intConst = left_int * right_int;
        }

        else {
          result_p->is_int = false;
          result_p->floatConst = (is_left_int ? left_int : left_float) * (is_right_int ? right_int : right_float);
        }
        return static_cast<expr_syntax*>(result_p);
      }
    }
    if (ctx->Divide()) {
      result->op = binop::divide;
      if (is_precalced_l && is_precalced_r)//如果两边都可以直接算，那么输出literal类型
      {
        auto result_p = new literal_syntax;
        result_p->line = result->line;
        result_p->pos = result->pos;
        if (is_right_int && is_left_int) {
          result_p->is_int = true;
          result_p->intConst = left_int / right_int;
        }

        else {
          result_p->is_int = false;
          result_p->floatConst = (is_left_int ? left_int : left_float) / (is_right_int ? right_int : right_float);
        }
        return static_cast<expr_syntax*>(result_p);
      }
    }
    if (ctx->Modulo()) {
      result->op = binop::modulo;
      if (is_precalced_l && is_precalced_r)//如果两边都可以直接算，那么输出literal类型
      {
        auto result_p = new literal_syntax;
        result_p->line = result->line;
        result_p->pos = result->pos;
        if (is_right_int && is_left_int) {
          result_p->is_int = true;
          result_p->intConst = left_int % right_int;
        }
        else {
          throw "the operands must be int!";
        }
        return static_cast<expr_syntax*>(result_p);
      }
    }
    return static_cast<expr_syntax*>(result);
  }
  // Otherwise, if `+` or `-` presented, it'll be a `unaryop_expr_syntax`.
  if (ctx->Plus() || ctx->Minus()) {
    auto result = new unaryop_expr_syntax;
    result->line = ctx->getStart()->getLine();
    result->pos = ctx->getStart()->getCharPositionInLine();
    result->rhs.reset(std::any_cast<expr_syntax*>(visit(expressions[0])));

    auto lval_r = dynamic_cast<lval_syntax*>(result->rhs.get());
    auto literal_r = dynamic_cast<literal_syntax*>(result->rhs.get());
    // auto binop_r = dynamic_cast<binop_expr_syntax *>(result->rhs.get());//unaryop
    // auto unaryop_r = dynamic_cast<unaryop_expr_syntax *>(result->rhs.get());
    if (literal_r != nullptr) {
      if (literal_r->is_int) {
        right_int = literal_r->intConst;
        is_right_int = true;
      }

      else {
        right_float = literal_r->floatConst;
        is_right_int = false;
      }
      is_precalced_r = true;
    }

    if (ctx->Plus()) {
      result->op = unaryop::plus;
      if (is_precalced_r)//如果可以直接算，那么输出literal类型
      {
        auto result_p = new literal_syntax;
        result_p->line = result->line;
        result_p->pos = result->pos;
        if (is_right_int) {
          result_p->is_int = true;
          result_p->intConst = right_int;
        }

        else {
          result_p->is_int = false;
          result_p->floatConst = (is_right_int ? right_int : right_float);
        }
        return static_cast<expr_syntax*>(result_p);
      }
    }
    if (ctx->Minus()) {
      result->op = unaryop::minus;
      if (is_precalced_r)//如果可以直接算，那么输出literal类型
      {
        auto result_p = new literal_syntax;
        result_p->line = result->line;
        result_p->pos = result->pos;
        if (is_right_int) {
          result_p->is_int = true;
          result_p->intConst = -right_int;
        }

        else {
          result_p->is_int = false;
          result_p->floatConst = -(is_right_int ? right_int : right_float);
        }
        return static_cast<expr_syntax*>(result_p);
      }
    }

    return static_cast<expr_syntax*>(result);
  }
  // In the case that `(` exists as a child, this is an expression like `'('
  // expressions[0] ')'`.
  if (ctx->LeftParen() && !ctx->Identifier())//attention ! function also has pare
    return visit(
      expressions[0]); // Any already holds expr_syntax* here, no need for
  // dispatch and re-patch with casting.
// If `number` exists as a child, we can say it's a literal integer
// expression.
  if (auto number = ctx->number())
    return visit(number);
  if (auto lval = ctx->lval()) {//handle variable type

    auto res = new lval_syntax;
    res->line = lval->getStart()->getLine();
    res->pos = lval->getStart()->getCharPositionInLine();
    res->name = lval->Identifier()->getSymbol()->getText();
    if (lval->LeftBracket().size()) {
      for (int i = 0; i < lval->exp().size(); ++i)
      {
          // auto tmp=visit(lval->exp()[i]).as<expr_syntax*>();
          // literal_syntax *tmp1;
          //  if((tmp1=dynamic_cast<literal_syntax*>(tmp))&&!tmp1->is_int)
          //  {
          //     static_cast<literal_syntax*>(tmp)->is_int=true;
          //     static_cast<literal_syntax*>(tmp)->intConst=static_cast<literal_syntax*>(tmp)->floatConst;
          //  }
          //  res->array_index.emplace_back(tmp);
          res->array_index.emplace_back(std::any_cast<expr_syntax*>(visit(lval->exp()[i])));
          
      }
     
    }

    //find the  value
    if (res->array_index.size())//arrary
    {
      //find dim list
      auto dim_list = this->scope.find_arrary_params_cum(res->name);
      if (dim_list != Scope::val_list_not_find && dim_list != nothing_list&&usepropagation&&res->array_index.size()==dim_list.size()) {
        auto result_p = new literal_syntax;
        result_p->line = res->line;
        result_p->pos = res->pos;

        //calc coordinate
        int coordinate = 0;
        int b;
        int i;
        bool available = true;
        for (i = 0; i < dim_list.size();i++) {
          int a = (static_cast<literal_syntax*>(dim_list[i])->intConst);

          auto literal = dynamic_cast<literal_syntax*>(res->array_index[i].get());
          if (!literal) {

            available = false;
            continue;
          }
          coordinate += (literal->intConst) * (static_cast<literal_syntax*>(dim_list[i])->intConst);
        }
        if (available) {
          //find val list
          auto val = this->scope.find_arrary_val(res->name,coordinate);
          auto tmp = dynamic_cast<literal_syntax*>(val);
          if (val != Scope::val_not_find && tmp&&(!control_flow||val->is_global))//没找到这个变量说明是普通全局变量
          {
            //assign the value
            if (tmp->is_int) {
              result_p->intConst = tmp->intConst;
              result_p->is_int = true;

            }
            else {
              result_p->floatConst = tmp->floatConst;
              result_p->is_int = false;

            }
            return static_cast<expr_syntax*>(result_p);
          }
        }

      }

    }
    else//normal variable
    {
      auto val = this->scope.find(res->name);
      auto literal = dynamic_cast<literal_syntax*>(val);
      if (val != Scope::val_not_find && literal&&usepropagation&&(!control_flow||val->is_global)) {
        auto result_p = new literal_syntax;
        result_p->line = res->line;
        result_p->pos = res->pos;
        auto tmp = literal;
        if (tmp->is_int) {
          result_p->intConst = tmp->intConst;
          result_p->is_int = true;
        }
        else {
          result_p->floatConst = tmp->floatConst;
          result_p->is_int = false;

        }
        return static_cast<expr_syntax*>(result_p);
      }
    }
    return static_cast<expr_syntax*>(res);
  }


  if (ctx->LeftParen() && ctx->Identifier())//function call pare is used to disdinguish variable,e... variables are wrapped by lval
  {
    auto res = new func_call_syntax;
    res->line = ctx->Identifier()->getSymbol()->getLine();
    res->pos = ctx->Identifier()->getSymbol()->getCharPositionInLine();
    res->name = ctx->Identifier()->getSymbol()->getText();
    if (ctx->exp().size())//if the  function call has para
    {
      for (int i = 0; i < ctx->exp().size(); ++i) {
        res->array_index.emplace_back(std::any_cast<expr_syntax*>(visit(ctx->exp()[i])));
      }
      //将传入的数组无效化
      for (auto para : res->array_index) {
        auto val = dynamic_cast<lval_syntax*>(para.get());//只判断变量
        if (val && this->scope.find_arrary_params_cum(val->name) != Scope::val_list_not_find)//发现传入的是数组变量,如果找到的是const的数组就完了TODO******
        {
          this->scope.find_arrary_params_cum(val->name) = nothing_list;
        }
      }

    }
    return static_cast<expr_syntax*>(res);
  }
}
antlrcpp::Any syntax_tree_builder::visitNumber(SYSYParser::NumberContext* ctx) {
  auto result = new literal_syntax;
  if (auto intConst = ctx->IntConst()) {
    result->is_int = true;
    result->line = intConst->getSymbol()->getLine();
    result->pos = intConst->getSymbol()->getCharPositionInLine();
    auto text = intConst->getSymbol()->getText();
    
    if (text[0] == '0' && (text[1] == 'x' || text[1] == 'X')) // Hexadecimal
      result->intConst =
      std::stoi(text, nullptr, 16); // std::stoi will eat '0x'
    /* you need to add other situations here */
    else if (text[0] == '0') { //8
      result->intConst = std::stoi(text, nullptr, 8);
    }
    else { // decade
      result->intConst = std::stoi(text);
    }

  //补丁，如果发现类型可以确定，那么将其转化到对应的类型
  // if(literal_type==2)//float
  // {
  //   result->is_int=false;
  //   result->floatConst=result->intConst;
  // }





    return static_cast<expr_syntax*>(result);
  }
  // else FloatConst
  else {

    auto floatConst = ctx->FloatConst();
    result->is_int = false;
    result->line = floatConst->getSymbol()->getLine();
    result->pos = floatConst->getSymbol()->getCharPositionInLine();
    auto text = floatConst->getSymbol()->getText();
    result->floatConst = std::stof(text);//float


  //    if(literal_type==1)//int
  // {
  //   result->is_int=true;
  //   result->intConst=result->floatConst;
  // }

    return static_cast<expr_syntax*>(result);
  }
}

ptr<syntax_tree_node> syntax_tree_builder::operator()(antlr4::tree::ParseTree* ctx) {
  auto result = visit(ctx);
  if (std::any_cast<syntax_tree_node*>(&result))
    return ptr<syntax_tree_node>(std::any_cast<syntax_tree_node*>(result));
  if (std::any_cast<assembly*>(&result))
    return ptr<syntax_tree_node>(std::any_cast<assembly*>(result));
  if (std::any_cast<global_def_syntax*>(&result))
    return ptr<syntax_tree_node>(std::any_cast<global_def_syntax*>(result));
  if (std::any_cast<func_def_syntax*>(&result))
    return ptr<syntax_tree_node>(std::any_cast<func_def_syntax*>(result));
  if (std::any_cast<cond_syntax*>(&result))
    return ptr<syntax_tree_node>(std::any_cast<cond_syntax*>(result));
  if (std::any_cast<expr_syntax*>(&result))
    return ptr<syntax_tree_node>(std::any_cast<expr_syntax*>(result));
  if (std::any_cast<binop_expr_syntax*>(&result))
    return ptr<syntax_tree_node>(std::any_cast<binop_expr_syntax*>(result));
  if (std::any_cast<unaryop_expr_syntax*>(&result))
    return ptr<syntax_tree_node>(std::any_cast<unaryop_expr_syntax*>(result));
  if (std::any_cast<lval_syntax*>(&result))
    return ptr<syntax_tree_node>(std::any_cast<lval_syntax*>(result));
  if (std::any_cast<literal_syntax*>(&result))
    return ptr<syntax_tree_node>(std::any_cast<literal_syntax*>(result));
  if (std::any_cast<stmt_syntax*>(&result))
    return ptr<syntax_tree_node>(std::any_cast<stmt_syntax*>(result));
  if (std::any_cast<var_def_stmt_syntax*>(&result))
    return ptr<syntax_tree_node>(std::any_cast<var_def_stmt_syntax*>(result));
  if (std::any_cast<assign_stmt_syntax*>(&result))
    return ptr<syntax_tree_node>(std::any_cast<assign_stmt_syntax*>(result));
  if (std::any_cast<func_call_stmt_syntax*>(&result))
    return ptr<syntax_tree_node>(std::any_cast<func_call_stmt_syntax*>(result));
  if (std::any_cast<block_syntax*>(&result))
    return ptr<syntax_tree_node>(std::any_cast<block_syntax*>(result));
  if (std::any_cast<if_stmt_syntax*>(&result))
    return ptr<syntax_tree_node>(std::any_cast<if_stmt_syntax*>(result));
  if (std::any_cast<while_stmt_syntax*>(&result))
    return ptr<syntax_tree_node>(std::any_cast<while_stmt_syntax*>(result));
  if (std::any_cast<bcr_stmt_syntax*>(&result))
    return ptr<syntax_tree_node>(std::any_cast<bcr_stmt_syntax*>(result));
  if (std::any_cast<func_param_syntax*>(&result))
    return ptr<syntax_tree_node>(std::any_cast<func_param_syntax*>(result));
  return nullptr;
}
