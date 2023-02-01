#include "utils.h"

#include <cerrno>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>

void die(const char *file, long line, const char *fmt, ...) {
  int err = errno;

  va_list ap;
  va_start(ap, fmt);
  vprintf(fmt, ap);
  va_end(ap);

  printf("\nstandard errno(%d) %s\n", err, strerror(err));
  printf("%s:%ld\n", file, line);
  exit(-1);
}
