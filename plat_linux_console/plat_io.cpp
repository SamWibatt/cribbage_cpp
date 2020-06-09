#include "plat_io.h"

#include <stdio.h>

void plprintf(const char *fmt, ...) {
  va_list args1;
  va_start(args1, fmt);
  vprintf(fmt, args1);
}