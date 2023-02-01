#pragma once

#include <wayland-client.h>

#include <sstream>
#include <string>

#include "xdg-shell.h"

void die(const char *file, long line, const char *fmt, ...);

template <typename T>
std::string dieStr(T value) {
  std::ostringstream oss;
  oss << value;
  return oss.str();
}

#define DIE(exp, cmp, ret)                                                \
  ({                                                                      \
    auto r = (exp);                                                       \
    if (r cmp ret)                                                        \
      die(__FILE__, __LINE__, #exp "(%s) " #cmp " %s", dieStr(r).c_str(), \
          dieStr(ret).c_str());                                           \
    r;                                                                    \
  })

#define DIE_NULL(exp) DIE(exp, ==, NULL)
#define DIE_EQ_0(exp) DIE(exp, ==, 0)
#define DIE_NE_0(exp) DIE(exp, !=, 0)
#define DIE_LT_0(exp) DIE(exp, <, 0)
#define DIE_LE_0(exp) DIE(exp, <=, 0)

// window
static const int buffer_width = 800;
static const int buffer_height = 600;
