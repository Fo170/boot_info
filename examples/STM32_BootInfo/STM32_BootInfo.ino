#define _boot_info_
#include "boot_info.h"

void setup()
{
  Serial.begin(115200);
  delay(1000);
  boot_info();
  printVersion();
}

void loop()
{
}