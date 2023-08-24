#include "ir_builder.h"
#include <variant>
#include "symbol_table.h"
#include <any>

// templates: std::any_cast<>();

using std::optional;
using std::pair;
using std::string;
using std::variant;
using std::vector;

std::any tmp_val;
// bool is_var_int=true;
VariableTableEntry *ASTVisitor::resolve(const string &name)
{
    if (cur_local_table)
        return cur_local_table->recursively_resolve(name);
    else
        return global_var.resolve(name);
}

IRValue ASTVisitor::to_IRValue(std::any &value)//将CondJump转化为IR_value
{
    if (std::any_cast<IRValue>(&value))
        return std::any_cast<IRValue>(value);
    assert(std::any_cast<CondJumpList>(&value));
    CondJumpList jump_list = std::any_cast<CondJumpList>(value);
    Medium::IrBlock *true_bb = new_block(), *false_bb = new_block(), *res_bb = new_block();
    for (Medium::IrBlock **i : jump_list.true_list)
        (*i) = true_bb;
    for (Medium::IrBlock **i : jump_list.false_list)
        (*i) = false_bb;
    Medium::Reg true_reg = new_medium_reg(true), false_reg = new_medium_reg(true), res_reg = new_medium_reg(true);
    true_bb->push(new Medium::LoadConst(true_reg, 1));
    true_bb->push(new Medium::JumpInstr(res_bb));
    false_bb->push(new Medium::LoadConst(false_reg, 0));
    false_bb->push(new Medium::JumpInstr(res_bb));
    Medium::PhiInstr *phi_inst = new Medium::PhiInstr(res_reg);
    phi_inst->uses.emplace_back(true_reg, true_bb);
    phi_inst->uses.emplace_back(false_reg, false_bb);
    res_bb->push(phi_inst);
    cur_block = res_bb;
    IRValue ret;
    ret.is_left_value = false;
    ret.reg = res_reg;
    return ret;
}

Medium::Reg ASTVisitor::new_medium_reg(bool is_int,int size)
{
    if (in_init)
        return init_func->new_Reg(is_int,size);
    if (cur_func)
        return cur_func->new_Reg(is_int,size);
    return Medium::Reg{};
}

Medium::IrBlock *ASTVisitor::new_block()
{
    if (in_init)
        return init_func->new_BB();
    if (cur_func)
        return cur_func->new_BB();
    return nullptr;
}

Medium::Reg ASTVisitor::get_value(const IRValue &value)
{
    Medium::Reg ret = value.reg;
    if (value.is_left_value)
    {
        Medium::Reg temp = new_medium_reg(value.type.is_int);//与原先的类型保持一致
        cur_block->push(new Medium::LoadInstr(temp, ret));
        ret = temp;
    }
    return ret;
}

Medium::Reg ASTVisitor::reg_changed(Medium::Reg src , bool is_int){
    if (src.is_int != is_int) {
        Medium::Reg res_reg = new_medium_reg(is_int);
        cur_block->push(new Medium::Convert(res_reg,src));
        return res_reg;
    }
    return src;
}


VariableTable *ASTVisitor::new_variable_table(VariableTable *parent)
{
    VariableTable *ret = new VariableTable(parent);
    local_var.push_back(ret);
    return ret;
}


CondJumpList ASTVisitor::to_CondJumpList(std::any& value)
{
    if (std::any_cast<CondJumpList>(&value))
        return std::any_cast<CondJumpList>(value);
    // std::cout<<value.type().name()<<std::endl;
    assert(std::any_cast<IRValue>(&value));
    IRValue irv = std::any_cast<IRValue>(value);
    Medium::Reg reg = get_value(irv);
    // reg.is_int = true;
    Medium::BranchInstr *inst = new Medium::BranchInstr(reg, nullptr, nullptr);
    cur_block->push(inst);
    cur_block = nullptr;
    CondJumpList jump_list;
    jump_list.true_list.push_back(&inst->target1); // 留到后面进行填充
    jump_list.false_list.push_back(&inst->target0);
    return jump_list;
}
void ASTVisitor::register_lib_function(
    string name, RetType rettype,
    vector<variant<Type, StringType>> params)
{
    FunctionInterface interface;
    interface.return_value_non_void =rettype==VOID?true:false;
    interface.is_float=rettype==FLOAT?true:false;
    interface.is_int=rettype==INT?true:false;
    interface.args_type = params;
    assert(ir->lib_funcs.find(name) != ir->lib_funcs.end());
    Medium::LibFunction *ir_func = ir->lib_funcs[name].get();
    functions.register_func(name, ir_func, interface);
}
void ASTVisitor::register_lib_functions()
{
    register_lib_function("getint", INT, {});
    register_lib_function("getch", INT, {});
    register_lib_function("getarray", INT, {Type::UnknownLengthArray});
    Type param_type=Type{};
    param_type.is_int=true;
    param_type.is_float=false;
    register_lib_function("putint", VOID, {param_type});
    param_type.is_int=true;
    param_type.is_float=false;
    register_lib_function("putch", VOID, {param_type});
    param_type.is_int=true;
    param_type.is_float=false;
    register_lib_function("putarray", VOID, {param_type, Type::UnknownLengthArray});
    register_lib_function("putf", VOID, {StringType{}});
    functions.resolve("putf")->interface.variadic = true;
    register_lib_function("starttime", VOID, {});
    register_lib_function("stoptime", VOID, {});
    register_lib_function("getfarray",INT,{Type::UnknownLengthArray});//******
    register_lib_function("getfloat",FLOAT,{});
    param_type.is_int=false;
    param_type.is_float=true;
    register_lib_function("putfloat",VOID,{param_type});
    param_type.is_int=true;
    param_type.is_float=false;
    register_lib_function("putfarray",VOID,{param_type,Type::UnknownLengthArray});
    param_type.is_int = true;
    param_type.is_float = false;
    register_lib_function("memset",VOID,{Type::UnknownLengthArray,param_type,param_type});
    // register_lib_function("memset",VOID,{Type::UnknownLengthArray,param_type,param_type});
}
// IRValue *ret_lval;
void ASTVisitor::visit(syntax_tree::lval_syntax &node)
{
    IRValue ret_lval;

    VariableTableEntry *entry = resolve(node.name); // 相当于之前的scope模块功能
    // 代码里面编译时的已经在前端处理了，这里只写运行时的
    Medium::Reg addr = new_medium_reg(true,INT_SIZE*2);//地址寄存器肯定是int类型的***********8
    if (entry->arg_id >= 0) { // 数组类型
        cur_block->push(new Medium::LoadArg(addr, entry->arg_id));
    }
    else { // 非数组类型
        cur_block->push(new Medium::LoadAddr(addr, entry->ir_obj));
    }
    if (!entry->type.is_array()) { // 如果不是数组类型
        ret_lval.type = entry->type;
        ret_lval.is_left_value = true;
        ret_lval.reg = addr;
        //   return ret; 用全局变量传递
        tmp_val = std::any(ret_lval);
        return;
    }
    MemSize step_size = 4; // 占用存储空间的大小
    Type new_type = entry->type;
    // 下面是为数组的每一个元素都生成对应的东西，然后加入bb中
    size_t cur = node.array_index.size() - 1; // 当前下标是长度-1
    // 如果出现了第一维被省略的情况
    if (entry->type.omit_first_dim)
        --cur;

    // 计算步长 如计算 a[10] 步长, 参考的是数组a[10][2]
    for (size_t i = cur + 1; i < entry->type.array_dims.size(); ++i)
        step_size *= entry->type.array_dims[i];

    int index_max = node.array_index.size() - 1;
    for (int i = index_max; i >= 0; --i) {

        new_type = new_type.deref_one_dim();

        node.array_index[i]->accept(*this);

        Medium::Reg cur_index = get_value(to_IRValue(tmp_val));
        Medium::Reg new_addr = new_medium_reg(true,INT_SIZE*2);//地址寄存器肯定是int类型的***********8

        cur_block->push(
            new Medium::ArrayIndex(new_addr, addr, cur_index, step_size, -1));

        if (cur < entry->type.array_dims.size())
        {
            step_size *= entry->type.array_dims[cur];
            --cur;
        }
        addr = new_addr;
    }
    ret_lval.type = new_type;
    ret_lval.is_left_value = true;
    ret_lval.reg = addr;

    tmp_val = std::any(ret_lval);
}

// 处理该结点时必定返回一个编译时值
void ASTVisitor::visit(syntax_tree::literal_syntax &node)
{
    Medium::Reg value = new_medium_reg(node.is_int);
    IRValue ret;
    if (node.is_int)
    {
        auto load_ins = new Medium::LoadConst(value, static_cast<int32_t>(node.intConst));
        //load_ins->set_float(0.0,0);
        cur_block->push(load_ins);
        ret.type.is_int=true;
        ret.type.is_float=false;
    }else {
        // if(!node.floatConst){
        //     IR::Reg start_addr = new_reg(true,INT_SIZE*2);
        //     IR::Reg zero = new_reg(true);
        //     cur_block->push(new IR::LoadConst(zero, 0));
        //     cur_block->push(new IR::StoreInstr(start_addr, zero));
        // }
        auto load_ins = new Medium::LoadConst(value, static_cast<float>(node.floatConst));
        //load_ins->set_float(node.floatConst,1);
        cur_block->push(load_ins);
        ret.type.is_int=false;
        ret.type.is_float=true;
    }
     // 目前我直接将这个指令当作int **********
    //cur_block->print(std::cout);
    
    ret.is_left_value = false;
    ret.reg = value;
    // std::cout<<typeid(std::any{ret}).name();
    //  std::cout<<std::any{ret}.isNotNull()<<std::endl;
    //  std::cout<<"1"<<std::endl;
    //  auto tmp = std::any{ret};
    tmp_val = std::any(ret);
    // auto typeofiii = tmp_val.is<IRValue>();
    // auto p = tmp_val.as<IRValue>();
    return;
}

// 定义一个常量或者变量语句, 并将信息进行register
void ASTVisitor::visit(syntax_tree::var_def_stmt_syntax &node) {
    Medium::Variable *ir_obj;
    Type type;
    type.is_const = node.is_constant;
    type.is_int = node.is_int;
    // bool type_tmp=is_var_int;
    // is_var_int=node.is_int;
    if(type.is_int)
        type.is_float=false;
    else 
        type.is_float=true;
    // 在前端已经进行处理，这里直接调用
    for (int i = 0; i < node.array_length.size(); i++) {
        auto liter_node = dynamic_cast<syntax_tree::literal_syntax *>(node.array_length[i].get());
        // if(!liter_node) assert() 如果不是应该直接报错
        type.array_dims.push_back(liter_node->intConst); // 这里一定是const
    }
    //计算出所有元素的个数
    int all_num = 1;
    for (int i = 0; i < type.array_dims.size(); i++)
        all_num *= type.array_dims[i];
    


    //补丁：如果数组的个数不是偶数，就将其补充为偶数
    // if(all_num%2==1)all_num+=1;
    // type.array_dims.reserve();
    // 这里可能要改库了，先别动，之后再说
    vector<int32_t> init_value(all_num,0); // 常量使用这个来进行初始化
    vector<float> init_value_f(all_num,0);
    for (size_t i = 0; i < node.initializers.size() && node.is_constant; i++) {
        auto liter_node = dynamic_cast<syntax_tree::literal_syntax *>(node.initializers[i].get());
        int idx = node.array_length.size() ? *node.initializers_index[i].get() : i;
        if (node.is_int) {
            if (liter_node->is_int) {
                init_value[idx] = liter_node->intConst;
            } else {
                init_value[idx] = liter_node->floatConst;
            }    
        } else {
            if (liter_node->is_int) {
                init_value_f[idx] = liter_node->intConst;
            } else {
                init_value_f[idx] = liter_node->floatConst;
            }
        }
    }

    auto init_var = [&](){
        vector<optional<Medium::Reg>> init;
        init.resize(all_num);
        for (size_t i = 0; i < node.initializers.size(); i++)
        {
            int idx = node.array_length.size()?*node.initializers_index[i].get():i;
            node.initializers[i]->accept(*this); //
            // 假设这个变量的名字是tmp_val
            init[idx] = get_value(to_IRValue(tmp_val));
        }
        return init;
    };
    
    if (cur_func) { // 如果是局部变量
        ir_obj = cur_func->scope.new_MemObject(node.name);
        // 创建指令，局部变量分配
        ir_obj->size = type.size();
        ir_obj->is_int = type.is_int;
        ir_obj->is_float = type.is_float;
        cur_block->push(new Medium::LocalVarDef(ir_obj));
        if (node.is_constant) { // 对于常量，需要进行这样的操作
            // load
            Medium::Reg start_addr = new_medium_reg(true,INT_SIZE*2);//**********
            cur_block->push(new Medium::LoadAddr(start_addr, ir_obj));
            for (size_t i = 0; i < all_num; ++i) {
                Medium::Reg temp = new_medium_reg(node.is_int);//******
                // 处理非浮点数字的情况
                if(node.is_int) {
                      cur_block->push(new Medium::LoadConst(temp, static_cast<int>(init_value[i])));
                } else {
                    cur_block->push(new Medium::LoadConst(temp, static_cast<float>(init_value_f[i])));
                }
                if (i == 0) {
                    cur_block->push(new Medium::StoreInstr(start_addr, temp));
                    continue;
                }
                Medium::Reg index = new_medium_reg(true);//********
                cur_block->push(new Medium::LoadConst(index, static_cast<int>(i)));
                Medium::Reg cur_addr = new_medium_reg(true,INT_SIZE*2);//*********8
                cur_block->push(
                    new Medium::ArrayIndex(cur_addr, start_addr, index, INT_SIZE, -1));
                cur_block->push(new Medium::StoreInstr(cur_addr, temp));
            }
        }
        else{
            // 对于变量，主要是因为定义中可能含有标识符，而不都是数字，我们需要一些全局变量来确定他们之间的关系
            // 对于有初始化的情况，有初始化的情况，变量和常量的方法不同
            // 对于常量，只需要使用int就可以了，对于变量需要使用reg类型
            // 如果不是数组，前端会给index里面传-1过来
            
            
            Medium::Reg start_addr = new_medium_reg(true,INT_SIZE*2);
            Medium::Reg zero =new_medium_reg(true);
            
            
            cur_block->push(new Medium::LoadAddr(start_addr, ir_obj));
            cur_block->push(new Medium::LoadConst(zero, 0));
            if(node.is_int) {
                Medium::Reg zerof = new_medium_reg(node.is_int);
                auto load_ins = new Medium::LoadConst(zerof, 0);
                cur_block->push(load_ins);
            }
            if (node.array_length.size() && node.initializers_index.size()) {
                // 说明是数组，此时要初始化
                //首先ld一个立即数到寄存器里面去，这个立即数是sizeiof
                auto temp = new_medium_reg(true);
                cur_block->push(new Medium::LoadConst(temp, static_cast<int>(all_num * INT_SIZE)));
                cur_block->push(new Medium::CallInstr(new_medium_reg(false), functions.resolve("memset")->ir_func, {{start_addr,true},{zero,false},{temp ,false}}, true));
            }
            auto init = init_var();

           //ccx add the condition **********************
            for (size_t i = 0; i < all_num; i++) {
                if (i == 0) {
                    if (init[i]){
                        Medium::Reg reg = init[i].value();
                        // reg.is_int = node.is_int;
                        reg = reg_changed(reg,node.is_int);
                        // auto reg = reg_changed(reg_origi)
                        cur_block->push(new Medium::StoreInstr(start_addr, reg));
                    }
                    else{
                         if(node.initializers_index.size()) //ccx add the condition **********************
                            cur_block->push(new Medium::StoreInstr(start_addr, zero));
                    }
                    continue;
                }
                if (init[i]) {
                    Medium::Reg index = new_medium_reg(true), addr = new_medium_reg(true,INT_SIZE*2);
                    cur_block->push(new Medium::LoadConst(index, static_cast<int>(i)));
                    cur_block->push(new Medium::ArrayIndex(addr, start_addr, index, INT_SIZE, -1));
                    Medium::Reg reg = init[i].value();
                    reg = reg_changed(reg,node.is_int);
                    cur_block->push(new Medium::StoreInstr(addr, reg));
                }
                else{
                    //使用memset之后可以不用这个了
                    // if(node.initializers_index.size()) //ccx add the condition **********************
                    // {
                    //     IR::Reg index = new_reg(true), addr = new_reg(true,INT_SIZE*2);
                    //     cur_block->push(new IR::LoadConst(index, static_cast<int>(i)));
                    //     cur_block->push(new IR::ArrayIndex(addr, start_addr, index, INT_SIZE, -1));
                    //     cur_block->push(new IR::StoreInstr(addr, zero));
                    // }
                    }
                }
        }
        if (node.is_constant)
            cur_local_table->register_const(node.name, ir_obj, type, std::move(init_value));
        else
            cur_local_table->register_var(node.name, ir_obj, type);
    }
    else { // 全局变量
        // 在顶层注册一个变量
        ir_obj = ir->scope.new_MemObject(node.name);
        // ir_obj->is_float = type.is_float;
        // ir_obj->is_int = type.is_int;
        if (node.initializers.size() && node.is_constant) { // 如果有赋值,还是常量的情况
            if (node.is_int)
            {
                int32_t *buf = new int32_t[init_value.size()];
                for (size_t i = 0; i < init_value.size(); ++i)
                    buf[i] = init_value[i];
                ir_obj->init(buf, type.size());
            }else {
                float *buf = new float[init_value_f.size()];
                for (size_t i = 0; i < init_value_f.size(); ++i)
                    buf[i] = init_value_f[i];
                ir_obj->init(buf, type.size());
                ir_obj->is_int = 0;
                // 打个小补丁 *******************************************
                // 后端暂时是按照 不是int处理的，这里设置成了不是int，有问题
            }
        }
        else
            ir_obj->initial_value = nullptr;
        if (node.initializers.size() && !node.is_constant) {
            cur_block = init_bb;
            in_init = true;
            auto init = init_var();
            assert(init.size() == type.count_elements());
            Medium::Reg start_addr = new_medium_reg(true,INT_SIZE*2);
            cur_block->push(new Medium::LoadAddr(start_addr, ir_obj));
            for (size_t i = 0; i < init.size(); ++i)
                if (init[i]) {
                    if (i == 0) {
                        // init[i].value().is_int = node.is_int;
                        auto reg = reg_changed(init[i].value(),node.is_int);
                        cur_block->push(new Medium::StoreInstr(start_addr, reg));
                        continue;
                    }
                    Medium::Reg index = new_medium_reg(true), addr = new_medium_reg(true,INT_SIZE*2);
                    cur_block->push(new Medium::LoadConst(index,static_cast<int>(i)));
                    cur_block->push(new Medium::ArrayIndex(addr, start_addr, index, INT_SIZE, -1));
                    // init[i].value().is_int=node.is_int;
                    auto reg = reg_changed(init[i].value(),node.is_int);
                    cur_block->push(new Medium::StoreInstr(addr, reg));
                }
            init_bb = cur_block;
            cur_block = nullptr;
            in_init = false;
        }
        if (node.is_constant)
            global_var.register_const(node.name, ir_obj, type, std::move(init_value));
        else
            global_var.register_var(node.name, ir_obj, type);
    }
    if (!node.is_constant) {
        ir_obj->size = type.size();
        ir_obj->is_int = node.is_int;
        if(ir_obj->is_int)ir_obj->is_float=false;
        else ir_obj->is_float=true;
    }
    for (MemSize i : type.array_dims) ir_obj->dims.push_back(static_cast<int>(i));

    // is_var_int=type_tmp;
}
// 通过retvalue来进行传递
void ASTVisitor::visit(syntax_tree::assign_stmt_syntax &node)
{
    assert(mode == normal);
    node.target->accept(*this);
    IRValue lhs =  std::any_cast<IRValue>(tmp_val);
    node.value->accept(*this);
    IRValue rhs = to_IRValue(tmp_val);
    // if (!lhs.assignable());
    // throw AssignmentTypeError("left hand side '" + ctx->lVal()->getText() +
                                //   "' is not assignable");
    // if (!lhs.type.check_assign(rhs.type));
    // throw AssignmentTypeError("type error on assignment '" + ctx->getText() +
                                //   "'");
    Medium::Reg rhs_value_origin = get_value(rhs);
    auto rhs_value = reg_changed(rhs_value_origin,lhs.type.is_int);
    cur_block->push(new Medium::StoreInstr(lhs.reg, rhs_value));
}

// std::any *tmp_val; 这里应该能重用
void ASTVisitor::visit(syntax_tree::bcr_stmt_syntax &node)
{
    if (node.type == 0)
    { // continue
        cur_block->push(new Medium::JumpInstr(*std::prev(continue_target.end())));
        cur_block = new_block(); // unreachable block
    }
    else if (node.type == 1)
    { // break
        cur_block->push(new Medium::JumpInstr(*std::prev(break_target.end())));
        cur_block = new_block(); // unreachable block
    }
    else  { //return
        assert(mode == normal);
        if (node.exp) {
            if (functions.resolve(cur_func_name)->interface.return_value_non_void) {
                node.exp->accept(*this);
                Medium::Reg ret_value_origin = get_value(to_IRValue(tmp_val));
                Medium::Reg ret_value = reg_changed(ret_value_origin,cur_func->is_int);
                cur_block->push(new Medium::JumpInstr(return_bb));
                return_value.emplace_back(ret_value, cur_block);
            }
        }
        else {
            if ( !functions.resolve(cur_func_name)->interface.return_value_non_void)
            
                cur_block->push(new Medium::JumpInstr(return_bb));
            
        }
        cur_block = new_block(); // unreachable block;
    }
}

//-------------------------completed by ccx----------------------------

void ASTVisitor::visit(syntax_tree::assembly &node)
{
    mode = normal;
    init_func = ir->new_NormalFunc(".init"); // 构建了一个函数名叫做.init的普通函数,这句话报错
    init_bb = init_func->new_BB();           // 在函数中创建一个bb
    init_func->entry = init_bb;              // 将上面创建的BB作为函数的entry BB
    // 初始化一堆东西
    cur_func_name = string{};
    cur_func = nullptr;
    cur_block = nullptr;
    return_bb = nullptr;
    return_value.clear();
    local_var.clear();
    cur_local_table = nullptr;
    in_init = false;
    found_main = false;
    break_target.clear();
    continue_target.clear();
    register_lib_functions();       // 注册库函数
    for (auto i : node.global_defs) // 访问子结点
        i->accept(*this);
    // visitChildren(ctx);
}
void ASTVisitor::visit(syntax_tree::func_def_syntax &node)
{

    bool return_value_non_void = (node.ret_type == "int"||node.ret_type =="float"); // 函数是否有返回值
    string name = node.name;
    vector<pair<string, Type>> params;
    vector<int> arg_type;
    // 处理函数参数
    if (node.params.size()) // 将函数参数的信息添加到params中
    // params = ctx->funcFParams()->accept(this).as<vector<pair<string, Type>>>();
    {
        for (auto i : node.params)
        {
            Type type;
            if (i->is_array)
                type.omit_first_dim = true; // 如果是数组。则需要丢弃第一个维度
            vector<MemSize> ret;
            ret.reserve(i->dimension_of_array.size());
            for (auto i : i->dimension_of_array) {
                int cur = static_cast<syntax_tree::literal_syntax *>(i.get())->intConst; // assume that the dim is int ************
                ret.push_back(static_cast<MemSize>(cur));
            }
            type.array_dims = ret; // 塞入维度信息
            if(i->type=="int") {
                type.is_int=true;
                type.is_float=false;
            }
            else if(i->type=="float") {
                type.is_int=false;
                type.is_float=true;
            }
            params.push_back(pair<string, Type>{i->ident, type});
            if(i->is_array)
                arg_type.push_back(2);//0:int ,float:1,address:3
            else if(i->type=="float")
                arg_type.push_back(1);
            else if(i->type=="int")
                arg_type.push_back(0);

        }
    }
    FunctionInterface interface;
    interface.return_value_non_void = return_value_non_void; // 记录是否需要返回
    if(node.ret_type == "int") {
        interface.is_int=true;
        interface.is_float=false;
    } else if(node.ret_type == "float") {
        interface.is_int=false;
        interface.is_float=true;
    } else if (node.ret_type == "void") {
        interface.is_int = false;
        interface.is_float = false;
    }
    
    for (auto &i : params)
        interface.args_type.emplace_back(i.second);
    cur_func = ir->new_NormalFunc(name);                // 在compiler unit中添加一个普通函数
    cur_func->arg_info=arg_type;
         //设置函数的参数类型信息
    //设置函数返回值类型，之前的符号表中设置了一遍，这边也设置一下吧。
    if(node.ret_type == "int") {
        cur_func->is_int=true;
        cur_func->is_float=false;
    } else if(node.ret_type == "float") {
        cur_func->is_int=false;
        cur_func->is_float=true;
    }else if (node.ret_type == "void") {
        cur_func->is_int=false;
        cur_func->is_float=false;
    }
    functions.register_func(name, cur_func, interface); // 将函数添加到符号表中
    cur_block = cur_func->new_BB();                        // 在函数中创建一个BB
    cur_func->entry = cur_block;                           // 将这个BB设置为function的entry
    cur_local_table = new_variable_table(&global_var);  // 创建一个局部符号表,相当于进入下一层
    if (name == "main")
    { // 主函数的一些操作
        found_main = true;
        cur_block->push(
            new Medium::CallInstr(new_medium_reg(true), init_func, vector<std::pair<Medium::Reg,bool>>{}, true));//set the type to int if the function don't have return value***********
    }
    // 又创建一个ret BB
    return_bb = cur_func->new_BB();
    // 构建entry BB
    for (int i = 0; i < static_cast<int>(params.size()); ++i)
    {
        if (params[i].second.is_array())
        { // 参数是数组
            Medium::Variable *obj =
                cur_func->scope.new_MemObject("arg_" + params[i].first); // 加了arg_的表示数组
            obj->size = 0;
            if(params[i].second.is_int) {
             obj->is_int = true;
             obj->is_float = false;
            }    
            if(params[i].second.is_float) {
                obj->is_int =false;
                obj->is_float = true;
            }
            obj->arg = true;         // 表示是数组
            obj->dims.push_back(-1); //**********
            for (MemSize i : params[i].second.array_dims)
                obj->dims.push_back(static_cast<int>(i));                              // 将维度信息添加到变量中
            cur_func->scope.set_arg(i, obj);                                           // 设置参数的编号
            cur_local_table->register_var(params[i].first, nullptr, params[i].second); // 注册函数参数
            cur_local_table->resolve(params[i].first)->arg_id = i;                     // 设置函数参数的编号
            cur_block->push(new Medium::LocalVarDef(obj));                                    // 将定义变量的语句放到BB中
        }
        else {                                                                        // 普通参数
            Medium::Variable *obj = cur_func->scope.new_MemObject(params[i].first); // 往scope中放入参数
            obj->size = params[i].second.size();                                 // 获取变量的大小（占用内存的大小）
            if(params[i].second.is_int) {
             obj->is_int = true;
             obj->is_float = false;
            }    
            if(params[i].second.is_float) {
                obj->is_int =false;
                obj->is_float = true;
            }
            cur_local_table->register_var(params[i].first, obj, params[i].second);
            cur_block->push(new Medium::LocalVarDef(obj)); // 将定义变量的语句放到BB中
            Medium::Reg value = new_medium_reg(params[i].second.is_int), addr = new_medium_reg(true,INT_SIZE*2);//*******
            cur_block->push(new Medium::LoadArg(value, i));       // 将传过来的参数的值放到寄存器中
            cur_block->push(new Medium::LoadAddr(addr, obj));     // 将参数的地址放到寄存器中
            cur_block->push(new Medium::StoreInstr(addr, value)); // 将传过来的值放到参数对应内存中
        }
    }
    cur_func_name = name;
    // 处理block
    node.body->accept(*this);
    assert(cur_block != nullptr);
    // 处理返回值
    if (return_value_non_void)
    { // 为什么这里要把上一个BB的结果指定为0，这边我感觉是这个BB不可能被执行，所以就弄了个默认值
        Medium::Reg zero = new_medium_reg(true);
        cur_block->push(new Medium::LoadConst(zero, 0));
        cur_block->push(new Medium::JumpInstr(return_bb)); // 当前的BB连接到return block
        return_value.emplace_back(zero, cur_block);    // 这个变量里面保存了可能的返回值可能的返回值
        Medium::Reg ret_value = cur_func->new_Reg(cur_func->is_int);//*****
        Medium::PhiInstr *inst = new Medium::PhiInstr(ret_value); // 针对ret_value的phi指令
        inst->uses = return_value;                        // 指定phi指令的数据来源
        return_bb->push(inst);
        return_bb->push(new Medium::ReturnInstr(ret_value, false));
    }
    else { // 没有返回值，直接忽略返回值
        cur_block->push(new Medium::JumpInstr(return_bb));
        Medium::Reg ret_value = cur_func->new_Reg(true);
        //为了解决，会添上0
        return_bb->push(new Medium::LoadConst(ret_value, 0));
        return_bb->push(new Medium::ReturnInstr(ret_value, true)); // true表示忽略返回值
    }
    // 函数结束后一定要清空这些玩意，否则会导致错误链接
    cur_func = nullptr;
    cur_block = nullptr;
    return_bb = nullptr;
    cur_local_table = nullptr;
    for (VariableTable *i : local_var)
        delete i;
    local_var.clear();
    return_value.clear();
}
void ASTVisitor::visit(syntax_tree::cond_syntax &node)
{
    syntax_tree::relop ops = node.op;
    Medium::BinaryOp::Type opt;
    bool rev;
    if (ops == syntax_tree::relop::less)
    {
        opt = Medium::BinaryOp::LESS;
        rev = false;
    }
    else if (ops == syntax_tree::relop::greater)
    {
        opt = Medium::BinaryOp::LESS;
        rev = true;
    }
    else if (ops == syntax_tree::relop::less_equal)
    {
        opt = Medium::BinaryOp::LEQ;
        rev = false;
    }
    else if (ops == syntax_tree::relop::greater_equal)
    {
        opt = Medium::BinaryOp::LEQ;
        rev = true;
    }
    else if (ops == syntax_tree::relop::equal)
    {
        opt = Medium::BinaryOp::EQ;
    }
    else if (ops == syntax_tree::relop::non_equal)
    {
        opt = Medium::BinaryOp::NEQ;
    }

    //上面是获取操作类型，下面是进行对应类型的操作

    if (ops == syntax_tree::relop::op_none)
    { // exp
        decltype(mode) tmp_mode;
        tmp_mode=mode;
        mode = normal;
        node.rhs->accept(*this); // 结果通过tmp_val传递
        mode = tmp_mode;
    }
    else if (ops == syntax_tree::relop::op_and || ops == syntax_tree::relop::op_or) // 需要短路的操作
    {

       
        if (mode == normal)
        { // 普通模式，只需要获得值，参与计算
            node.lhsc->accept(*this);
            IRValue lhs = to_IRValue(tmp_val);  // 获得左cond语句的结果，0或1
            Medium::Reg lhs_value = get_value(lhs); // 获得cond值所在的寄存器的编号
            // 后面的操作和to_IRValue中的类似
            Medium::IrBlock *lhs_end = cur_block, *rhs_entry = new_block(), *res_entry = new_block();


            lhs_end->push(new Medium::BranchInstr(lhs_value, rhs_entry, res_entry)); // 失败了直接跳转到resBB,否则到rhsBB
            cur_block = rhs_entry;
            node.rhsc->accept(*this);
            IRValue rhs = to_IRValue(tmp_val);
            Medium::Reg rhs_value = get_value(rhs);
            cur_block->push(new Medium::JumpInstr(res_entry)); // 跳转到resBB
            Medium::IrBlock *rhs_end = cur_block;
            cur_block = res_entry;
            // 根据是从lhsBB跳转过来的还是从rhsBB跳转过来的，决定最后的值
            Medium::Reg res_reg = new_medium_reg(true);//condition value is int type
            Medium::PhiInstr *phi_inst = new Medium::PhiInstr(res_reg);
            phi_inst->uses.emplace_back(lhs_value, lhs_end);
            phi_inst->uses.emplace_back(rhs_value, rhs_end);
            cur_block->push(phi_inst);
            IRValue ret;
            ret.is_left_value = false;
            ret.reg = res_reg;
            tmp_val = ret;
        }
        else // conditon模式，需要实现短路求值和Br,就是在填各种跳转方向
        {
            node.lhsc->accept(*this);                    // condition模式，用于构建
            CondJumpList lhs = to_CondJumpList(tmp_val); // 左操作数建立一个branch?
            // 后面的做法可以实现短路求值
            cur_block = new_block();                     // 下一个BB，但是是给谁用的呢？我猜是给右操作数用的，应该是的
            if (ops == syntax_tree::relop::op_and) // and 只有成功了才会看rhsc
            {
                for (Medium::IrBlock **i : lhs.true_list)
                    (*i) = cur_block;
            }
            else // or 只有失败了才会看rhsc
            {
                for (Medium::IrBlock **i : lhs.false_list)
                    (*i) = cur_block;
            }

            node.rhsc->accept(*this);                    // true指向下一个BB,左操作数成功了就执行下一个BB，false留到后续进行填写
            CondJumpList rhs = to_CondJumpList(tmp_val); // 又操作作数
            CondJumpList ret = std::move(rhs);           // ret的行为和rhs的保持一致
            if (ops == syntax_tree::relop::op_and)       // and 的两个操作数的false目标一样
            {
                for (Medium::IrBlock **i : lhs.false_list)
                    ret.false_list.push_back(i);
            }
            else
            {
                for (Medium::IrBlock **i : lhs.true_list)
                    ret.true_list.push_back(i);
            }
            tmp_val = ret;
        }
    }
    else if (ops == syntax_tree::relop::op_not) // 置反
    {

        if (mode == normal) // 只是计算出一个值，创建(处理)BB
        {
            IRValue ret;
            ret.is_left_value = false;
            node.rhsc->accept(*this);
            IRValue rhs = to_IRValue(tmp_val);
            Medium::Reg rhs_value = get_value(rhs);
            Medium::Reg res_value = rhs_value;
            switch (node.op)
            {
            case syntax_tree::relop::op_not:
                res_value = new_medium_reg(true);
                cur_block->push(new Medium::UnaryOpInstr(res_value, rhs_value,
                                                  Medium::UnaryOp(Medium::UnaryOp::LNOT)));
                break;
            default:; //+, do nothing
            }
            ret.reg = res_value;
            tmp_val = ret;
        }
        else // condition 模式，需要设置跳转，连接BB
        {
            node.rhsc->accept(*this);
            CondJumpList rhs = to_CondJumpList(tmp_val); // 构建Br指令
            std::swap(rhs.true_list, rhs.false_list);    // 跳转的对象互换，填写是在if那边
            tmp_val = rhs;
        }
    }
    else if (ops == syntax_tree::relop::op_minus )
    {
        //  decltype(mode) tmp_mode;
        // tmp_mode=mode;
        // mode = normal;
        // if (mode == normal) // 只是计算出一个值
        // {
        //     IRValue ret;
        //     ret.is_left_value = false;
        //     node.rhsc->accept(*this);
        //     IRValue rhs = to_IRValue(tmp_val);
        //     IR::Reg rhs_value = get_value(rhs);
        //     IR::Reg res_value = new_reg(true);//默认是int寄存器，如果遇到float，后面会进行转化
        //     cur_block->push(new IR::UnaryOpInstr(res_value, rhs_value,IR::UnaryOp(IR::UnaryOp::NEG)));
        //     ret.reg = res_value;
        //     tmp_val = ret;
        // }
        //  // condition 模式，什么都不用做，跳转的方向和child一样
        //  mode=tmp_mode;

        decltype(mode) tmp_mode;
        tmp_mode=mode;
        mode = condition;
    
            IRValue ret;
            ret.is_left_value = false;
            node.rhsc->accept(*this);
            IRValue rhs = to_IRValue(tmp_val);
            Medium::Reg rhs_value = get_value(rhs);
            Medium::Reg res_value = new_medium_reg(rhs_value.is_int);
            cur_block->push(new Medium::UnaryOpInstr(res_value, rhs_value,Medium::UnaryOp(Medium::UnaryOp::NEG)));
            ret.reg = res_value;
            tmp_val = ret;
         
         // condition 模式，什么都不用做，跳转的方向和child一样
         mode=tmp_mode;
        
           
    }
    else if (ops == syntax_tree::relop::op_plus)
    {
        decltype(mode) tmp_mode;
        tmp_mode=mode;
        mode = condition;
        node.rhsc->accept(*this);    
         mode=tmp_mode;
       
        
    }
    else // 大小比较
    {
        decltype(mode) tmp_mode;
        tmp_mode=mode;
        mode = condition;
         //if (mode == normal){
        Medium::BinaryOp op{opt};
        ValueMode prev_mode = mode;
        mode = normal;
        node.lhsc->accept(*this);
        IRValue lhs = to_IRValue(tmp_val);
        node.rhsc->accept(*this);
        IRValue rhs = to_IRValue(tmp_val);
        Medium::Reg lhs_reg_origin = get_value(lhs), rhs_reg_origin = get_value(rhs);
        bool res_is_int = lhs_reg_origin.is_int && rhs_reg_origin.is_int;
        Medium::Reg res_reg = new_medium_reg(true);
    
        Medium::Reg lhs_reg = reg_changed(lhs_reg_origin,res_is_int);
        Medium::Reg rhs_reg = reg_changed(rhs_reg_origin,res_is_int);
        if (rev)
            std::swap(lhs_reg, rhs_reg); // 这个的作用是减少比较指令的个数,>变为<
        cur_block->push(new Medium::BinaryOpInstr(res_reg, lhs_reg, rhs_reg, op));
        IRValue ret;
        ret.is_left_value = false;
        ret.reg = res_reg;
        mode = prev_mode;
        tmp_val = ret;
        // }
         // condition 模式，什么都不用做，跳转的方向和child一样
         mode=tmp_mode;
        
    }
}
void ASTVisitor::visit(syntax_tree::binop_expr_syntax &node)
{
    bool res_is_float=false;
    ValueMode prev_mode = mode;
    mode = normal;
    // 获取操作数
    node.lhs->accept(*this);
    IRValue lhs = to_IRValue(tmp_val);
    if(lhs.type.is_float) res_is_float=true;
    node.rhs->accept(*this);
    IRValue rhs = to_IRValue(tmp_val);
    if(rhs.type.is_float) res_is_float=true;
    // 获取操作数所在的寄存器
    Medium::Reg lhs_reg_origin = get_value(lhs), rhs_reg_origin = get_value(rhs),
            res_reg = new_medium_reg(!res_is_float);
    
    Medium::Reg lhs_reg = reg_changed(lhs_reg_origin,!res_is_float);
    Medium::Reg rhs_reg = reg_changed(rhs_reg_origin,!res_is_float);
    
    // 添加操作指令
    switch (node.op)
    {
    case syntax_tree::binop::plus:
        cur_block->push(new Medium::BinaryOpInstr(res_reg, lhs_reg, rhs_reg,
                                           Medium::BinaryOp(Medium::BinaryOp::ADDW)));
        break;
    case syntax_tree::binop::minus:
        cur_block->push(new Medium::BinaryOpInstr(res_reg, lhs_reg, rhs_reg,
                                           Medium::BinaryOp(Medium::BinaryOp::SUBW)));
        break;
    case syntax_tree::binop::multiply:
        cur_block->push(new Medium::BinaryOpInstr(res_reg, lhs_reg, rhs_reg,
                                           Medium::BinaryOp(Medium::BinaryOp::MULW)));
        break;
    case syntax_tree::binop::divide:
        cur_block->push(new Medium::BinaryOpInstr(res_reg, lhs_reg, rhs_reg,
                                           Medium::BinaryOp(Medium::BinaryOp::DIVW)));
        break;
    case syntax_tree::binop::modulo:
        cur_block->push(new Medium::BinaryOpInstr(res_reg, lhs_reg, rhs_reg,
                                           Medium::BinaryOp(Medium::BinaryOp::MODW)));
        break;
    }
    IRValue ret;
    ret.is_left_value = false;
    ret.reg = res_reg;
    if(res_is_float)
    {
        ret.type.is_float=true;
        ret.type.is_int=false;
    }
    else 
    {
        ret.type.is_int=true;
        ret.type.is_float=false;
    }

    mode = prev_mode;
    tmp_val = ret;
}
void ASTVisitor::visit(syntax_tree::unaryop_expr_syntax &node)
{
    bool res_is_float=false;
    IRValue ret;
    ret.is_left_value = false;
    node.rhs->accept(*this);
    IRValue rhs = to_IRValue(tmp_val);
    if(rhs.type.is_float)res_is_float=true;
    Medium::Reg rhs_value = get_value(rhs);
    Medium::Reg res_value = rhs_value;
    switch (node.op)
    {
    case syntax_tree::unaryop::minus:
        res_value = new_medium_reg(rhs.type.is_int);//*********8
        cur_block->push(new Medium::UnaryOpInstr(res_value, rhs_value,Medium::UnaryOp(Medium::UnaryOp::NEG)));
        break;
    case syntax_tree::unaryop::plus:
        break;
    default:; //+, do nothing
    }
    ret.reg = res_value;
    if(res_is_float)ret.type.is_float=true;if(res_is_float)
    {
        ret.type.is_float=true;
        ret.type.is_int=false;
    }
    else 
    {
        ret.type.is_int=true;
        ret.type.is_float=false;
    }
    tmp_val = ret;
}
void ASTVisitor::visit(syntax_tree::func_call_syntax &node)
{
    string func_name = node.name;
    FunctionTableEntry *entry = functions.resolve(func_name);
    vector<variant<Medium::Variable *, IRValue>> args;
    //如果有参数
    if (node.array_index.size() != 0)
    { //****************
        for (auto i : node.array_index)
        {
            //将参数放到args中，其中参数有两种类型，一种是通过寄存器进行传递的，另一种是通过栈空间进行传递的
            i->accept(*this);
            
            if (std::any_cast<IRValue>(&tmp_val))
                args.emplace_back(std::any_cast<IRValue>(tmp_val));
            else
                args.emplace_back(std::any_cast<Medium::Variable *>(tmp_val));
        }
    }
    vector<std::pair<Medium::Reg,bool>> arg_regs;
    //遍历参数并
    for (size_t i = 0; i < args.size(); ++i)
    {
        if (IRValue *cur = std::get_if<IRValue>(&args[i]))
        {
            if (i < entry->interface.args_type.size())
            {//检查参数类型，并进行转化
                if (Type *interface_type =std::get_if<Type>(&entry->interface.args_type[i]))
                {
                    if (interface_type->is_array())//数组直接放入
                        arg_regs.push_back({cur->reg,true});
                    else {//普通变量检查是否需要转化
                        Medium::Reg origin_reg = get_value(*cur);
                        Medium::Reg res_reg = reg_changed(origin_reg,interface_type->is_int);
                        arg_regs.push_back({res_reg,false});
                    }
                }
            }
            else
                arg_regs.push_back({get_value(*cur),false});
            
        }
        else
        {
            Medium::Variable *&cur_str = std::get<Medium::Variable *>(args[i]);
            if (i < entry->interface.args_type.size())
            {
                if (std::get_if<StringType>(&entry->interface.args_type[i])) {
                    Medium::Reg addr = new_medium_reg(true,INT_SIZE*2);
                    cur_block->push(new Medium::LoadAddr(addr, cur_str));
                    arg_regs.push_back({addr,true});
                }
            }
            else
            { //这里面我也不知道是干什么的
                Medium::Reg addr = new_medium_reg(true,INT_SIZE*2);
                cur_block->push(new Medium::LoadAddr(addr, cur_str));
                arg_regs.push_back({addr,true});
            }
        }
    }
    if (func_name == "starttime" || func_name == "stoptime")
    {
        // int line_no = static_cast<int>(ctx->start->getLine());
        int line_no = static_cast<int>(node.line); // modified by ccx
        Medium::Reg line_no_reg = new_medium_reg(true);//*********?
        cur_block->push(new Medium::LoadConst(line_no_reg, line_no));
        arg_regs.push_back({line_no_reg,false});
    }
    Medium::Reg return_value = new_medium_reg(entry->interface.is_int);//*****************8
    cur_block->push(new Medium::CallInstr(return_value, entry->ir_func, arg_regs, false));
    
    // auto return_entry = resolve(cur_func_name);
    IRValue ret;
    //ret.reg.is_int=entry->interface.is_int;
    ret.is_left_value = false;
    ret.reg = return_value;
    ret.type.is_int = entry->interface.is_int;
    ret.type.is_float = !entry->interface.is_int;
    tmp_val = ret; // Global Value
    // auto ignorance = std::any_cast<IRValue>(tmp_val);
}

void ASTVisitor::visit(syntax_tree::func_call_stmt_syntax &node)
{
    string func_name = node.name;
    FunctionTableEntry *entry = functions.resolve(func_name);
    vector<variant<Medium::Variable *, IRValue>> args;
    if (node.array_index.size())
    { //****************
        for (auto i : node.array_index)
        {
            i->accept(*this);
            
            if (std::any_cast<IRValue>(&tmp_val))
                args.emplace_back(std::any_cast<IRValue>(tmp_val));
            else
                args.emplace_back(std::any_cast<Medium::Variable *>(tmp_val));//这行有问题
        }
    }
    vector<std::pair<Medium::Reg,bool>> arg_regs;
    for (size_t i = 0; i < args.size(); ++i)
    {
        if (IRValue *cur = std::get_if<IRValue>(&args[i]))
        {
            if (i < entry->interface.args_type.size())
            {
                if (Type *interface_type =
                        std::get_if<Type>(&entry->interface.args_type[i]))
                {
                    if (interface_type->is_array())
                        arg_regs.push_back({cur->reg,true});
                    else {
                        Medium::Reg origin_reg = get_value(*cur);
                        Medium::Reg res_reg = reg_changed(origin_reg,interface_type->is_int);
                        arg_regs.push_back({res_reg,false});
                    }
                }
            }
            else
                arg_regs.push_back({get_value(*cur),false});
            
        }
        else
        {
            Medium::Variable *&cur_str = std::get<Medium::Variable *>(args[i]);
            if (i < entry->interface.args_type.size())
            {
                if (std::get_if<StringType>(&entry->interface.args_type[i]))
                {
                    Medium::Reg addr = new_medium_reg(true,INT_SIZE*2);
                    cur_block->push(new Medium::LoadAddr(addr, cur_str));
                    arg_regs.push_back({addr,true});
                }
            }
            else
            {
                Medium::Reg addr = new_medium_reg(true,INT_SIZE*2);
                cur_block->push(new Medium::LoadAddr(addr, cur_str));
                arg_regs.push_back({addr,true});
            }
        }
    }
    if (func_name == "starttime" || func_name == "stoptime")
    {
        // int line_no = static_cast<int>(ctx->start->getLine());
        int line_no = static_cast<int>(node.line); // modified by ccx
        Medium::Reg line_no_reg = new_medium_reg(true);
        cur_block->push(new Medium::LoadConst(line_no_reg, line_no));
        arg_regs.push_back({line_no_reg,false});
    }
    cur_block->push(new Medium::CallInstr(new_medium_reg(entry->interface.is_int || (!entry->interface.is_int && !entry->interface.is_float)), entry->ir_func, arg_regs, true));//**********
    return;
}

void ASTVisitor::visit(syntax_tree::block_syntax &node)
{
    VariableTable *parent = cur_local_table;
    VariableTable *child = new_variable_table(parent);
    cur_local_table = child;
    // modified by ccx****
    // node.body->accept(*this);
    for (auto i : node.body)
        i->accept(*this);
    cur_local_table = parent;
    return;
}

void ASTVisitor::visit(syntax_tree::if_stmt_syntax &node)
{
    // ccx modified it
    mode = condition; // 切换为condition模式
    node.pred->accept(*this);
    mode = normal;
    //-----------
    CondJumpList cond =  to_CondJumpList(tmp_val); //*************modified by ccx
    if (node.else_body == nullptr)
    {
        VariableTable *parent = cur_local_table;
        VariableTable *child = new_variable_table(parent);
        Medium::IrBlock *true_entry = new_block();
        cur_block = true_entry;
        cur_local_table = child;
        node.then_body->accept(*this);
        Medium::IrBlock *true_end = cur_block;
        Medium::IrBlock *out = new_block();
        cur_local_table = parent;
        cur_block = out;
        true_end->push(new Medium::JumpInstr(out));
        for (Medium::IrBlock **i : cond.true_list)
            (*i) = true_entry;
        for (Medium::IrBlock **i : cond.false_list)
            (*i) = out;
    }
    else
    {
        VariableTable *parent = cur_local_table;
        VariableTable *child_true = new_variable_table(parent),
                      *child_false = new_variable_table(parent);
        Medium::IrBlock *true_entry = new_block();
        cur_block = true_entry;
        cur_local_table = child_true;
        node.then_body->accept(*this);
        Medium::IrBlock *true_end = cur_block;
        Medium::IrBlock *false_entry = new_block();
        cur_block = false_entry;
        cur_local_table = child_false;
        node.else_body->accept(*this);
        Medium::IrBlock *false_end = cur_block;
        Medium::IrBlock *out = new_block();
        cur_local_table = parent;
        cur_block = out;
        true_end->push(new Medium::JumpInstr(out));
        false_end->push(new Medium::JumpInstr(out));
        for (Medium::IrBlock **i : cond.true_list)
            (*i) = true_entry;
        for (Medium::IrBlock **i : cond.false_list)
            (*i) = false_entry;
    }
    return;
}

void ASTVisitor::visit(syntax_tree::while_stmt_syntax &node)
{
    Medium::IrBlock *cond_entry = new_block();
    cur_block->push(new Medium::JumpInstr(cond_entry));
    cur_block = cond_entry;
    // ccx modified it
    mode = condition; // 切换为condition模式
    node.pred->accept(*this);
    mode = normal;
    //-----------
    CondJumpList cond = to_CondJumpList(tmp_val); // Gloable variable
    Medium::IrBlock *out = new_block(), *body_entry = new_block(), *jump_back = new_block();
    VariableTable *parent = cur_local_table;
    VariableTable *child = new_variable_table(parent);
    cur_block = body_entry;
    cur_local_table = child;
    break_target.push_back(out);
    continue_target.push_back(jump_back);
    node.body->accept(*this);
    break_target.pop_back();
    continue_target.pop_back();
    Medium::IrBlock *body_end = cur_block;
    body_end->push(new Medium::JumpInstr(jump_back));
    jump_back->push(new Medium::JumpInstr(cond_entry));
    cur_block = out;
    cur_local_table = parent;
    for (Medium::IrBlock **i : cond.true_list)
        (*i) = body_entry;
    for (Medium::IrBlock **i : cond.false_list)
        (*i) = out;
    return;
}

void ASTVisitor::visit(syntax_tree::empty_stmt_syntax &node)
{
    return; // do nothing
}

void ASTVisitor::visit(syntax_tree::func_param_syntax &node)
{
    // do nothing
}
