#include "pass.h"
#define Case(T, a, b) if (auto a = dynamic_cast<T *>(b))
#define CaseNot(T, b) 
void ssa_construction(UserFunction *f, std::function<bool(Reg)> check) {

  auto S = build_info_node(f);//构建支配树
  struct DefState {
    std::unordered_set<IrBlock *> bbs;
    Instr *def_pos = NULL;
    Reg reachingDef = 0;  // reg 0 is uninitialized value
    int id = 0;
  };
  struct InstrState {
    IrBlock *fa;
    int id;
  };
  std::unordered_map<Reg, DefState> defs;
  std::unordered_map<Instr *, InstrState> Si;
  std::unordered_map<PhiInstr *, Reg> phi_var;
  for (auto& blocks : f->bbs) {
    for (auto& instrs : blocks->instrs) {
      if (auto y = dynamic_cast<RegWriteInstr*>(instrs.get()))
        if (check(y->d1)) 
          defs[y->d1].bbs.insert(blocks.get());
    }
  }

  for (auto &kv : defs) {
    // SetDebugState _(dbg, 0);
    Reg v(kv.first);
    auto &D = kv.second.bbs;
    std::unordered_set<IrBlock *> F, W = D;
    while (!W.empty()) {
      IrBlock *x = *W.begin();
      W.erase(W.begin());
      for (IrBlock *y : S[x].DF) {
        if (!F.count(y)) {
          auto phi = new PhiInstr(v);
          phi_var[phi] = v;
          y->push_front(phi);
          F.insert(y);
          if (!D.count(y)) W.insert(y);
        }
      }
    }
  }

  for (auto& blocks : f->bbs) {
    int id = 0;
    for (auto& instrs : blocks->instrs) {
      Si[instrs.get()] = InstrState{blocks.get(), id++}; 
    }
  }


  auto updateReachingDef = [&](Reg v, Instr *i) {
    Reg &r = defs[v].reachingDef;
    while (r.id) {
      auto &dr = defs[r];
      Instr *def = dr.def_pos;
      IrBlock *b1 = Si[def].fa, *b2 = Si[i].fa;
      if (S[b1].is_strict_dom(S[b2])) break;
      if (b1 == b2 && Si[def].id < Si[i].id) break;
      r = dr.reachingDef;
    }
  };
  dom_tree_dfs(S, [&](IrBlock *bbb) {
    for (auto& instrs : bbb->instrs) {
      auto i = instrs.get();
      Case(PhiInstr, i1, i) {}
      else i->map_use([&](Reg &v) {
        if (check(v)) {

          updateReachingDef(v, i);
          v = defs[v].reachingDef;
        }
      });
      Case(RegWriteInstr, i0, i) {
        Reg &v = i0->d1;
        if (check(v)) {

          updateReachingDef(v, i0);
          Reg v1 =
              f->new_Reg(f->get_name(v) + "_" + std::to_string(++defs[v].id),v.is_int);//*********

          defs[v1].def_pos = i0;
          defs[v1].reachingDef = defs[v].reachingDef;
          defs[v].reachingDef = v1;
          v = v1;
        }
      }
    }
    for (IrBlock *bb1 : S[bbb].cfg.out) {
      for (auto& instrs : bb1->instrs) {
        auto i = instrs.get();
        Case(PhiInstr, phi, i) {
          if (phi_var.count(phi)) {
            // dbg << "visit " << *phi << "\n";
            Reg v = phi_var[phi];
            updateReachingDef(v, bbb->back());
            v = defs[v.id].reachingDef;
            phi->add_use(v, bbb);
          } else {
            // dbg << "visit old " << *phi << "\n";
            for (auto &u : phi->uses) {
              auto &v = u.first;
              if (u.second == bbb && check(v)) {
                updateReachingDef(v, bbb->back());
                v = defs[v.id].reachingDef;
              }
            }
          }
        }
      }

      }
  });
  remove_unused_def(f);
}

void ssa_construction_mem(UserFunction *f, std::function<bool(Reg)> check) {

  auto S = build_info_node(f);//构建支配树
  struct DefState {
    std::unordered_set<IrBlock *> bbs;
    Instr *def_pos = NULL;
    Reg reachingDef = 0;  // reg 0 is uninitialized value，这个是能够接触的def，用于替换use中的寄存器
    int id = 0;
  };
  struct InstrState {//指令所在的BB和位置
    IrBlock *fa;
    int id;
  };
  std::unordered_map<Reg, DefState> defs;//需要修改的新的寄存器和对应的BB块们
  std::unordered_map<Instr *, InstrState> Si;
  std::unordered_map<PhiInstr *, Reg> phi_var;//phi指令和对应的目标地址
  for (auto& blocks : f->bbs) {
    for (auto& instrs : blocks->instrs) {
      if (auto y = dynamic_cast<RegWriteInstr*>(instrs.get()))//只针对写寄存器指令中指定的寄存器进行操作
        if (check(y->d1)) 
          defs[y->d1].bbs.insert(blocks.get());
    }
  }
  for (auto &kv : defs) {
    Reg v(kv.first);
    auto &D = kv.second.bbs;
    std::unordered_set<IrBlock *> F, W = D;//F用于记录是否已经插入了phi指令，W用于保存需要往其他地方插入phi指令的BB
    while (!W.empty()) {
      //弹出bbs中第一个BB
      IrBlock *x = *W.begin();
      W.erase(W.begin());
      for (IrBlock *y : S[x].DF) {//遍历所有支配边界
      //插入针对D中新变量b的phi指令
        if (!F.count(y)) {
          auto phi = new PhiInstr(v);
          phi_var[phi] = v;
          y->push_front(phi);
          F.insert(y);
          if (!D.count(y)) W.insert(y);//将插入的phi指令所在的BB放到W中
        }
      }
    }
  }
//将BB中每条指令和对应的BB对应起来，id是位置
  for (auto& blocks : f->bbs) {
    int id = 0;
    for (auto& instrs : blocks->instrs) {
      Si[instrs.get()] = InstrState{blocks.get(), id++}; 
    }
  }


  auto updateReachingDef = [&](Reg v, Instr *i) {
    Reg &r = defs[v].reachingDef;//没有reachingdef会直接退出
    // r.is_int=v.is_int;//*******
    // dbg << v << ".reachingDef: " << r;
    while (r.id) {//会在没有初始化的地方停下来
      auto &dr = defs[r];
      Instr *def = dr.def_pos;//v的reaching def所在的def指令
      IrBlock *b1 = Si[def].fa, *b2 = Si[i].fa;//该指令所在的BB和原来指令所在的BB
      if (S[b1].is_strict_dom(S[b2])) break;//如果严格支配，说明就是这个目标
      if (b1 == b2 && Si[def].id < Si[i].id) break;//如果是在同一个BB中，并且reach def的指令在原指令的上面，也不用更新
      r = dr.reachingDef;//获取reaching def的reaching def
    //   dbg << " -> " << r;
    }
    // dbg << "\n";
  };
  dom_tree_dfs(S, [&](IrBlock *bbb) {
    for (auto& instrs : bbb->instrs) {
      auto i = instrs.get();
      Case(PhiInstr, i1, i) {}
      else i->map_use([&](Reg &v) {//如果不是phi指令
        if (check(v)) {//如果使用了之前新加的寄存器
          //对这个寄存器进行替换
          updateReachingDef(v, i);
          v = defs[v].reachingDef;
        }
      });
      Case(RegWriteInstr, i0, i) {//如果是写寄存器指令
        Reg &v = i0->d1;
        if (check(v)) {//如果写的寄存器是新添加的寄存器

          updateReachingDef(v, i0);//更新reaching def
          Reg v1 =
              f->new_Reg(f->get_name(v) + "_" + std::to_string(++defs[v].id),v.is_int);//v1相当于是一个新的名字

          defs[v1].def_pos = i0;//记录寄存器所在的指令
          defs[v1].reachingDef = defs[v].reachingDef;//继承v的reaching def
          defs[v].reachingDef = v1;
          v = v1;//将寄存器替换
        }
      }
    }


    for (IrBlock *bb1 : S[bbb].cfg.out) {
      for (auto& instrs : bb1->instrs) {
        auto i = instrs.get();
        Case(PhiInstr, phi, i) {
          if (phi_var.count(phi)) {
            // dbg << "visit " << *phi << "\n";
            Reg v = phi_var[phi];
            updateReachingDef(v, bbb->back());
            v = defs[v.id].reachingDef;
            phi->add_use(v, bbb);
          } else {
            // dbg << "visit old " << *phi << "\n";
            for (auto &u : phi->uses) {
              auto &v = u.first;
              if (u.second == bbb && check(v)) {
                updateReachingDef(v, bbb->back());
                v = defs[v.id].reachingDef;
              }
            }
          }
        }
      }
      }
  });
  // remove_unused_def(f);
}


std::unordered_map<IrFunction *, ArrayRW> get_array_rw(CompileUnit &c) {
  // inter-procedure array read write analysis

  std::unordered_map<IrFunction *, ArrayRW> rw;

//标准输入输出
  auto _in = c.scope.array_args.at(0);
  auto _out = c.scope.array_args.at(1);

  for (auto &kv : c.lib_funcs) {
    //记录lib funcion的数组参数的读写情况
    LibFunction *f = kv.second.get();
    auto &s = rw[f];
    for (auto &kv : f->array_args) {//库函数的数组参数的读写情况事先是知道的
      s.arg_rw[kv.first].may_write = kv.second;
    }
    //记录标准输入输出的读写情况
    if (f->in) s.global_rw[_in] = 1;
    if (f->out) s.global_rw[_out] = 1;
  }
//获得函数调用的顺序，这个是按照函数返回的顺序来的，main函数在最后
  auto seq = get_call_order(c);

  // {
  //   ir_serializer irprinter(std::cout);
  //   for (auto i : seq)i->accept(irprinter);
  //   std::cout<<"\n";
  // }

  for (UserFunction *f : seq) {
    auto &sf = rw[f];
    //将存有地址的寄存器和对应的变量对应起来
    auto &types = sf.types = simple_type_check(f);
    f->for_each([&](Instr *x) {
      Case(CallInstr, x0, x) {
        auto &sg = rw[x0->f];
        //将调用函数的全局数组读写情况记录到当前函数
        for (auto &kv : sg.global_rw) {
          sf.global_rw[kv.first] |= kv.second;
        }
        //如果调用的函数是普通函数，将这次调用记录下来
        Case(UserFunction, g, x0->f) {
          (g == f ? sf.rec_calls : sf.normal_calls).push_back(x0);
        }
      }
      else Case(LoadInstr, x0, x) {
        auto m = types.at(x0->addr).base;//获取地址对应的变量
        //创建这个变量对应的项
        if (m->global)
          sf.global_rw[m];
        else if (m->arg) {
          int id = f->scope.array_arg_id.at(m);
          sf.arg_rw[id];
        }
      }
      else Case(StoreInstr, x0, x) {
        auto m = types.at(x0->addr).base;
        //记录对该变量的写操作
        if (m->global)
          sf.global_rw[m] = 1;
        else if (m->arg) {
          int id = f->scope.array_arg_id.at(m);
          sf.arg_rw[id].may_write = 1;
        }
      }
    });
    //更新arg中的数组的读写情况
    auto update = [&](CallInstr *call) {
      bool flag = 0;
      auto &sg = rw.at(call->f);
      for (auto &kv : sg.arg_rw) {
        int id1 = kv.first;//被调函数参数编号
        auto m = types.at(call->args.at(id1).first).base;//对应的调用者函数的变量
        if (!m->global && m->arg) {//如果这个变量是调用者函数的数组参数
          int id0 = f->scope.array_arg_id.at(m);//这个变量在调用者参数中的参数编号
          // caller's id0 is callee's id1
          flag |= !sf.arg_rw.count(id0);//如果是这个参数的第一次use,flag置为1
          bool &v = sf.arg_rw[id0].may_write;//考虑调用者之前是否修改了这个参数
          bool v0 = v;
          v |= kv.second.may_write;//考虑被调用者是否修改了这个参数，并更新数据
          flag |= (!v0 && v);//被调用者修改了，但是调用者之前没有修改
        }
      }
      return flag;
    };
    //根据传入的参数，更新调用者函数中的数组参数的读写情况
    for (auto call : sf.normal_calls) update(call);
    for (bool flag = 0;;) {
      for (auto call : sf.rec_calls) flag |= update(call);
      if (!flag) break;
    }
  }

  std::reverse(seq.begin(), seq.end());
//逆着调用顺序再来一遍，这时候读写情况已经统计好了
//更新maybe信息（参数和被调用函数中的变量的同名关系）
  for (UserFunction *f : seq) {
    auto &sf = rw.at(f);
    auto &types = sf.types;
    auto update = [&](auto &calls) -> bool {
      bool flag = 0;
      for (auto x0 : calls) {//被调用的函数
        Case(UserFunction, g, x0->f) {
          auto &sg = rw.at(g);
          for (auto &kv : sg.arg_rw) {//遍历被调用函数的参数
            int id1 = kv.first;//被调用者函数的参数编号
            auto m = types.at(x0->args.at(id1).first).base;//这个参数对应的调用者函数中的变量
            if (!m->global && m->arg) {//如果这个变量是调用者函数的数组参数
              int id0 = f->scope.array_arg_id.at(m);//调用者函数的这个变量的编号
              for (auto m0 : sf.arg_rw[id0].maybe) {
                flag |= kv.second.maybe.insert(m0).second;//将may的信息从外层向内层传递
              }
            } else {
              flag |= kv.second.maybe.insert(m).second;//如果不是函数，将这个变量添加到被调用者函数参数的maybe中
            }
          }
        }
        else assert(0);
      }
      return flag;
    };

    while (update(sf.rec_calls))
      ;
    update(sf.normal_calls);
//记录不同函数之间以及函数内部的数组参数的同名情况
    for (auto &kv : sf.arg_rw) {
      int id1 = kv.first;//函数参数的编号
      auto m1 = f->scope.array_args.at(id1);//参数对应的变量
      //记录同名变量
      for (auto m2 : kv.second.maybe) {
        sf.alias[m1].insert(m2);
        sf.alias[m2].insert(m1);
      }
      //统计参数之间的同名关系
      for (auto &kv2 : sf.arg_rw) {
        int id2 = kv2.first;
        if (id1 >= id2) continue;//减少计算量
        auto m2 = f->scope.array_args.at(id2);
        for (auto m3 : kv.second.maybe) {
          if (kv2.second.maybe.count(m3)) {//参数1和参数2中的maybe中的变量同名，则参数1和参数2同名
            sf.alias[m1].insert(m2);
            sf.alias[m2].insert(m1);
            break;
          }
        }
      }
    }
  }

  return rw;
}

std::unordered_map<Reg, Variable *> infer_array_regs(UserFunction *f) {
  std::unordered_map<Reg, Variable *> mp;//array ssa的寄存器和对应的变量的关系
  auto defs = build_defs(f);
  std::function<void(Instr *)> dfs;
  dfs = [&](Instr *_i) {
    Case(RegWriteInstr, i, _i) {
      if (mp.count(i->d1)) return;//已经记录过了，直接返回
      mp[i->d1] = NULL;
      Case(MemWrite, i0, i) { mp[i0->d1] = i0->data; }
      else Case(MemDef, i0, i) {
        mp[i0->d1] = i0->data;
      }
      else Case(MemEffect, i0, i) {
        mp[i0->d1] = i0->data;
      }
      else Case(PhiInstr, i0, i) {
        Variable *ans = NULL;
        for (auto &kv : i0->uses) {
          dfs(defs.at(kv.first));//找到源寄存器对应的变量
          auto v = mp.at(kv.first);
          if (v != NULL) {//保证所有源寄存器对应的变量一样
            assert(ans == NULL || ans == v);
            ans = v;
          }
        }
        mp[i0->d1] = ans;//将源寄存器的变量更新到目标寄存器上
      }
      else Case(UnaryOpInstr, i0, i) {
        dfs(defs.at(i0->s1));//找到源寄存器对应的变量
        mp[i0->d1] = mp.at(i0->s1);
      }//怎么没有add有关的？
    }
  };
  f->for_each(dfs);//遍历所有指令
  return mp;
}

