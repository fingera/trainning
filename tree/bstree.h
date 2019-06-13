#pragma once

#include <memory>
#include <cassert>

template <typename KEY>
class BSTree {
 public:
  class BSNode {
   public:
    std::unique_ptr<BSNode> left;
    std::unique_ptr<BSNode> right;
    BSNode *parent = nullptr;
    KEY key;

    BSNode(const KEY &k) : key(k) {}

    bool isRoot() { return parent == nullptr; }
    bool isLeft() { return parent->left.get() == this; }
    bool isRight() { return parent->right.get() == this; }
  };

  std::unique_ptr<BSNode> root;

  template <typename NODE>
  static NODE *next(NODE *node) {
    if (node->right) {
      return minimum(node->right.get());
    }
    while (!node->isRoot()) {
      if (node->isLeft()) return node->parent;
      node = node->parent;
    }
    return nullptr;
  }
  template <typename NODE>
  static NODE *prev(NODE *node) {
    if (node->left) {
      return maximum(node->left.get());
    }
    while (!node->isRoot()) {
      if (node->isRight()) return node->parent;
      node = node->parent;
    }
    return nullptr;
  }
  template <typename NODE>
  static NODE *minimum(NODE *node) {
    while (node->left) {
      node = node->left.get();
    }
    return node;
  }
  template <typename NODE>
  static NODE *maximum(NODE *node) {
    while (node->right) {
      node = node->right.get();
    }
    return node;
  }

  void insert(BSNode *node) {
    if (!root) {
      root.reset(node);
      return;
    }
    std::unique_ptr<BSNode> *position;
    BSNode *x = root.get();
    while (true) {
      position = node->key < x->key ? &x->left : &x->right;
      if (position->get() == nullptr) {
        break;
      }
      x = position->get();
    }
    position->reset(node);
    node->parent = x;
  }
  void insert(const KEY &key) { insert(new BSNode(key)); }
  BSNode *find(const KEY &key) {
    BSNode *node = root.get();
    while (node) {
      if (node->key < key) {
        node = node->right.get();
      } else if (key < node->key) {
        node = node->left.get();
      } else {  // ==
        break;
      }
    }
    return node;
  }
  bool erase(const KEY &key) {
    auto node = find(key);
    if (node != nullptr) erase(node);
    return node != nullptr;
  }
  void erase(BSNode *node) {
    BSNode *child;
    if (!node->left) {
      child = node->right.release();
    } else if (!node->right) {
      child = node->left.release();
    } else {
      BSNode *next = minimum(node->right.get());
      node->key = next->key;
      child = next->right.release();
      node = next;
    }
    if (child != nullptr) {
      child->parent = node->parent;
    }
    if (node->isRoot()) {
      root.reset(child);
    } else if (node->isLeft()) {
      node->parent->left.reset(child);
    } else {
      node->parent->right.reset(child);
    }
  }
  void leftRotate(BSNode *node) {
    auto right = node->right.release();
    right->parent = node->parent;
    if (!node->isRoot()) {
      if (node->isLeft()) {
        node->parent->left.release();
        node->parent->left.reset(right);
      } else {
        node->parent->right.release();
        node->parent->right.reset(right);
      }
    } else {
      assert(node == root.get());
      root.release();
      root.reset(right);
    }
    node->right.reset(right->left.release());
    if (node->right) node->right->parent = node;
    right->left.reset(node);
    node->parent = right;
  }
  void rightRotate(BSNode *node) {
    auto left = node->left.release();
    left->parent = node->parent;
    if (!node->isRoot()) {
      if (node->isLeft()) {
        node->parent->left.release();
        node->parent->left.reset(left);
      } else {
        node->parent->right.release();
        node->parent->right.reset(left);
      }
    } else {
      assert(node == root.get());
      root.release();
      root.reset(left);
    }
    node->left.reset(left->right.release());
    if (node->left) node->left->parent = node;
    left->right.reset(node);
    node->parent = left;
  }
};