// Copyright (c) 2018 ace (https://github.com/ru-ace)
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#ifndef CAVSPROVIDERAT24CX_H
#define CAVSPROVIDERAT24CX_H

//ref: http://ww1.microchip.com/downloads/en/devicedoc/doc0336.pdf

#include "cavsProvider.h"
#include <Wire.h>

#define AVS_AT24CX_I2C_ADDRESS 0x57

class cavsProviderAT24CX : public cavsProvider
{
  public:
    cavsProviderAT24CX()
    {
        //Wire.begin();
    }
    uint8_t read(int address)
    {
        Wire.beginTransmission(AVS_AT24CX_I2C_ADDRESS);
        Wire.write(address >> 8);
        Wire.write(address & 0xFF);
        Wire.endTransmission();
        Wire.requestFrom(AVS_AT24CX_I2C_ADDRESS, 1);
        return (uint8_t)Wire.read();
    }
    void write(int address, uint8_t value)
    {
        Wire.beginTransmission(AVS_AT24CX_I2C_ADDRESS);
        Wire.write(address >> 8);
        Wire.write(address & 0xFF);
        Wire.write(value);
        Wire.endTransmission();
        delay(20);
    }
};

#endif //CAVSPROVIDERAT24CX_H
