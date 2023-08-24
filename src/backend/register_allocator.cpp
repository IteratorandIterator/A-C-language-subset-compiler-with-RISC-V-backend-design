#include "register_allocator.h"

#include <cassert>
#include <memory>
#include <queue>
#include <set>
#include <utility>
#include <vector>

#include "arch.h"
#include "inst.h"
#include "code_gen.h"

using std::make_unique;
using std::pair;
using std::set;
using std::vector;

namespace Archriscv {
SimpleColoringAllocator::SimpleColoringAllocator(Function *_func) : func(_func) {}
//处理溢出操作
void SimpleColoringAllocator::spill(const vector<int> &spill_nodes,vector<int> all_size) {
  vector<StackObject *> spill_obj;//溢出的值放到栈里面
  set<int> constant_spilled;//固定寄存器溢出
  
  auto constant_reg=func->constant_reg;
  for (size_t i = 0; i < spill_nodes.size(); ++i)//遍历所有需要spill的结点
    if (constant_reg.find(Reg{spill_nodes[i],this->is_float?FP:INT}) !=constant_reg.end() ||func->symbol_reg.find(Reg{spill_nodes[i]}) != func->symbol_reg.end()) {//
      constant_spilled.insert(spill_nodes[i]);
      spill_obj.push_back(nullptr);
    } else {//普通寄存器溢出
      int type_size =all_size[spill_nodes[i]];
      // if(func->regs[])
      StackObject *t = new StackObject(8);//在这确定变量的大小,****
      func->stack_objects.emplace_back(t);
      spill_obj.push_back(t);
    }
  for (auto &block : func->blocks)//遍历所有的块
    for (auto i = block->insts.begin(); i != block->insts.end();) {//遍历每个块的每一条指令遍历每个块的每一条指令
      bool need_continue = false;//?
      for (Reg r : (*i)->def_reg())//遍历每个指令的def
        if (constant_spilled.find(r.id) != constant_spilled.end()) {//固定寄存器不用溢出操作
          auto nxt = std::next(i);
          block->insts.erase(i);
          i = nxt;
          need_continue = true;//表示跳过当前inst
          break;
        }
      if (need_continue) continue;
      for (size_t j = 0; j < spill_nodes.size(); ++j) {//遍历溢出的结点
        int id = spill_nodes[j];
        bool cur_def = (*i)->def(Reg{id}), cur_use = (*i)->use(Reg{id});//确定是use还是def
        if (constant_reg.find(Reg{id,this->is_float?FP:INT}) != constant_reg.end()) {//固定寄存器
          assert(!cur_def);
          if (cur_use) {//保存常量的寄存器溢出，这边还有问题，就是如果保存了浮点数常量的寄存器移除了。。。。
            Reg tmp{func->reg_n++};
            func->spilling_reg.insert(tmp);
            constant_reg[tmp] = constant_reg[Reg{id,this->is_float?FP:INT}];//传递常量
           
            block->insts.insert(
                i, make_unique<LoadImm>(tmp, constant_reg[Reg{id,this->is_float?FP:INT}]));
            (*i)->replace_reg(Reg{id}, tmp);
          }
        } else if (func->symbol_reg.find(Reg{id}) != func->symbol_reg.end()) {//符号寄存器
          assert(!cur_def);
          if (cur_use) {//保存跳转地址的寄存器溢出
            Reg tmp{func->reg_n++};//啥，插入的load,store的寄存器是在虚拟寄存器的总数上加，那我之前的猜测错了
            func->spilling_reg.insert(tmp);
            func->symbol_reg[tmp] = func->symbol_reg[Reg{id}];
            block->insts.insert(
                i, make_unique<LoadLabelAddr>(tmp, func->symbol_reg[Reg{id}]));
            (*i)->replace_reg(Reg{id}, tmp);
          }
        } else {//普通寄存器，通过检查，发现有些地方没有进行替换
          if (cur_def || cur_use) {
            StackObject *cur_obj = spill_obj[j];
            Reg tmp{func->reg_n++,this->is_float?FP:INT};
            func->spilling_reg.insert(tmp);
            if (cur_use)
            {
                  block->insts.insert(i, make_unique<LoadStack>(cur_obj, tmp, 0));
                  // std::cout<<"what?";
            }
          
            if (cur_def)
              block->insts.insert(std::next(i),make_unique<StoreStack>(cur_obj, tmp, 0));
            (*i)->replace_reg(Reg{id}, tmp);
          }
        }
      }
      ++i;
    }
}
//构建冲突图
void SimpleColoringAllocator::build_conflict_graph() {//用来构建我们的冲突图
 //构建冲突图
  vector<int> temp, new_nodes;
  this->clear();
  if (this->is_float) {
    this->filter = [](Reg r){
      return r.is_float();
    };
  }else {
    this->filter = [](Reg r){
      return !r.is_float();
    };
  }
   //计算每个BB的live_in和live_out
  func->calc_live(this->filter);
  appear.resize(func->reg_n);//不知道有什么用，反正是虚拟寄存器的一个属性
  conflict_edges.resize(func->reg_n);//冲突图，每个元素表示相邻的虚拟寄存器编号
  //初始化这两个东西
  for(auto &i :appear)i=0;
  for(auto &i :conflict_edges)i=set<int>{};
 
 
  //这边构建冲突图的操作和那本书中的差不多，除了多了两个部分
  
  for (auto &block : func->blocks) {//遍历每一个block,应该是按照后序的顺序遍历CFG就行了
    set<Reg> live = block->live_out;//当前存活的寄存器（指的是后续还需要用到这个寄存器中的值，如果经过def后，就会被剔除）
    temp.clear();//a tool 方便，将这里面的虚拟寄存器都连接起来


  //   //将所有live中没用用于特殊用途的寄存器都放到temp中，我感觉似乎不需要这步，这个反而会导致不必要的冲突
    for (auto r : live) if (filter(r)&&(r.is_virtual() || integer_allocable(r.id))) temp.push_back(r.id);
     
      //将BB中的def的寄存器都放到temp中，因为他们和live_out
    if (block->insts.size() > 0)//如果BB中有指令
    {
       for (Reg r : (*block->insts.rbegin())->def_reg())//倒着遍历左边的寄存器
       {
           if (filter(r)&&(r.is_virtual() || integer_allocable(r.id))) temp.push_back(r.id);
       }
       
    }
     
  

    //先把def的结点和live_out连接是为了防止后面的def和前面的use连接在一起******

    //开始从BB中最后一条指令开始不断修改live，并构建冲突图
    for (auto i = block->insts.rbegin(); i != block->insts.rend(); ++i) {//倒着遍历中的BB中的所有指令
      new_nodes.clear();//作用和temp差不多
      //将def的指令从live中移除，这是由live的定义决定的
      for (Reg r : (*i)->def_reg())
        if (filter(r)&&(r.is_virtual() || integer_allocable(r.id))) {
          appear[r.id] = 1;//occur的作用是标识那些在BB中出现过的虚拟寄存器
          live.erase(r);//从live中移出
        }
      //将use加入到live中
      for (Reg r : (*i)->use_reg())
        if (filter(r)&&(r.is_virtual() || integer_allocable(r.id))) {
         
          if (live.find(r) == live.end()) {//如果live中原来没有这个寄存器
          //将这个新的寄存器和原来的live的结点连接起来并添加到live中
            for (Reg o : live) {
              conflict_edges[r.id].insert(o.id);
              conflict_edges[o.id].insert(r.id);
            }
            live.insert(r);
          }
          new_nodes.push_back(r.id);//将use的虚拟寄存器放到new_node中
          appear[r.id] = 1;//occur设置为1
        }

//让上一条指令的def寄存器和当前的指令的use连接
      if (std::next(i) != block->insts.rend())//下一条指令不是第一条指令
        for (Reg r : (*std::next(i))->def_reg())
          if (filter(r)&&(r.is_virtual() || integer_allocable(r.id))) {
            if (live.find(r) == live.end())//如果前一条指令的def不在live中；不在，所以不用移除！！等等，一个寄存器def的时候没有在live中，不是说明这个寄存器没有使用吗，难道是load?
              for (Reg o : live) {
                conflict_edges[r.id].insert(o.id);
                conflict_edges[o.id].insert(r.id);
              }
              new_nodes.push_back(r.id);//将def添加到new_node中
          }
        //对newnode进行处理，newnode之间相互连接,这个new_node似乎没有用啊
      for (size_t idx1 = 0; idx1 < new_nodes.size(); ++idx1)
        for (size_t idx0 = 0; idx0 < idx1; ++idx0)
          if (new_nodes[idx0] != new_nodes[idx1]) {
            conflict_edges[new_nodes[idx0]].insert(new_nodes[idx1]);
            conflict_edges[new_nodes[idx1]].insert(new_nodes[idx0]);
          }
    }
    //将temp中的寄存器全部连接起来，因为live_out中的寄存器会和
    for (size_t idx1 = 0; idx1 < temp.size(); ++idx1)
      for (size_t idx0 = 0; idx0 < idx1; ++idx0)
        if (temp[idx0] != temp[idx1]) {
          conflict_edges[temp[idx0]].insert(temp[idx1]);
          conflict_edges[temp[idx1]].insert(temp[idx0]);
        }
  }
}
//在冲突图中移除一个结点
void SimpleColoringAllocator::remove(int id) {//这个是删除这个结点吗
  if(id < RegCount)assert(0);
  for (auto i : conflict_edges[id]) {
    conflict_edges[i].erase(id);
    if (i >= RegCount && conflict_edges[i].size() == ALLOCABLE_REGISTER_COUNT - 1 )deletable_nodes.push(i);
  }
  remain_virual_nodes.erase(id);//***注意这步
  conflict_edges[id].clear();
}
//将冲突图中可以乱序删除的结点都删除（就是那些可以很容易着色的结点）
void SimpleColoringAllocator::delete_node() {
  while (deletable_nodes.size()) {//不断弹出simply中的结点
    int cur = deletable_nodes.front();//取出第一个结点着色点
    deletable_nodes.pop();
    vector<int> neighbors;
    //将这个结点相邻的结点保存到neighbors这个vector里面
    neighbors.reserve(conflict_edges[cur].size());
    for (int i : conflict_edges[cur]) {
      neighbors.push_back(i);
    }
    delete_history.emplace_back(cur, neighbors);//将一个结点及其邻居结点保存到history中
    remove(cur);
  }
}
//清空一切
void SimpleColoringAllocator::clear() {
  // this->func->reg_def.clear();
  // this->func->reg_use.clear();
  delete_history.clear();
  remain_virual_nodes.clear();
  appear.clear();
  conflict_edges.clear();
  deletable_nodes = std::queue<int>{};

}
//选择需要溢出的结点，就是找连接结点最多的结点
int SimpleColoringAllocator::choose_spill_node() {
  int spill_node = -1;
  //就是找到那些还没溢出，且不是constant_reg，symbol_reg寄存器的spill寄存器中连接结点最多的结点
  for (int i : remain_virual_nodes)
    if (func->spilling_reg.find(Reg{i}) == func->spilling_reg.end()&&(func->constant_reg.find(Reg{i}) != func->constant_reg.end() || func->symbol_reg.find(Reg{i}) != func->symbol_reg.end()))//不是已经spill过的
        if (spill_node == -1 ||conflict_edges[i].size() > conflict_edges[spill_node].size())//找到连接结点数最大的溢出结点
          spill_node = i;
  //上面的都找完了后就去找constant_reg和symbol_reg中的溢出的寄存器，这个部分和上个部分之后有一个被执行
  if(spill_node!=-1);
  else{
    for (int i : remain_virual_nodes)
      if (func->spilling_reg.find(Reg{i}) == func->spilling_reg.end()&&(spill_node == -1 || conflict_edges[i].size() > conflict_edges[spill_node].size()))
          spill_node = i;
  }
  assert(spill_node != -1);
  return spill_node;
}
void SimpleColoringAllocator::get_alloc_result(std::vector<int> &alloc_result)
{
  alloc_result.resize(func->reg_n);//着色方案的最终输出
  bool used_by_neighbor[RegCount] = {};//物理寄存器是否使用
  // std::fill(alloc_result.begin(), alloc_result.end(), -1);
  for(auto &i :alloc_result)i=-1;
  for (int i = 0; i < RegCount; ++i)//如果虚拟寄存器编号小于物理寄存器编号的寄存器在BB中出现过，就将其分配为对应的物理寄存器，这边应该是为了直接指定这个虚拟寄存器分配给哪个寄存器
    if (appear[i]) {
      used_by_neighbor[i] = true;
       alloc_result[i] = i;
    }
  //开始使用之前保存的history中的东西进行寄存器分配!
  for (size_t i = delete_history.size() - 1; i < delete_history.size();--i) {//从最后面开始
    assert(alloc_result[delete_history[i].first] == -1);//保证这个虚拟寄存器还没分配
    bool flag[RegCount] = {};//暂存那些物理寄存器已经被使用了
    for (int neighbor : delete_history[i].second) flag[alloc_result[neighbor]] = true;//将neibor对应的物理寄存器（就是着色方案）值为true
    for (int j = 0; j < RegCount; ++j)//遍历物理寄存器，如果发现可分配（不是保留的寄存器），而且没有使用，就将我输出为对应的物理寄存器
      if ((REGISTER_USAGE[j] == caller_save ||(REGISTER_USAGE[j] == callee_save && used_by_neighbor[j])) &&!flag[j]) {
        alloc_result[delete_history[i].first] = j;
        break;
      }
    //如果上面的过程没有中找到对应的物理寄存器  
    if(alloc_result[delete_history[i].first] != -1)
    {
       used_by_neighbor[alloc_result[delete_history[i].first]] = true;
    }
    else
    {
      for (int j = 0; j < RegCount; ++j)
        if (integer_allocable(j) && !flag[j]) {//找到那些没有被neiber使用，而且可用于分配的寄存器(应该就是直接指定的分配寄存器)，分配给这个虚拟寄存器，难道是用于优化
          alloc_result[delete_history[i].first] = j;
          break;
        }
         used_by_neighbor[alloc_result[delete_history[i].first]] = true;
    }
        
  }
}
//整个的运行流程
vector<int> SimpleColoringAllocator::run(bool *succeed,bool is_float) {

  this->is_float = is_float;
  build_conflict_graph();//构建冲突图
  for (int i = RegCount; i < func->reg_n; ++i)//遍历所有值超过寄存器个数的虚拟寄存器
  {
    if(appear[i]&&conflict_edges[i].size() < ALLOCABLE_REGISTER_COUNT)
    {
      remain_virual_nodes.insert(i);
      deletable_nodes.push(i);
    }
    else if(appear[i])
    {
      remain_virual_nodes.insert(i);
    }
  }
    
  delete_node();//这个函数的作用大致就是将simplify中的结点不断弹出，并将其周围结点保存到history中，还会在冲突图和remain_pesudo_nodes中删除自己
  if (remain_virual_nodes.size()) {//这个是最后将所有可以直接染色的结点去除后留下的无法直接染色的结点
  //无法直接染色，开始spill
    vector<int> spill_nodes;
    while (remain_virual_nodes.size()) {//移除所有移除的结点
      int cur = choose_spill_node();//选择需要处理的溢出的结点，按照书上的，应该选择的是连接节点数最大的结点，这样后面的结点才更有可能可以simplify
      spill_nodes.push_back(cur);//放到溢出结点列表中
      remove(cur);//将这些溢出的结点从冲突图中删除,同时也在remain_pesudo_nodes中删除
      delete_node();//进行溢出处理后，这个结点不再处于冲突中，这时候需要重新进行冲突图的简化（就是删除那些可以直接着色的结点）
    }
    spill(spill_nodes,this->func->all_size);//进行溢出操作
    *succeed = false;//赋值为失败，表示进行了溢出操作
    return {};//失败就返回空，表明无法直接通过着色算法进行分配，应该是让spill添加指令后，后面重新构建冲突图，然后再重新来一边，直到可以分配
  }
  else  //下面是没有溢出结点的情况,就可以给出着色的方案了
  {
      *succeed = true;
    vector<int> alloc_result;
    get_alloc_result(alloc_result);
    return alloc_result;
  }
  return {};

}

}  