#include "pass.h"

#include <cstdlib>
using namespace Medium;
#define CaseNot(T, b)                  \
  if (auto _ = dynamic_cast<T *>(b)) { \
  } else

std::unordered_map<Reg, RegWriteInstr *> build_defs(UserFunction *f) {
  std::unordered_map<Reg, RegWriteInstr *> def;
  f->for_each([&](IrBlock *w) { get_defs(def, w); });
  return def;
}
std::vector<IrBlock *> get_BB_out(IrBlock *w) {
  Instr *i = w->back();
  if(auto  i0 = dynamic_cast<JumpInstr*>( i)) return {i0->target};
  if(auto  i0 = dynamic_cast<BranchInstr*>( i)) return {i0->target1, i0->target0};
  return {};
}

void phi_src_rewrite(IrBlock *bb_cur, IrBlock *bb_old) {
  for (IrBlock *u : get_BB_out(bb_cur)) {
    for (auto& instr : u->instrs) {
      if(auto  phi = dynamic_cast<PhiInstr*>(instr.get())) {
        for (auto &kv : phi->uses) {
          if (kv.second == bb_old) kv.second = bb_cur;
        }
      }
    }
  }
}


void get_defs(std::unordered_map<Reg, RegWriteInstr *> &def, IrBlock *w) {
  for (auto& instrs : w->instrs) {
    if(auto i = dynamic_cast<RegWriteInstr*>(instrs.get())) {
      if (def.count(i->d1)) {//不会出现这个情况，由SSA保证
        assert(0);
      }
      def[i->d1] = i;
    }
  }

}
void get_use_count(std::unordered_map<Reg, int> &uses, IrBlock *w) {
  for (auto& instrs : w->instrs) {
    instrs->map_use([&](Reg &r) { ++uses[r]; });
  }
}

std::unordered_map<Reg, RegWriteInstr *> map_defreg_inst(UserFunction *f) {
  std::unordered_map<Reg, RegWriteInstr *> def;
  for (auto& blocks : f->bbs) {
    get_defs(def, blocks.get());
  }
  return def;
}

std::unordered_map<Reg, int> build_const_int(UserFunction *f) {
  std::unordered_map<Reg, int> def;
  for (auto& blocks : f->bbs) {
    for (auto& instrs : blocks->instrs) {
      if(auto  i = dynamic_cast<LoadConst*>(instrs.get())) { 
        if(i->d1.is_int)
          def[i->d1] = i->value;
      }
    }
  }
  return def;
}

std::unordered_map<Reg, float> build_const_float(UserFunction *f) {
  std::unordered_map<Reg, float> def;
  for (auto& blocks : f->bbs) {
    for (auto& instrs : blocks->instrs) {
      if(auto  i = dynamic_cast<LoadConst*>(instrs.get())) { 
        if(!i->d1.is_int)
          def[i->d1] = i->value_f;
      }
    }
  }
  return def;
}

std::unordered_map<Instr *, IrBlock *> build_in2bb(UserFunction *f) {
  std::unordered_map<Instr *, IrBlock *> def;
  for (auto& blocks : f->bbs) {
    for (auto& instrs : blocks->instrs) {
      def[instrs.get()] = blocks.get();
    }
  }
  return def;
}

std::unordered_map<Reg, int> build_use_count(UserFunction *f) {
  std::unordered_map<Reg, int> uses;
  for (auto& blocks : f->bbs) {
    get_use_count(uses, blocks.get());
  }
  return uses;
}

void array_index_to_muladd(UserFunction *f) {
  for (auto& blocks : f->bbs) {
    for (auto it = blocks->instrs.begin(); it != blocks->instrs.end(); ++it) {
      if(auto  x = dynamic_cast<ArrayIndex*>( it->get())) {
        Reg offset = f->new_Reg(true), size = f->new_Reg(true);
        f->entry->push_front(new LoadConst(size, x->size));
        blocks->instrs.insert(it, std::unique_ptr<Instr>(new BinaryOpInstr(
                                  offset, x->s2, size, BinaryOp::MUL)));
        *it = std::unique_ptr<Instr>(
            new BinaryOpInstr(x->d1, x->s1, offset, BinaryOp::ADD));
      }
    }
  }

}


std::unordered_map<Reg, SimpleTypeInfo> simple_type_check(UserFunction *f) {
  auto S = build_info_node(f);
  std::unordered_map<Reg, SimpleTypeInfo> T;
  //按照支配树的顺序遍历BB中的指令是为了
  dom_tree_dfs(S, [&](IrBlock *bb) {
    // bb->for_each([&](Instr *_x) {
    for (auto& instrs : bb->instrs) {
      auto _x = instrs.get();
      //获取reg write指令的def寄存器的类型
      if(auto  x = dynamic_cast<RegWriteInstr*>( _x)) 
      {
        assert(!T.count(x->d1));//检查ssa
        auto &t = T[x->d1];
        if(auto  x0 = dynamic_cast<LoadArg*>( x)) {
          if (f->scope.array_args.count(x0->id)) {
            t.base = f->scope.array_args.at(x0->id);//将数组参数和对应的变量对应起来获取变量的类型
          }
        }
        else if(auto  x0 = dynamic_cast<LoadAddr*>( x)) {
          t.base = x0->offset;
        }
        else if(auto  x0 = dynamic_cast<LoadConst*>( x)) {
          t.base = NULL;//没有对应的类型
        }
        else if(auto  x0 = dynamic_cast<LoadInstr*>( x)) {
          auto &t1 = T.at(x0->addr);//确保地址寄存器已经对应了一个变量
          assert(t1.base);
        }
        else if(auto  x0 = dynamic_cast<UnaryOpInstr*>( x)) {
          if (x0->op.type == UnaryOp::ID) {//mv指令，将目标寄存器对应的变量和源寄存器对应的变量对应起来
            auto &t1 = T.at(x0->s1);
            t.base = t1.base;
          }
        }
        else if(auto  x0 = dynamic_cast<BinaryOpInstr*>( x)) {
          auto &t1 = T.at(x0->s1), &t2 = T.at(x0->s2);//获取两个源寄存器对应的变量
          assert(!t1.base || !t2.base);//确保有一个是空的
          if (x0->op.type == BinaryOp::ADD||x0->op.type == BinaryOp::ADDW) {//如果是加法
          //将目标的类型指定为有类型的那个寄存器对应的变量
            if (t1.base) {
              t.base = t1.base;
            } else if (t2.base) {
              t.base = t2.base;
            }
          } else {//如果不是加法，那么两个都必须是空的
            assert(!t1.base);
            assert(!t2.base);
          }
        }
        else if(auto  x0 = dynamic_cast<PhiInstr*>( x)) {
        }
        else if(auto  x0 = dynamic_cast<ArrayIndex*>( x)) {
          auto &t1 = T.at(x0->s1);
          auto &t2 = T.at(x0->s2);
          assert(x0->size >= 0);
          assert(t1.base);
          assert(!t2.base);
          t.base = t1.base;//目标的类型和基地址对应的类型一样
        }
        else if(auto  x0 = dynamic_cast<CallInstr*>( x)) 
        {
            if(auto  f = dynamic_cast<UserFunction*>( x0->f)) {
              for (int i = 0; i < (int)x0->args.size(); ++i) {
                if (f->scope.array_args.count(i)) {//如果这个参数是数组类型
                  auto &ti = T.at(x0->args.at(i).first);
                  assert(ti.base);//确保这个函数的参数已经和变量进行了对应
                }
              }
            }
            else if(auto  f = dynamic_cast<LibFunction*>( x0->f)) {
              for (int i = 0; i < (int)x0->args.size(); ++i) {
                if (f->array_args.count(i)) {
                  auto &ti = T.at(x0->args.at(i).first);
                  assert(ti.base);
                }
              }
            }
            else assert(0);
        }
        else if(auto  x0 = dynamic_cast<Convert*>( x))
        {

        }
        else {
          // ir_serializer ir_printer(std::cout);

          // x->accept(ir_printer);
          // std::cout<<std::endl;
          assert(0);
        }
      }
      else if(auto  x0 = dynamic_cast<LocalVarDef*>( _x)) {
      }
      else if(auto  x0 = dynamic_cast<StoreInstr*>( _x)) {
        auto &t1 = T.at(x0->addr), &t2 = T.at(x0->s1);
        assert(t1.base && !t2.base);//确保store的地址和源都没有对应的变量，因为并不会将数组的地址保存起来
      }
      else if(auto  x0 = dynamic_cast<JumpInstr*>( _x)) {
      }
      else if(auto  x0 = dynamic_cast<BranchInstr*>( _x)) {
        auto &t1 = T.at(x0->cond);
        assert(!t1.base);//cond寄存器不能对应数组
      }
      else if(auto  x0 = dynamic_cast<ReturnInstr*>( _x)) {
        auto &t1 = T.at(x0->s1);//返回值寄存器也不能对应数组
        assert(!t1.base);
      }
      else assert(0);
    }
  });
  return T;
}


void code_reorder_naive(UserFunction *f) {
  auto S = build_info_node(f);
  std::function<void(IrBlock *)> dfs;
  std::unordered_map<IrBlock *, int> order;
  int tk = 0;
  dfs = [&](IrBlock *w) {
    if (order[w]) return;
    order[w] = ++tk;
    Instr *x = w->back();
    if(auto  y = dynamic_cast<JumpInstr*>( x)) { dfs(y->target); }
    else if(auto  y = dynamic_cast<BranchInstr*>( x)) {
      dfs(y->target1);
      dfs(y->target0);
    }
    else if(auto  y = dynamic_cast<ReturnInstr*>( x)) {
      ;
    }
    else assert(0);
  };
  dfs(f->entry);
  std::sort(f->bbs.begin(), f->bbs.end(),
            [&](const std::unique_ptr<IrBlock> &x, const std::unique_ptr<IrBlock> &y) {
              return order[x.get()] < order[y.get()];
            });
}

void Pass::inst_schedule::_inst_schedule(UserFunction *f)
{
    f->for_each([&](IrBlock *bb) {
    std::unordered_map<Reg, unique_ptr<Instr>> r2i;
    std::list<unique_ptr<Instr>> newInstrs;
    assert(bb->instrs.size() != 0);
    auto end = bb->instrs.end();
    end--;
    Instr *x = end->get();
    CaseNot(ControlInstr, x) { assert(false); }
    for (auto it = bb->instrs.begin(); it != end; ++it) {
      Instr *x = it->get();
      if(auto  x0 = dynamic_cast<LoadConst*>( x)) { r2i[x0->d1] = std::move(*it); }
      else if(auto  x0 = dynamic_cast<Convert*>( x)) {   //Newly Added
        if (r2i.count(x0->src)) {
          newInstrs.emplace_back(std::move(r2i[x0->src]));
          r2i.erase(r2i.find(x0->src));
        }
        newInstrs.emplace_back(std::move(*it));
      }
      else if(auto  x0 = dynamic_cast<UnaryOpInstr*>( x)) {
        if (r2i.count(x0->s1)) {
          newInstrs.emplace_back(std::move(r2i[x0->s1]));
          r2i.erase(r2i.find(x0->s1));
        }
        newInstrs.emplace_back(std::move(*it));
      }
      else if(auto  x0 = dynamic_cast<BinaryOpInstr*>( x)) {
        if (r2i.count(x0->s1)) {
          newInstrs.emplace_back(std::move(r2i[x0->s1]));
          r2i.erase(r2i.find(x0->s1));
        }
        if (r2i.count(x0->s2)) {
          newInstrs.emplace_back(std::move(r2i[x0->s2]));
          r2i.erase(r2i.find(x0->s2));
        }
        newInstrs.emplace_back(std::move(*it));
      }
      else if(auto  x0 = dynamic_cast<LoadInstr*>( x)) {
        if (r2i.count(x0->addr)) {
          newInstrs.emplace_back(std::move(r2i[x0->addr]));
          r2i.erase(r2i.find(x0->addr));
        }
        newInstrs.emplace_back(std::move(*it));
      }
      else if(auto  x0 = dynamic_cast<StoreInstr*>( x)) {
        if (r2i.count(x0->s1)) {
          newInstrs.emplace_back(std::move(r2i[x0->s1]));
          r2i.erase(r2i.find(x0->s1));
        }
        if (r2i.count(x0->addr)) {
          newInstrs.emplace_back(std::move(r2i[x0->addr]));
          r2i.erase(r2i.find(x0->addr));
        }
        newInstrs.emplace_back(std::move(*it));
      }
      else if(auto  x0 = dynamic_cast<BranchInstr*>( x)) {
        if (r2i.count(x0->cond)) {
          newInstrs.emplace_back(std::move(r2i[x0->cond]));
          r2i.erase(r2i.find(x0->cond));
        }
        newInstrs.emplace_back(std::move(*it));
      }
      else if(auto  x0 = dynamic_cast<ReturnInstr*>( x)) {
        if (r2i.count(x0->s1)) {
          newInstrs.emplace_back(std::move(r2i[x0->s1]));
          r2i.erase(r2i.find(x0->s1));
        }
        newInstrs.emplace_back(std::move(*it));
      }
      else if(auto  x0 = dynamic_cast<CallInstr*>( x)) {
        std::unordered_set<Reg> regs;
        for (auto reg : x0->args) {
          if (r2i.count(reg.first)) {
            regs.insert(reg.first);
          }
        }
        for (auto reg : regs) {
          newInstrs.emplace_back(std::move(r2i[reg]));
          r2i.erase(r2i.find(reg));
        }
        newInstrs.emplace_back(std::move(*it));
      }
      else if(auto  x0 = dynamic_cast<ArrayIndex*>( x)) {
        if (r2i.count(x0->s1)) {
          newInstrs.emplace_back(std::move(r2i[x0->s1]));
          r2i.erase(r2i.find(x0->s1));
        }
        if (r2i.count(x0->s2)) {
          newInstrs.emplace_back(std::move(r2i[x0->s2]));
          r2i.erase(r2i.find(x0->s2));
        }
        newInstrs.emplace_back(std::move(*it));
      }
      else if(auto  x0 = dynamic_cast<PhiInstr*>( x)) {
        std::unordered_set<Reg> regs;
        for (auto p : x0->uses) {
          if (r2i.count(p.first)) {
            regs.insert(p.first);
          }
        }
        for (auto reg : regs) {
          newInstrs.emplace_back(std::move(r2i[reg]));
          r2i.erase(r2i.find(reg));
        }
        newInstrs.emplace_back(std::move(*it));
      }
      else if(auto  x0 = dynamic_cast<MemUse*>( x)) {
        if (r2i.count(x0->s1)) {
          newInstrs.emplace_back(std::move(r2i[x0->s1]));
          r2i.erase(r2i.find(x0->s1));
        }
        newInstrs.emplace_back(std::move(*it));
      }
      else if(auto  x0 = dynamic_cast<MemEffect*>( x)) {
        if (r2i.count(x0->s1)) {
          newInstrs.emplace_back(std::move(r2i[x0->s1]));
          r2i.erase(r2i.find(x0->s1));
        }
        newInstrs.emplace_back(std::move(*it));
      }
      else if(auto  x0 = dynamic_cast<MemRead*>( x)) {
        if (r2i.count(x0->mem)) {
          newInstrs.emplace_back(std::move(r2i[x0->mem]));
          r2i.erase(r2i.find(x0->mem));
        }
        if (r2i.count(x0->addr)) {
          newInstrs.emplace_back(std::move(r2i[x0->addr]));
          r2i.erase(r2i.find(x0->addr));
        }
        newInstrs.emplace_back(std::move(*it));
      }
      else if(auto  x0 = dynamic_cast<MemWrite*>( x)) {
        if (r2i.count(x0->mem)) {
          newInstrs.emplace_back(std::move(r2i[x0->mem]));
          r2i.erase(r2i.find(x0->mem));
        }
        if (r2i.count(x0->addr)) {
          newInstrs.emplace_back(std::move(r2i[x0->addr]));
          r2i.erase(r2i.find(x0->addr));
        }
        if (r2i.count(x0->s1)) {
          newInstrs.emplace_back(std::move(r2i[x0->s1]));
          r2i.erase(r2i.find(x0->s1));
        }
        newInstrs.emplace_back(std::move(*it));
      }
      else {
        newInstrs.emplace_back(std::move(*it));
      }
    }
    for (auto &p : r2i) {
      newInstrs.emplace_back(std::move(p.second));
    }
    newInstrs.emplace_back(std::move(*end));
    assert(newInstrs.size() == bb->instrs.size());
    bb->instrs = std::move(newInstrs);
  });
}

void Pass::inst_schedule::run()
{
   for(auto & [_,f]: c->funcs) _inst_schedule(f.get());
}

std::unordered_map<IrBlock *, LoopInfo> check_loop(UserFunction *f,int type) {

  auto S = build_info_node(f);
  auto defs = build_defs(f);
  auto i2bb = build_in2bb(f);
  auto A = infer_array_regs(f);//得到重新构建ssa后得到的数组变量和变量的对应关系
  // print_cfg(S, f);
  std::unordered_map<IrBlock *, LoopInfo> LI;
  
  auto for_each_child = [&](IrBlock* bb,std::function<void(IrBlock*)> f){
    auto sw = S[bb].loop;
    assert(sw.loop_rt);//确保是循环的根节点
    for (IrBlock *u : sw.loop_ch) f(u);
  };

  std::function<void(IrBlock *)> dfs;
  dfs = [&](IrBlock *w) {

    for_each_child(w,[&](IrBlock* child){
      if (S[child].loop.loop_rt) dfs(child);//遍历内部的循环
    });

    auto &liw = LI[w];
    for_each_child(w,[&](IrBlock* child){
      if (S[child].loop.loop_rt) {//用子循环的信息更新当前循环的信息
        auto &rw = LI.at(child).rw;
        for (auto &[var,array_rw_info] : rw) {
          liw.rw[var].update(array_rw_info);
        }
      }
    });

    auto sw = S[w];
    if (!sw.loop.loop_simple) return;//非简单循环，不会执行下面的操作
    IrBlock *pre_exit = sw.loop.loop_pre_exit,
    *exit = *sw.loop.loop_exit.begin();

    if (pre_exit != w) return;  // TODO，如果条件判断比较复杂，也不管

    liw.simple = 1;//是简单循环，并且条件判断比较简单的循环的simple为true

    auto is_control_bb = [&](Reg r) { return S.at(i2bb.at(defs.at(r))).is_strict_dom(sw); };//寄存器def所在的BB是否严格支配当前BB

    std::unordered_map<Reg, int> loop_uses;//寄存器和使用次数
    loop_tree_for_each_bfs(S, w, [&](IrBlock *u) { get_use_count(loop_uses, u); });//统计当前循环下面的所有寄存器的use数量

    w->for_each([&](Instr *i) {
      if(auto  phi = dynamic_cast<PhiInstr*>( i)) {
        Reg r = phi->d1;
        auto &var = liw.loop_var[r];//loop var指的是用于控制循环走向的操作数寄存器
        var.mem = A.at(r);
        assert(phi->uses.size() == 2);
        if (!var.mem) {
          auto &u0 = phi->uses.at(0);
          auto &u1 = phi->uses.at(1);
          if (S[u0.second].is_strict_dom(sw)) std::swap(u0, u1);//将循环的入口BB放到u1的位置
          assert(is_in_loop(S, u0.second, w));
          assert(!is_in_loop(S, u1.second, w));
          // u0: last loop
          // u1: before loop
          Reg r0 = u0.first;
          auto i0 = defs.at(r0);//循环回来的地方的BB中修改条件判断源寄存器的指令
          if(auto  bop = dynamic_cast<BinaryOpInstr*>( i0)) {//循环中修改条件判断源寄存器的指令
            switch (bop->op.type) {
              case BinaryOp::ADD:
              case BinaryOp::SUB:
              case BinaryOp::MUL:
              case BinaryOp::DIV:
              case BinaryOp::ADDW:
              case BinaryOp::SUBW:
              case BinaryOp::MULW:
              case BinaryOp::DIVW:
              {
                Reg s1 = bop->s1, s2 = bop->s2;
                if (s2 == r && bop->op.comm()) std::swap(s1, s2);//如果和phi指令的目标寄存器的名字一样，将其放到s1的位置
                if (s1 == r) {
                  // r0 = r op s2
                  // r = phi(r0,c)
                  var.reduce.base = u1.first;//loop的初始值是来自循环外面的那个寄存器的值
                  var.reduce.step = s2;//loop var每次增加的值是来自修改条件判断源寄存器的指令中不是循环的那个寄存器
                  var.reduce.op = bop->op.type;//步进的操作类型
                  if (is_control_bb(s2)) {//如果定义s2的BB严格支配root，说明这个寄存器在循环的时候是不会变得
                    var.reduce.ind_var = 1;//说明在循环过程中不会变
                  } else {
                    var.reduce.reduce_var =
                        (loop_uses.at(r0) == 1 && loop_uses.at(r) == 1) &&
                        (bop->op.type == BinaryOp::ADD)||(loop_uses.at(r0) == 1 && loop_uses.at(r) == 1) &&
                        (bop->op.type == BinaryOp::ADDW);//loop var对应的root中的寄存器和jump_back中的寄存器都只使用了一次
                  }
                }
                break;
              }
            }
          }
        }
      }
    });

    auto i0 = dynamic_cast<BranchInstr*>( pre_exit->back());
    if(!i0) assert(0);
    bool rev = 0;
    if (i0->target1 == exit) {//获取最后有无加！运算
        rev = 1;  // while(s1 !op s2)
    } else if (i0->target0 == exit) {
        rev = 0;  // while(s1 op s2)
    } else assert(0);
    auto bop = dynamic_cast<BinaryOpInstr*>( defs.at(i0->cond));
    if(!bop) return;
    Reg s1 = bop->s1, s2 = bop->s2;
    auto tp = bop->op.type;
    //将检测到的！运算施加到条件判断上，就是改成反运算
    if (rev) {
      if (tp == BinaryOp::LESS) tp = BinaryOp::LEQ;
      else if (tp == BinaryOp::LEQ) tp = BinaryOp::LESS;
      std::swap(s1, s2);
      rev = 0;
    }
    if (is_control_bb(s1)) {//如果操作数在循环中不变，将其放到s2的位置
      std::swap(s1, s2);
      rev = 1;
    }
    if (is_control_bb(s2)) {//如果s2的值在循环中保持不变
      if (liw.loop_var.count(s1)) {//如果s1是loop var
        auto &var = liw.loop_var.at(s1);
        if (var.reduce.ind_var) {//如果update loop var的寄存器在循环过程中不变，并且用于判断循环是否结束的另一个操作数也在循环过程中不变，就认为这是一个simple cond
          // while(rev ^ (s1 op c2))
          switch (tp) {
            case BinaryOp::LESS:
            case BinaryOp::LEQ:
              liw.simple_cond = SimpleCond{tp, rev, s1, s2};
              break;
          }
        }
      }
    }
  };

  f->for_each([&](IrBlock *w) {
    auto &sw = S[w];
    //从最外层的循环开始dfs
    if (sw.loop.loop_rt && !sw.loop.loop_fa) dfs(w);
  });

  return LI;
}
