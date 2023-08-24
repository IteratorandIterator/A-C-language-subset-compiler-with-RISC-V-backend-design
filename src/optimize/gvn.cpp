#include "pass.h"

using namespace Pass;

void gvn_pass::init() {
  defreg_inst.clear();
  vn_addr.clear();
  vn_arg.clear();
  intconst_vn.clear();
  vn_intconst.clear();

  floatcosnt_vn.clear();
  vn_floatconst.clear();

  vn_uop.clear();
  vn_mem_read.clear();

  vn_op.clear();
  vn_ai.clear();

  reg_vn.clear();
  vn_reg.clear();

  radd_c.clear();
  vn_call.clear();

  mp_reg.clear();

  id = 0;
  curf = nullptr;

  sudden_inst = false;

}
void gvn_pass::insert_loadconst(IrBlock* bb) {
  std::vector<int> new_hash;//保存gvn寄存器编号
    for (auto it = bb->instrs.begin(); it !=  bb->instrs.end(); ++it) {
      Instr *i = it->get();
      if(auto i0 = dynamic_cast<RegWriteInstr*>(i)) {
        int v = reg_vn[i0->d1] = get_vn(i0);//记录def寄存器对应的gvn编号

        if (sudden_inst) {//如果这个表达式已经有对应的值了，直接在指令前面插入load指令加载对应的值
          auto inst_binary = dynamic_cast<BinaryOpInstr*>(i);
          assert(inst_binary);
          if(vn_intconst_sudden.count(reg_vn[inst_binary->s2])){
            auto loadconst=new LoadConst(inst_binary->s2, vn_intconst_sudden[reg_vn[inst_binary->s2]]);
            bb->instrs.insert(it,std::unique_ptr<Instr>(loadconst));
            sudden_inst = false;
          } else assert(0);
        }
        if (vn_reg.count(v)) {//如果这个gvn编号已经有对应的寄存器了
          //assert(vn_reg[v].is_int==i0->d1.is_int);
          mp_reg[i0->d1] = vn_reg[v];//将新旧寄存器对应起来
        } else {//这个是一个新的表达式
          vn_reg[v] = i0->d1;
          new_hash.push_back(v);
          if (vn_intconst.count(v)) {//如果这个表达式已经有对应的值了，直接在指令前面插入load指令加载对应的值
            *it = std::unique_ptr<Instr>(new LoadConst(i0->d1, vn_intconst[v]));

          }
          else if (vn_floatconst.count(v)) {
            *it = std::unique_ptr<Instr>(new LoadConst(i0->d1, static_cast<float> (vn_floatconst[v])));
          }
         
        }
      }
      else if(auto i0 = dynamic_cast<BranchInstr*>(i))//针对分支语句的优化
      {

        //如果发现分支语句的cond已知已知，就直接将其转化为对应的jump指令，并在后端直接干掉
         int vn = reg_vn.at(i0->cond);
         
        if(vn_intconst.count(vn))//条件已知
        {

          int condition=vn_intconst[vn];
          if(condition==0)
          {
            *it=std::unique_ptr<Instr>(new JumpInstr(i0->target0));
            //删除另一个分支对应的BB块中有关本BB的phi指令的相关use
            i0->target1->for_each([&](Instr * inst){
                if(auto i1 = dynamic_cast<PhiInstr*>(inst))
                {
                  auto i=i1->uses.begin();
                  while(i!=i1->uses.end())
                  {
                    if((*i).second==bb)//删除丢弃分支中有关本BB的phi指令中的use
                    {
                      i=i1->uses.erase(i);

                    }
                    else
                      i++;
                  }
                }
            });
          }
          else
          {
            *it=std::unique_ptr<Instr>(new JumpInstr(i0->target1));
             //删除另一个分支对应的BB块中有关本BB的phi指令的相关use
            i0->target0->for_each([&](Instr * inst){
                if(auto i1 = dynamic_cast<PhiInstr*>(inst))
                {
                  auto i=i1->uses.begin();
                  while(i!=i1->uses.end())
                  {
                    if((*i).second==bb)//删除丢弃分支中有关本BB的phi指令中的use
                    {
                      i=i1->uses.erase(i);

                    }
                    else
                      i++;
                  }
                }
            });
          }
          // else
          // {
          //     std::cout<<condition<<std::endl;
          //     assert(0);
          // }
      

        }
      }
    }
    //按照支配树的dfs顺序递归访问
    for (IrBlock *ch : bb_dlc[bb].dom.dom_ch) {
      insert_loadconst(ch);
      ;
    }
    for (int v : new_hash) {
          vn_reg.erase(vn_reg.find(v));
    }
}
int gvn_pass::get_vn_c(int c) {
    int &v_c = intconst_vn[c];
    if (!v_c) {
      vn_intconst[v_c = ++id] = c;//保存常量
      Reg r = vn_reg[v_c] = curf->new_Reg(true);//保存常量对应的寄存器
      curf->entry->push_front(new LoadConst(r, c));
    }
    return v_c;
}
int gvn_pass::get_vn_c_f(float c) {//通过具体的数值获取对应的寄存器编号
    int &v_c = floatcosnt_vn[c];//常量和寄存器数组编号的映射关系
    if (!v_c) {//如果是0（说明没有对应的寄存器）
      vn_floatconst[v_c = ++id] = c;//将id+1并将数值放到对应的位置
      Reg r = vn_reg[v_c] = curf->new_Reg(false);

      curf->entry->push_front(new LoadConst(r, static_cast<float>(c)));//将常量的值保存到寄存器中
    }
    return v_c;
  };
int gvn_pass::get_id(int &x) {
    if (!x) x = ++id;
    return x;
  };
int gvn_pass::get_vn_uop(int s1, UnaryOp::Type type) {
    return get_id(vn_uop[std::make_pair(s1, (int)type)]);
  };
int gvn_pass::get_vn_op(int s1, int s2, BinaryOp::Type type) {
    return get_id(vn_op[std::make_tuple(s1, s2, (int)type)]);
  };
int gvn_pass::get_vn(RegWriteInstr *x0) {
   

    if(auto inst = dynamic_cast<UnaryOpInstr*>(x0)) {
      int vn1 = reg_vn.at(inst->s1);
      auto it1 = vn_intconst.find(vn1);
      if (inst->d1.is_int&& it1 != vn_intconst.end()) return get_vn_c(inst->compute(it1->second));
    
      auto itf1=vn_floatconst.find(vn1);
      if(!inst->d1.is_int&&itf1!=vn_floatconst.end()) return get_vn_c_f(inst->compute(itf1->second));

      if (inst->op.type == UnaryOp::ID) return vn1; 
      return get_vn_uop(vn1, inst->op.type);
    }
     if(auto inst = dynamic_cast<BinaryOpInstr*>(x0)) {
      int vn1 = reg_vn.at(inst->s1);
      int vn2 = reg_vn.at(inst->s2);
     if(inst->d1.is_int) {
          //如果v1已经算出来了，就换到第二个位置
          if (vn_intconst.count(vn1) && (inst->op.comm())) {//该数值已经放在了寄存器中，并且是加、乘、大小比较这样的常规操作，那么就交换两个操作数的位置，这个是针对后面的数值削减做的预备操作
            std::swap(vn1, vn2);
            std::swap(inst->s1, inst->s2);
          }
          auto it1 = vn_intconst.find(vn1);
          auto it2 = vn_intconst.find(vn2);
          bool isc1 = 0, isc2 = 0;
          int c1 = 0, c2 = 0;
          if (it1 != vn_intconst.end()) c1 = it1->second, isc1 = 1;
          if (it2 != vn_intconst.end()) c2 = it2->second, isc2 = 1;
          if (isc1 && isc2) {//两个操作数都算出来了，直接计算并返回
            int res = inst->calc(c1, c2);
            return get_vn_c(res);
          }

          // //针对浮点数比较指令的优化
          auto itf1 = vn_floatconst.find(vn1);
          auto itf2 = vn_floatconst.find(vn2);
          bool iscf1 = 0, iscf2 = 0;

          float  c1_f = 0, c2_f = 0;//****
          if (itf1 != vn_floatconst.end()) c1_f = itf1->second, iscf1 = 1;
          if (itf2 != vn_floatconst.end()) c2_f = itf2->second, iscf2 = 1;
          if (iscf1 && iscf2) {//两个操作数都是常量
            int res = inst->calc(c1_f, c2_f);
            return get_vn_c(res>0.5?1:0);
          }

            //第二个操作数可以直接算出值
          if (isc2) switch (inst->op.type) {
              case BinaryOp::SUB:  case BinaryOp::SUBW:
                c2 = -c2;
                vn2 = get_vn_c(c2);
                inst->op = BinaryOp::ADDW;
                inst->s2 = vn_reg.at(vn2);
                // (s1 - c2) = (s1 + (-c2))
              case BinaryOp::ADD: case BinaryOp::ADDW:{
                auto it = radd_c.find(vn1);
                if (it != radd_c.end()) {
                  inst->s1 = it->second.first;
                  vn1 = reg_vn.at(inst->s1);
                  c2 += it->second.second;
                  vn2 = get_vn_c(c2);
                  inst->s2 = vn_reg.at(vn2);
                }
                if (c2 == 0) return vn1; 
                int ret = get_vn_op(vn1, vn2, inst->op.type);
                radd_c[ret] = std::make_pair(inst->s1, c2);
                return ret;
              }
              case BinaryOp::MUL:   case BinaryOp::MULW: {
                if (c2 == 0) return vn2; 
                if (c2 == 1) return vn1; 
                // int ret = get_vn_op(vn1, vn2, inst->op.type);
                // radd_mul[ret] = std::make_pair(vn1, c2);
                // return ret;
              }

                break;
              case BinaryOp::DIV: case BinaryOp::DIVW:
                if (c2 == 1) return vn1; 
                break;
              default:
                break;
            }

          if(vn1 == vn2) {
            switch (inst->op.type) {
              case BinaryOp::ADD: case BinaryOp::ADDW:{
                int ret = get_vn_op(vn1, vn2, inst->op.type);
                radd_mul[ret] = std::make_pair(vn1, 2);
                return ret;
              }
            }
          }

          if(radd_mul.count(vn2)){
            std::swap(vn2,vn1);
          }

          if(radd_mul.count(vn1)) {
            if(inst->op.type == BinaryOp::ADD || inst->op.type == BinaryOp::ADDW){
              inst->op.type = BinaryOp::MULW;
              auto src1_reg = vn_reg[radd_mul[vn1].first];
              inst->s1 = src1_reg;

              int next_factor = radd_mul[vn1].second + 1;
              int vn_const = get_vn_c(next_factor);
              
              auto new_rhs = curf->new_Reg(true);
              reg_vn[new_rhs] = vn_const;
              vn_reg[vn_const] = new_rhs;

              intconst_vn_sudden[next_factor] = vn_const; 
              vn_intconst_sudden[vn_const] = next_factor;

              inst->s2 = new_rhs;

              sudden_inst = true;

              int ret = get_vn_op(vn1, vn2, inst->op.type);
              radd_mul[ret] = std::make_pair(radd_mul[vn1].first, next_factor);
            }
              

          }

     }
      
    else
    {
      //补丁，有关浮点数寄存器的情况
      if(vn_floatconst.count(vn1)&&(inst->op.comm()))
      {
          std::swap(vn1, vn2);
          std::swap(inst->s1, inst->s2);
      }
      auto itf1 = vn_floatconst.find(vn1);
      auto itf2 = vn_floatconst.find(vn2);
      bool iscf1 = 0, iscf2 = 0;

      float  c1_f = 0, c2_f = 0;//****
      if (itf1 != vn_floatconst.end()) c1_f = itf1->second, iscf1 = 1;
      if (itf2 != vn_floatconst.end()) c2_f = itf2->second, iscf2 = 1;
      if (iscf1 && iscf2) {//两个操作数都是常量
        float res = inst->calc(c1_f, c2_f);
        return get_vn_c_f(res);
      }
   
      if (iscf2) switch (inst->op.type) {
          case BinaryOp::SUB: case BinaryOp::SUBW:
            c2_f = -c2_f;
            vn2 = get_vn_c_f(c2_f);
            inst->op = BinaryOp::ADDW;
            inst->s2 = vn_reg.at(vn2);
          case BinaryOp::ADD: case BinaryOp::ADDW:{
            auto it = radd_c.find(vn1);
            if (it != radd_c.end()) {
              inst->s1 = it->second.first;
              vn1 = reg_vn.at(inst->s1);
              c2_f += it->second.second;
              vn2 = get_vn_c_f(c2_f);
              inst->s2 = vn_reg.at(vn2);
            }
            if (c2_f == 0) return vn1;
            int ret = get_vn_op(vn1, vn2, inst->op.type);
            radd_c[ret] = std::make_pair(inst->s1, c2_f);
            return ret;
          }
          case BinaryOp::MUL: case BinaryOp::MULW:
            if (c2_f == 0) return vn2; 
            if (c2_f == 1) return vn1; 
            break;
          case BinaryOp::DIV: case BinaryOp::DIVW:
            if (c2_f == 1) return vn1; 
            break;
          default:
            break;
        }

    }
      
      return get_vn_op(vn1, vn2, inst->op.type);
    }
    if(auto inst = dynamic_cast<LoadArg*>(x0)) {
       return get_id(vn_arg[inst->id]); 
       }//后续可以针对相同的参数值，对这里进行优化
    if(auto inst = dynamic_cast<LoadAddr*>(x0)) {
       return get_id(vn_addr[inst->offset]);
        }
     if(auto inst = dynamic_cast<LoadConst*>(x0)) {
      if (inst->d1.is_int) {
        get_vn_c_f(inst->value);//给float也搞一份，这是前中端没设计好的无奈之举
        return get_vn_c(inst->value);
      }
      else {
        return get_vn_c_f(inst->value_f);
      }
    }
    if(auto inst = dynamic_cast<MemRead*>(x0)) {
      int vn1 = reg_vn.at(inst->mem);
      int vn2 = reg_vn.at(inst->addr);
      return get_id(vn_mem_read[std::make_pair(vn1, vn2)]);
    }
    if(auto inst = dynamic_cast<ArrayIndex*>(x0)) {
      int vn1 = reg_vn.at(inst->s1);
      int vn2 = reg_vn.at(inst->s2);
      return get_id(vn_ai[std::make_tuple(vn1, vn2, inst->size)]);
    }
    if(auto inst = dynamic_cast<LoadInstr*>(x0)) { return ++id; }
    if(auto inst = dynamic_cast<CallInstr*>(x0)) {
      if (inst->pure) {
        std::vector<int> uses;
        for (MemUse *use : inst->in) {
          uses.push_back(reg_vn.at(use->s1));
        }
        for (auto&[r,f] : inst->args) uses.push_back(reg_vn.at(r));
        return get_id(vn_call[std::make_pair(inst->f, uses)]);
      }
      return ++id;
    }
    if(auto inst = dynamic_cast<PhiInstr*>(x0)) { return ++id; }
    if(auto inst = dynamic_cast<Convert*>(x0)){
      int vn2 = reg_vn.at(inst->src);
      auto it = vn_intconst.find(vn2);
      auto itf =vn_floatconst.find(vn2);
      if (it != vn_intconst.end()) {
          // return get_id(floatcosnt_vn[static_cast<float>(it->second)]);
          return get_vn_c_f(static_cast<float>(it->second));
      }
      if (itf != vn_floatconst.end()) {
          // return get_id(intconst_vn[static_cast<int>(itf->second)]);
          return get_vn_c(static_cast<int>(itf->second));
      }
    }
    
    return ++id;
  };
void gvn_pass::reg2reg()
{
  
  auto op_phi=[&](IrBlock *bb) {
    std::map<std::vector<std::pair<Reg, IrBlock *>>, Reg> mp_phi;
    for (auto it = bb->instrs.begin(); it != bb->instrs.end(); ++it) {
      if(auto x0 = dynamic_cast<PhiInstr*>(it->get())) {
        Reg r0;
        int cnt = 0, flag = 1;
        x0->map_use([&](Reg &r) {
          if (!cnt++)
            r0 = r;//目标寄存器
          else
            flag &= (r0 == r);
        });
        if (flag && cnt >= 1) {//如果phi指令中有多个源寄存器，并且全部都是同一个寄存器
        assert(x0->d1.is_int==r0.is_int);
          mp_reg[x0->d1] = r0;//直接将目标寄存器和源寄存器对应起来
        }
       
        else {//根据phi指令的uses列表对目标寄存器进行缓存
          Reg &r = mp_phi[x0->uses];//找到相同的uses对应的其他phi指令的目标reg
          if (r.id)
          {
            assert(x0->d1.is_int==r.is_int);
              mp_reg[x0->d1] = r;//直接将目标寄存器和已有的寄存器对应起来
          }
         
          else
            r = x0->d1;
        }
      }
    }
  };
  for (auto &bb :  curf->bbs) op_phi(bb.get());
}
void gvn_pass::do_gvn(UserFunction* f){
  init();
  curf = f;
  defreg_inst = map_defreg_inst(f);
  bb_dlc = build_info_node(f);
  insert_loadconst(f->entry);//构建gvn的信息，并在可以计算出结果的寄存器前面插入loadconst指令
  reg2reg();//获取值相同的寄存器
  //根据map reg2reg的关系进行寄存器换名
  map_use(f, mp_reg);
  //移除没有使用的指令
  repeat(dce_BB)(f);
  remove_unused_def(f);
}

void gvn_pass::run(){
  for(auto & [_,f]: c->funcs) do_gvn(f.get());
  remove_unused_memobj(*c);
}