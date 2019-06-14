#include <array>
#include <iostream>
#include <limits>
#include <memory>

template <size_t BITS, typename ValueType, ValueType InvalidValue>
class RadixTree {
  static_assert(BITS >= 1 && BITS <= 8, "只支持1-8位");
  class Node {
    std::array<std::unique_ptr<Node>, 1 << BITS> _nodes;
    ValueType _value = InvalidValue;
    Node *_parent;

   public:
    Node(Node *parent = nullptr) : _parent(parent) {}
    const Node *get(size_t idx) const { return _nodes[idx].get(); }
    Node *get(size_t idx) { return _nodes[idx].get(); }
    Node *getOrCreate(size_t idx, Node *parent) {
      if (_nodes[idx]) return _nodes[idx].get();
      _nodes[idx] = std::unique_ptr<Node>(new Node(parent));
      return _nodes[idx].get();
    }
    void setValue(const ValueType &val) { _value = val; }
    const ValueType &getValue() const { return _value; }
    Node *getParent() { return _parent; }
    void clear(const Node *n) {
      for (auto &node : _nodes) {
        if (node.get() == n) {
          node.reset();
        }
      }
    }
    bool isEmpty() const {
      if (_value != InvalidValue) return false;
      for (auto &node : _nodes) {
        if (node) return false;
      }
      return true;
    }
  };

  static uint8_t _getBits(uint8_t *key, size_t off) {
    size_t idx = off / 8;
    off %= 8;
    size_t end = off + BITS;
    uint8_t r = (key[idx] >> off) & ((1 << BITS) - 1);
    if (end > 8) {  // cross byte
      end -= 8;
      r = (r << end) | (key[idx + 1] & ((1 << end) - 1));
    }
    return r;
  }

  Node _root;

 public:
  void insert(uint8_t *key, size_t keyBits, const ValueType &value) {
    Node *node = &_root;
    for (size_t i = 0; i < keyBits; i += BITS) {
      uint8_t idx = _getBits(key, i);
      node = node->getOrCreate(idx, node);
    }
    node->setValue(value);
  }
  const ValueType &find(uint8_t *key, size_t keyBits) const {
    static ValueType invalid = InvalidValue;
    const Node *node = &_root;
    for (size_t i = 0; i < keyBits; i += BITS) {
      uint8_t idx = _getBits(key, i);
      node = node->get(idx);
      if (!node) return invalid;
    }
    return node->getValue();
  }
  bool remove(uint8_t *key, size_t keyBits) {
    Node *node = &_root, *parent;
    for (size_t i = 0; i < keyBits; i += BITS) {
      uint8_t idx = _getBits(key, i);
      node = node->get(idx);
      if (!node) return false;
    }
    node->setValue(InvalidValue);
    while ((parent = node->getParent()) && node->isEmpty()) {
      parent->clear(node);
      node = parent;
    }
    return true;
  }

  template <typename T>
  const ValueType &find(T key) const {
    return find((uint8_t *)&key, sizeof(T) * 8);
  }
  template <typename T>
  bool remove(T key) {
    return remove((uint8_t *)&key, sizeof(T) * 8);
  }
  template <typename T>
  void insert(T key, const ValueType &value) {
    insert((uint8_t *)&key, sizeof(T) * 8, value);
  }
};

// g++ radix_tree.cpp && ./a.out

template <typename T, size_t BITS = 1>
void test() {
  RadixTree<BITS, int, std::numeric_limits<int>::min()> tree;
  for (T i = 0; i < 16; i++) {
    tree.insert(i, i);
    if (i > 0) {
      tree.insert(i << 4, i << 4);
    }
  }
  for (T i = 0; i < 16; i++) {
    if (tree.find(i) != i) {
      std::cerr << "Bad radix tree: " << i << std::endl;
    }
    if (!tree.remove(i)) {
      std::cerr << "Bad radix tree remove: " << i << std::endl;
    }
    if (tree.find(i) != std::numeric_limits<int>::min()) {
      std::cerr << "Bad radix tree invalid: " << i << std::endl;
    }
    if (i > 0) {
      if (tree.find(i << 4) != i << 4) {
        std::cerr << "Bad radix tree: " << (i << 4) << std::endl;
      }
      if (!tree.remove(i << 4)) {
        std::cerr << "Bad radix tree remove: " << (i << 4) << std::endl;
      }
      if (tree.find(i << 4) != std::numeric_limits<int>::min()) {
        std::cerr << "Bad radix tree invalid: " << (i << 4) << std::endl;
      }
    }
  }
}

int main(int argc, char const *argv[]) {
  test<uint8_t>();
  test<uint16_t>();
  test<uint32_t>();
  test<uint64_t>();
  test<uint8_t, 2>();
  test<uint16_t, 2>();
  test<uint32_t, 2>();
  test<uint64_t, 2>();
  test<uint8_t, 4>();
  test<uint16_t, 4>();
  test<uint32_t, 4>();
  test<uint64_t, 4>();
  test<uint8_t, 8>();
  test<uint16_t, 8>();
  test<uint32_t, 8>();
  test<uint64_t, 8>();

  uint8_t key[16] = {0};
  RadixTree<3, int, std::numeric_limits<int>::min()> tree6;
  for (size_t i = 3; i < sizeof(key) * 8; i += 3) {
    tree6.insert(key, i, i);
  }
  for (size_t i = 3; i < sizeof(key) * 8; i += 3) {
    if (tree6.find(key, i) != i) {
      std::cerr << "Bad radix tree3: " << i << std::endl;
    }
    if (!tree6.remove(key, i)) {
      std::cerr << "Bad radix tree3 remove: " << i << std::endl;
    }
    if (tree6.find(key, i) != std::numeric_limits<int>::min()) {
      std::cerr << "Bad radix tree3 invalid: " << i << std::endl;
    }
  }

  return 0;
}
