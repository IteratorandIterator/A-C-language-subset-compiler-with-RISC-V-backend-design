#include "backend_pass.h"
using namespace Archriscv::backend_pass;
using std::make_unique;
using std::map;
using std::set;
using std::unique_ptr;
using std::vector;



void backend_pass_manager::add_backend_pass(std::string pass_name){
    if (pass_name == "inline_constant") {
      my_pass.emplace_back(std::move(new inline_constant(program_)));
      my_pass.back()->set_name("inline_constant");
      return;
    } else if (pass_name == "merge_addi_lw_sw"){
        my_pass.emplace_back(std::move(new merge_addi_lw_sw(program_)));
        my_pass.back()->set_name("merge_addi_lw_sw");
        return;
    } else if (pass_name== "remove_unused_before_alloc")
    {
        my_pass.emplace_back(std::move(new remove_unused_before_alloc(program_)));
        my_pass.back()->set_name("remove_unused_before_alloc");
        return;
    }
    else
    {
        assert(0);
    }
    
};
void backend_pass_manager::run_pass(){
    for(auto & a_pass : this->my_pass)
        a_pass->run();
};

void backend_pass_manager::run_opt_after_reg_allocate(){
    auto last_pass = new opt_after_reg_allocate(program_);
    last_pass->run();
}
