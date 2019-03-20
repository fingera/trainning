#pragma once

#include <cstdint>

extern "C" {
void asmContextEntry(void *to, void *realEntry);
void *asmContextCall(void *to, void *args);
}

namespace coroutine {

struct Context {
  static const int RSI_IDX = 4;
  static const int RDI_IDX = 5;
  static const int RIP_IDX = 8;
  static const int RSP_IDX = 9;
  static const int ARGS_IDX = 10;
  void *_regs[11];  // R12 R13 R14 R15 RSI RDI RBP RBX RIP RSP ARGS

  /**
   * @brief 创建一个协程
   *
   * @param entry 协程入口
   * @param stack 协程的堆栈
   */
  Context(Context *(*entry)(Context *), void *stack) {
    _regs[RIP_IDX] = (void *)asmContextEntry;
    _regs[RSP_IDX] = stack;

    _regs[RDI_IDX] = (void *)this;
    _regs[RSI_IDX] = (void *)entry;
  }
  Context *Call(void *args) {
    // 在当前堆栈中生成一个context(只要被唤醒则context失效) 作为目标的参数
    // 恢复当前的Context
    return (Context *)asmContextCall(_regs, args);
  }
  inline void *getArgs() { return _regs[ARGS_IDX]; }
  inline Context *setArgs(void *args) {
    _regs[ARGS_IDX] = args;
    return this;
  }
};

static_assert(sizeof(Context) == 88, "bad align");

}  // namespace coroutine
