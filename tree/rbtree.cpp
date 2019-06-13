
#include <iostream>
#include <vector>

#include "bstree.h"

// g++ rbtree.cpp && ./a.out

template <typename KEY>
class RBTree : public BSTree<KEY> {
 public:
  const static int RED = 0;
  const static int BLACK = 1;
  class RBNode : public BSTree<KEY>::BSNode {
   public:
    int color = RED;
    RBNode(const KEY &k) : BSTree<KEY>::BSNode(k) {}
    int parentColor() { return ((RBNode *)this->parent)->color; }
    void setParentColor(int cr) { ((RBNode *)this->parent)->color = cr; }
  };
  RBNode *Root() { return (RBNode *)this->root.get(); }

  void insert(RBNode *node) {
    BSTree<KEY>::insert(node);
    // 平衡
    while (true) {
      if (node->isRoot()) break;
      if (node->parentColor() == BLACK) break;

      RBNode *parent = (RBNode *)node->parent;
      RBNode *grand = (RBNode *)parent->parent;
      RBNode *uncle =
          (RBNode *)(parent->isLeft() ? grand->right.get() : grand->left.get());

      grand->color = RED;
      if (uncle && uncle->color == RED) {
        // 1. 叔叔节点为红色，红红的风险传递到祖父节点
        parent->color = BLACK;
        uncle->color = BLACK;
        node = grand;
      } else {
        // 2. 叔叔节点为黑色
        //    1) 如不在同侧先左旋转将父节点和本节点旋转到同侧
        //    2) 将祖父节点设为红色 父节点设为黑色 反侧旋转
        //    此时红色传递至另外一个子树且符合规则
        if (parent->isLeft()) {
          if (!node->isLeft()) {
            node = parent;
            this->leftRotate(parent);
          }
          this->rightRotate(grand);
        } else {
          if (node->isLeft()) {
            node = parent;
            this->rightRotate(parent);
          }
          this->leftRotate(grand);
        }
        node->setParentColor(BLACK);
        break;
      }
    }
    Root()->color = BLACK;
  }
  void insert(const KEY &key) { insert(new RBNode(key)); }

  bool erase(const KEY &key) {
    auto node = (RBNode *)this->find(key);
    if (node != nullptr) erase(node);
    return node != nullptr;
  }

  void erase(RBNode *node) {
    RBNode *child, *brother;
    if (!node->left) {
      child = (RBNode *)node->right.release();
    } else if (!node->right) {
      child = (RBNode *)node->left.release();
    } else {
      RBNode *next = (RBNode *)this->minimum(node->right.get());
      node->key = next->key;
      child = (RBNode *)next->right.release();
      node = next;
    }
    if (child != nullptr) {
      child->parent = node->parent;
    }
    int realColor = node->color;
    bool isLeft;
    if (node->isRoot()) {
      this->root.reset(child);
      if (child) child->color = BLACK;
      return;
    } else if (node->isLeft()) {
      node = (RBNode *)node->parent;
      brother = (RBNode *)node->right.get();
      isLeft = true;
      node->left.reset(child);
    } else {
      node = (RBNode *)node->parent;
      brother = (RBNode *)node->left.get();
      isLeft = false;
      node->right.reset(child);
    }
    if (realColor == RED) return;
    // 退出路径 缺黑 1. 根节点(整个树缺黑不影响平衡) 2. 红节点(改成黑则平衡) 3. 去兄弟节点借
    while (child != Root() && (!child || child->color == BLACK)) {
      assert(node != nullptr);
      assert(brother != nullptr);
      // 1. 兄弟节点是红色(没得借) 把兄弟节点的内侧子节点转成新的兄弟节点
      if (brother->color == RED) {
        brother->color = BLACK;
        node->color = RED;
        if (isLeft) {
          this->leftRotate(node);
          brother = (RBNode *)node->right.get();
        } else {
          this->rightRotate(node);
          brother = (RBNode *)node->left.get();
        }
      }
      auto bLeft = (RBNode *)brother->left.get();
      auto bRight = (RBNode *)brother->right.get();
      // 2. 兄弟节点的两个子节点是黑色 借完后不好平衡 将缺黑转移到父节点
      if ((!bLeft || bLeft->color == BLACK) &&
          (!bRight || bRight->color == BLACK)) {
        brother->color = RED;
        child = node;
        node = (RBNode *)node->parent;
        if (node) {
          isLeft = child->isLeft();
          brother = (RBNode *)(isLeft ? node->right.get() : node->left.get());
        }
      } else {
        // 3. 兄弟节点的外侧子孙是黑色 先将兄弟节点旋转 外侧子孙变成红色
        if (isLeft) {
          if (!bRight || bRight->color == BLACK) {
            bLeft->color = BLACK;
            brother->color = RED;
            this->rightRotate(brother);
            brother = (RBNode *)node->right.get();
          }
        } else {
          if (!bLeft || bLeft->color == BLACK) {
            bRight->color = BLACK;
            brother->color = RED;
            this->leftRotate(brother);
            brother = (RBNode *)node->left.get();
          }
        }
        // 4. 把兄弟节点的黑色借过来 其外侧子孙替换其位置修改为黑色补偿这边的黑色
        brother->color = node->color;
        node->color = BLACK;
        if (isLeft) {
          ((RBNode *)brother->right.get())->color = BLACK;
          this->leftRotate(node);
        } else {
          ((RBNode *)brother->left.get())->color = BLACK;
          this->rightRotate(node);
        }
        return;
      }
    }
    if (child) child->color = BLACK;
  }

  // 检测红黑树是否合法
  int getBlack(RBNode *node) {
    int r = 0;
    while (node) {
      if (node->color == BLACK) r++;
      node = (RBNode *)node->parent;
    }
    return r;
  }
  bool verify(bool result = false) {
    if (!Root()) return true;
    if (Root()->color != BLACK) {
      std::cout << "根节点不为黑" << std::endl;
      return false;
    }
    RBNode *cur = Root();
    std::vector<std::pair<RBNode *, int>> arr;
    arr.emplace_back(Root(), 1);
    int maxDepth = 0;
    int minDepth = 0;
    int firstBlack = 0;
    while (!arr.empty()) {
      auto node = arr.back().first;
      auto depth = arr.back().second;
      arr.pop_back();

      if (!node->left && !node->right) {
        if (firstBlack == 0) {
          firstBlack = getBlack(node);
        }
        if (firstBlack != getBlack(node)) {
          std::cout << "黑节点数量错误" << std::endl;
          return false;
        }

        if (maxDepth == 0) {
          maxDepth = depth;
          minDepth = depth;
        }
        maxDepth = depth < maxDepth ? maxDepth : depth;
        minDepth = depth < minDepth ? depth : minDepth;
      }

      if (node->color == RED && node->parentColor() == RED) {
        std::cout << "连续红色节点" << std::endl;
        return false;
      }

      if (node->right) {
        arr.emplace_back((RBNode *)node->right.get(), depth + 1);
      }
      if (node->left) {
        arr.emplace_back((RBNode *)node->left.get(), depth + 1);
      }
    }
    if (result) {
      std::cout << "黑节点数量: " << firstBlack << std::endl;
      std::cout << "最大深度: " << maxDepth << std::endl;
      std::cout << "最小深度: " << minDepth << std::endl;
    }
    return true;
  }
};

int main(int argc, char const *argv[]) {
  RBTree<int> tree;

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
    tree.verify();
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
