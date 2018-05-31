// Copyright (c) 2018 ace (https://github.com/ru-ace)
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#ifndef CVDBSTORAGE
#define CVDBSTORAGE

#include <Arduino.h>

typedef uint16_t vdbid_t; //type for VDB record ID
typedef uint8_t vatid_t;  //type for VA Table ID
typedef uint8_t vtail_t;  //type for tail length

//VAT = Value Allocation Table, VAT contains (byte)address_id's
//DS  = Data Storage

//#define VDB_DOUBLE_VAT_TEST_CONFIG //define only for testing config with 2 vats on single eeprom with 1024 bytes

#ifdef VDB_DOUBLE_VAT_TEST_CONFIG

#define VDBS_VAT_OFFSET 0    // offset address for vat in provider
#define VDBS_VAT_COUNT 2     // amout of VA Tables
#define VDBS_VAT_FREE_ID 0   // id in VAT = free sector
#define VDBS_VAT_END_ID 0xFF // id in VAT = end of var

#define VDBS_DS_OFFSET 512    // offset address for ds in provider
#define VDBS_DS_SECTOR_SIZE 1 // bytes in sector

#else //testing config for single eeprom with 1024 bytes.

#define VDBS_VAT_OFFSET 0    // offset address for vat in provider
#define VDBS_VAT_COUNT 1     // amout of VA Tables
#define VDBS_VAT_FREE_ID 0   // id in VAT = free sector
#define VDBS_VAT_END_ID 0xFF // id in VAT = end of var

#define VDBS_DS_OFFSET 256    // offset address for ds in provider
#define VDBS_DS_SECTOR_SIZE 3 // bytes in sector

#endif
//VDBS record ID structure(2 bytes, letter=bit) : TTTTVVVV AAAAAAAA
//T = leght of tail in last sector in bytes (for root value store in vat[VDBS_VAT_END_ID])
//V = id of VA Table
//A = start address_id in VA Table
//Size of T & I can be changed by:
#define VDBS_ID_TAIL_LENGTH_MASK (vdbid_t)0xf000 //(T)mask for obtaining tail length
#define VDBS_ID_TAIL_LENGTH_SHIFT 12             //(T)binary shift for tail length
#define VDBS_ID_VAT_ID_MASK (vdbid_t)0x0f00      //(V)mask for obtaining vat id
#define VDBS_ID_VAT_ID_SHIFT 8                   //(V)binary shift for vat id
#define VDBS_ID_VAT_ADDRESS_MASK (vdbid_t)0xff   //(A)mask for obtaining address_id in vat

// VDB_ID for initialization of vdbid_t vars and "not set" checking
#define VDBS_NOTSET_VDBID (vdbid_t)0

//root value
#define VDBS_ROOT_VDBID (vdbid_t)1                                                                  // root vdb_ID
#define VDBS_ROOT_VAT_ID (vatid_t)((VDBS_ROOT_VDBID & VDBS_ID_VAT_ID_MASK) >> VDBS_ID_VAT_ID_SHIFT) // root vat_id
#define VDBS_ROOT_VAT_ADDRESS (uint8_t)(VDBS_ROOT_VDBID & VDBS_ID_VAT_ADDRESS_MASK)                 // root vat_address

//address_id in VDBS_VAT_FREE_ID = free elements in current VAT (254 initialy)
//address_id in VDBS_VAT_END_ID = tail length of ROOT_ID record

#include "cvdbProvider.h"
#include "cvdbValue.h"

class cvdbValue;

class cvdbStorage
{
private:
  cvdbProvider *_pvat; //VAT Provider
  cvdbProvider *_pds;  // DS Provider

  bool start();

public:
  uint8_t vat_freespace[VDBS_VAT_COUNT];

  cvdbStorage(cvdbProvider *vatProvider, cvdbProvider *dsProvider);

  void format();

  vdbid_t allocate(vatid_t vat_id, int length);
  vdbid_t move(vdbid_t src_vdb_id, vatid_t dst_vat_id);
  void free(vdbid_t vdb_id);

  vdbid_t insert(uint8_t *data, int data_length);
  vdbid_t update(vdbid_t vdb_id, uint8_t *data, int data_length);

  uint8_t *select(vdbid_t vdb_id, int *length);
  uint8_t *substr(vdbid_t vdb_id, int pos, int length);
  vdbid_t replace(vdbid_t vdb_id, int pos, uint8_t *data, int data_length);

  cvdbValue *value_get(vdbid_t vdb_id);
  cvdbValue *value_add();
  void value_del(vdbid_t vdb_id);

  vdbid_t create_vdb_id(vatid_t vat_id, uint8_t vat_address, vtail_t tail_length);
  void parse_vdb_id(vdbid_t vdb_id, vatid_t *vat_id, uint8_t *vat_address, vtail_t *tail_length);

  uint8_t vat_read(vatid_t vat_id, uint8_t vat_address);
  void vat_write(vatid_t vat_id, uint8_t vat_address, uint8_t data);

  uint8_t ds_read(vatid_t vat_id, uint8_t vat_address, vtail_t offset);
  void ds_write(vatid_t vat_id, uint8_t vat_address, vtail_t offset, uint8_t data);
  /**
   * @brief Get free sector address in VAT vat_id
   * 
   * @param vat_id - ID of VAT
   * @return uint8_t = VDBS_VAT_FREE_ID if not free sector found
  */
  uint8_t get_free_vat_address(vatid_t vat_id);
  vatid_t get_emptiest_vat_id();

  void update_vat_freespace(vatid_t vat_id);
};

#endif //CVDBSTORAGE
