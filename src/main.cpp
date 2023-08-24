#include <iostream>
#include <fstream>

#include <recognizer.h>

#include <sys/resource.h>

#include <cstdlib>
#include <iostream>

#include "inst.h"
#include "code_gen.h"

#include "ir_builder.h"
#include "program_builder.h"
#include "ir.h"
#include "pass.h"
// #include "ast_priter.h"
// #include "ir_priter.h"
#include "backend_pass.h"
// #include "prettywriter.h"
// #include "stringbuffer.h"

int main(int argc, char **argv)
{
  //1.read file
  std::ifstream file;
  file.open(argv[4],std::ios::in);
  ast::recognizer rcg(file);
  if (!rcg.execute())
      return 1;

  std::shared_ptr<ast::syntax_tree::syntax_tree_node> ast = rcg.get_syntax_tree();
  file.close();
  //3.print AST，用json打印出来的东西具有层次感，适合前端

  // rapidjson::StringBuffer s;
  // rapidjson::PrettyWriter<decltype(s)> writer(s); 
  // sysy_recognizer::syntax_tree::syntax_tree_serializer<decltype(writer)> serializer(writer);
  // serializer.serialize(*ast);
  // std::cout<<"*************AST************"<<std::endl;
  // std::cout << s.GetString() << std::endl;
  //4.convert AST to IR  ,generate IR from syntax tree directly may be better?
  
  ASTVisitor irbuilder;
  irbuilder.ir = new Medium::CompileUnit;
  ast->accept(irbuilder);//start generating
  //5.print IR，中端主要看BB之间的连接关系和BB中的线性IR的内容
  // std::cout<<"*************IR************"<<std::endl;
  // 6.start optimization
  
  // std::ofstream debug;
  // debug.open("../debug.txt",std::ios::out);
  // ir_serializer ir_printer(debug);
  //传入输出流用来构造输出ir_priter
  //利用priter来打印输入的ir的内容
  
  (*irbuilder.ir).for_each(dce_BB);
  // ir_printer.visit(*(irbuilder.ir));
  auto passconfig = new Pass::PassManager(irbuilder.ir);
  passconfig->add_pass("mem2reg");
  passconfig->add_pass("gvn");

  // // if(argc==6)
  // // {
    passconfig->add_pass("function_inline");
    passconfig->add_pass("mem2reg");
    passconfig->add_pass("gvn");

    passconfig->add_pass("gcm_schedule_late");
    passconfig->add_pass("move_condtion_out_of_loop");
    passconfig->add_pass("while_true_round");
    //循环优化
    passconfig->add_pass("loop_unroll");
    passconfig->add_pass("gvn");

    (*irbuilder.ir).for_each(dce_BB);
     passconfig->add_pass("gvn");
    passconfig->add_pass("array2var");
    passconfig->add_pass("recursive_cut_branch");
    passconfig->add_pass("gvn");
  // }

  passconfig->run_pass();
  // ir_printer.visit(*(irbuilder.ir));
  // std::cout<<"----------optimize-----------"<<std::endl;
  // ir_printer.visit(*(irbuilder.ir));
  //7.backend
  //start backend
  // std::cout<<"*************CodeGen************"<<std::endl;
  program_visitor program_builder;
  
  irbuilder.ir->accept(program_builder);

  auto prog = program_builder.prog;


  //backend pass
  auto backend_passconfig = new Archriscv::backend_pass::backend_pass_manager(prog);
  backend_passconfig->add_backend_pass("inline_constant");
  backend_passconfig->add_backend_pass("merge_addi_lw_sw");
  backend_passconfig->add_backend_pass("remove_unused_before_alloc");
  backend_passconfig->run_pass();


  prog->reg_allocate();
  prog->build_code_before_func();

  backend_passconfig->run_opt_after_reg_allocate();

  prog->build_code_funcs();

  std::ofstream file_1;
  file_1.open(argv[3],std::ios::out);
  prog->get_asm(file_1);
  file_1<<std::endl;
  file_1.close();
  return 0;
}