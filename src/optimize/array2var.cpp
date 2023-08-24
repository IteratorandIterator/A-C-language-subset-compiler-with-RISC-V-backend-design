
#include "pass.h"
#include "algorithm"
#define Case(T, a, b) if (auto a = dynamic_cast<T *>(b))
void Pass::array2var::run()
{
  for(auto & [_,f]: c->funcs) _array2var(f.get());
  remove_unused_memobj(*c);
}

void Pass::array2var::init() {
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

  vn_call.clear();

  mp_reg.clear();

  id = 0;
  curf = nullptr;



}

bool Pass::array2var::check_fun(UserFunction *f)
{
  bool has_array=false;
  bool has_no_array_fun=true;
  bool has_no_global=true;
  //如果函数中有有关数组的函数调用，则，不进行优化
  f->for_each([&](Instr * inst){
    Case(CallInstr,i1,inst){//针对call指令
        auto fun_in_call=i1->f;
      Case(UserFunction,fun,fun_in_call){
        //判断是否有数组参数,如果有，直接放弃优化
        if(fun->scope.array_arg_id.size())
          has_no_array_fun=false;
      
    }
    }
    Case(LoadAddr,i2,inst){
       if(i2->offset->global)
        has_no_global=false;
      
       else if(i2->offset->dims.size()) 
        has_array=true;
    }
   
  });
  
  return has_array&&has_no_array_fun&&has_no_global;


}
void Pass::array2var::delete_memwrite(UserFunction *f)
{
    // std::cout<<"delete unused store"<<std::endl;
if(can_delete)
    for(auto &bb :f->bbs)
    {
      for(auto &inst: bb->instrs)
      {
         Case(StoreInstr,x1,inst.get()){
            if(std::find(deletable_store.begin(),deletable_store.end(),inst.get())!=deletable_store.end())
            {//删除这条指令
              inst.reset(new UnaryOpInstr(f->new_Reg(x1->s1.is_int),x1->s1,UnaryOp::ID));//写个没用的东西然后面自己删
            }
        }

      }
    }
        // Case(StoreInstr,x1,inst){
        //     if(std::find(deletable_store.begin(),deletable_store.end(),inst)!=deletable_store.end())
        //     {//删除这条指令
        //       inst=new UnaryOpInstr(x1->s1,x1->s1,UnaryOp::ID);//写个没用的东西然后面自己删
        //     }
        // }

}
int Pass::array2var::getarrayindex(std::pair<Reg, int> bo)
{
  //需要将实现array_vn和vn_array的双向保存
  int &vn=arrayindex_vn[bo];
  if(!vn)//如果没有这个基地址和偏移地址对，给一个新的gvn编号
  {
    vn=++id;
    vn_arrayindex[vn]=bo;
    arrayindex_vn[bo]=vn;
  }
  return vn;
}

bool Pass::array2var::checkaddr(IrBlock* bb) {
  std::vector<int> new_hash;//保存gvn寄存器编号
    for (auto it = bb->instrs.begin(); it !=  bb->instrs.end(); ++it) {
      Instr *i = it->get();

      if(auto i0 = dynamic_cast<RegWriteInstr*>(i)) {
        // ir_serializer irprinter(std::cout);
        // i0->accept(irprinter);
        // std::cout<<"\n";
        int v = reg_vn[i0->d1] = get_vn(i0);//记录def寄存器对应的gvn编号
        if(!succeed) 
        {
         
           return false;
        }
           
        if(load2loadconst.available)//将load指令转化为loadconst指令
        {
           if(load2loadconst.is_int)
           {
             *it = std::unique_ptr<Instr>(new LoadConst(i0->d1, load2loadconst.int_value));
             
           }
            load2loadconst.available=false;
        }
        if (vn_reg.count(v)) {//如果这个gvn编号已经有对应的寄存器了
          //assert(vn_reg[v].is_int==i0->d1.is_int);
          mp_reg[i0->d1] = vn_reg[v];//将新旧寄存器对应起来
        } else {//这个是一个新的表达式
          vn_reg[v] = i0->d1;
          new_hash.push_back(v);
          if (vn_intconst.count(v)) {//如果这个表达式已经有对应的值了，直接在指令替换为loadconst
            *it = std::unique_ptr<Instr>(new LoadConst(i0->d1, vn_intconst[v]));
          }
          else if (vn_floatconst.count(v)) {
            *it = std::unique_ptr<Instr>(new LoadConst(i0->d1, static_cast<float> (vn_floatconst[v])));
          }
         
        }
      }
       else if(auto inst = dynamic_cast<StoreInstr*>(i)) { 
      //已经确保地址是已知的,要么就是经过了load addr的考验，但是gvn带来了变数
      

      int vn=arrayaddr_vn[inst->addr];
      if(inst->s1.is_int)//先只管int
      {
        //打个小布丁，将数组名字转化为第一个参数
        if(vn_arrayindex[vn].second<0)
        {
            // std::cout<<new_addr<<std::endl;
            auto new_bo=std::make_pair(vn_arrayindex[vn].first,0);//这里的数据类型都是4B的，所以这样写
            vn=getarrayindex(new_bo);//如果有对应的对，直接返回对应的vn，如果没有，就创建一个新的gvn编号
        }
        // std::cout<<"store"<<std::endl;
        // std::cout<<vn<<std::endl;
        
        //写入的值是已知的，更新数据
        if(vn_intconst.count(reg_vn[inst->s1]))
        {
            int const_int_value=vn_intconst[reg_vn[inst->s1]];
            
            arrayindexval_int_vn[const_int_value]=vn;
            vn_arrayindexval_int[vn]=const_int_value;
            deletable_store.push_back(i);//可以删除
        }
        else//写入的值未知，放弃优化
        {
          succeed=false;
          return false;
        }
        
      }
      }
    }

    //按照支配树的dfs顺序递归访问
    bool is_succeed=true;
    for (IrBlock *ch : bb_dlc[bb].dom.dom_ch) {
 
      if(!checkaddr(ch))
      {
        is_succeed=false;
        break;
      }
    }
    for (int v : new_hash) {
          vn_reg.erase(vn_reg.find(v));
    }

    return is_succeed;
}
int  Pass::array2var::get_vn_c(int c) {
    int &v_c = intconst_vn[c];
    if (!v_c) {
      vn_intconst[v_c = ++id] = c;//保存常量
      Reg r = vn_reg[v_c] = curf->new_Reg(true);//保存常量对应的寄存器
      curf->entry->push_front(new LoadConst(r, c));
    }
    return v_c;
}
int  Pass::array2var::get_vn_c_f(float c) {//通过具体的数值获取对应的寄存器编号
    int &v_c = floatcosnt_vn[c];//常量和寄存器数组编号的映射关系
    if (!v_c) {//如果是0（说明没有对应的寄存器）
      vn_floatconst[v_c = ++id] = c;//将id+1并将数值放到对应的位置
      Reg r = vn_reg[v_c] = curf->new_Reg(false);

      curf->entry->push_front(new LoadConst(r, static_cast<float>(c)));//将常量的值保存到寄存器中
    }
    return v_c;
  };
int  Pass::array2var::get_id(int &x) {
    if (!x) x = ++id;
    return x;
  };
int  Pass::array2var::get_vn_uop(int s1, UnaryOp::Type type) {
    return get_id(vn_uop[std::make_pair(s1, (int)type)]);
  };
int  Pass::array2var::get_vn_op(int s1, int s2, BinaryOp::Type type) {
    return get_id(vn_op[std::make_tuple(s1, s2, (int)type)]);
  };
int  Pass::array2var::get_vn(RegWriteInstr *x0) {
   

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
 //有关数组地址东西
      if(arrayaddr_vn.count(inst->s2) && inst->op.type == BinaryOp::ADD) //地址必须是64位加法
      {
        std::swap(inst->s1, inst->s2);
      } 
      if(arrayaddr_vn.count(inst->s1)&& inst->op.type == BinaryOp::ADD)//某个寄存器里面放的是地址
      {
        //判断第二个操作数是不是常数
        auto it2 = vn_intconst.find(vn2);
        if(it2!=vn_intconst.end())
        {
            
            int vn_pre_addr=arrayaddr_vn[inst->s1];
            //获取其对应的<base,offset>
            auto bo=vn_arrayindex[vn_pre_addr];
            //计算新的<base,offset>
            int new_addr=bo.second+vn_intconst[vn2];
            if(bo.second<0)new_addr=vn_intconst[vn2];
            auto new_bo=std::make_pair(bo.first,new_addr);//这里的数据类型都是4B的，所以这样写
            int vn=getarrayindex(new_bo);//如果有对应的对，直接返回对应的vn，如果没有，就创建一个新的gvn编号
            //更新到vn和arrayaddr的对应关系中
            vn_arrayaddr[vn]=inst->d1;
            arrayaddr_vn[inst->d1]=vn;
            
        }
        else //计算出来的地址未知，直接放弃优化
            succeed=false;

      }

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
               
                if (c2 == 0) return vn1; 
                int ret = get_vn_op(vn1, vn2, inst->op.type);
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
            if (c2_f == 0) return vn1;
            int ret = get_vn_op(vn1, vn2, inst->op.type);
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
       int vn=getarrayindex(std::make_pair(inst->d1,-4));//如果有，返回对应的vn，如果没有返回一个新的vn
        //更新到vn和arrayaddr的对应关系中
        vn_arrayaddr[vn]=inst->d1;
        arrayaddr_vn[inst->d1]=vn;
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
    if(auto inst = dynamic_cast<ArrayIndex*>(x0)) {
      int vn1 = reg_vn.at(inst->s1);
      int vn2 = reg_vn.at(inst->s2);

      //差点把这里搞漏了
      auto it2=vn_intconst.find(vn2);
        if(it2!=vn_intconst.end())//如果偏移地址是常数
        {
            
            int vn_pre_addr=arrayaddr_vn[inst->s1];
            //获取其对应的<base,offset>
            auto bo=vn_arrayindex[vn_pre_addr];
            //计算新的<base,offset>
            int new_addr=bo.second+vn_intconst[vn2]*inst->size;
            if(bo.second<0)new_addr=vn_intconst[vn2]*inst->size;
            //  std::cout<<"addr:"<<std::endl;
            // std::cout<<new_addr<<std::endl;
            auto new_bo=std::make_pair(bo.first,new_addr);//这里的数据类型都是4B的，所以这样写
            int vn=getarrayindex(new_bo);//如果有对应的对，直接返回对应的vn，如果没有，就创建一个新的gvn编号
            //更新到vn和arrayaddr的对应关系中
            vn_arrayaddr[vn]=inst->d1;
            arrayaddr_vn[inst->d1]=vn;
            
        }
        else //偏移地址未知，直接放弃优化
            succeed=false;


      return get_id(vn_ai[std::make_tuple(vn1, vn2, inst->size)]);
    }
   
    if(auto inst = dynamic_cast<CallInstr*>(x0)) {
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
     if(auto inst = dynamic_cast<LoadInstr*>(x0)) { 
      //如果是不确定读，直接放弃删除写操作
      if(!arrayaddr_vn.count(inst->addr))
      {
        can_delete=false;
        return ++id;
      }
      //获取地址对应的gvn编号
      // std::cout<<"load"<<std::endl;
      int vn=arrayaddr_vn[inst->addr];
      // std::cout<<vn<<std::endl;
      //打个小布丁，将数组名字转化为第一个参数
        if(vn_arrayindex[vn].second<0)
        {
            
            auto new_bo=std::make_pair(vn_arrayindex[vn].first,0);//这里的数据类型都是4B的，所以这样写
            vn=getarrayindex(new_bo);//如果有对应的对，直接返回对应的vn，如果没有，就创建一个新的gvn编号
        }
      //先只管int
      if(vn_arrayindexval_int.count(vn))
      {
          load2loadconst.available=true;
          load2loadconst.is_int=true;
          load2loadconst.int_value=vn_arrayindexval_int[vn];
      }
      return ++id; 
      }
   
    
    return ++id;
  };



void Pass::array2var::_array2var( UserFunction *f)
{
  init();
  curf = f;
  defreg_inst = map_defreg_inst(f);
  bb_dlc = build_info_node(f);
  if(!check_fun(f))return;
  if(checkaddr(f->entry))
    delete_memwrite(f);
  else 
    return ;

  remove_unused_def(f);
}