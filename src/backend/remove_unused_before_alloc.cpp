#include "backend_pass.h"
#include "ir.h"
using std::set;
void Archriscv::backend_pass::remove_unused_before_alloc::run()
{

    for (auto &f : program_->functions)
        this->remove_unused(f.get());  

}

void Archriscv::backend_pass::remove_unused_before_alloc::remove_unused(Function *func)
{
        func->calc_live();
  for (auto &block : func->blocks) {
    set<Reg> live = block->live_out;
    for (auto it = block->insts.rbegin(); it != block->insts.rend();) {
      bool used = (*it)->side_effect();
      for (Reg r : (*it)->def_reg())
        if ((r.is_machine() && !integer_allocable(r.id)) || live.find(r) != live.end())
          used = true;
      if (!used) {
        auto base = it.base();
        block->insts.erase(std::prev(base));
        it = std::make_reverse_iterator(base);
      } else {
        (*it)->update_live(live);
        ++it;
      }
    }
  }
}
