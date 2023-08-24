#include "pass.h"
#define Case(T, a, b) if (auto a = dynamic_cast<T *>(b))
#define CaseNot(T, b) 
void del_edge(IrBlock *src, IrBlock *dst) {

  for (auto& instrs : dst->instrs) {
    if(PhiInstr* phi = dynamic_cast<PhiInstr*>(instrs.get())) {
      for (auto begin1 = phi->uses.begin(); begin1 != phi->uses.end();) {
        if (begin1->second == src)
          begin1 = phi->uses.erase(begin1);
        else 
          begin1++;
      }
    }
  }
}

bool dce_BB(UserFunction *f) {
  std::queue<IrBlock *> q;  //定义一个队列
  struct State { //定义状态
    bool visited = 0, one_goto_entry = 0;
    IrBlock *pre = NULL, *nxt = NULL;
  };
  auto defs = map_defreg_inst(f);//返回所有def的寄存器
  bool changed = 0;
  std::unordered_map<IrBlock *, State> S;
  auto visit = [&](IrBlock *&x, IrBlock *pre, bool jump) {
    State &s = S[x];
    if (!s.visited) {
      s.visited = 1;
      q.push(x);
      s.pre = pre;
      S[pre].nxt = x;
      s.one_goto_entry = jump;
    } else {
      s.one_goto_entry = 0;
    }
  };
  visit(f->entry, NULL, 0);
  while (!q.empty()) {
    IrBlock *ptr_block = q.front();
    q.pop();
    Instr *x = ptr_block->back();
    if(auto  y = dynamic_cast<JumpInstr*>( x)) { 
      visit(y->target, ptr_block, 1); 
    }
    else if(auto  y = dynamic_cast<BranchInstr*>( x)) {
      auto elim_branch = [&](IrBlock *z) {
        IrBlock *d = (z == y->target1 ? y->target0 : y->target1);
        del_edge(ptr_block, d);
        ptr_block->pop();
        ptr_block->push(new JumpInstr(z));
        changed = 1;
        visit(z, ptr_block, 1);
      };
      if (y->target1 == y->target0) {
        assert(0);
      } else {
        auto cond = defs.at(y->cond);
        if(auto  lc = dynamic_cast<LoadConst*>( cond)) {
          bool flag = false;
          if (lc->is_float) {
            flag = lc->value_f==0?false:true;
          }else {
            flag = lc->value==0?false:true;
          }
          IrBlock *z = (flag ? y->target1 : y->target0);
          elim_branch(z);
        }
        else {
          visit(y->target1, ptr_block, 0);
          visit(y->target0, ptr_block, 0);
        }
      }
    }
    else if(auto  y = dynamic_cast<ReturnInstr*>( x)) {
      ;
    }
    else {
      Reg r = f->new_Reg(true);
      ptr_block->push(new LoadConst(r, 0));
      ptr_block->push(new ReturnInstr(r, true));
    }
  }

  auto nex = [&](IrBlock *bb) -> bool { return !S[bb].visited; };

  for (auto& blocks : f->bbs) {
    auto ptr_block = blocks.get();
    if (nex(ptr_block)) {
      for (IrBlock *u : get_BB_out(ptr_block)) del_edge(ptr_block, u);
    }
  }

  for (auto& blocks : f->bbs) {
    auto ptr_block = blocks.get();
    State &s = S[ptr_block];
    while (s.visited && !s.one_goto_entry && s.nxt && S[s.nxt].one_goto_entry) {
      for (auto it = s.nxt->instrs.begin(); it != s.nxt->instrs.end(); ++it) {
        if(auto  i0 = dynamic_cast<PhiInstr*>( it->get())) {
          // std::cout<<i0->uses.size()<<std::endl;
          assert(i0->uses.size() == 1);
          assert(i0->uses.at(0).second == ptr_block);
          *it = std::unique_ptr<Instr>(
              new UnaryOpInstr(i0->d1, i0->uses.at(0).first, UnaryOp::ID));
        }
      }
      ptr_block->pop();
      ptr_block->instrs.splice(ptr_block->instrs.end(), s.nxt->instrs);
      phi_src_rewrite(ptr_block, s.nxt);
      S[s.nxt].visited = 0;
      s.nxt = S[s.nxt].nxt;
      changed = 1;
    }
  }

  // remove unused BBs
  f->bbs.erase(remove_if(f->bbs.begin(), f->bbs.end(),
                         [&](auto &bb) { return nex(bb.get()); }),
               f->bbs.end());
  return changed;
}

void remove_unused_memobj(CompileUnit &c) {
  std::unordered_set<Variable *> used;//保存所有有使用的局部变量

  for (auto& funcs : c.funcs) {
    for (auto& blocks : funcs.second->bbs) {
      for (auto& instrs : blocks->instrs) {
        if(auto  la = dynamic_cast<LoadAddr*>(instrs.get())) { 
          used.insert(la->offset); 
        }
      }
      for (auto &kv : funcs.second->scope.array_args) {
        used.insert(kv.second);
        // array args are never removed
      }
    }
  }


  for (auto& funcs : c.funcs) {
    for (auto& blocks : funcs.second->bbs) {
      blocks->instrs.remove_if([&](auto &nextinstr) {//移出BB中未使用的局部变量（非数组）的定义指令
        if(auto instr = dynamic_cast<LocalVarDef*>( nextinstr.get())) {
          if (!used.count(instr->data)) 
            return 1;
        }
        return 0;
      });
    }
  }

  for (auto& funcs : c.funcs) {
    auto &ms = funcs.second->scope.objects;
    ms.erase(std::remove_if(ms.begin(), ms.end(), [&](std::unique_ptr<Variable> &m) {
    return !used.count(m.get()) && !m->arg;
    }), ms.end());
  }

  auto &ms = c.scope.objects;
  ms.erase(std::remove_if(ms.begin(), ms.end(), [&](std::unique_ptr<Variable> &m) {
    return !used.count(m.get()) && !m->arg;
  }), ms.end());
}

bool remove_unused_loop(UserFunction *f) {
  bool is_main = (f->name == "main");

  auto uses = build_use_count(f);
  auto S = build_info_node(f);
  for (auto &blocks : f->bbs) {
    IrBlock *ptr_block = blocks.get();
    auto &sw = S.at(ptr_block);
    if (!sw.loop.loop_rt || sw.loop.loop_exit.size() != 1) 
      continue;
    IrBlock *exit = *sw.loop.loop_exit.begin();
    std::unordered_map<Reg, RegWriteInstr *> loop_defs;
    std::unordered_map<Reg, int> loop_uses;
    bool unused = 1;
    auto ptr_block2 = ptr_block;
    assert(S[ptr_block2].loop.loop_rt);
    std::queue<IrBlock *> q;
    q.push(ptr_block2);
    while (!q.empty()) {
      ptr_block2 = q.front();
      q.pop();
      [&](IrBlock *u) {
      
      get_defs(loop_defs, u);
      get_use_count(loop_uses, u);
      u->for_each([&](Instr *i) {
        if(auto  i0 = dynamic_cast<MemEffect*>( i)) {
          if (i0->data->global && i0->data->arg || !is_main) unused = 0;
        }
        else if(auto  i0 = dynamic_cast<MemWrite*>( i)) {
          if (!is_main) unused = 0;
        }
      });
    }(ptr_block2);
      for (IrBlock *ch : S[ptr_block2].loop.loop_ch) q.push(ch);
    }

    if (unused) {

      for (auto& instrs : exit->instrs) {
        if(auto  i = dynamic_cast<PhiInstr*>( instrs.get())) {
          i->map_use([&](Reg &r) {
            if (loop_defs.count(r)) unused = 0;
          });
        }
      }

      for (auto &kv : loop_defs) {
        Reg r = kv.first;
        if (loop_uses[r] != uses[r]) {
          unused = 0;
          break;
        }
      }

      IrBlock *exit = *sw.loop.loop_exit.begin();
      ptr_block->instrs.clear();
      ptr_block->push(new JumpInstr(exit));
      std::unordered_set<IrBlock *> del;
      auto ptr_block3 = ptr_block;
      assert(S[ptr_block3].loop.loop_rt);
      std::queue<IrBlock *> q;
      q.push(ptr_block3);
      while (!q.empty()) {
        ptr_block3 = q.front();
        q.pop();
        [&](IrBlock *u) {
          if (u != ptr_block3) del.insert(u);
        }(ptr_block3);
        for (IrBlock *ch : S[ptr_block3].loop.loop_ch) q.push(ch);
      }
      auto nex = [&](auto &bb) { return del.count(bb.get()); };
      f->bbs.erase(remove_if(f->bbs.begin(), f->bbs.end(), nex), f->bbs.end());
      repeat(dce_BB)(f);
      remove_unused_def(f);
      return 1;
    }
  }
  return 0;
}

void remove_unused_def(UserFunction *f) {
  std::unordered_set<Instr *> used;
  auto defs = map_defreg_inst(f);
  std::function<void(Instr * i)> dfs;
  dfs = [&](Instr *i) {
    if (used.count(i)) 
      return;
    else 
      used.insert(i);
    i->map_use([&](Reg &v) {
      if (v.id) dfs(defs.at(v));
    });
  };

  for (auto& blocks : f->bbs) {
    for (auto& instrs : blocks->instrs) {
      bool t = 0;
      auto ptr_instr = instrs.get();
    if(auto  i = dynamic_cast<LocalVarDef*>(ptr_instr)) 
      t = 1;

    if(auto  i = dynamic_cast<CallInstr*>(ptr_instr)) 
      t = !(i->pure);
    if(auto  i = dynamic_cast<ControlInstr*>(ptr_instr)) 
      t = 1;
    if(auto  i = dynamic_cast<StoreInstr*>(ptr_instr)) 
      t = 1;
    if(auto  i = dynamic_cast<MemDef*>(ptr_instr)) 
      t = 1;
    if(auto  i = dynamic_cast<MemUse*>(ptr_instr)) 
      t = 1;
    if(auto  i = dynamic_cast<MemEffect*>(ptr_instr)) 
      t = 1;
    if(auto  i = dynamic_cast<MemWrite*>(ptr_instr)) {
      if (i->data->arg)
        t = 1;
      else if (i->data->global) {
        if (f->name != "main") 
          t = 1;
      }
    }
    if (t) dfs(ptr_instr);
    }
  }
  bool flag = 0;
  for (auto& blocks : f->bbs) {
    blocks->instrs.remove_if([&](std::unique_ptr<Instr> &i) {
      if (!used.count(i.get())) {
        return 1;
      }
      return 0;});
  }
}

std::vector<UserFunction *> get_call_order(CompileUnit &c) {
  // std::unordered_set<UserFunction *> visited;
  // std::vector<UserFunction *> vec;
  // std::function<void(UserFunction *)> dfs;
  // dfs = [&](UserFunction *f) {
  //   if (visited.count(f)) return;
  //   visited.insert(f);
  //   f->for_each([&](Instr *x) {
  //     if(auto  y = dynamic_cast<CallInstr*>( x)) {
  //       if(auto  g = dynamic_cast<UserFunction*>( y->f)) {
  //         // f call g
  //         if (g != f) dfs(g);
  //       }
  //     }
  //   });
  //   vec.push_back(f);
  // };
  // dfs(c.funcs.at("main").get());
  // return vec;

  std::vector<UserFunction *>call_order;//深度优先顺序，按照完成顺序放置
  std::unordered_set<UserFunction *> visited;
  std::function<void(UserFunction *)> getcallorder;
  getcallorder=[&](UserFunction *f)
  {
    if(visited.find(f)!=visited.end())return ;
    visited.insert(f);
    for(auto &bb : f->bbs)
    {
      for(auto &inst: bb->instrs)
      {
        Case(CallInstr,i0,inst.get())
        {
           Case(UserFunction,fun,i0->f)
           {
            if (fun != f) getcallorder(fun);

           }

        }
      }
      
    }
   call_order.push_back(f);
  };
  getcallorder(c.funcs.at("main").get());
  return call_order;


}

void remove_unused_func(CompileUnit &c) {
  auto order = get_call_order(c);
  std::unordered_set<UserFunction *> visited(order.begin(), order.end());

  std::vector<std::string> del;
  for (auto& funcs : c.funcs) {
    if (!visited.count(funcs.second.get())) 
      del.push_back(funcs.second->name);
  }
  for (auto &x : del) {
    c.funcs.erase(x);
  }
}
