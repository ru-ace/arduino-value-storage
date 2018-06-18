// Copyright (c) 2018 ace (https://github.com/ru-ace)
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#ifndef AVSDEBUG_H
#define AVSDEBUG_H

#include <Arduino.h>
#include "cavsProviderEEPROM.h"
void AVS_DEBUG_dump_provider(cavsProvider *storage, unsigned int start_address, unsigned int end_address);
void AVS_DEBUG_freeRam();

#ifdef AVS_DEBUG

#define AVS_DEBUG_SERIAL Serial

#define AVS_DEBUG_DUMP_EEPROM AVS_DEBUG_dump_provider(&avsProviderEEPROM, 0, 1023)
#define AVS_DEBUG_DUMP_PROVIDER(storage, start_address, end_address) AVS_DEBUG_dump_provider(storage, start_address, end_address)
#define AVS_DEBUG_LOG_FREEMEM AVS_DEBUG_freeRam()

#define AVS_DEBUG_START                                                                   \
    AVS_DEBUG_SERIAL.begin(9600);                                                         \
    pinMode(LED_BUILTIN, OUTPUT);                                                         \
    int avs_led_val = 0, avs_led_dir = 1;                                                 \
    while (!AVS_DEBUG_SERIAL)                                                             \
    {                                                                                     \
        analogWrite(LED_BUILTIN, avs_led_val);                                            \
        avs_led_val += avs_led_dir;                                                       \
        avs_led_dir = avs_led_val == 10 || avs_led_val == 0 ? -avs_led_dir : avs_led_dir; \
        delay(50);                                                                        \
    };                                                                                    \
    AVS_DEBUG_SERIAL.println("Begin AVS Debug Log");                                      \
    digitalWrite(LED_BUILTIN, LOW)

#define AVS_GET_OVERRIDE(_1, _2, N, ...) N

#define AVS_DEBUG_PRINT_1(x)                     \
    AVS_DEBUG_SERIAL.print(__PRETTY_FUNCTION__); \
    AVS_DEBUG_SERIAL.print(": ");                \
    AVS_DEBUG_SERIAL.println(x)

#define AVS_DEBUG_PRINT_2(x, y)                  \
    AVS_DEBUG_SERIAL.print(__PRETTY_FUNCTION__); \
    AVS_DEBUG_SERIAL.print(": ");                \
    AVS_DEBUG_SERIAL.print(x);                   \
    AVS_DEBUG_SERIAL.println(y)
#define AVS_DEBUG_LOG(...)                                              \
    AVS_GET_OVERRIDE(__VA_ARGS__, AVS_DEBUG_PRINT_2, AVS_DEBUG_PRINT_1) \
    (__VA_ARGS__)

#else
#define AVS_DEBUG_START
#define AVS_DEBUG_DUMP_EEPROM
#define AVS_DEBUG_LOG(...)
#define AVS_DEBUG_LOG_FREEMEM
#define AVS_DEBUG_DUMP_PROVIDER(...)
#endif

#endif //AVSDEBUG_H
