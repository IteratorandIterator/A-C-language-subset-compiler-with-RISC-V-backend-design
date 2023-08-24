#include "program_builder.h"
#include "inst.h"
// using namespace Medium;
using namespace Archriscv;
using std::make_unique;
void program_visitor::visit(Medium::Reg &node) {
    
}
void program_visitor::visit(Medium::Variable &node){
    
}
void program_visitor::visit(Medium::MediumScope &node){
    
}
void program_visitor::visit(Medium::IrBlock &node){
  for (auto &i : node.instrs) 
    i.get()->accept(*this);
}
void program_visitor::visit(Medium::IrFunction &node){
    
}
void program_visitor::visit(Medium::LibFunction &node){
    
}
void program_visitor::visit(Medium::UserFunction &node){
  this->curf = prog->functions.back().get();
  //传递寄存器的大小
  for (int i=0;i<node.reg_names.size();i++)
    curf->all_size.push_back(node.reg_names[i].second);
  Medium2Backend info;
  int lenth = node.scope.objects.size();
  for (size_t i = 0; i < lenth; ++i) {
    Medium::Variable *cur = node.scope.objects[i].get();
    if (cur->size == 0) continue;
    std::unique_ptr< StackObject> res = std::make_unique<StackObject>(cur->size);  //对于每一个在函数中的memobj，都变成一个stackobj
    info.obj_mapping[cur] = res.get();  //把两个对象建立映射
    curf->stack_objects.push_back(std::move(res)); //把对象加入到列表
  }
  curf->entry = new Block(".entry_" + func_name); //创建一个后端的block
  curf->blocks.emplace_back(curf->entry);  
  for (size_t i = 0; i < node.bbs.size(); ++i) {  //构造后端的block
    Medium::IrBlock *cur = node.bbs[i].get();
    std::string cur_name = ".L" + std::to_string(prog->block_n++);
    std::unique_ptr<Block> res = std::make_unique< Block>(cur_name);
    info.block_mapping[cur] = res.get();
    info.rev_block_mapping[res.get()] = cur;
    curf->blocks.push_back(std::move(res));
  }
  //处理参数
  int arg_n = 0;  //计算参数个数
  for (auto &bb : node.bbs)
    for (auto &inst : bb->instrs)
      if (auto *cur = dynamic_cast<Medium::LoadArg *>(inst.get()))
        arg_n = std::max(arg_n, cur->id + 1);

  for (int i = 0; i < arg_n; ++i) {
    int arg_type=node.arg_info[i];////0:int ,float:1,address:3
    Reg cur_arg;
    switch (arg_type) {
     case 0: cur_arg= info.new_backend_reg(false);
      break;
     case 1: cur_arg = info.new_backend_reg(true);
      break;
     case 2: cur_arg = info.new_backend_reg(false);
      break;
     default: assert(0);
      break;
    }
    if (i <  ARGUMENT_REGISTER_COUNT) {
      curf->entry->append(std::make_unique<Move>(cur_arg, Reg{ ARGUMENT_REGISTERS[i],cur_arg.is_float()? FP: INT},cur_arg.is_float()==true? Move::FLOATMOV: Move::INTMOV));
    } else {
      auto reg_memobj = node.scope.array_args;
      int size_arg = 4;
      if(reg_memobj.find(i) != reg_memobj.end()){
        size_arg = 8;
      }
      std::unique_ptr< StackObject> t = std::make_unique< StackObject>(size_arg);
      curf->entry->append(std::make_unique< LoadStack>(t.get(), cur_arg, 0));
      curf->caller_stack_object.push_back(std::move(t));
    }
    curf->arg_reg.push_back(cur_arg);
  }
  //实际入口块和函数入口块不一致，添加一条jump指令
  Block *real_entry = info.block_mapping[node.entry];
  if (curf->blocks[1].get() != real_entry)
    curf->entry->append(std::make_unique< Jump>(real_entry));
  curf->entry->out_edge.push_back(real_entry);
  real_entry->in_edge.push_back(curf->entry);
  //构建后端块的内容，并且把前后的块进行连接
  this->cmp_info.clear();
  for (size_t i = 0; i < curf->blocks.size(); ++i)
    if (curf->blocks[i].get() != curf->entry) {
      Medium::IrBlock *cur_ir_bb = info.rev_block_mapping[curf->blocks[i].get()];
      this->inner_info = &info;
      this->next_block = nullptr;
      if (i + 1 < curf->blocks.size()) this->next_block = curf->blocks[i + 1].get();
      this->cur_block = curf->blocks[i].get();
      cur_ir_bb->accept(*this);
    }
  //讲phi指令进行转化
  struct PendingMove {
     Block *block;
     Reg to, from;
  };
  std::vector<PendingMove> pending_moves;
  for (auto &bb : node.bbs)
    for (auto &inst : bb->instrs)
      if (auto *cur = dynamic_cast<Medium::PhiInstr *>(inst.get()))
        for (auto &prev : cur->uses) {
           Block *b = inner_info->block_mapping[prev.second];
           Reg temp = inner_info->reg_medium2backend(prev.first);
           Reg mid = inner_info->new_backend_reg(temp.is_float());
          b->insert_before_jump(
              std::make_unique< Move>(mid, temp,mid.is_float()==true? Move::FLOATMOV: Move::INTMOV));
          pending_moves.push_back({b, inner_info->reg_medium2backend(Medium::Reg{cur->d1.id,cur->d1.is_int,4}), mid});
        }
  for (PendingMove &i : pending_moves)
    i.block->insert_before_jump(std::make_unique< Move>(i.to, i.from,i.to.is_float()==true? Move::FLOATMOV: Move::INTMOV));
  curf->reg_n = info.reg_n;
}
void program_visitor::visit(Medium::CompileUnit &node){
  prog = new Program;
  for (auto &[name,f] : node.funcs) {
    this->func_name = name;
    prog->functions.push_back(std::make_unique<Function>(func_name));
    f.get()->accept(*this);
  }
  for (size_t i = 0; i < node.scope.objects.size(); ++i) {//获取变量，这个时候应该获取的全局变量
    Medium::Variable *cur = node.scope.objects[i].get();
    if (cur->size == 0) continue; //考察占用内存的情况，如果为0，什么也不干
    std::unique_ptr<GlobalObject> res = make_unique<GlobalObject>();//全局对象
    res->name = generate_name(cur->name);
    res->size = cur->size;//申请空间的大小
    res->init = cur->initial_value;//初始化列表
    res->is_int = cur->is_int;//是不是整形
    res->is_const = cur->is_const;//是不是const
    res->is_float = cur->is_float;
    prog->global_objects.push_back(std::move(res));
  }
  for (auto & f : prog->functions)
    for (auto & b : f->blocks)
      for (auto & var : b->global_objects)
        prog->global_objects.push_back(std::move(var));
}
void program_visitor::visit(Medium::UnaryOp &node){
    
}
void program_visitor::visit(Medium::BinaryOp &node){
    
}
void program_visitor::visit(Medium::ArrayIndex &node){
  Reg dst = inner_info->reg_medium2backend(node.d1),
      s1 = inner_info->reg_medium2backend(node.s1),
      s2 = inner_info->reg_medium2backend(node.s2),
      size = inner_info->new_backend_reg(),
      mid = inner_info->new_backend_reg();
  cur_block->append(make_unique<LoadImm>(size, node.size));
  assert(!s2.is_float());//抽象，中端传了一个浮点数的数组维度过来
  cur_block->append(make_unique<RegRegInst>(RegRegInst::Mul, mid, s2, size));
  cur_block->append(make_unique<RegRegInst>(RegRegInst::Add, dst, s1, mid));
  curf->constant_reg[size] = node.size;
}
void program_visitor::visit(Medium::LoadAddr &node){
  Reg dst = inner_info->reg_medium2backend(node.d1);
   if (node.offset->global) {//全局变量
     cur_block->append(std::make_unique< LoadLabelAddr>(
         dst, generate_name(node.offset->name)));
     curf->symbol_reg[dst] = generate_name(node.offset->name);
   } else {//普通变量
     cur_block->append(std::make_unique< LoadStackAddr>(
         inner_info->obj_mapping[node.offset], dst, 0));//offset放后面算
     curf->stack_addr_reg[dst] = std::pair< StackObject *, int32_t>{
         inner_info->obj_mapping[node.offset], 0};//reg，memobject,不知道
   }
}
void program_visitor::visit(Medium::LoadConst &node){
  Reg dst = inner_info->reg_medium2backend(node.d1);
    // if (node.value == 0 && dst.type == INT) {//这坨代码有问题，如果return指令的解析在这个loadconst之前，就会导致return语句的寄存器映射变化，但是return那边不知道，于是就出现了错误的返回值的情况
    //   // inner_info->reg_mapping[node.d1.id] = Reg{zero};
    //   std::cout<<"load const"<<std::endl;
    //    std::cout<<node.d1.id<<"->"<<Reg{zero}<<std::endl;
    // }
    // else {
      if (dst.type == FP) {
          std::unique_ptr<GlobalObject> res = std::make_unique<GlobalObject>();
          res->name = "."+generate_name(std::to_string(dst.id) + curf->name);
          res->size = 4;
          res->init = &(node.value_f);
          res->is_int = true;
          res->is_const = true;
          cur_block->global_objects.push_back(std::move(res));
          curf->constant_reg_float[dst] = node.value_f;
        Reg reg_base = inner_info->new_backend_reg(false);
        auto instLui = std::make_unique<LuiInst>( reg_base,"."+generate_name(std::to_string(dst.id) + curf->name ) );
        auto instflw = std::make_unique<FlwInst>(dst,reg_base,"."+generate_name(std::to_string(dst.id) + curf->name ));
        cur_block->append(std::move(instLui));
        cur_block->append(std::move(instflw));
        
      }else{
        cur_block->append(std::make_unique<LoadImm>(dst, node.value));
        curf->constant_reg[dst] = node.value;
      }
    // }
}
void program_visitor::visit(Medium::LoadArg &node){  
  auto src = inner_info->reg_medium2backend(node.d1);
  auto dst = curf->arg_reg[node.id];
  cur_block->append(std::make_unique<Move>(src, dst,src.is_float()==true?Archriscv::Move::FLOATMOV:Archriscv::Move::INTMOV));
}
void program_visitor::visit(Medium::UnaryOpInstr &node){ 
  Reg dst = inner_info->reg_medium2backend(node.d1),
        src = inner_info->reg_medium2backend(node.s1);
    switch (node.op.type) {
      case Medium::UnaryOp::LNOT:
        cur_block->append(std::make_unique<RegImmInst>(RegImmInst::Sltiu, dst, src, 1));
        break;
      case Medium::UnaryOp::NEG:
      //负数的逻辑没有改
        if(dst.is_float()) {
          auto zero_f = inner_info->new_backend_reg(true);
          cur_block->append( std::make_unique<Convert>(Convert::Fcvtsw,zero_f,Reg{zero}));
          cur_block->append(std::make_unique<RegRegInst>(RegRegInst::Fsub, dst, zero_f, src));
        }
        else
            cur_block->append(std::make_unique<RegRegInst>(RegRegInst::Sub, dst, Reg{zero}, src));
        break;
      case Medium::UnaryOp::ID:
        cur_block->append(std::make_unique<Move>(dst, src,dst.is_float()==true?Archriscv::Move::FLOATMOV:Archriscv::Move::INTMOV));
        break;
    }
}
void program_visitor::visit(Medium::BinaryOpInstr &node){
  auto info = inner_info;
  auto binary = &node;
  auto func = curf;
  Reg dst = info->reg_medium2backend(binary->d1),
        s1 = info->reg_medium2backend(binary->s1),
        s2 = info->reg_medium2backend(binary->s2);
    if (binary->op.type == Medium::BinaryOp::ADD ||
        binary->op.type == Medium::BinaryOp::SUB ||
        binary->op.type == Medium::BinaryOp::MUL ||
        binary->op.type == Medium::BinaryOp::DIV ||
        binary->op.type == Medium::BinaryOp::MOD ||
         binary->op.type == Medium::BinaryOp::ADDW ||
        binary->op.type == Medium::BinaryOp::SUBW ||
        binary->op.type == Medium::BinaryOp::MULW ||
        binary->op.type == Medium::BinaryOp::DIVW ||
        binary->op.type == Medium::BinaryOp::MODW
        ) {
          auto _op=RegRegInst::from_ir_binary_op(binary->op.type);
          if(!dst.is_float())
              cur_block->append(make_unique<RegRegInst>(_op, dst, s1, s2));
          else
          {
             if(_op==Archriscv::RegRegInst::Add||_op==Archriscv::RegRegInst::Addw)
              _op=Archriscv::RegRegInst::Fadd;
            else if(_op==Archriscv::RegRegInst::Sub||_op==Archriscv::RegRegInst::Subw)
              _op=Archriscv::RegRegInst::Fsub;
            else if(_op==Archriscv::RegRegInst::Mul||_op==Archriscv::RegRegInst::Mulw)
              _op=Archriscv::RegRegInst::Fmul;
            else if(_op==Archriscv::RegRegInst::Div||_op==Archriscv::RegRegInst::Divw)
              _op=Archriscv::RegRegInst::Fdiv;
             cur_block->append(make_unique<RegRegInst>(_op, dst, s1, s2));
          }
    } else {
      Compare c;
      switch (binary->op.type) {
        case Medium::BinaryOp::LESS:
          c = Lt;
          break;
        case Medium::BinaryOp::LEQ:
          c = Le;
          break;
        case Medium::BinaryOp::EQ:
          c = Eq;
          break;
        case Medium::BinaryOp::NEQ:
          c = Ne;
          break;
        default:
          assert(0);
      }
      // push_back(make_unique<VirtualDefPoint>(dst));
      auto bintype=RegRegInst::from_ir_binary_op(binary->op.type);
      if(bintype==RegRegInst::Sle)
      {
          if(s1.is_float())
            cur_block->append(make_unique<RegRegInst>(RegRegInst::Fgt,dst , s1, s2));
          else
            cur_block->append(make_unique<RegRegInst>(RegRegInst::Sgt,dst , s1, s2));
          cur_block->append(make_unique<RegImmInst>(RegImmInst::Xori, dst, dst, 1));
      }
      else if(bintype==RegRegInst::Seq)
      {
         if(!s1.is_float())
          {
          cur_block->append(make_unique<RegRegInst>(RegRegInst::Sub,dst , s1, s2));
          cur_block->append(make_unique<RegImmInst>(RegImmInst::Seqz, dst, dst, 0));
          }
            else
          {
             auto tmp1=info->new_backend_reg(true);
             auto tmp2=info->new_backend_reg(true);
             cur_block->append(make_unique<RegRegInst>(RegRegInst::Sub,tmp1 , s1, s2));
             cur_block->append(make_unique<Move>(tmp2,Reg{0},Archriscv::Move::FLOATMOVX));
            cur_block->append(make_unique<RegRegInst>(RegRegInst::Feq, dst, tmp2, tmp1));
          }
      }
      else if(bintype==RegRegInst::Sne)
      {
          if(!s1.is_float())
          {
             cur_block->append(make_unique<RegRegInst>(RegRegInst::Sub,dst , s1, s2));
            cur_block->append(make_unique<RegImmInst>(RegImmInst::Snez, dst, dst, 0));
          }
         
          else
          {
             auto tmp1=info->new_backend_reg(true);
             auto tmp2=info->new_backend_reg(true);
             cur_block->append(make_unique<RegRegInst>(RegRegInst::Sub,tmp1 , s1, s2));
             cur_block->append(make_unique<Move>(tmp2,Reg{0},Archriscv::Move::FLOATMOVX));
            cur_block->append(make_unique<RegRegInst>(RegRegInst::Feq, dst, tmp2, tmp1));
            cur_block->append(make_unique<RegImmInst>(RegImmInst::Xori, dst, dst, 1));
          }
      }
      //我先注释了你！
      else if(!s1.is_float())
        cur_block->append(std::make_unique<RegRegInst>(bintype, dst, s1, s2));
      cmp_info[dst] = CmpInfo{.type = c, .lhs = s1, .rhs = s2};
    }

}
void program_visitor::visit(Medium::LoadInstr &node){
  Reg dst = inner_info->reg_medium2backend(node.d1),
        addr = inner_info->reg_medium2backend(node.addr);
    cur_block->append(make_unique<Load>(dst, addr, 0,dst.is_float()?Archriscv::Load::Flw:Archriscv::Load::Lw));
}
void program_visitor::visit(Medium::StoreInstr &node){
  Reg addr = inner_info->reg_medium2backend(node.addr),
        src = inner_info->reg_medium2backend(node.s1);
  cur_block->append(make_unique<Store>(src, addr, 0,src.is_float()?Archriscv::Store::Fsw:Archriscv::Store::Sw));
}
void program_visitor::visit(Medium::JumpInstr &node){
  Block *jump_target = inner_info->block_mapping[node.target];
  if (jump_target != next_block)
    cur_block->append(make_unique<Jump>(jump_target));
  cur_block->out_edge.push_back(jump_target);
  jump_target->in_edge.push_back(cur_block);
}
void program_visitor::visit(Medium::BranchInstr &node){
  auto branch = &node;
  CmpInfo cond;
  Block *true_target = inner_info->block_mapping[branch->target1],
        *false_target = inner_info->block_mapping[branch->target0];
  if (cmp_info.find(inner_info->reg_medium2backend(branch->cond)) != cmp_info.end()) {
    cond = cmp_info[inner_info->reg_medium2backend(branch->cond)];
  } else {
    cond.type = Ne;
    cond.lhs = inner_info->reg_medium2backend(branch->cond);
    cond.rhs = Reg{zero};
  }
  if (false_target == next_block) {
    if(cond.lhs.is_float())
      cur_block->append(make_unique<FBranch>(true_target, inner_info->new_backend_reg(),cond.lhs, cond.rhs, cond.type));
    else
      cur_block->append(make_unique<Branch>(true_target, cond.lhs, cond.rhs, cond.type));
  } else if (true_target == next_block) {
     if(cond.lhs.is_float())
      cur_block->append(make_unique<FBranch>(false_target,inner_info->new_backend_reg(), cond.lhs, cond.rhs,logical_not(cond.type)));
     else
      cur_block->append(make_unique<Branch>(false_target, cond.lhs, cond.rhs,logical_not(cond.type)));
  } else {//****
  if(cond.lhs.is_float())
    cur_block->append(make_unique<FBranch>(true_target, inner_info->new_backend_reg(),cond.lhs, cond.rhs, cond.type));
  else
    cur_block->append(make_unique<Branch>(true_target, cond.lhs, cond.rhs, cond.type));
  cur_block->append(make_unique<Jump>(false_target));
  }
  cur_block->out_edge.push_back(false_target);
  cur_block->out_edge.push_back(true_target);
  false_target->in_edge.push_back(cur_block);
  true_target->in_edge.push_back(cur_block);
}
void program_visitor::visit(Medium::ReturnInstr &node){
  auto ret = &node;
  if (ret->ignore_return_value) {
      cur_block->append(make_unique<Return>(false));
    } 
    else {
      
      auto src = inner_info->reg_medium2backend(ret->s1);
      // std::cout<<"return"<<std::endl;
      // std::cout<<ret->s1.id<<"->"<<src.id<<std::endl;
      if (src.is_float()) {
        auto dst = Reg{ARGUMENT_REGISTERS[0],Archriscv::FP};
        cur_block->append(make_unique<Move>(dst,
          src,Archriscv::Move::FLOATMOV
        ));
      }else {
        auto dst = Reg{ARGUMENT_REGISTERS[0]};
        cur_block->append(make_unique<Move>(dst,src,Archriscv::Move::INTMOV));
      }
      cur_block->append(make_unique<Return>(true));
    }
}
void program_visitor::visit(Medium::CallInstr &node){
  auto call = &node;
  if (call->args.size() > ARGUMENT_REGISTER_COUNT) {
      int32_t sp_move = 0;
      for (int i = ARGUMENT_REGISTER_COUNT; i < call->args.size(); i++) {
        sp_move += call->args[i].second ? INT_SIZE * 2 : INT_SIZE;
      }
      if(sp_move%8!=0)sp_move+=4;
      for (size_t i = 0; i < ARGUMENT_REGISTER_COUNT; ++i) {
        auto res_reg = inner_info->reg_medium2backend(call->args[i].first);
        auto dst=Reg{ARGUMENT_REGISTERS[i],res_reg.type};//为什么这里没有类型判断？害，就是忘加了
        cur_block->append(make_unique<Move>(dst,
                                    inner_info->reg_medium2backend(call->args[i].first),dst.is_float()==true?Archriscv::Move::FLOATMOV:Archriscv::Move::INTMOV));
      }
       int32_t sp_move_iter=sp_move;
       while(sp_move_iter>=1024) {
         cur_block->append(make_unique<MoveSP>(-1024));
         sp_move_iter-=1024;
       }
       if(sp_move_iter>0)cur_block->append(make_unique<MoveSP>(-sp_move_iter));
      int32_t pos = 0;
      Archriscv::Reg tmp_addr_reg;
      int value_tmp=0;
      if (sp_move > IMM12_R || -sp_move < IMM12_L) {
       tmp_addr_reg=inner_info->new_backend_reg(false);//用于放置一部分的数据
       cur_block->append(make_unique<Move>(tmp_addr_reg, Reg{sp}));
      }
      for (size_t i = ARGUMENT_REGISTER_COUNT; i < call->args.size(); ++i) {//处理溢出的指令
        auto src=inner_info->reg_medium2backend(call->args[i].first);
        bool flag_addr = call->args[i].second;
        if(pos>1024){
          if(pos-value_tmp>1024){
            value_tmp+=1024;
            cur_block->append(make_unique<RegImmInst>(Archriscv::RegImmInst::Addi,tmp_addr_reg,tmp_addr_reg,1024));
          }
          cur_block->append(make_unique<Store>(src,
                                     Reg{tmp_addr_reg}, pos-value_tmp,src.is_float()?Archriscv::Store::Fsw:Archriscv::Store::Sw,flag_addr));
        }
        else
            cur_block->append(make_unique<Store>(src,
                                     Reg{sp}, pos,src.is_float()?Archriscv::Store::Fsw:Archriscv::Store::Sw,flag_addr));
        if(flag_addr)
          pos += INT_SIZE * 2;
        else
          pos += INT_SIZE;
      }
      cur_block->append(make_unique<FuncCall>(call->f->name,
                                      static_cast<int>(call->args.size())));
      sp_move_iter=sp_move;
       while(sp_move_iter>=1024){
         cur_block->append(make_unique<MoveSP>(1024));
         sp_move_iter-=1024;
       }
       if(sp_move_iter>0)cur_block->append(make_unique<MoveSP>(sp_move_iter));
    } else {
      for (size_t i = 0; i < call->args.size(); ++i){
        auto res_reg = inner_info->reg_medium2backend(call->args[i].first);
        auto dst=Reg{ARGUMENT_REGISTERS[i],res_reg.type};
        cur_block->append(make_unique<Move>(dst,
          res_reg,dst.is_float()==true?Archriscv::Move::FLOATMOV:Archriscv::Move::INTMOV));
      }
      cur_block->append(make_unique<FuncCall>(call->f->name,
                                      static_cast<int>(call->args.size())));
    }
    if (!call->ignore_return_value){
      auto res_reg = inner_info->reg_medium2backend(call->d1);
      auto dst_reg = Reg{ARGUMENT_REGISTERS[0],res_reg.type};
      cur_block->append(make_unique<Move>(res_reg,dst_reg,res_reg.is_float()==true?Archriscv::Move::FLOATMOV:Archriscv::Move::INTMOV));
    }
    if (call->f->name == "__create_threads") {
        curf->spilling_reg.insert(inner_info->reg_medium2backend(call->d1));
    }
}
void program_visitor::visit(Medium::LocalVarDef &node){
}
void program_visitor::visit(Medium::PhiInstr &node){
    
}
void program_visitor::visit(Medium::MemDef &node){
    
}
void program_visitor::visit(Medium::MemUse &node){
    
}
void program_visitor::visit(Medium::Convert &node){
  auto convert = &node;
  Reg dst = inner_info->reg_medium2backend(convert->d1),
        src = inner_info->reg_medium2backend(convert->src);
    if (dst.type == FP) {
      cur_block->append(
        make_unique<Convert>(Convert::Fcvtsw,dst,src)
      );
    } else {
      cur_block->append(
        make_unique<Convert> (Convert::Fcvtws,dst,src)
      );
    }
}
void program_visitor::visit(Medium::MemEffect &node){
    
}
void program_visitor::visit(Medium::MemRead &node){
    
}
void program_visitor::visit(Medium::MemWrite &node){
    
}