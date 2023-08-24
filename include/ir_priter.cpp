#include "ir_priter.h"

using namespace Medium;

void ir_serializer::visit(Reg &node) {
    out<<"R"<<(node.is_int?"i":"f")<<"["<<node.id<<"]";
}
void ir_serializer::visit(Variable &node) {
    out<<node.name<<"("<<(node.global ? "gp" : "sp" )<<"+"<<node.offset<<","<<(node.size==INT_SIZE ? "var" : "addr" )<<","<<(node.is_int ? "int" : "float" )<<")";//变量的位置和类型
}
void ir_serializer::visit(MediumScope &node){
    out << "MemScope(" << node.name << "){\n";
    for (auto &x : node.objects) 
    {
      out << "\t";
      this->visit(*(x.get()));
      out<<"\n";
    }
    out  << "}\n";
}
void ir_serializer::visit(IrBlock &node){
      out << "BB(" << node.name << "){\n";
      for (auto &x : node.instrs) {
        out << "\t";
        x->accept(*this);
        out << "\n";
      }
      out << "}\n";
    }
void ir_serializer::visit(IrFunction &node){

}
void ir_serializer::visit(UserFunction &node){
      out << "userFunc: " << node.name << "\n";
      out << "entry: " << node.entry->name << "\n";
      for (auto &bb : node.bbs)
      {
        bb->accept(*this);
      }
      out<< "\n";
    }
void ir_serializer::visit(LibFunction &node){}
void ir_serializer::visit(CompileUnit &node){
      node.scope.accept(*this);//全局变量
      // out<<"libfuns:"<<"\n";
      for(auto &f:node.lib_funcs)//函数
      {
        f.second->accept(*this);
      }
      // out<<"userfuns:"<<"\n";
      for (auto &f : node.funcs)
      {
        f.second->accept(*this);
      }
      out<<"\n";
    }
void ir_serializer::visit(UnaryOp &node){
    out<<node.get_name();
}
void ir_serializer::visit(BinaryOp &node){
      out<<node.get_name();
    }
void ir_serializer::visit(ArrayIndex &node){
      node.d1.accept(*this);
      out<<"=";
      node.s1.accept(*this);//数组基地址
      out<<"+";
      node.s2.accept(*this);//偏移地址，就是元素坐标乘以元素的大小
      out<<"*"<<node.size;
    }
void ir_serializer::visit(LoadAddr &node) {
      node.d1.accept(*this);
      out<<"=&";
      node.offset->accept(*this);
     }
void ir_serializer::visit(LoadConst &node) {
      node.d1.accept(*this);
      out<<"="<<node.value;
    }
void ir_serializer::visit(LoadArg &node) {
       node.d1.accept(*this);
       out<<"= arg("<<node.id<<")";
    }
void ir_serializer::visit(UnaryOpInstr &node) {
      node.d1.accept(*this);
      out<<"=";
      out<<" ";
      node.op.accept(*this);
      node.s1.accept(*this);
    }
void ir_serializer::visit(BinaryOpInstr &node) {
       node.d1.accept(*this);
      out<<"=";
      out<<" ";
      node.s1.accept(*this);
       out<<" ";
      node.op.accept(*this);
       out<<" ";
       node.s2.accept(*this);
      
    }
void ir_serializer::visit(LoadInstr &node) {
      node.d1.accept(*this);
      out<<"=";
      out<<" M[";
      node.addr.accept(*this);
      out<<"]";
    }
void ir_serializer::visit(StoreInstr &node) {
     
      out<<" M[";
      node.addr.accept(*this);
      out<<"]";
      out<<"=";
      node.s1.accept(*this);
      
    }
void ir_serializer::visit(JumpInstr &node) {
      out<<"goto"<<node.target->name;
    }
void ir_serializer::visit(BranchInstr &node) {
      node.cond.accept(*this);
      out<<"?"<<" goto "<<node.target1->name<<":"<<" goto "<<node.target0->name;
    }
void ir_serializer::visit(ReturnInstr &node) {
      out<<"return";
      node.s1.accept(*this);
    }
void ir_serializer::visit(CallInstr &node) {
      node.d1.accept(*this);
      out<<"="<<node.f->name<<"(";
      for (auto &[s,flag] : node.args) {
      s.accept(*this);
      out<<",";
      }
      out<<")";
    }
void ir_serializer::visit(LocalVarDef &node) {
      out<<"define"<<node.data->name;
    }
void ir_serializer::visit(PhiInstr &node) {
      node.d1.accept(*this);
      out<< " = phi";
      char c = '(';
      for (auto s : node.uses) {
        out << c << " " ;
         s.first.accept(*this);
         out<< ":"<<s.second->name;
        c = ',';
      }
      if (c == '(')out << c;
      out << " )";
    }

void ir_serializer::visit(Convert &node) {
      if (node.src.is_int == true && node.d1.is_int == false)
      {
        out<<"convert ";
        out<<" int32 ";
        node.src.accept(*this);
        out<<" to ";
        out<<" float32 ";
        node.d1.accept(*this);
      }
        
      else if (node.src.is_int ==false  && node.d1.is_int == true)
      {
         out<<"convert ";
        out<<" float32 ";
        node.src.accept(*this);
        out<<" to ";
        out<<" int32 ";
        node.d1.accept(*this);
      }
      else
       assert(false);
    }

