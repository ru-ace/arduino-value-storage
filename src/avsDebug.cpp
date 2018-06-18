// Copyright (c) 2018 ace (https://github.com/ru-ace)
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT
#define AVS_DEBUG
#include "avsDebug.h"
#include "cavsProvider.h"
// ==================== storage dumper

#define AVS_DEBUG_DUMPER_VALUES_IN_LINE 32
void AVS_DEBUG_dump_provider(cavsProvider *storage, unsigned int start_address, unsigned int end_address)
{
    unsigned int start_line = start_address / AVS_DEBUG_DUMPER_VALUES_IN_LINE;
    unsigned int end_line = end_address / AVS_DEBUG_DUMPER_VALUES_IN_LINE;
    int lines = end_line - start_line + 1;
    char buffer[AVS_DEBUG_DUMPER_VALUES_IN_LINE];
    char byte_buffer[4];
    AVS_DEBUG_SERIAL.print(F("Dumping storage "));
    AVS_DEBUG_SERIAL.print(AVS_DEBUG_DUMPER_VALUES_IN_LINE * lines);
    AVS_DEBUG_SERIAL.println(F(" bytes. "));
    AVS_DEBUG_SERIAL.print("baseAddr   ");
    for (int i = 0; i < AVS_DEBUG_DUMPER_VALUES_IN_LINE; i++)
    {
        if (i < 16)
            AVS_DEBUG_SERIAL.print("0");
        AVS_DEBUG_SERIAL.print(i, HEX);
        AVS_DEBUG_SERIAL.print(" ");
        if (i % 8 == 7)
            AVS_DEBUG_SERIAL.print("  ");
    }
    AVS_DEBUG_SERIAL.println();

    AVS_DEBUG_SERIAL.print(F("---------"));
    for (int i = 0; i < AVS_DEBUG_DUMPER_VALUES_IN_LINE / 8; i++)
    {
        AVS_DEBUG_SERIAL.print(" ");
        for (int j = 0; j < 25; j++)
            AVS_DEBUG_SERIAL.print("-");
    }
    AVS_DEBUG_SERIAL.println();
    for (int line_id = 0; line_id < lines; line_id++)
    {
        unsigned int baseAddr = line_id * AVS_DEBUG_DUMPER_VALUES_IN_LINE;
        sprintf((char *)buffer, " 0x%05X: ", baseAddr);
        AVS_DEBUG_SERIAL.print((char *)buffer);
        for (int i = 0; i < AVS_DEBUG_DUMPER_VALUES_IN_LINE; i++)
        {
            uint8_t val = storage->read(baseAddr + i);
            buffer[i] = val;
            if (i != 0 && i % 8 == 0)
                AVS_DEBUG_SERIAL.print("  ");
            sprintf(byte_buffer, " %02X", val);
            AVS_DEBUG_SERIAL.print(byte_buffer);
        }
        AVS_DEBUG_SERIAL.print("  ");
        for (int i = 0; i < AVS_DEBUG_DUMPER_VALUES_IN_LINE; i++)
        {
            if (i != 0 && i % 8 == 0)
                AVS_DEBUG_SERIAL.print(" ");
            uint8_t val = buffer[i];
            val = (val == 0 || val == 0xee) ? '.' : ((val > 31 && val < 127) ? val : ':');
            AVS_DEBUG_SERIAL.write((char)val);
        }
        AVS_DEBUG_SERIAL.println();
    }
    AVS_DEBUG_SERIAL.print(F("---------"));
    for (int i = 0; i < AVS_DEBUG_DUMPER_VALUES_IN_LINE / 8; i++)
    {
        AVS_DEBUG_SERIAL.print(" ");
        for (int j = 0; j < 25; j++)
            AVS_DEBUG_SERIAL.print("-");
    }
    AVS_DEBUG_SERIAL.println();
    AVS_DEBUG_SERIAL.println();
}

// ==================== end of storage dumper

void AVS_DEBUG_freeRam()
{
    extern int __heap_start, *__brkval;
    int v;
    int freeram = (int)&v - (__brkval == 0 ? (int)&__heap_start : (int)__brkval);
    AVS_DEBUG_SERIAL.print(" ");
    AVS_DEBUG_SERIAL.print(freeram);
    AVS_DEBUG_SERIAL.println(" bytes of free memory.");
}
