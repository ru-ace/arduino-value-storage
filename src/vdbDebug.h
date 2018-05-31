// Copyright (c) 2018 ace (https://github.com/ru-ace)
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#ifndef VDBDEBUG_H
#define VDBDEBUG_H

#include <Arduino.h>

void VDB_DEBUG_printASCII(uint8_t *buffer);
void VDB_DEBUG_dump_eeprom();
void VDB_DEBUG_freeRam();

#define VDB_DEBUG

#ifdef VDB_DEBUG

#define VDB_DEBUG_SERIAL Serial

#define VDB_DEBUG_DUMP_EEPROM VDB_DEBUG_dump_eeprom()
#define VDB_DEBUG_LOG_FREEMEM VDB_DEBUG_freeRam()

#define VDB_DEBUG_START                                                                   \
    VDB_DEBUG_SERIAL.begin(9600);                                                         \
    pinMode(LED_BUILTIN, OUTPUT);                                                         \
    int vdb_led_val = 0, vdb_led_dir = 1;                                                 \
    while (!VDB_DEBUG_SERIAL)                                                             \
    {                                                                                     \
        analogWrite(LED_BUILTIN, vdb_led_val);                                            \
        vdb_led_val += vdb_led_dir;                                                       \
        vdb_led_dir = vdb_led_val == 10 || vdb_led_val == 0 ? -vdb_led_dir : vdb_led_dir; \
        delay(50);                                                                        \
    };                                                                                    \
    VDB_DEBUG_SERIAL.println("Begin VDB Debug Log");                                      \
    digitalWrite(LED_BUILTIN, LOW)

#define VDB_GET_OVERRIDE(_1, _2, N, ...) N

#define VDB_DEBUG_PRINT_1(x)                     \
    VDB_DEBUG_SERIAL.print(__PRETTY_FUNCTION__); \
    VDB_DEBUG_SERIAL.print(": ");                \
    VDB_DEBUG_SERIAL.println(x)

#define VDB_DEBUG_PRINT_2(x, y)                  \
    VDB_DEBUG_SERIAL.print(__PRETTY_FUNCTION__); \
    VDB_DEBUG_SERIAL.print(": ");                \
    VDB_DEBUG_SERIAL.print(x);                   \
    VDB_DEBUG_SERIAL.println(y)
#define VDB_DEBUG_LOG(...)                                              \
    VDB_GET_OVERRIDE(__VA_ARGS__, VDB_DEBUG_PRINT_2, VDB_DEBUG_PRINT_1) \
    (__VA_ARGS__)

#else
#define VDB_DEBUG_START
#define VDB_DEBUG_DUMP_EEPROM
#define VDB_DEBUG_LOG(...)
#define VDB_DEBUG_LOG_FREEMEM
#endif

#endif //VDBDEBUG_H
