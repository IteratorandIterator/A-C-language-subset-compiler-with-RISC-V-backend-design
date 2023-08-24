#include"backend_pass.h"
using namespace Archriscv::backend_pass;
using std::set;
void opt_after_reg_allocate::remove_identical_move(Function *func) {
  for (auto &block : func->blocks)
    for (auto it = block->insts.begin(); it != block->insts.end();) {
      auto nxt = std::next(it);
      if (Move *mov = (*it)->as<Move>())
        if (mov->dst == mov->src) block->insts.erase(it);
      it = nxt;
    }
}

void opt_after_reg_allocate::remove_sequential_move(Function* func) {
  for (auto &block : func->blocks) {
    // std::cout << 3 << "\n";
    for (auto it = block->insts.begin(); it != block->insts.end(); ++it) {
      // std::cout << 2 << "\n";
      if (Move *mov = (*it)->as<Move>()) {
        for (auto it_n = std::next(it); it_n != block->insts.end(); ++it_n) {
          // std::cout << 1 << "\n";
          bool is_exit = false;
          auto def = (*it_n)->def_reg();
          auto use = (*it_n)->use_reg();
          for (auto &def_reg : def) {
            if (def_reg.id == mov->dst.id && def_reg.type == mov->dst.type) {
              // it_n = mov->src;
              auto reg_ref = (*it_n)->regs();
              for (auto regs : reg_ref)
                if (*regs == mov->dst) {
                  // std::cout << "remove_sequential_move" <<std::endl;
                  *regs = mov->dst;
                }
            }
            if (def_reg.id == mov->dst.id && def_reg.type == mov->dst.type)
              is_exit = true;
          }
          if (is_exit)
            break;
        }
      }
    }
  }
}

void opt_after_reg_allocate::direct_jump(Function *func) {
  size_t n = func->blocks.size();
  std::map<Block *, size_t> pos;
  for (size_t i = 0; i < n; ++i) pos[func->blocks[i].get()] = i;
  std::vector<size_t> parent;
  parent.resize(n);
  for (size_t i = 0; i < n; ++i) {
    parent[i] = i;
    Block *cur = func->blocks[i].get();
    if (cur->insts.size() == 1) {
      if (Jump *b = (*cur->insts.begin())->as<Jump>())
        parent[i] = pos[b->target];
    } else if (cur->insts.size() == 0) {
      assert(i + 1 < n);
      parent[i] = i + 1;
    }
  }
  std::function<size_t(size_t)> get_root = [&](size_t x) {
    return parent[x] == x ? x : parent[x] = get_root(parent[x]);
  };
  for (size_t i = 0; i < n; ++i) {
    auto &block = func->blocks[i];
    for (auto &inst : block->insts)
      if (Branch *b = inst->as<Branch>()) {
        size_t final_target = get_root(pos[b->target]);
        b->target = func->blocks[final_target].get();
        b->target->label_used = true;
      } else if (Jump *j = inst->as<Jump>()) {
        size_t final_target = get_root(pos[j->target]);
        j->target = func->blocks[final_target].get();
        j->target->label_used = true;
      }
  }
}
void opt_after_reg_allocate::remove_unused_after_alloc(Function  *func) {
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
void opt_after_reg_allocate::run(){
    for(auto & func : program_->functions){
        remove_identical_move(func.get());
        remove_sequential_move(func.get());
        direct_jump(func.get());
        remove_unused_after_alloc(func.get());
    }
}
