
#include <algorithm>
#include <iostream>
#include <vector>

#include "bstree.h"

// g++ avltree.cpp && ./a.out

template <typename KEY>
class AVLTree : public BSTree<KEY> {
 public:
  class AVLNode : public BSTree<KEY>::BSNode {
   public:
    int height = 0;
    AVLNode(const KEY &k) : BSTree<KEY>::BSNode(k) {}
    AVLNode *Left() { return (AVLNode *)this->left.get(); }
    AVLNode *Right() { return (AVLNode *)this->right.get(); }
    AVLNode *Parent() { return (AVLNode *)this->parent; }
    int Height() { return height; }
    void adjustHeight() {
      height = 1 + std::max(Left() ? Left()->Height() : 0,
                            Right() ? Right()->Height() : 0);
    }
  };
  AVLNode *leftRotate(AVLNode *node) {
    BSTree<KEY>::leftRotate(node);
    node->adjustHeight();
    node->Parent()->adjustHeight();
    return node->Parent();
  }
  AVLNode *rightRotate(AVLNode *node) {
    BSTree<KEY>::rightRotate(node);
    node->adjustHeight();
    node->Parent()->adjustHeight();
    return node->Parent();
  }
  AVLNode *Root() { return (AVLNode *)this->root.get(); }
  AVLNode *balance(AVLNode *node) {
    auto lh = node->Left() ? node->Left()->Height() : 0;
    auto rh = node->Right() ? node->Right()->Height() : 0;
    if (lh - rh > 1) {
      rh = node->Left()->Right() ? node->Left()->Right()->Height() : 0;
      lh = node->Left()->Left() ? node->Left()->Left()->Height() : 0;
      if (rh > lh) {
        leftRotate(node->Left());
      }
      node = rightRotate(node);
    } else if (rh - lh > 1) {
      rh = node->Right()->Right() ? node->Right()->Right()->Height() : 0;
      lh = node->Right()->Left() ? node->Right()->Left()->Height() : 0;
      if (lh > rh) {
        rightRotate(node->Right());
      }
      node = leftRotate(node);
    }
    return node;
  }

  void insert(const KEY &key) { insert(new AVLNode(key)); }
  void insert(AVLNode *node) {
    BSTree<KEY>::insert(node);
    while (node->Parent()) {
      node = node->Parent();
      node->adjustHeight();
      node = balance(node);
    }
  }
  bool erase(const KEY &key) {
    auto node = (AVLNode *)this->find(key);
    if (node != nullptr) erase(node);
    return node != nullptr;
  }

  void erase(AVLNode *node) {
    auto parent = node->Parent();
    node = (AVLNode *)BSTree<KEY>::erase(node);
    if (!node) node = parent;
    while (node) {
      node->adjustHeight();
      node = balance(node);
      node = node->Parent();
    }
  }

  // 检测是否合法
  bool verify(bool result = false) {
    if (!Root()) return true;

    AVLNode *cur = Root();
    std::vector<AVLNode *> arr = {cur};
    int maxDepth = 0;
    int minDepth = std::numeric_limits<int>::max();
    while (!arr.empty()) {
      auto node = arr.back();
      arr.pop_back();
      if (node->right) {
        arr.push_back(node->Right());
        if (std::abs(node->Height() - node->Right()->Height()) > 2) {
          std::cout << "上右高度差距太大" << std::endl;
          return false;
        }
      }
      if (node->left) {
        arr.push_back(node->Left());
        if (std::abs(node->Height() - node->Left()->Height()) > 2) {
          std::cout << "上左高度差距太大: " << node->Height() << " , "
                    << node->Left()->Height() << std::endl;
          return false;
        }
      }
      if (node->left && node->right) {
        if (std::abs(node->Right()->Height() - node->Left()->Height()) > 1) {
          std::cout << "左右高度差距太大" << std::endl;
          return false;
        }
      }
    }
    if (result) {
      std::cout << "深度: " << cur->height << std::endl;
    }
    return true;
  }
};

int main(int argc, char const *argv[]) {
  AVLTree<int> tree;

  std::vector<int> values;
  for (size_t i = 0; i < 1000; i++) {
    values.push_back(rand());
    tree.insert(values.back());
    tree.verify();
  }
  tree.verify(true);
  for (size_t i = 0; i < 1000; i++) {
    assert(tree.find(values[i]) != nullptr);
  }
  int last = tree.minimum(tree.root.get())->key;
  for (auto i = tree.minimum(tree.root.get()); i != nullptr; i = tree.next(i)) {
    assert(last <= i->key);
    last = i->key;
  }
  last = tree.maximum(tree.root.get())->key;
  for (auto i = tree.maximum(tree.root.get()); i != nullptr; i = tree.prev(i)) {
    assert(last >= i->key);
    last = i->key;
  }
  for (size_t i = 0; i < 1000; i++) {
    if (!tree.verify()) {
      std::cout << i << std::endl;
      break;
    }
    assert(tree.erase(values[i]));
  }
  for (size_t i = 0; i < 1000; i++) {
    assert(!tree.erase(values[i]));
    assert(tree.find(values[i]) == nullptr);
  }

  values.clear();
  for (size_t i = 0; i < 1000; i++) {
    values.push_back(rand());
    tree.insert(values.back());
    if ((rand() % 2) == 0) {
      tree.erase(values[rand() % values.size()]);
    }
    tree.verify();
  }

  tree.verify(true);
  return 0;
}
