#include "pass.h"
 //构建支配树，循环树，cfg
UserFunction *cur_fun;
void addconnection(Dom_Loop_CFG& S,IrBlock *y, IrBlock *x)
{
   S[x].cfg.out.push_back(y);
  S[y].cfg.in.push_back(x);
}
void handle_cfg(Dom_Loop_CFG &S)
{
    auto buildcfg =  [&](IrBlock *bb) {
    S[bb].self = bb;
    Instr *inst_last = bb->back();  //取bb中的最后一条指令
    if(auto  inst = dynamic_cast<JumpInstr*>(inst_last)) { addconnection(S,inst->target, bb); }  //如果是jump，添加一条从目标BB到w边
    else if(auto  inst = dynamic_cast<ReturnInstr*>( inst_last)) {
      ;
    }
    else if(auto  inst = dynamic_cast<BranchInstr*>(inst_last)) {   //成功分支和失败分支都有一个边
      addconnection(S,inst->target1, bb);
      addconnection(S,inst->target0, bb);
    }
    else
    {
      // ir_serializer irprinter(std::cout);
      // inst_last->accept(irprinter);
      // std::cout<<"\n";
   assert(0);
    } 
 
  };
  cur_fun->for_each(buildcfg);  //对于每一个BB都构建边得到cfg

}
void handle_dom(Dom_Loop_CFG &S,int &tag,std::vector<IrBlock *> &dfs_indices)
{
   std::function<void(IrBlock *, IrBlock *)> visit_others=[&](IrBlock *w, IrBlock *father) {
    auto &s = S[w];
    if (s.tag == tag) return;
    s.tag = tag;
    if (tag == 1) dfs_indices.push_back(w);
    for (IrBlock *u : s.cfg.out) visit_others(u, w);
  };
  for (IrBlock *bb : dfs_indices) {
    ++tag;
    S[bb].tag = tag;
    visit_others(cur_fun->entry,nullptr);
    for (IrBlock *remain : dfs_indices) {
      if (S[remain ].tag != tag) 
      {
          S[remain ].dom.dom_fa = bb;
          // S[bb].dom.dom_ch.push_back(remain );
      }
      
    }
  }
    for (IrBlock *w : dfs_indices) {
    IrBlock *f = S[w].dom.dom_fa;
    if (f) S[f].dom.dom_ch.push_back(w);
  }


}
void handle_loop(Dom_Loop_CFG &S,int &tag,std::vector<IrBlock *> &dfs_indices)
{
  // 找到循环并标记循环中的BB
   std::function<void(IrBlock *, IrBlock *)> mark_loop;
  mark_loop = [&](IrBlock *bb, IrBlock *root) {
    if (bb == root) return;
    if (S[bb].tag == tag) {//处理循环中的循环
       mark_loop(S[bb].loop.loop_fa, root);
    } else {
      S[bb].tag = tag;
      S[S[bb].loop.loop_fa = root].loop.loop_ch.push_back(bb);
      for (IrBlock *u : S[bb].cfg.in)  mark_loop(u, root);
    }
  };

  ++tag;

  for (int i = dfs_indices.size() - 1;i>0; --i) {
    IrBlock *bb = dfs_indices[i];
    assert(!S[bb].loop.loop_fa);
    for (IrBlock *u : S[bb].cfg.in) {
      auto &su = S[u];
      if (S[bb].is_dom(su)) {//找到循环
         mark_loop(u, bb);
        S[bb].loop.loop_rt = 1;
      }
    }

    if (S[bb].loop.loop_rt) {
      for (IrBlock *u : S[bb].cfg.in) {
        auto &su = S[u];
        if (!S[bb].is_dom(su)) {//循环外的BB
          assert(su.is_dom(S[bb]));
          assert(!S[bb].loop.loop_pre);
          S[bb].loop.loop_pre = u;
        } else {//从后面回来的那个BB,是循环的最后一个BB
          assert(S[bb].loop.loop_last == nullptr);
          S[bb].loop.loop_last = u;
        }
      }
      assert(S[bb].loop.loop_pre);
      assert(S[bb].loop.loop_last);
      //root退出的结点中，不在循环中的结点就是退出的BB
      loop_tree_for_each_bfs(S,bb, [&](IrBlock *u) {
        for (IrBlock *u1 : S[u].cfg.out) {
          if (!is_in_loop(S, u1,bb)) S[bb].loop.loop_exit.insert(u1);
        }
      });
    }
  }

  for (IrBlock *w : dfs_indices) {
    auto &sw = S[w];
    if (!sw.loop.loop_rt) continue;
    sw.loop.loop_simple = 0;
    if (sw.loop.loop_exit.size() != 1) continue;
    assert(sw.cfg.in.size() == 2);
    IrBlock *exit = *sw.loop.loop_exit.begin();
    int exit_cnt = 0;
    for (IrBlock *u : S[exit].cfg.in) {
      if (is_in_loop(S, u, w)) ++exit_cnt, sw.loop.loop_pre_exit = u;
    }
    if (exit_cnt != 1) {
      sw.loop.loop_pre_exit = NULL;
      continue;
    }
    sw.loop.loop_simple = 1;
  }
  for (IrBlock *w : dfs_indices) {
    for (IrBlock *u = S[w].get_loop_rt(); u; u = S[u].loop.loop_fa) ++S[w].loop.loop_depth;
  }

}
Dom_Loop_CFG build_info_node(UserFunction *f) { 
  Dom_Loop_CFG S; //domtree
  cur_fun=f;
  handle_cfg(S);
  // build domination tree
  int tag = 1;
  std::vector<IrBlock *> dfs_indices;
  //获取深度优先遍历顺序
  std::function<void(IrBlock *, IrBlock *)> get_dfs_indices;
  get_dfs_indices = [&](IrBlock *w, IrBlock *father) {
    auto &s = S[w];
    if (s.tag == tag) return;
    s.tag = tag;
    if (tag == 1) dfs_indices.push_back(w);
    for (IrBlock *u : s.cfg.out) get_dfs_indices(u, w);
  };
  get_dfs_indices(f->entry, nullptr);
  //构建支配关系
  handle_dom(S,tag,dfs_indices);

  //获取BB的dom tree深度优先顺序，并记录遍历的时间
  ++tag;

  int time = 0;
  std::vector<IrBlock *> dom_dfs_indices;
  std::function<void(IrBlock *, IrBlock *)> get_dom_dfs_indices;
  get_dom_dfs_indices = [&](IrBlock *bb, IrBlock *fa) {
    auto &a = S[bb];
    if (a.tag == tag) return;
    a.tag = tag;
    a.dom.df_start = ++time;
    dom_dfs_indices.push_back(bb);
    for (IrBlock *u : a.dom.dom_ch) get_dom_dfs_indices(u, bb);
    a.dom.df_finish = time;
  };
  get_dom_dfs_indices(f->entry, nullptr);

  ++tag;

  // calc DF
  f->for_each([&](IrBlock *b) {
    if(S[b].cfg.in.size()>1)
    {
      for (IrBlock *a : S[b].cfg.in) {
      //放入所有支配的BB
        for (IrBlock *x = a; x;) {
          if (S[x].is_strict_dom(S[b])) break;
          S[x].DF.insert(b);
          x = S[x].dom.dom_fa;
        }
      }
    }
    
  });
  //获取循环相关的信息
  handle_loop(S,tag,dfs_indices);
  return S;
}

bool is_in_loop(Dom_Loop_CFG &S, IrBlock *bb, IrBlock *loop) {
  if (loop == nullptr) return true;
  auto &sl = S.at(loop);
  assert(sl.loop.loop_rt);
  while (bb && bb != loop) bb = S.at(bb).loop.loop_fa;
  return bb == loop;
}

void dom_tree_dfs(Dom_Loop_CFG &S, std::function<void(IrBlock *)> F) {
  std::unordered_map<int, IrBlock *> dfs_indices;
  for (auto &kv : S) dfs_indices[kv.second.dom.df_start] = kv.first;
  for (int i = 1; i <= (int)dfs_indices.size(); ++i) F(dfs_indices.at(i));
}

void dom_tree_dfs_reverse(Dom_Loop_CFG &S, std::function<void(IrBlock *)> F) {
  std::unordered_map<int, IrBlock *> dfs_indices;
  for (auto &kv : S) dfs_indices[kv.second.dom.df_start] = kv.first;
  for (int i = (int)dfs_indices.size(); i; --i) F(dfs_indices.at(i));
}
void loop_tree_for_each_bfs(Dom_Loop_CFG &S, IrBlock *bb, std::function<void(IrBlock *)> F) {
  assert(S[bb].loop.loop_rt);
  std::queue<IrBlock *> q;
  q.push(bb);
  while (!q.empty()) {
    bb = q.front();
    q.pop();
    F(bb);
    for (IrBlock *ch : S[bb].loop.loop_ch) q.push(ch);
  }
}



