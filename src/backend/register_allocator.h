#pragma once
#include"code_gen.h"
#include <queue>
#include <set>
#include <utility>
#include <vector>

namespace Archriscv {

struct RegAllocStat;
struct Function;
// struct RegFilter;

// no coalescing
class SimpleColoringAllocator {
  Function *func;
  std::vector<unsigned char> appear;  // in fact it's boolean array
  std::vector<std::set<int>> conflict_edges;
  std::queue<int> deletable_nodes;
  std::vector<std::pair<int, std::vector<int>>> delete_history;
  std::set<int> remain_virual_nodes;
  RegFilter filter;
  bool is_float;

  void build_conflict_graph();  // build occur, interfere_edge
  void spill(const std::vector<int> &spill_nodes,std::vector<int> all_size);
  void remove(int id);
  void delete_node();
  void clear();
  int choose_spill_node();
  void get_alloc_result(std::vector<int> &ans);

 public:
  SimpleColoringAllocator(Function *_func);
  std::vector<int> run(bool *succeed,bool is_float=false);
};

}  // namespace Archriscv