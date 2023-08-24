#include "pass.h"
// #include "ir_builder.h"
using namespace Pass;
#include "pass.h"
#include "ir_builder.h"

#define Case(T, a, b) if (auto a = dynamic_cast<T *>(b))
void function_inline::func_inline(CompileUnit &c) {

  struct State {
    std::vector<UserFunction *> pre;
    int deg = 0;
    bool recursive = 0;
  };
  std::map<UserFunction *, State> S;
  c.for_each([&](UserFunction *f) {
    f->for_each([&](Instr *x) {
      Case(CallInstr, y, x) {
        Case(UserFunction, g, y->f) {
          // f call g
          if (g != f) { //f is caller and g is callee
            ++S[f].deg;
            S[g].pre.push_back(f);
          } else {
            S[f].recursive = true;
          }
        }
      }
    });
  });
  auto Inline = [&](UserFunction *f) {

    struct InlineState {
      std::map<Variable *, Variable *> mp;
      int cnt = 0;
    };
    std::map<UserFunction *, InlineState> IS; //用来记录所有被内联的函数中局部变量在caller中的映射
    std::vector<IrBlock *> _bbs;
    f->for_each([&](IrBlock *_bb) { _bbs.push_back(_bb); });  //将f中的所有bb放入_bbs中
    std::for_each(_bbs.cbegin(), _bbs.cend(), [&](IrBlock *bb) { //遍历_bbs中的所有bb
      // for (IrBlock *bb = _bb;;) {
        for (auto it = bb->instrs.begin(); it != bb->instrs.end(); ++it) {  //遍历每个bb中所有指令
          Case(CallInstr, call, it->get()) {  //寻找每个bb中的caller
            Case(UserFunction, g, call->f) {  //判断callee是不是函数
              if (g != f && !S[g].recursive) {  //caller != callee，等价于非递归函数
                if (!IS.count(g)) { //map<UserFunction *, InlineState> IS：如果IS中没有该callee
                  g->scope.for_each([&](Variable *x0, Variable *x1) {
                    assert(!x1->global);  //不能为全局变量，如果为可能就是中端出错
                    if (x1->arg) {  //如果是参数则直接删除，因为caller里面自带参数
                      delete x1;
                    } else {  //否则为callee内部的局部变量
                      f->scope.add(x1); //将该局部变量添加到caller中
                      IS[g].mp[x0] = x1;  //建立callee中局部变量到caller中添加的变量的映射
                    }
                  });
                }
                auto &mp_mem = IS[g].mp;  //把callee的InlineState对象取出
                int _il_id = ++IS[g].cnt; //in_line_id?? 可能是这个callee的id,或者说被内联几次id就为几？
                std::string _il_name = g->name + std::to_string(_il_id);  //原callee函数名+其id就为被内联函数的名字
                std::map<IrBlock *, IrBlock *> mp_bb; //caller与callee间block的映射？
                std::map<Reg, Reg> mp_reg;  //caller与callee间reg的映射？
                g->for_each([&](IrBlock *bb) {  //遍历callee中所有的block
                  // alloc BBs
                  mp_bb[bb] = f->new_BB();  //callee有多少个blockcaller就新创建多少个block,对callee中每一个block都建立到caller中新建立的block的映射
                  bb->for_each([&](Instr *i) {  //遍历callee每个block中的所有指令
                    Case(RegWriteInstr, i0, i) {  //筛选其中的regwrite类型指令
                      Reg r = i0->d1; //取出该指令目的寄存器
                      assert(r.id); //目的寄存器不能为0号寄存器
                      if (!mp_reg.count(r)) //如果还没有目的寄存器映射则添加callee到caller的寄存器映射
                        if(r.is_int) {
                          mp_reg[r] = f->new_Reg(g->get_name(r) + "_" + _il_name,true);
                        } //整数寄存器映射
                        else {
                          mp_reg[r] = f->new_Reg(g->get_name(r) + "_" + _il_name,false);  //Newly Added
                        } //浮点数寄存器映射
                    }
                  });
                });
                IrBlock *nxt = f->new_BB();
                nxt->instrs.splice(nxt->instrs.begin(), bb->instrs, ++it,
                                   bb->instrs.end()); //将所有callee block中所有指令插入到nxtblock中
                auto f1 = [&](Reg &x) {
                  if (!mp_reg.count(x)) { //如果存在寄存器没有建立映射关系则报错
                    assert(0);
                  }
                  x = mp_reg.at(x); //返回callee reg与caller reg的映射寄存器
                };
                auto f2 = [&](IrBlock *&x) { 
                  if (!mp_bb.count(x)) { //如果存在block没有建立映射关系则报错
                    assert(0);  // Newly Added
                  }
                  x = mp_bb.at(x); //返回callee block与caller block的映射寄存器
                };
                auto f3 = [&](Variable *&x) {
                  if (!x->global) {
                    assert(!x->arg);  //如果变量arg则报错，在前面理论上应该被过滤掉
                    x = mp_mem.at(x); //返回callee中局部变量在caller中的映射变量
                  }
                };
                g->for_each([&](IrBlock *bb0) {
                  // copy all BBs
                  IrBlock *bb1 = mp_bb.at(bb0);
                  bb0->for_each([&](Instr *x) {
                    Instr *y = NULL;
                    Case(LocalVarDef, x1, x) {
                      if (x1->data->arg) return;
                    }
                    Case(LoadArg, x1, x) {
                      auto y1 = new UnaryOpInstr(x1->d1, call->args.at(x1->id).first,
                                                 UnaryOp::ID);  //将callee中的函数参数替换为move caller中的寄存器变量
                      f1(y1->d1); //将新 建指令的callee目的寄存器替换为caller目的寄存器
                      y = y1;
                    }
                    else Case(ReturnInstr, x1, x) {
                      // copy return value
                      // auto cvt = new Convert(call->d1, x1->s1);
                      // bb1->push(cvt);z
                      // if (call->d1.is_int != x1->s1.is_int) {
                      //   std::cout << 111<<std::endl;
                      //   Reg res_reg = f->new_Reg(call->d1.is_int);
                      //   bb1->push(new Convert(res_reg,x1->s1));
                      //   bb1->push(new UnaryOpInstr(call->d1, res_reg, UnaryOp::ID));
                      // }
                      // else {
                        auto y1 = new UnaryOpInstr(call->d1, x1->s1, UnaryOp::ID);  //将callee中的return语句替换为caller中的move指令
                        f1(y1->s1); //将新 建指令的callee源寄存器替换为caller源寄存器
                        bb1->push(y1);  //return语句代表callee函数结束，因此需要额外插入一条跳转指令跳出该bb
                      // }
                      y = new JumpInstr(nxt); //插入一条跳转指令
                    }
                    else {
                      y = x->map(f1, f2, f3); //其他指令就直接copy一份过去
                    }
                    bb1->push(y); //插入指令
                  });
                });
                bb->pop();  //bb为caller的bb
                bb->push(new JumpInstr(mp_bb.at(g->entry)));  // replace call fun with jump entry_block
                phi_src_rewrite(nxt, bb); //更新phi指令的源值
                bb = nxt; //将当前的bb替换为内联函数后的一整块bb，然后接着插入nxt_bb
                // goto o;
              }
            }
          }
        }
      //   break;
      // o:;
      // }
    });
  };
  std::queue<UserFunction *> q;
  c.for_each([&](UserFunction *f) {
    if (S[f].deg == 0) q.push(f);
  });
  size_t f_cnt = 0;
  while (!q.empty()) {
    auto f = q.front();
    q.pop();
    for (auto g : S[f].pre) {
      if (--S[g].deg == 0) q.push(g);
    }
    ++f_cnt;
    Inline(f);
  }
  assert(f_cnt == c.funcs.size());
  // co-recursive call not supportted
  remove_unused_func(c);
  c.for_each(dce_BB);
}
void function_inline::run() {
  this->func_inline(*(this->c));
}