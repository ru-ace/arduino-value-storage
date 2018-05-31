#ifndef CVDBPROVIDER
#define CVDBPROVIDER

#include <Arduino.h>

class cvdbProvider
{
public:
  virtual uint8_t read(int address) = 0;
  virtual void write(int address, uint8_t value) = 0;
};

#endif //CVDBPROVIDER
