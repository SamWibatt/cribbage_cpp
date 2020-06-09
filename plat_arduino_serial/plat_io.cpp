#include "plat_io.h"
#include "Arduino.h"

#include <stdio.h>

char pbuf[256];  // FIGURE OUT A BETTER WAY

void plprintf(const char *fmt, ...) {
  va_list args1;
  va_start(args1, fmt);
  vsprintf(pbuf, fmt, args1);
  Serial.print(pbuf);
}