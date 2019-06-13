#include <cstdlib>
#include <iostream>
#include <vector>

#include "bstree.h"

// g++ bstree.cpp && ./a.out

int main(int argc, char const *argv[]) {
  BSTree<int> tree;

  std::vector<int> values;
  for (size_t i = 0; i < 1000; i++) {
    values.push_back(rand());
    tree.insert(values.back());
  }
  for (size_t i = 0; i < 1000; i++) {
    assert(tree.find(values[i]) != nullptr);
    auto t = tree.find(values[i]);
    if (t->right && (rand() % 3 == 0)) {
      tree.leftRotate(t);
    }
    if (t->left && (rand() % 3 == 0)) {
      tree.rightRotate(t);
    }
  }
  int last = tree.minimum(tree.root.get())->key;
  for (auto i = tree.minimum(tree.root.get()); i != nullptr;
       i = tree.next(i)) {
    assert(last <= i->key);
    last = i->key;
  }
  last = tree.maximum(tree.root.get())->key;
  for (auto i = tree.maximum(tree.root.get()); i != nullptr; i = tree.prev(i)) {
    assert(last >= i->key);
    last = i->key;
  }
  for (size_t i = 0; i < 1000; i++) {
    assert(tree.erase(values[i]));
  }
  for (size_t i = 0; i < 1000; i++) {
    assert(!tree.erase(values[i]));
    assert(tree.find(values[i]) == nullptr);
  }

  return 0;
}
