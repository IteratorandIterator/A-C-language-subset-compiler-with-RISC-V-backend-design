#include <antlr4-runtime.h>
#include <SYSYLexer.h>
#include <SYSYParser.h>
#include <recognizer.h>
#include <syntax_tree_builder.h>

using namespace ast;
using namespace syntax_tree;

using namespace antlr4;
using namespace antlrcpp;

recognizer::recognizer(const std::string &input_string) : ast(nullptr)
{
    input = new ANTLRInputStream(input_string);
}

recognizer::recognizer(std::istream &input_stream) : ast(nullptr)
{
    input = new ANTLRInputStream(input_stream);
}

std::shared_ptr<syntax_tree::syntax_tree_node> recognizer::get_syntax_tree() { return ast; }

recognizer::~recognizer()
{
    delete input;
}

bool recognizer::execute()
{
    SYSYLexer lexer(input);
    CommonTokenStream tokens(&lexer);
    SYSYParser parser(&tokens);

    // Change the `exp` to the non-terminal name you want to examine as the top level symbol.
    // It should be `compilationUnit` for final submission.
    auto tree = parser.compilationUnit();

    syntax_tree_builder ast_builder;
    ast = ast_builder(tree);
    return true;
}
