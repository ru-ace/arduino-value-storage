#include "vdbDebug.h"
#include <EEPROM.h>

// ==================== eeprom dumper
#ifdef VDB_DEBUG
#define VDB_DEBUG_EEPROM_VALUES_IN_LINE 32
#define VDB_DEBUG_EEPROM_LINES 32
void VDB_DEBUG_freeRam()
{
    extern int __heap_start, *__brkval;
    int v;
    int freeram = (int)&v - (__brkval == 0 ? (int)&__heap_start : (int)__brkval);
    VDB_DEBUG_SERIAL.print(" ");
    VDB_DEBUG_SERIAL.print(freeram);
    VDB_DEBUG_SERIAL.println(" bytes of free memory.");
}

void VDB_DEBUG_printASCII(uint8_t *buffer)
{
    for (int i = 0; i < VDB_DEBUG_EEPROM_VALUES_IN_LINE; i++)
    {
        if (i % 8 == 0)
            VDB_DEBUG_SERIAL.print(" ");

        if (buffer[i] > 31 && buffer[i] < 127)
            VDB_DEBUG_SERIAL.write(buffer[i]);
        else if (buffer[i] == 0 || buffer[i] == 0xee)
            VDB_DEBUG_SERIAL.write('.');
        else
            VDB_DEBUG_SERIAL.write(':');
    }
};
void VDB_DEBUG_dump_eeprom()
{
    uint8_t buffer[VDB_DEBUG_EEPROM_VALUES_IN_LINE];
    char valuePrint[4];
    byte value;
    unsigned int address;
    //uint8_t trailingSpace = 2;

    VDB_DEBUG_SERIAL.print("Dumping ");
    VDB_DEBUG_SERIAL.print(VDB_DEBUG_EEPROM_VALUES_IN_LINE * VDB_DEBUG_EEPROM_LINES);
    VDB_DEBUG_SERIAL.print(" of ");
    VDB_DEBUG_SERIAL.print((E2END + 1));
    VDB_DEBUG_SERIAL.println(" bytes from EEPROM.");
    VDB_DEBUG_SERIAL.print("baseAddr   ");

    for (int x = 0; x < VDB_DEBUG_EEPROM_VALUES_IN_LINE; x++)
    {
        if (x < 16)
            VDB_DEBUG_SERIAL.print("0");
        VDB_DEBUG_SERIAL.print(x, HEX);
        VDB_DEBUG_SERIAL.print(" ");
        if (x % 8 == 7)
            VDB_DEBUG_SERIAL.print("  ");
    }
    VDB_DEBUG_SERIAL.println();
    VDB_DEBUG_SERIAL.print("---------");
    for (int x = 0; x < VDB_DEBUG_EEPROM_VALUES_IN_LINE / 8; x++)
    {
        VDB_DEBUG_SERIAL.print(" ");
        for (int y = 0; y < 25; y++)
            VDB_DEBUG_SERIAL.print("-");
    }

    for (address = 0; address < VDB_DEBUG_EEPROM_VALUES_IN_LINE * VDB_DEBUG_EEPROM_LINES; address++)
    {

        value = EEPROM.read(address);
        if (address % 8 == 0)
            VDB_DEBUG_SERIAL.print("  ");

        if (address % VDB_DEBUG_EEPROM_VALUES_IN_LINE == 0)
        {
            if (address > 0 && address % VDB_DEBUG_EEPROM_VALUES_IN_LINE == 0)
                VDB_DEBUG_printASCII(buffer);

            sprintf((char *)buffer, "\n 0x%05X: ", address);
            VDB_DEBUG_SERIAL.print((char *)buffer);

            //memset(buffer, 32, VDB_DEBUG_EEPROM_VALUES_IN_LINE);
        }
        buffer[address % VDB_DEBUG_EEPROM_VALUES_IN_LINE] = value;

        sprintf(valuePrint, " %02X", value);
        VDB_DEBUG_SERIAL.print(valuePrint);
    }

    VDB_DEBUG_SERIAL.print("  ");
    VDB_DEBUG_printASCII(buffer);
    VDB_DEBUG_SERIAL.println("\n");
};

// ==================== end of eeprom dumper
#endif
