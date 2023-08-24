#include "pass.h"

#include <cstdlib>
#define Case(T, a, b) if (auto a = dynamic_cast<T *>(b))
using namespace Medium;
#define CaseNot(T, b)                  \
  if (auto _ = dynamic_cast<T *>(b)) { \
  } else
//不能移动的指令
bool pinned(RegWriteInstr *i) {
  Case(LocalVarDef, i0, i) return 1;
  Case(PhiInstr, i0, i) return 1;
  Case(CallInstr, i0, i) return 1;
  Case(LoadInstr, i0, i) return 1;

  // Case(MemDef, i0, i) return 1;
  // Case(MemUse, i0, i) return 1;
  // Case(MemEffect, i0, i) return 1;
  // Case(MemRead, i0, i) {
  //   // return 0; //warning: may cause error!
  //   return !i0->data->is_normal_var();
  // }
  // Case(MemWrite, i0, i) return 1;
  return 0;
};


void Pass::gcm_schedule_late::schedule_late(UserFunction *f)
{



  auto S = build_info_node(f);

  std::unordered_map<Reg, IrBlock *> uses_lca;

  auto update_use_lca = [&](Reg r,IrBlock *new_use) {
    IrBlock *&old_dom = uses_lca[r];
    if (!old_dom)
      old_dom = new_use;
    else
      while (!S[old_dom].is_dom(S[new_use])) 
          old_dom = S[old_dom].dom.dom_fa;//更新公共祖先，利用支配的传递性质
  };


  dom_tree_dfs_reverse(S, [&](IrBlock *w) {
    //更新所有phi指令的use的寄存器的lca
    for (IrBlock *u : S[w].cfg.out) {
      // u->for_each([&](Instr *i) {
      //   Case(PhiInstr, i0, i) {
      //     for (auto &kv : i0->uses) {
      //       if (kv.second == w) update_use_lca(kv.first, w);
      //     }
      //   }
      // });
      auto fun=[&](Instr *i) {
        Case(PhiInstr, i0, i) {
          for (auto &kv : i0->uses) {
            if (kv.second == w) update_use_lca(kv.first, w);
          }
        }
      };
      for (auto &x : u->instrs) fun(x.get());
    }


    for (auto it = w->instrs.end(); it != w->instrs.begin();) {
      --it;
      Instr *inst = it->get();
      Case(RegWriteInstr, i0, inst) {
        if (!pinned(i0)) {
          // CaseNot(MemRead, i0) {
            IrBlock *lca ;//支配这个def的所有use的BB块的位置
            if (lca= uses_lca[i0->d1]) {
              if (!S[w].is_dom(S[lca])) {//确保当前块支配这个BB
                assert(0);
              }
              IrBlock *loop_cur = S[w].get_loop_rt();//当前块所在的循环
              // if (!loop_cur || is_in_loop(S, lca, loop_cur))//让lca位于当前循环中
              // {
              //   while (S[lca].get_loop_rt() != loop_cur) {
              //     lca = S[lca].dom.dom_fa;
              //   }
              // }
              //让lca位于当前循环中
              if(!loop_cur)
              {
                 while (S[lca].get_loop_rt()) {
                  lca = S[lca].dom.dom_fa;
                }
              }
              else if(!is_in_loop(S, lca, loop_cur))
              {
                 while (S[lca].get_loop_rt() != loop_cur) {
                  lca = S[lca].dom.dom_fa;
                }
              }
              //需要移动指令
              if (lca != w) {
                auto del = it;
                it++;
                lca->instrs.emplace_front(std::move(*del));
                inst->map_use([&](Reg &r) { update_use_lca(r, lca); });
                w->instrs.erase(del);
                continue;
              }
            }
          // }
        }
      }
      CaseNot(PhiInstr, inst) {
        inst->map_use([&](Reg &r) { update_use_lca(r, w); });
      }
    }


  });

}

void Pass::gcm_schedule_late::run()
{
   for(auto & [_,f]: c->funcs) schedule_late(f.get());
}



bool Pass::move_condtion_out_of_loop::condtion_out_of_loop(UserFunction *f)
{
  auto S = build_info_node(f);
  auto i2bb = build_in2bb(f);
  auto defs = build_defs(f);

 return f->for_each_until([&](IrBlock *bb) {
    //找到循环中的分支语句
    Case(BranchInstr, i0, bb->back()) {
      // IrBlock *u = S[bb].get_loop_rt();
      // if (!u) return 0;
      if(!S[bb].get_loop_rt()) return 0;//不在循环中
      IrBlock *cond_def_BB = i2bb.at(defs.at(i0->cond));
      // ir_serializer ir_printer(std::cout);
      // cond_def_BB->accept(ir_printer);
      // S[bb].get_loop_rt()->accept(ir_printer);
      if (!S[cond_def_BB].is_strict_dom(S[S[bb].get_loop_rt()])) return 0;//分支语句在循环中不确定
      IrBlock *root = S[bb].get_loop_rt();
      //尽可能往外提
      while(S[root].loop.loop_fa) {
       IrBlock *curroot = S[root].loop.loop_fa;
        // if (!curroot) break;
        if (!S[cond_def_BB].is_dom(S[curroot])) return 0;
        root = curroot;
      }
  
      
      if (!S[root].loop.loop_simple) return 0;
      IrBlock *merge_exit = f->new_BB("merge_exit");
      IrBlock *exit = *S[root].loop.loop_exit.begin();
      Reg cond = i0->cond;
      IrBlock *pre_entry = f->new_BB("pre_entry");
       merge_exit->push(new JumpInstr(exit));

      S[root].loop.loop_pre->back()->map_BB([&](IrBlock *&bb) {
        if (bb == root) bb = pre_entry;
      });

      std::unordered_set<Reg> new_reg;
      std::unordered_map<IrBlock *, IrBlock *> mp_bb;
      
      loop_tree_for_each_bfs(S, root, [&](IrBlock *bb) {
       
        // bb->for_each([&](Instr *i) {
        //   Case(RegWriteInstr, i0, i) { new_reg.insert(i0->d1); }
        // });
        auto collect_def=[&](Instr *i) {
          Case(RegWriteInstr, i0, i) { new_reg.insert(i0->d1); }
        };
        mp_bb[bb] = f->new_BB(bb->name + "_m");
        for (auto &x : bb->instrs) collect_def(x.get());
      });
      for (auto &kv : mp_bb) {
        IrBlock *new_mp_bb = kv.second,*old_mp_bb = kv.first;
        if (old_mp_bb == exit) continue;
        // old_mp_bb->for_each([&](Instr *i) {
        //   auto j = i->copy();
        //   new_mp_bb->push(j);
        // });
        for(auto &i : old_mp_bb->instrs)
        {
          new_mp_bb->push(i->copy());
        }
        new_mp_bb->map_BB(partial_map(exit, merge_exit));
        old_mp_bb->map_BB(partial_map(exit, merge_exit));
        new_mp_bb->map_BB(partial_map(mp_bb));

        new_mp_bb->back()->map_use([&](Reg &r) {
          if (r == cond) {
            r = f->new_Reg(true);
            old_mp_bb->push1(new LoadConst(r, static_cast<int>(0)));
          }
        });
        old_mp_bb->back()->map_use([&](Reg &r) {
          if (r == cond) {
            r = f->new_Reg(true);
            old_mp_bb->push1(new LoadConst(r, static_cast<int>(1)));
          }
        });
      
      }
      auto new_root=mp_bb.at(root);
      pre_entry->push(new BranchInstr(cond, root, new_root));
      ssa_construction(f, [&](Reg r) { return new_reg.count(r); });
      repeat(dce_BB)(f);
      return 1;
    }
    return 0;
  });
}

void Pass::move_condtion_out_of_loop::run()
{
    for(auto & [_,f]: c->funcs) condtion_out_of_loop(f.get());
}
