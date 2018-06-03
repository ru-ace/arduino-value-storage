// Copyright (c) 2018 ace (https://github.com/ru-ace)
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#ifndef CAVSVALUE_H
#define CAVSVALUE_H

#include "cavsStorage.h"
class cavsStorage;

class cavsValue
{
private:
  cavsStorage *_avs;

  bool _is_root_value = false; // root value - has some hacks for store _tail_length

  int _offset = 0;                 // current offset in bytes from beginning of the value
  uint8_t _offset_address;         // address of sector for current _offset
  uint8_t _offset_insector_offset; // inSector offset of _offset

  int _allocated_sectors = 0; // amount of allocated sectors for value.
  int _length = 0;            // value length in bytes
  vtail_t _tail_length;       // amount bytes used in tail sector (_vat_tail_address)

  vatid_t _vat_id;           // id of vat where value is store
  uint8_t _vat_head_address; // address of first sector used by value
  uint8_t _vat_tail_address; // address of last sector used by value

public:
  cavsValue(cavsStorage *avsStorage);
  cavsValue(cavsStorage *avsStorage, valueid_t value_id);

  bool create(int length = 0);
  void open(valueid_t value_id);
  valueid_t close();

  bool eof();
  int length();
  int offset();
  void seek(int offset);
  void trim(int length = -1);

  void read(uint8_t *data, int data_length);
  uint8_t read8();
  uint16_t read16();
  uint32_t read32();

  bool write(uint8_t *data, int data_length);
  bool write8(uint8_t data);
  bool write16(uint16_t data);
  bool write32(uint32_t data);
};

#endif //CAVSVALUE_H
