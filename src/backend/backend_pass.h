#include "code_gen.h"
#include <vector>
#include "inst.h"
#include <map>
#include <limits>
using std::unique_ptr;
using std::make_unique;

namespace Archriscv {
namespace backend_pass {
static std::map<int32_t, int> log2_map = []() {
    std::map<int32_t, int> ret;
    for (int i = 0; i < 31; ++i) ret[1 << i] = i;
    ret[std::numeric_limits<int32_t>::min()] = 31;
    return ret;
}();
class backend_pass_base {
protected:
    Program* program_;
    std::string name_;
public:
    backend_pass_base(Program* program) : program_(program) { };
    ~backend_pass_base() = default;
    void set_name(std::string name) {this->name_ = name;}
    std::string get_name() {return this->name_;}
    virtual void run() = 0;
};
class inline_constant : public backend_pass_base {
public:
    inline_constant(Program* program_) : backend_pass_base(program_) {};
    ~inline_constant() = default;
    virtual void run() override;
protected:
    unique_ptr<Inst> opt_regreg(RegRegInst *rr);
    Function* func;
};
class remove_unused_before_alloc : public backend_pass_base {
public:
   remove_unused_before_alloc(Program* program_) : backend_pass_base(program_) {};
    ~remove_unused_before_alloc() = default;
    virtual void run() override;
protected:
    // Function* func;
    void remove_unused(Function *func);
};

class merge_addi_lw_sw : public backend_pass_base
{
public:
    merge_addi_lw_sw(Program* program) : backend_pass_base(program) { };
    ~merge_addi_lw_sw() = default;
    virtual void run() override;
protected:
    void merge_inst(Function *func);
    bool check_def(Inst *def){
      if (RegImmInst *ri = def->as<RegImmInst>()) {
        return ri->op == RegImmInst::Addi;
      } else
        return false;
    };
    std::unique_ptr<Inst> check_use(Inst *use, Reg r, Inst *def) {
      RegImmInst *ri = def->as<RegImmInst>();
      assert(ri->dst == r);
      if (Load *lw = use->as<Load>()) {
        if (lw->base == r && is_imm12(lw->offset + ri->rhs))
          return make_unique<Load>(lw->dst, ri->lhs, lw->offset + ri->rhs);
      } else if (Store *sw = use->as<Store>()) {
        if (sw->base == r && is_imm12(sw->offset + ri->rhs))
          return make_unique<Store>(sw->src, ri->lhs, sw->offset + ri->rhs);
      }
      return nullptr;
    };
};

class opt_after_reg_allocate : public backend_pass_base 
{
public:
    opt_after_reg_allocate(Program* program) : backend_pass_base(program) { };
    ~opt_after_reg_allocate() = default;
    virtual void run() override;
protected:
    void remove_identical_move(Function *func);
    void remove_sequential_move(Function* func);
    void direct_jump(Function *func);
    void remove_unused_after_alloc(Function  *func);
};



class backend_pass_manager {
protected:
    Program* program_;
    std::vector<backend_pass_base*> my_pass;
public:
    backend_pass_manager(Program* program) : program_(program) { };
    ~backend_pass_manager() = default;
    void add_backend_pass(std::string pass_name);
    void run_pass();
    void run_opt_after_reg_allocate();
};
} // namespace backend_pass
}