#include "pass.h"

#include <cstdlib>
#define Case(T, a, b) if (auto a = dynamic_cast<T *>(b))
using namespace Medium;
#define CaseNot(T, b)                  \
  if (auto _ = dynamic_cast<T *>(b)) { \
  } else
void for_each_phi(IrBlock *w, std::function<void(PhiInstr *)> F) {
  w->for_each([&](Instr *i) { Case(PhiInstr, phi, i) F(phi); });
}

void Pass::loop_unroll::_loop_unroll(const std::unordered_map<IrBlock *, LoopInfo> &LI, UserFunction *f)
{
 auto Const = build_const_int(f);//将存有int常量的变量和对应的寄存器对应起来
 auto Const_float=build_const_float(f);
 auto S = build_info_node(f);
  std::vector<IrBlock *> while_to_unroll;//保存需要展开的循环的根结点
  f->for_each([&](IrBlock *w) {
    auto &sw = S[w];
    if (!sw.loop.loop_rt||!sw.loop.loop_simple) return;//不是loop root或简单循环
    for(auto i :sw.loop.loop_ch)//不是嵌套循环
    {
      if(S[i].loop.loop_rt)return ;
    }
    if (w->name.find("_dw2w") != std::string::npos) return;//不是do while to while 的BB
    assert(S[sw.loop.loop_pre].cfg.out.size() == 1);
    while_to_unroll.push_back(w);
  });

  std::unordered_set<Reg> dup_reg;//需要重新ssa的寄存器，不同的份用的def的寄存器的名字是一样的

  for (IrBlock *cur_loop_root : while_to_unroll) {
    auto &sw = S[cur_loop_root];

   IrBlock *exit = *sw.loop.loop_exit.begin();
     IrBlock *entry = sw.loop.loop_pre;
    const auto &liw = LI.at(cur_loop_root);
    if (!(liw.simple_cond)) continue;//确保是simple 的循环（步进和比较的另一个操作数在循环中都是不变的）
    if (cur_loop_root->for_each_until([&](Instr *x) {
          // Case(MemWrite, x0, x) return 1;
          // Case(MemEffect, x0, x) return 1;
          Case(CallInstr, x0, x) return 1;
          Case(StoreInstr, x0, x) return 1;
         
          return 0;
        }))
      continue;//如果循环中有store，call,memwrite,memeffect这些指令那么，就不对这个循环进行展开

    const auto &cond = liw.simple_cond.value();//决定循环跳转的判断条件
    const auto &ind = liw.loop_var.at(cond.ind).reduce;//循环中的起点和步距
    assert(ind.ind_var);//确保步进的寄存器在循环中保持不变
    if (!Const.count(ind.step)) continue;//确保步进的值是常数
    // if (std::abs(Const.at(ind.step)) > 2) continue;//确保步进的绝对值小于2

    bool half_unroll = (ind.op == BinaryOp::ADD||ind.op == BinaryOp::ADDW);
    // type1=0;
    bool full_unroll = ( Const.count(cond.c)&&Const.count(ind.base));//起点和最终值都是常数
    // type2=0;

    int UNROLL_CNT = 4;//默认拆成四份
    int tot_instr = 0, BB_cnt = 0;
    //统计循环中的指令条数和BB的个数
    loop_tree_for_each_bfs(S, cur_loop_root, [&](IrBlock *bb0) {
      BB_cnt += 1;
      tot_instr += bb0->instrs.size() + 1;
     
    });

    if (full_unroll) {
      //下面三个常数分别是起点，终点，步进值
      int start = Const.at(ind.base);
      int end = Const.at(cond.c);
      int step = Const.at(ind.step);
      int cnt = 0;
      //计算循环进行的次数
      while (cnt <= 128) {
        //确定起点和终点
        int s1 = start, s2 = end;
        if (cond.rev) std::swap(s1, s2);
        if (!BinaryOp(cond.op).compute(s1, s2)) break;//上来条件就不成立
        start = BinaryOp(ind.op).compute(start, step);
        ++cnt;
      }
      if (cnt < 1 || cnt > 128 || tot_instr * cnt > 2000)//如果指令太多，循环次数过多或者过少就不展开了
       {//这种情况还有bug，来不及修了，直接不展开了
        half_unroll = 0;
        full_unroll=0;
       }
      else {//如果可以展开，就将循环彻底展开
        UNROLL_CNT = cnt;
        half_unroll = 0;
      }
    }

    if (half_unroll) {
      UNROLL_CNT = 4;
      if (tot_instr > 20 || BB_cnt > 2) UNROLL_CNT = 2;
      if (tot_instr > 200) half_unroll = 0;
    }

    if (!half_unroll && !full_unroll) continue;//不展开

    std::vector<std::unordered_map<IrBlock *, IrBlock *>> mp_bb(UNROLL_CNT);//制作UNROLL_CNT个map

    IrBlock *new_entry2 = f->new_BB("NewEntry");
    IrBlock *new_entry = f->new_BB("newEntry");
    IrBlock *new_exit = f->new_BB("newExit");
    IrBlock *new_exit2 = f->new_BB("NewExit");
    // assert(0);//检测是否触发
    //     std::cout<<"loop unroll"<<std::endl;
    // std::cout<<type1<<type2<<std::endl;
    
    for (int i = 0; i < UNROLL_CNT; ++i) {
      loop_tree_for_each_bfs(S, cur_loop_root, [&](IrBlock *bb0) {
        mp_bb[i][bb0] = f->new_BB(bb0->name + "_" + std::to_string(i) + "_");//每个BB都赋值一遍，通过名字后面的数字后缀来区分
        mp_bb[i][bb0]->disable_schedule_early = 1;
      });
    }

    Reg unroll_cnt = f->new_Reg(true);//拆分的个数
    Reg new_step = f->new_Reg(true);//新的步距
    Reg new_end = f->new_Reg(true);//新的终点

    new_entry2->push(new LoadConst(unroll_cnt, UNROLL_CNT));
    new_entry2->push(new BinaryOpInstr(new_step, ind.step, unroll_cnt, BinaryOp::MULW));
    new_entry2->push(new BinaryOpInstr(new_end, cond.c, new_step, BinaryOp::SUBW));


     loop_tree_for_each_bfs(S, cur_loop_root, [&](IrBlock *bb0) {
      for (int i = 0; i < UNROLL_CNT; ++i) {
        IrBlock *bb1 = mp_bb[i].at(bb0);//目标BB
        bb0->for_each([&](Instr *x) {
          Case(RegWriteInstr, x0, x) { dup_reg.insert(x0->d1); }
          Case(JumpInstr, x0, x) {
            if (x0->target == cur_loop_root) {//只针对jump back进行特殊处理
              if (half_unroll) {//半展开
                bb1->push(new JumpInstr(mp_bb[(i + 1) % UNROLL_CNT].at(cur_loop_root)));//指向下一份中的root
              } 
              else if (full_unroll) {//完全展开
                if (i == UNROLL_CNT - 1) {//最后一份，直接指向出口
                  bb1->push(new JumpInstr(new_exit));
                } else {
                  bb1->push(new JumpInstr(mp_bb[i + 1].at(cur_loop_root)));//指向下一份中的root
                }
              } 
              else
                assert(0);
              return;
            }
          }
          //针对loop root的特殊处理
          if (bb0 == cur_loop_root) {
            Case(BranchInstr, x0, x) {
             IrBlock *tg1 = x0->target1;
              IrBlock *tg0 = x0->target0;
              if (half_unroll) {
                if (i > 0) {//不是第一份
                  IrBlock *tg = (tg1 == exit ? tg0 : tg1);//选择循环内的那个分支，考虑到可能有rev
                  bb1->push(new JumpInstr(mp_bb[i].at(tg)));//将分支指令转化为jump指令
                } 
                else {//第一份
                  if (tg1 == exit)
                    tg1 = new_exit, tg0 = mp_bb[i].at(tg0);
                  else
                    tg0 = new_exit, tg1 = mp_bb[i].at(tg1);
                  Reg cond1 = f->new_Reg(true);//条件表达式的结果是int
                  Reg s1 = cond.ind, s2 = new_end;//起点和终点
                  if (cond.rev) std::swap(s1, s2);
                  bb1->push(new BinaryOpInstr(cond1, s1, s2, cond.op));
                  bb1->push(new BranchInstr(cond1, tg1, tg0));
                }
              } else if (full_unroll) {//完全展开
                IrBlock *tg = (tg1 == exit ? tg0 : tg1);
                bb1->push(new JumpInstr(mp_bb[i].at(tg)));//直接将分支指令转化为jmp循环内的部分
              } else
                assert(0);
              return;
            }
            Case(PhiInstr, phi, x) {//注意，这个phi指令是在root处的
              if (i > 0) {//不是第一份
                for (auto &kv : phi->uses) {
                  if (kv.second != entry) {//将phi指令替换为mv(循环体寄存器 -> phi指令目标寄存器)
                    assert(kv.second == sw.loop.loop_last);
                    bb1->push(new UnaryOpInstr(phi->d1, kv.first, UnaryOp::ID));
                    return;
                  }
                }
                assert(0);
              } 
              else if (full_unroll) {//不是第一份，并且是完全展开
                for (auto &kv : phi->uses) {
                  if (kv.second == entry) {//来自entry，只有type的第一份才会这样
                    bb1->push(new UnaryOpInstr(phi->d1, kv.first, UnaryOp::ID));
                    return;
                  }
                }
              }
            }
          }

          //其他情况直接拷贝指令，其中将BB改为对应的新的份中的BB，其余不变
          bb1->push(x->map([&](Reg &reg) {},
                           [&](IrBlock *&bb) {
                             if (bb == entry)
                               bb = new_entry;
                             else
                               bb = mp_bb[i].at(bb);
                           },
                           [](Variable *&) {}));
        });
      }
    });


    if (half_unroll)
      for_each_phi(cur_loop_root, [&](PhiInstr *phi) {
        for (auto &kv : phi->uses) {
          if (kv.second == entry) {
            kv.first = phi->d1;
            kv.second = new_exit2;
          }
        }
      });

    if (full_unroll) {
      for (auto it = cur_loop_root->instrs.begin(); it != cur_loop_root->instrs.end(); ++it) {
        Instr *x = it->get();
        Case(PhiInstr, phi, x) {
          Instr *x1 = NULL;
          for (auto &kv : phi->uses) {
            if (kv.second != entry) {
              assert(kv.second == sw.loop.loop_last);
              x1 = new UnaryOpInstr(phi->d1, kv.first, UnaryOp::ID);
              break;
            }
          }
          assert(x1);
          *it = std::unique_ptr<Instr>(x1);
        }
        else Case(BranchInstr, x0, x) {
          Reg tmp = f->new_Reg(true);//
          cur_loop_root->instrs.insert(it, std::unique_ptr<Instr>(
                                   new LoadConst(tmp, x0->target1 == exit)));
          x0->cond = tmp;
        }
      }
    }

    Reg s1 = ind.base, s2 = new_end;
    if (cond.rev) std::swap(s1, s2);
    Reg cond2 = f->new_Reg(true);//就是int
    // new_entry2->push(new BinaryOpInstr(cond2,s1,s2,cond.op));
    // new_entry2->push(new BranchInstr(cond2,new_entry,no_unroll));
     new_entry->push(new JumpInstr(mp_bb[0].at(cur_loop_root)));
    new_entry2->push(new JumpInstr(new_entry));



    // no_unroll->push(new JumpInstr(new_exit2));
    new_exit2->push(new JumpInstr(cur_loop_root));
    new_exit->push(new JumpInstr(new_exit2));

 

    entry->back()->map_BB(partial_map(cur_loop_root, new_entry2));

    IrBlock *w0 = S[cur_loop_root].loop.loop_last;
    auto f = partial_map(mp_bb[0].at(w0), mp_bb[UNROLL_CNT - 1].at(w0));
    for_each_phi(mp_bb[0].at(cur_loop_root), [&](PhiInstr *phi) { phi->map_BB(f); });
    cur_loop_root->map_BB(partial_map(entry, new_exit2));

  }

  ssa_construction(f, [&](Reg r) { return dup_reg.count(r); });

  
}

void Pass::loop_unroll::run()
{
    auto PM = PassManager(c);
    std::unordered_map<UserFunction *, std::unordered_map<IrBlock *, LoopInfo>> LI;
    remove_unused_func(*c);
    remove_unused_memobj(*c);
    // into_array_ssa(*c);
    // c->for_each(simplify_load_store);
    // c->for_each(remove_unused_loop);
    PM.add_pass("gcm_schedule_late");
    PM.run_pass();
    // ir_serializer irprint(std::cout);
    // c->accept(irprint);
    c->for_each([&](UserFunction *f) { LI[f] = check_loop(f, 1); });
    // exit_array_ssa(*c);
    for(auto & [_,f]: c->funcs) _loop_unroll(LI[f.get()],f.get());
}
