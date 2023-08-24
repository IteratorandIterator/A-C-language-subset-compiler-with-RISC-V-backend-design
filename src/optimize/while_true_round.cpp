#include "pass.h"
using namespace Pass;

void while_true_round::loop_tree_dfs(UserFunction *f, std::function<void(IrBlock *)> F) {
  std::vector<IrBlock *> bbs;
  f->for_each([&](IrBlock *w) {
    auto &sw = S[w].loop;
    if (!sw.loop_rt) return;  //如果不是一个循环，直接退出
    if (sw.loop_fa) return;   //如果是这个循环支配树的第一个结点，直接退出
    bbs.push_back(w);     //都不是的情况下把这个block加入bbs
  });
  //bbs就是这个func里面所有的block，当然，这个func里面需要有循环
  std::function<void(IrBlock *)> dfs;
  dfs = [&](IrBlock *bb) {
    F(bb);
    for (IrBlock *ch : S[bb].loop.loop_ch) //对于所有的loop，ch从根节点开始遍历
      if (S[ch].loop.loop_rt) dfs(ch);
  };
  for (IrBlock *bb : bbs) dfs(bb);
}

void while_true_round::remove_dull_block(UserFunction* f){
    std::unordered_set<IrBlock *> del;  //被删除的block
    S = build_info_node(f);    // block -> state
    loop_tree_dfs(f, [&](IrBlock *w) {
      auto &sw = S[w].loop;
      if(!check_loop_condition(w)) return;
      bool flag = 0;
      //对于
      sw.loop_pre_exit->back()->map_BB([&](IrBlock *&bb) {
        //如果loop_pre最后一个指令 的 目的bb是loop_last那么 目的bb赋值为w，falg=1
        if (bb == sw.loop_last) {
          bb = w;
          flag = 1;
        }
      });
      if (!flag) return;
      //如果loop_last block的最后一条指令是jump指令看目标不是是不是w
      if(auto x0 = dynamic_cast<JumpInstr*>(sw.loop_last->back())) { assert(x0->target == w); }
      w->map_BB(partial_map(sw.loop_last, sw.loop_pre_exit));
      del.insert(sw.loop_last); //del里面插入最后一个block
    });
    //从函数里面也删除这些bb
    f->bbs.erase(remove_if(f->bbs.begin(), f->bbs.end(),
                           [&](auto &bb) { return del.count(bb.get()); }),f->bbs.end());
}

void while_true_round::convert_to_double_while(UserFunction * f){

  S = build_info_node(f);              //支配树
  auto defs = build_defs(f);        //构建def
  std::unordered_set<Reg> duplicated_reg;

  loop_tree_dfs(f, [&](IrBlock *w) {
    auto &sw = S[w].loop;
    if (sw.loop_depth > 3) return; //太深的loop不管
    if (!sw.loop_simple) return;  //不是简单的loop不管
    if (w == sw.loop_pre_exit) return;  //条件和上面一样 用来限制while1的
    if (S[sw.loop_last].cfg.out.size() != 2) return; //last有两个出口才能展开，一个出口不能展开 用来限制break

    // assert(0);
    IrBlock *w0 = S[w].loop.loop_last;  //查看出口结点
    if(auto br = dynamic_cast<BranchInstr*>(w0->back())) {  //搞一个last block里面的最后一个指令
      if(auto bop = dynamic_cast<BinaryOpInstr*>(defs.at(br->cond))) { //查看分支的条件 ？？？？这里再
        if (bop->op.type == BinaryOp::LESS ||
            bop->op.type == BinaryOp::LEQ ||
            bop->op.type == BinaryOp::EQ ||
            bop->op.type == BinaryOp::fEQ || 
            bop->op.type == BinaryOp::fLEQ || 
            bop->op.type == BinaryOp::fLESS ||
            bop->op.type == BinaryOp::fNEQ);
        else return;
      }
      else return;
    }
    IrBlock *entry = sw.loop_pre;                     //找到循环前面的一个结点    BB1
    IrBlock *exit = *sw.loop_exit.begin();            //找到退出的那一个结点      BB9

    std::unordered_map<IrBlock *, IrBlock *> mp_bb;

    IrBlock *new_entry2 = f->new_BB("NewEntry"),
            *new_entry = f->new_BB("newEntry"),
            *new_exit = f->new_BB("newExit"),
            *new_exit2 = f->new_BB("NewExit");

    loop_tree_for_each_bfs(
        S, w, [&](IrBlock *bb0) { mp_bb[bb0] = f->new_BB(bb0->name + "_whileTrue"); });
    //对于每一个loop里面的block，创建一个完全一样的结点

    loop_tree_for_each_bfs(S, w, [&](IrBlock *bb0) {
      IrBlock *bb1 = mp_bb.at(bb0); //找到新创建的结点
      bb0->for_each([&](Instr *x) { //对里面的每一条指令遍历
        if(auto x0 = dynamic_cast<RegWriteInstr*>(x)) { duplicated_reg.insert(x0->d1); }
        if(auto x0 = dynamic_cast<BranchInstr*>(x)) { //要让重复的循环跳到一个新的结束bb
          if (x0->target1 == exit || x0->target0 == exit) {
            bb1->push(new JumpInstr(new_exit));
            return;
          }
        }
        if (bb0 == w) {  //如果检查到是自身的映射
          if(auto phi = dynamic_cast<PhiInstr*>(x)) {  //如果里面又phi指令
            for (auto &[reg,bb_] : phi->uses) {
              if (bb_ == entry) {
                //在新的里面加一个mov指令
                bb1->push(new UnaryOpInstr(phi->d1, reg, UnaryOp::ID));
                return;
              }
            }
          }
        }

        bb1->push(x->map([&](Reg &reg) {},
                         [&](IrBlock *&bb) {  //一般传入的是 bb = target
                          if (bb == entry)        //如果是pre块，x跳转到new_entry
                             bb = new_entry;      
                           else
                             bb = mp_bb.at(bb); //如果x跳转到的目的不是entry，那就跳转到bb的copy block
                         },
                         [](Variable *&) {}));
      });
    });

    new_entry2->push(new JumpInstr(new_entry));   // 13 -> 14
    new_entry->push(new JumpInstr(mp_bb.at(w)));  // 14 -> bb3_17
    new_exit->push(new JumpInstr(new_exit2));        //new_15-> new_16
    entry->back()->map_BB(partial_map(w, new_entry2));  //BB1 -> BB13

    std::map<Instr*,int> is_del;
    w->for_each([&](Instr* inst){
      if(auto phi = dynamic_cast<PhiInstr*>(inst)) {
        Reg r;
        for (auto &[reg,bb] : phi->uses) {
          if (bb!= entry) {
            r = reg;
          }
        }
        assert(r.id);
        new_exit2->push(new UnaryOpInstr(phi->d1, r, UnaryOp::ID));
        is_del[inst] = 1;
      }
    });

    w->instrs.remove_if([&is_del](std::unique_ptr<Medium::Instr> &inst){
      return is_del[inst.get()] == 1;
    });
    if(auto br = dynamic_cast<BranchInstr*>(w0->back())) {  //
      new_exit2->push(defs.at(br->cond)->copy()); //添加新的跳转指令？
    }
    else assert(0);
    new_exit2->instrs.emplace_back(std::move(w0->instrs.back())); // 把bb8的跳转目标放进去
    w0->instrs.pop_back();//BB8去掉这条指令
    w0->push(new JumpInstr(new_exit2));//BB8跳回去
    assert(S[exit].cfg.in.size() == 1);
  });
  ssa_construction(f, [&](Reg r) { return duplicated_reg.count(r); });
}

bool while_true_round::check_loop_condition(IrBlock* w){
  auto sw = S[w].loop;
  if (!sw.loop_simple) return false;  //只有简单的loop才进行处理
  if (w == sw.loop_pre_exit) return false;   
  if (S[sw.loop_last].cfg.out.size() != 1) return false; //没有唯一出口return
  if (S[sw.loop_last].cfg.in.size() != 1) return false;  //没有唯一入口return
  if (sw.loop_last->instrs.size() != 1) return false;  // 出口必须只有一条指令
  if (!sw.loop_pre_exit) return false;  //如果没有这个pre结点return

  return true;
}


void while_true_round::run(){
    for(auto & [_,f] : c->funcs){
      if (f->bbs.size() > 100) continue;
      remove_dull_block(f.get());
      S.clear();
      convert_to_double_while(f.get());
      S.clear();
    }
}