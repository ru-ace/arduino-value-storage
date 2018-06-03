// Copyright (c) 2018 ace (https://github.com/ru-ace)
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#ifndef CAVSPROVIDER_H
#define CAVSPROVIDER_H

#include <Arduino.h>

class cavsProvider
{
public:
  virtual uint8_t read(int address) = 0;
  virtual void write(int address, uint8_t value) = 0;
};

#endif //CAVSPROVIDER_H
