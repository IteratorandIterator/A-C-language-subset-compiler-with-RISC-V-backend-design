#pragma once


#include <vector>
#include <memory>
#include <string>

namespace ast
{
namespace syntax_tree
{
// Use unique postd::stringtype to reference stored objects
template <typename T>
using ptr = std::shared_ptr<T>;

// List of reference of type
template <typename T>
using ptr_list = std::vector<ptr<T>>;

// Enumerations
enum class relop
{
    equal = 0,
    non_equal,
    less,
    less_equal,
    greater,
    greater_equal,
    op_and,//ccx adds it
    op_or,
    op_not,
    op_none,
    op_plus,
    op_minus
};
enum class binop
{
    plus = 0,
    minus,
    multiply,
    divide,
    modulo
};

enum class unaryop
{
    plus = 0,
    minus
};

// Forward declaration
struct syntax_tree_node;
struct assembly;
struct global_def_syntax;
struct func_def_syntax;
struct cond_syntax;

struct expr_syntax;
struct binop_expr_syntax;
struct unaryop_expr_syntax;
struct lval_syntax;  //zovijxdm
struct literal_syntax;

struct stmt_syntax;
struct var_def_stmt_syntax;
//struct var_def_stmt_syntaxs;
struct assign_stmt_syntax;
struct func_call_stmt_syntax;
struct func_param_syntax;
struct block_syntax;
struct if_stmt_syntax;
struct while_stmt_syntax;
struct bcr_stmt_syntax;
struct func_call_syntax;
struct syntax_tree_visitor;

// Virtual base of all kinds of syntax tree nodes.
struct syntax_tree_node
{
    int line;
    int pos;
    // Used in syntax_tree_visitor. Irrelevant to syntax tree generation.
    virtual void accept(syntax_tree_visitor &visitor) = 0;
};

// Root node of an ordinary syntax tree.
struct assembly : syntax_tree_node
{
    ptr_list<global_def_syntax> global_defs;
    virtual void accept(syntax_tree_visitor &visitor) override final;
};

// Virtual base of lobal definitions, function or variable one.
struct global_def_syntax : virtual syntax_tree_node
{
    virtual void accept(syntax_tree_visitor &visitor) override = 0;
};

// Function definition.
struct func_def_syntax : global_def_syntax
{
    std::string name;
    ptr<block_syntax> body;
    std::string ret_type;//和老肖的统一一下，不过用枚举类会不会更好
    ptr_list<func_param_syntax> params;
    virtual void accept(syntax_tree_visitor &visitor) override final;
};

// Condition expression. (Not actually treated as expression, enough for C1)
struct cond_syntax : syntax_tree_node
{
    relop op;
    ptr<expr_syntax> rhs;
    ptr<cond_syntax> lhsc;
    ptr<cond_syntax> rhsc;//ccx adds it
    virtual void accept(syntax_tree_visitor &visitor) override final;
};

// Virtual base of expressions.
struct expr_syntax : virtual syntax_tree_node
{
        bool is_global=false;
    virtual void accept(syntax_tree_visitor &visitor) = 0;
};

// Expression like `lhs op rhs`.
struct binop_expr_syntax : expr_syntax
{
    binop op;
    ptr<expr_syntax> lhs, rhs;
    bool is_int;
    bool is_preclaced;//whether the exp is calculated
    int intConst;
    double floatConst;
    virtual void accept(syntax_tree_visitor &visitor) override final;
};

// Expression like `op rhs`.
struct unaryop_expr_syntax : expr_syntax
{
    unaryop op;
    ptr<expr_syntax> rhs;
    bool is_int;//判断是不是int类型的
    bool is_preclaced;
    int intConst;
    double floatConst;
    virtual void accept(syntax_tree_visitor &visitor) override final;
};

// Expression like `ident` or `ident[exp]`.
struct lval_syntax : expr_syntax
{
    std::string name;
    ptr_list<expr_syntax> array_index; // nullptr if not indexed as array
    virtual void accept(syntax_tree_visitor &visitor) override final;
};
//function call
struct func_call_syntax : expr_syntax
{
    std::string name;//函数名
    ptr_list<expr_syntax> array_index; // 参数列表
    virtual void accept(syntax_tree_visitor &visitor) override final;
};

// Expression constructed by a literal number.
struct literal_syntax : expr_syntax
{
    bool is_int;

    int intConst;
    double floatConst;
    virtual void accept(syntax_tree_visitor &visitor) override final;
};

// Virtual base for statements.
struct stmt_syntax : virtual syntax_tree_node
{
    virtual void accept(syntax_tree_visitor &visitor) = 0;
};
//for this node, you can keep a groups of var_def_stmt_syntax , this is a middle node
//ccx delete it******************
// struct var_def_stmt_syntaxs : stmt_syntax, global_def_syntax
// {
//     ptr_list<var_def_stmt_syntax> defs;
//     virtual void accept(syntax_tree_visitor &visitor) override final;
// };

// Variable definition. Multiple of this would be both a statement and a global definition; however, itself only
// represents a single variable definition.
struct var_def_stmt_syntax : stmt_syntax, global_def_syntax
{
    bool is_constant;
    bool is_int;
    std::string name;
    ptr_list<expr_syntax> array_length; // nullptr for non-array variables
    ptr_list<expr_syntax> initializers;
    ptr_list<int> initializers_index;
    virtual void accept(syntax_tree_visitor &visitor) override final;
};

// Assignment statement.
struct assign_stmt_syntax : stmt_syntax
{
    ptr<lval_syntax> target;
    ptr<expr_syntax> value;
    virtual void accept(syntax_tree_visitor &visitor) override final;
};

// Function call statement.
struct func_call_stmt_syntax : stmt_syntax
{
    std::string name;
    ptr_list<expr_syntax> array_index; // 参数列表
    virtual void accept(syntax_tree_visitor &visitor) override final;
};

struct func_param_syntax : stmt_syntax
{
    ptr_list<expr_syntax> dimension_of_array;
    std::string type;
    std::string ident;
    bool is_array;
    virtual void accept(syntax_tree_visitor &visitor) override final;
};

// Block statement.
struct block_syntax : stmt_syntax
{
    ptr_list<stmt_syntax> body;
    virtual void accept(syntax_tree_visitor &visitor) override final;
};

// If statement.
struct if_stmt_syntax : stmt_syntax
{
    ptr<cond_syntax> pred;
    ptr<stmt_syntax> then_body;
    ptr<stmt_syntax> else_body;
    virtual void accept(syntax_tree_visitor &visitor) override final;
};

// While statement.
struct while_stmt_syntax : stmt_syntax
{
    ptr<cond_syntax> pred;
    ptr<stmt_syntax> body;
    virtual void accept(syntax_tree_visitor &visitor) override final;
};

//continue,break,return  0,1,2
struct bcr_stmt_syntax: stmt_syntax
{
    int type;
    ptr<expr_syntax> exp;
    virtual void accept(syntax_tree_visitor &visitor) override final;
};

// Empty statement (aka a single ';').
struct empty_stmt_syntax : stmt_syntax
{
    virtual void accept(syntax_tree_visitor &visitor) override final;
};

// Visitor base type
class syntax_tree_visitor
{
  public:
    virtual void visit(assembly &node) = 0;
    virtual void visit(func_def_syntax &node) = 0;
    virtual void visit(cond_syntax &node) = 0;
    virtual void visit(binop_expr_syntax &node) = 0;
    virtual void visit(unaryop_expr_syntax &node) = 0;
    virtual void visit(lval_syntax &node) = 0;
    virtual void visit(literal_syntax &node) = 0;
    virtual void visit(var_def_stmt_syntax &node) = 0;
    virtual void visit(assign_stmt_syntax &node) = 0;
    virtual void visit(func_call_stmt_syntax &node) = 0;
    virtual void visit(block_syntax &node) = 0;
    virtual void visit(if_stmt_syntax &node) = 0;
    virtual void visit(while_stmt_syntax &node) = 0;
    virtual void visit(empty_stmt_syntax &node) = 0;
    virtual void visit(func_param_syntax &node) = 0;
    virtual void visit(bcr_stmt_syntax &node) = 0;
    virtual void visit(func_call_syntax &node) = 0;
};
} // end namespace syntax_tree
} // end namespace c1_recognizer
