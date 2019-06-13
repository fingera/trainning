#include <cassert>
#include <iostream>
#include <set>
#include <vector>

// g++ heap.cpp && ./a.out

size_t P(size_t i) { return (i - 1) / 2; }
size_t C(size_t i) { return 2 * i + 1; }

template <typename SequenceContainer, typename T>
void pushHeap(SequenceContainer& c, const T& val) {
  c.push_back(val);
  if (c.size() > 1) {
    size_t cur = c.size() - 1;
    for (size_t p = P(cur); c[cur] < c[p] && cur > 0; p = P(p)) {
      std::swap(c[cur], c[p]);
      cur = p;
    }
  }
}

template <typename SequenceContainer, typename T>
void popHeap(SequenceContainer& c, T& val) {
  val = c[0];
  T back = c.back();
  c.pop_back();
  if (c.empty()) return;
  c[0] = back;

  size_t cur = 0;
  while (true) {
    size_t child0 = C(cur);
    size_t child1 = child0 + 1;
    size_t child;
    if (c.size() > child1) {
      child = c[child0] < c[child1] ? child0 : child1;
    } else if (c.size() == child1) {
      child = child0;
    } else {
      break; // 到尾部了
    }
    if (c[child] < c[cur]) {
      std::swap(c[child], c[cur]);
      cur = child;
    } else {
      break; // 不能继续下沉了
    }
  }
}

int main(void) {
  std::vector<int> container;
  for (size_t i = 0; i < 1000; i++) {
    pushHeap(container, rand());
  }
  std::set<int> checker(container.begin(), container.end());

  auto setIt = checker.begin();
  while (!container.empty()) {
    int val;
    int checkVal = *setIt++;
    popHeap(container, val);
    assert(val == checkVal);
  }
  return 0;
}