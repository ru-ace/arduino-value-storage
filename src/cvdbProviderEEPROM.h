// Copyright (c) 2018 ace (https://github.com/ru-ace)
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#ifndef CVDBPROVIDEREEPROM
#define CVDBPROVIDEREEPROM

#include "cvdbProvider.h"
#include <EEPROM.h>

class cvdbProviderEEPROM : public cvdbProvider
{
  public:
    uint8_t read(int address)
    {
        return EEPROM.read(address);
    }
    void write(int address, uint8_t value)
    {
        EEPROM.write(address, value);
    }
};

#endif //CVDBPROVIDEREEPROM
