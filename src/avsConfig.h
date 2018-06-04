// Copyright (c) 2018 ace (https://github.com/ru-ace)
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#ifndef AVSCONFIG_H
#define AVSCONFIG_H

#include <Arduino.h>

typedef uint16_t valueid_t; //type for Value ID
typedef uint8_t vatid_t;    //type for Value Allocation Table ID
typedef uint8_t vtail_t;    //type for tail length of Value

//VAT = Value Allocation Table, contains singly linked lists of sector addresses(byte)
//DS  = Data Storage, divided by sectors with AVS_DS_SECTOR_SIZE bytes, contains value data

//! ========== STORAGES CONFIG SELECTOR: DEFINE ONLY ONE =================================
// Config for current project
#define AVS_CONFIG_CURRENT_PROJECT

// testing config for single eeprom with 1024 bytes.
//#define AVS_CONFIG_EEPROM_1K_TEST_SINGLE_VAT

// testing config with 2 vats on single eeprom with 1024 bytes
//#define AVS_CONFIG_EEPROM_1K_TEST_DOUBLE_VAT

//! ========== END OF DEFINE ONLY ONE ====================================================

#ifdef AVS_CONFIG_CURRENT_PROJECT
//Config for current project
#define AVS_VAT_OFFSET 0     // offset address for vat in provider
#define AVS_VAT_COUNT 4      // amout of VA Tables
#define AVS_VAT_FREE_ID 0    // id in VAT = free sector
#define AVS_VAT_END_ID 0xFF  // id in VAT = end of var
#define AVS_DS_OFFSET 0      // offset address for ds in provider
#define AVS_DS_SECTOR_SIZE 4 // bytes in sector
#endif                       //AVS_CONFIG_EEPROM_1K_TEST_SINGLE_VAT

#ifdef AVS_CONFIG_EEPROM_1K_TEST_SINGLE_VAT
//testing config for single eeprom with 1024 bytes.
#define AVS_VAT_OFFSET 0     // offset address for vat in provider
#define AVS_VAT_COUNT 1      // amout of VA Tables
#define AVS_VAT_FREE_ID 0    // id in VAT = free sector
#define AVS_VAT_END_ID 0xFF  // id in VAT = end of var
#define AVS_DS_OFFSET 256    // offset address for ds in provider
#define AVS_DS_SECTOR_SIZE 3 // bytes in sector
#endif                       //AVS_CONFIG_EEPROM_1K_TEST_SINGLE_VAT

#ifdef AVS_CONFIG_EEPROM_1K_TEST_DOUBLE_VAT
//testing config with 2 vats on single eeprom with 1024 bytes
#define AVS_VAT_OFFSET 0     // offset address for vat in provider
#define AVS_VAT_COUNT 2      // amout of VA Tables
#define AVS_VAT_FREE_ID 0    // id in VAT = free sector
#define AVS_VAT_END_ID 0xFF  // id in VAT = end of var
#define AVS_DS_OFFSET 512    // offset address for ds in provider
#define AVS_DS_SECTOR_SIZE 1 // bytes in sector
#endif                       //AVS_CONFIG_EEPROM_1K_TEST_DOUBLE_VAT

//value_id structure(2 bytes, letter=bit) : TTTTVVVV AAAAAAAA
//T = length of tail in last sector in bytes (for root Value store in vat[AVS_VAT_END_ID])
//V = id of VAT
//A = head address in VAT
//Size of T & I can be changed by:
#define AVS_VALUE_ID_TAIL_LENGTH_MASK (valueid_t)0xf000 //(T)mask for obtaining tail length
#define AVS_VALUE_ID_TAIL_LENGTH_SHIFT 12               //(T)binary shift for tail length
#define AVS_VALUE_ID_VAT_ID_MASK (valueid_t)0x0f00      //(V)mask for obtaining vat id
#define AVS_VALUE_ID_VAT_ID_SHIFT 8                     //(V)binary shift for vat id
#define AVS_VALUE_ID_VAT_ADDRESS_MASK (valueid_t)0xff   //(A)mask for obtaining address in vat

// Value ID for initialization of valueid_t vars and "not set" checking
#define AVS_NOTSET_VALUE_ID (valueid_t)0

//root value
#define AVS_ROOT_VALUE_ID (valueid_t)1                                                                         // root value_id
#define AVS_ROOT_VAT_ID (vatid_t)((AVS_ROOT_VALUE_ID & AVS_VALUE_ID_VAT_ID_MASK) >> AVS_VALUE_ID_VAT_ID_SHIFT) // root vat_id
#define AVS_ROOT_VAT_ADDRESS (uint8_t)(AVS_ROOT_VALUE_ID & AVS_VALUE_ID_VAT_ADDRESS_MASK)                      // root vat_address

//address_id in AVS_VAT_FREE_ID = free elements in current VAT (254 initialy)
//address_id in AVS_VAT_END_ID = tail length of ROOT_ID record

#endif //AVSCONFIG_H
