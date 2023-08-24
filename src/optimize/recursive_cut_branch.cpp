#include "pass.h"
using namespace Pass;
void recursive_cut_branch::run() {
    for(auto & [name,f] : c->funcs){
        if(!func_check(f.get())) continue;
        insert_remember_inst(f.get());
    }
}
void recursive_cut_branch::insert_remember_inst(UserFunction*f){
    //1. 添加一个数组，用来进行记忆化操作
    //2. 添加一个if语句，判断是否完成了记忆化
    //3. 添加true
    //4. 添加false
    assert(f->bbs.size());
    //在第一个block添加上载入数组的指令
    auto first_block = f->bbs[0].get();
    auto ir_obj = c->scope.new_MemObject("mem_index");
    ir_obj->size = 32 * 4;
    ir_obj->is_int = !type;
    ir_obj->is_float = type;
    Medium::Reg start_addr = f->new_Reg(true,INT_SIZE*2);
        //初始化这个数组
    auto temp = f->new_Reg(true);
    auto zero = f->new_Reg(true);
    ir_obj->dims.push_back(32);

    // for(int i = 31 ; i >= 0; --i){
        
    //     Medium::Reg value = f->new_Reg(!type);
    //     auto load_ins = new Medium::LoadConst(value, static_cast<float>(-2));
        
    //     if (i == 0) {
    //         first_block->push_front(new Medium::StoreInstr(start_addr, value));
    //         first_block->push_front(load_ins);
    //         continue;
    //     }
    //     Medium::Reg index = f->new_Reg(true),
    //     addr = f->new_Reg(true,INT_SIZE*2);
    //     first_block->push_front(new Medium::StoreInstr(addr, value));
    //     first_block->push_front(new Medium::ArrayIndex(addr, start_addr, index, INT_SIZE, -1));
    //     auto load_index = new Medium::LoadConst(index,static_cast<int>(i));
    //     first_block->push_front(load_index);
        
    //     first_block->push_front(load_ins);
    // }

    first_block->push_front(new Medium::LoadAddr(start_addr, ir_obj));
    auto arg_copy = f->new_Reg(type);
    // auto load_arg = new LoadArg(arg_copy,1);
    // first_block->push_front(load_arg);
    // first_block->push_front(new Medium::LocalVarDef(ir_obj));
    //添加if...then...else
    //只考虑一个递归的情况
    //首先确定else所在的bb
    IrBlock* cur_block = nullptr;
    auto iter = f->bbs.end();
    IrBlock* true_block = f->new_BB("true_add");
    IrBlock* false_block = f->new_BB("false_add");
    IrBlock* succ = f->new_BB("new_succ");
    auto new_func_ret = f->new_Reg(!type);
    Medium::Reg origin_func_ret;
    


    list<unique_ptr<Instr>>::iterator itfunc;
    for(auto i = f->bbs.begin();i!= iter;++i){
        bool end_bb = false;
        auto bb = i->get();
        for(auto it = bb->instrs.begin(); it!= bb->instrs.end();++it){
            if(auto y1 = dynamic_cast<RegWriteInstr*>(it->get())){
            if(auto y = dynamic_cast<CallInstr*>(it->get())){
                if(y->f->name != f->name) continue;
                //首先载入地址，然后和0比较大小
                if(!end_bb){
                    cur_block = bb;
                    origin_func_ret = y->d1;
                    //先使用一个寄存器保存参数
                    

                    //使用数组之前先加载数组
                    auto start_addr_new = f->new_Reg(true,8);
                    auto load_addr = new LoadAddr(start_addr_new,ir_obj);

                    //算地址
                    Medium::Reg new_addr = f->new_Reg(true,INT_SIZE*2);
                    auto arr_index = new Medium::ArrayIndex(new_addr, start_addr_new, arg_copy, 4, -1);
                    
                    //加载内存中的值
                    auto tar_reg = f->new_Reg(!type);
                    auto load_instr = new LoadInstr(tar_reg,new_addr);
                    
                    //然后是比较
                    auto zero = f->new_Reg(!type);
                    auto output_compare = f->new_Reg(true);
                    auto ldconst = new LoadConst(zero,static_cast<float>(-1));
                    auto binary = new BinaryOpInstr(output_compare,zero,tar_reg,BinaryOp::LESS);
                    
                    //根据结果进行跳转
                    auto branch = new BranchInstr(output_compare,true_block,false_block);
                    
                    //添加这些指令

                    bb->instrs.insert(it,unique_ptr<Instr>(load_addr));
                    bb->instrs.insert(it,unique_ptr<Instr>(arr_index));
                    bb->instrs.insert(it,unique_ptr<Instr>(load_instr));
                    bb->instrs.insert(it,unique_ptr<Instr>(ldconst));
                    bb->instrs.insert(it,unique_ptr<Instr>(binary));
                    bb->instrs.insert(it,unique_ptr<Instr>(branch));

                    //false block
                    // 1. 函数调用
                    // 2. 在数组中保存返回结果
                    // 3. 跳转到新的后继block
                    false_block->push(y);
                    //把值写到数组里面去
                    // 1. 加载基地址
                    // 2. 算最终地址
                    // 3. 将返回值放回去
                    auto addrreg2 = f->new_Reg(true,8);
                    auto load_addr2 = new LoadAddr(addrreg2,ir_obj);
                    false_block->push(load_addr2);
                    auto one_reg1 = f->new_Reg(true);
                    auto load_offset1 = new LoadConst(one_reg1,1);
                    false_block->push(load_offset1);
                    auto index_reg1 = f->new_Reg(true);
                    auto cal_index = new BinaryOpInstr(index_reg1,arg_copy,one_reg1,BinaryOp::ADDW); 
                    false_block->push(cal_index);

                    auto addr_cal = f->new_Reg(true,8);
                    auto addr_cal_ins = new ArrayIndex(addr_cal,addrreg2,index_reg1,4,-1);
                    false_block->push(addr_cal_ins);

                    false_block->push(new StoreInstr(addr_cal,y->d1));
                    
                    //下面是trueblock
                    //1. 将数组里面的值取出来
                    //2. 将这个值放到返回值的寄存器
                    
                    auto addrreg3 = f->new_Reg(true,8);
                    auto load_addr3 = new LoadAddr(addrreg3,ir_obj);
                    true_block->push(load_addr3);

                    auto addr_cal3 = f->new_Reg(true,8);
                    auto addr_cal_ins3 = new ArrayIndex(addr_cal3,addrreg3,arg_copy,4,-1);
                    true_block->push(addr_cal_ins3);

                    auto tar_reg_copy = f->new_Reg(!type);
                    auto load_instr_new = new LoadInstr(tar_reg_copy,addr_cal3);
                    true_block->push(load_instr_new);
                    
                    true_block->push(new UnaryOpInstr(new_func_ret,load_instr_new->d1,UnaryOp::ID));

                    //在新的后继里面添加跳到原来后继的指令
                    auto jmp = dynamic_cast<JumpInstr*>(bb->back());
                    assert(jmp);
                    succ->push(jmp);
                    
                    //在true和false后面添加jmp指令
                    auto jmp_succ = new JumpInstr(succ);
                    itfunc = it;
                    true_block->push(jmp_succ);
                    false_block->push(jmp_succ);
                    end_bb = true;

                    //改变原来的后继的phi指令的use
                    auto origin_succ = jmp->target;
                    if(auto phi = dynamic_cast<PhiInstr*>(origin_succ->instrs.front().get())){
                      phi->map_BB([&](IrBlock* &cur_bb){
                        if(cur_bb == cur_block){
                            cur_bb = succ;
                        }
                      });
                    }else{
                        //TODO
                    }

                }else {
                    break;
                }
            }
            else if(auto y = dynamic_cast<LoadArg*>(it->get())){
                if(y->id == 1)
                    arg_copy = y1->d1;
            }else if(auto bi = dynamic_cast<BinaryOpInstr*>(it->get())){
                if(bi->s1 == arg_copy || bi->s2 == arg_copy)
                    if(bi->op.type == BinaryOp::ADDW || bi->op.type == BinaryOp::ADD)
                        arg_copy = bi->d1;
            }
            }
            if(end_bb){
               break;
            }
        }
    }
    
    //将原来block中剩下的指令添加到新的后继里面，然后把use改变掉
    auto eventually_func_ret = f->new_Reg(!type);
    auto it_start = --cur_block->instrs.end();
    for(auto it = --it_start; it != itfunc; --it){
        succ->push_front(it->get());
        succ->instrs.front().get()->map_use([&](Reg& v){
            if(v == origin_func_ret){
                v = eventually_func_ret;
            }
        });
    }

    //在新的后继的最开始加入phi指令
    auto new_phi = new PhiInstr(eventually_func_ret);
    new_phi->add_use(origin_func_ret,false_block);
    new_phi->add_use(new_func_ret,true_block);
    succ->push_front(new_phi);

    //将原来block里面的指令给删掉
    auto it_bb = itfunc;
    while(it_bb != cur_block->instrs.end()){
        it_bb->release();
        it_bb = cur_block->instrs.erase(it_bb);
    }


    IrBlock* true_block1 = f->new_BB("true_add1");
    IrBlock* false_block1 = f->new_BB("false_add1");
    IrBlock* succ1 = f->new_BB("new_succ1");
    auto new_func_ret1 = f->new_Reg(!type);
    Medium::Reg origin_func_ret1;
    list<unique_ptr<Instr>>::iterator itfunc1;
    bool end_bb1 = false;
    auto bb1 = succ;
    
    for(auto it = bb1->instrs.begin(); it!= bb1->instrs.end();++it){
        if(auto y1 = dynamic_cast<RegWriteInstr*>(it->get())){
        if(auto y = dynamic_cast<CallInstr*>(it->get())){
            if(y->f->name != f->name) continue;
    //         //首先载入地址，然后和0比较大小
            if(!end_bb1){
                cur_block = bb1;
                origin_func_ret1 = y->d1;
                //先使用一个寄存器保存参数
                // auto arg_copy1 = f->new_Reg(true);
                // auto load_arg = new UnaryOpInstr(arg_copy1,arg_copy,UnaryOp::ID);
                //使用数组之前先加载数组
                auto start_addr_new = f->new_Reg(true,8);
                auto load_addr = new LoadAddr(start_addr_new,ir_obj);
                //算地址
                Medium::Reg new_addr = f->new_Reg(true,INT_SIZE*2);
                auto arr_index = new Medium::ArrayIndex(new_addr, start_addr_new, arg_copy, 4, -1);

                //加载内存中的值
                auto tar_reg = f->new_Reg(!type);
                auto load_instr = new LoadInstr(tar_reg,new_addr);

                //然后是比较
                auto zero = f->new_Reg(!type);
                auto output_compare = f->new_Reg(true);
                auto ldconst = new LoadConst(zero,static_cast<float>(-1));
                auto binary = new BinaryOpInstr(output_compare,zero,tar_reg,BinaryOp::LESS);

                //根据结果进行跳转
                auto branch = new BranchInstr(output_compare,true_block1,false_block1);

                //添加这些指令
                // bb1->instrs.insert(it,unique_ptr<Instr>(load_arg));
                bb1->instrs.insert(it,unique_ptr<Instr>(load_addr));
                bb1->instrs.insert(it,unique_ptr<Instr>(arr_index));
                bb1->instrs.insert(it,unique_ptr<Instr>(load_instr));
                bb1->instrs.insert(it,unique_ptr<Instr>(ldconst));
                bb1->instrs.insert(it,unique_ptr<Instr>(binary));
                bb1->instrs.insert(it,unique_ptr<Instr>(branch));
    //                //false block
    //                // 1. 函数调用
    //                // 2. 在数组中保存返回结果
    //                // 3. 跳转到新的后继block
                false_block1->push(y);
    //                //把值写到数组里面去
    //                // 1. 加载基地址
    //                // 2. 算最终地址
    //                // 3. 将返回值放回去
                // auto addrreg2 = f->new_Reg(true,8);
                // auto load_addr2 = new LoadAddr(addrreg2,ir_obj);
                // false_block1->push(load_addr2);
                // auto one_reg1 = f->new_Reg(true);
                // auto load_offset1 = new LoadConst(one_reg1,0);
                // false_block1->push(load_offset1);
                // auto index_reg1 = f->new_Reg(true);
                // auto cal_index = new BinaryOpInstr(index_reg1,arg_copy,one_reg1,BinaryOp::ADDW); 
                // false_block1->push(cal_index);
                // auto addr_cal = f->new_Reg(true,8);
                // auto addr_cal_ins = new ArrayIndex(addr_cal,addrreg2,index_reg1,4,-1);
                // false_block1->push(addr_cal_ins);
                // false_block1->push(new StoreInstr(addr_cal,y->d1));

    //                //下面是trueblock
    //                //1. 将数组里面的值取出来
    //                //2. 将这个值放到返回值的寄存器

                auto addrreg3 = f->new_Reg(true,8);
                auto load_addr3 = new LoadAddr(addrreg3,ir_obj);
                true_block1->push(load_addr3);
                auto addr_cal3 = f->new_Reg(true,8);
                auto addr_cal_ins3 = new ArrayIndex(addr_cal3,addrreg3,arg_copy,4,-1);
                true_block1->push(addr_cal_ins3);
                auto tar_reg_copy = f->new_Reg(!type);
                auto load_instr_new = new LoadInstr(tar_reg_copy,addr_cal3);
                true_block1->push(load_instr_new);

                true_block1->push(new UnaryOpInstr(new_func_ret1,load_instr_new->d1,UnaryOp::ID));
                //在新的后继里面添加跳到原来后继的指令
                auto jmp = dynamic_cast<JumpInstr*>(bb1->back());
                assert(jmp);
                succ1->push(jmp);

    //                //在true和false后面添加jmp指令
                auto jmp_succ = new JumpInstr(succ1);
                itfunc1 = it;
                true_block1->push(jmp_succ);
                false_block1->push(jmp_succ);
                end_bb1 = true;
    //                //改变原来的后继的phi指令的use
                auto origin_succ = jmp->target;
                if(auto phi = dynamic_cast<PhiInstr*>(origin_succ->instrs.front().get())){
                  phi->map_BB([&](IrBlock* &cur_bb){
                    if(cur_bb == cur_block){
                        cur_bb = succ1;
                    }
                  });
                }else{
    //                    //TODO
                }
            }else {
                break;
            }
        } else if(auto y = dynamic_cast<LoadArg*>(it->get())){
                if(y->id == 1)
                    arg_copy = y1->d1;
            }else if(auto bi = dynamic_cast<BinaryOpInstr*>(it->get())){
                if(bi->s1 == arg_copy || bi->s2 == arg_copy)
                    if(bi->op.type == BinaryOp::ADDW || bi->op.type == BinaryOp::ADD)
                        arg_copy = bi->d1;
            }
        }
        if(end_bb1){
           break;
        }
    }
    
    // //将原来block中剩下的指令添加到新的后继里面，然后把use改变掉
    auto eventually_func_ret1 = f->new_Reg(!type);
    auto it_start1 = --cur_block->instrs.end();
    for(auto it = --it_start1; it != itfunc1; --it){
        succ1->push_front(it->get());
        succ1->instrs.front().get()->map_use([&](Reg& v){
            if(v == origin_func_ret1){
                v = eventually_func_ret1;
            }
        });
    }

    // //在新的后继的最开始加入phi指令
    auto new_phi1 = new PhiInstr(eventually_func_ret1);
    new_phi1->add_use(origin_func_ret1,false_block1);
    new_phi1->add_use(new_func_ret1,true_block1);
    succ1->push_front(new_phi1);

    // //将原来block里面的指令给删掉
    auto it_bb1 = itfunc1;
    while(it_bb1 != cur_block->instrs.end()){
        it_bb1->release();
        it_bb1 = cur_block->instrs.erase(it_bb1);
    }


    // ssa_construction(f,[&](Reg r) { return new_reg.count(r); });
}
bool recursive_cut_branch::func_check(UserFunction* ff){
    bool found_call_self = false;
    ff->for_each([&](IrBlock* bb){
        bb->for_each([&](Instr* x){
            if(auto y = dynamic_cast<CallInstr*>(x)){ //如果在一个函数中发现了函数调用
                if(y->f->name == ff->name && !y->ignore_return_value){ //call self
                    found_call_self = true;
                    int int_count = 0;
                    for(auto & arg : y->args){
                        if(arg.first.is_int) {
                            cur_index = arg.first;
                            int_count += 1;
                        }
                    }
                    if(int_count != 1) found_call_self = false;
                    if(!ff->is_float && ff->is_int) {
                        type = 0; //float
                        found_call_self = false;
                    }else{
                        type = 1;
                    }
                    
                }
            }

        });
    });
    if(!found_call_self) return false;
    return true;
}