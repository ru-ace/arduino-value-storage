// Copyright (c) 2018 ace (https://github.com/ru-ace)
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#ifndef CAVSPROVIDEREEPROM_H
#define CAVSPROVIDEREEPROM_H

#include "cavsProvider.h"
#include <EEPROM.h>

class cavsProviderEEPROM : public cavsProvider
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

#endif //CAVSPROVIDEREEPROM_H
