#pragma once
#include <syntax_tree.h>
#include <string>
#include <iostream>

namespace antlr4
{
class ANTLRInputStream;
}

namespace ast
{

class recognizer
{
  public:
    recognizer() = delete;
    recognizer(const std::string &input_string);
    recognizer(std::istream &input_stream);

    bool execute();
    std::shared_ptr<syntax_tree::syntax_tree_node> get_syntax_tree();

    ~recognizer();

  private:
    std::shared_ptr<syntax_tree::syntax_tree_node> ast;
    antlr4::ANTLRInputStream *input;
    std::string source;
};
}

