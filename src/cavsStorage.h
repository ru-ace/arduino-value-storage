// Copyright (c) 2018 ace (https://github.com/ru-ace)
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#ifndef CAVSSTORAGE_H
#define CAVSSTORAGE_H

#include "avsConfig.h"
#include "cavsProvider.h"
#include "cavsValue.h"

class cavsValue;

class cavsStorage
{
private:
  cavsProvider *_pvat; // VAT Provider
  cavsProvider *_pds;  //  DS Provider

  uint8_t vat_freespace[AVS_VAT_COUNT];

  bool start();
  void update_vat_freespace(vatid_t vat_id);
  uint8_t get_free_vat_address(vatid_t vat_id);

public:
  cavsStorage(cavsProvider *vatProvider, cavsProvider *dsProvider);

  void format();

  vatid_t get_emptiest_vat_id();

  valueid_t allocate(vatid_t vat_id, int length);
  valueid_t move(valueid_t src_value_id, vatid_t dst_vat_id);
  void free(valueid_t value_id);

  valueid_t insert(uint8_t *data, int data_length = -1);
  valueid_t update(valueid_t value_id, uint8_t *data, int data_length = -1);
  valueid_t replace(valueid_t value_id, int pos, uint8_t *data, int data_length);

  uint8_t *select(valueid_t value_id, int *length);
  uint8_t *substr(valueid_t value_id, int pos, int length);

#ifdef AVS_USE_ARDUINO_STRING_METHODS
  valueid_t insert(String *data_str);
  valueid_t update(valueid_t value_id, String *data_str);
  String *select(valueid_t value_id);
#endif //AVS_USE_ARDUINO_STRING_METHODS

  cavsValue *value_get(valueid_t value_id);
  cavsValue *value_add();
  void value_del(valueid_t value_id);

  valueid_t create_value_id(vatid_t vat_id, uint8_t vat_address, vtail_t tail_length);
  void parse_value_id(valueid_t value_id, vatid_t *vat_id, uint8_t *vat_address, vtail_t *tail_length);

  uint8_t vat_read(vatid_t vat_id, uint8_t vat_address);
  void vat_write(vatid_t vat_id, uint8_t vat_address, uint8_t data);

  uint8_t ds_read(vatid_t vat_id, uint8_t vat_address, vtail_t offset);
  void ds_write(vatid_t vat_id, uint8_t vat_address, vtail_t offset, uint8_t data);
};

#endif //CAVSSTORAGE_H
