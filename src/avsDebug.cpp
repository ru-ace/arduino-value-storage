// Copyright (c) 2018 ace (https://github.com/ru-ace)
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT
#define AVS_DEBUG
#include "avsDebug.h"
#include <EEPROM.h>

// ==================== eeprom dumper

#define AVS_DEBUG_EEPROM_VALUES_IN_LINE 32
#define AVS_DEBUG_EEPROM_LINES 32
void AVS_DEBUG_freeRam()
{
    extern int __heap_start, *__brkval;
    int v;
    int freeram = (int)&v - (__brkval == 0 ? (int)&__heap_start : (int)__brkval);
    AVS_DEBUG_SERIAL.print(" ");
    AVS_DEBUG_SERIAL.print(freeram);
    AVS_DEBUG_SERIAL.println(" bytes of free memory.");
}

void AVS_DEBUG_printASCII(uint8_t *buffer)
{
    for (int i = 0; i < AVS_DEBUG_EEPROM_VALUES_IN_LINE; i++)
    {
        if (i % 8 == 0)
            AVS_DEBUG_SERIAL.print(" ");

        if (buffer[i] > 31 && buffer[i] < 127)
            AVS_DEBUG_SERIAL.write(buffer[i]);
        else if (buffer[i] == 0 || buffer[i] == 0xee)
            AVS_DEBUG_SERIAL.write('.');
        else
            AVS_DEBUG_SERIAL.write(':');
    }
};
void AVS_DEBUG_dump_eeprom()
{
    uint8_t buffer[AVS_DEBUG_EEPROM_VALUES_IN_LINE];
    char valuePrint[4];
    byte value;
    unsigned int address;
    //uint8_t trailingSpace = 2;

    AVS_DEBUG_SERIAL.print("Dumping ");
    AVS_DEBUG_SERIAL.print(AVS_DEBUG_EEPROM_VALUES_IN_LINE * AVS_DEBUG_EEPROM_LINES);
    AVS_DEBUG_SERIAL.print(" of ");
    AVS_DEBUG_SERIAL.print((E2END + 1));
    AVS_DEBUG_SERIAL.println(" bytes from EEPROM.");
    AVS_DEBUG_SERIAL.print("baseAddr   ");

    for (int x = 0; x < AVS_DEBUG_EEPROM_VALUES_IN_LINE; x++)
    {
        if (x < 16)
            AVS_DEBUG_SERIAL.print("0");
        AVS_DEBUG_SERIAL.print(x, HEX);
        AVS_DEBUG_SERIAL.print(" ");
        if (x % 8 == 7)
            AVS_DEBUG_SERIAL.print("  ");
    }
    AVS_DEBUG_SERIAL.println();
    AVS_DEBUG_SERIAL.print("---------");
    for (int x = 0; x < AVS_DEBUG_EEPROM_VALUES_IN_LINE / 8; x++)
    {
        AVS_DEBUG_SERIAL.print(" ");
        for (int y = 0; y < 25; y++)
            AVS_DEBUG_SERIAL.print("-");
    }

    for (address = 0; address < AVS_DEBUG_EEPROM_VALUES_IN_LINE * AVS_DEBUG_EEPROM_LINES; address++)
    {

        value = EEPROM.read(address);
        if (address % 8 == 0)
            AVS_DEBUG_SERIAL.print("  ");

        if (address % AVS_DEBUG_EEPROM_VALUES_IN_LINE == 0)
        {
            if (address > 0 && address % AVS_DEBUG_EEPROM_VALUES_IN_LINE == 0)
                AVS_DEBUG_printASCII(buffer);

            sprintf((char *)buffer, "\n 0x%05X: ", address);
            AVS_DEBUG_SERIAL.print((char *)buffer);
        }
        buffer[address % AVS_DEBUG_EEPROM_VALUES_IN_LINE] = value;

        sprintf(valuePrint, " %02X", value);
        AVS_DEBUG_SERIAL.print(valuePrint);
    }

    AVS_DEBUG_SERIAL.print("  ");
    AVS_DEBUG_printASCII(buffer);
    AVS_DEBUG_SERIAL.println("\n");
};

// ==================== end of eeprom dumper
