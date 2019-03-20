#include <cstdio>
#include "coroutine/context.h"

// g++ main.cpp coroutine/context.S && ./a.out

using coroutine::Context;

Context *myRoutine(Context *ctx) {
  printf("协程 第一次: %p\n", ctx->getArgs());
  ctx = ctx->Call((void *)0x2222);
  printf("协程 第二次: %p\n", ctx->getArgs());
  ctx = ctx->Call((void *)0x4444);
  printf("协程 第三次: %p\n", ctx->getArgs());
  return ctx->setArgs((void *)0x6666);
}

int main(int argc, char const *argv[]) {
  char stack[1024 * 100];
  printf("协程堆栈: %p => %p\n", stack, stack + sizeof(stack));
  Context routine(myRoutine, stack + sizeof(stack));
  auto r = routine.Call((void *)0x1111);
  printf("主函数 第一次返回: %p %d\n", r->getArgs(), r == &routine);
  r = r->Call((void *)0x3333);
  printf("主函数 第二次返回: %p %d\n", r->getArgs(), r == &routine);
  r = r->Call((void *)0x5555);
  printf("主函数 第三次返回: %p %d\n", r->getArgs(), r == &routine);
  return 0;
}
