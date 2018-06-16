// Copyright (c) 2018 ace (https://github.com/ru-ace)
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "cavsValue.h"

//#define AVS_DEBUG
#include "avsDebug.h"

cavsValue::cavsValue(cavsStorage *avsStorage)
{
    this->_avs = avsStorage;
}

cavsValue::cavsValue(cavsStorage *avsStorage, valueid_t value_id, uint8_t open_mode)
{
    this->_avs = avsStorage;
    this->open(value_id, open_mode);
}

bool cavsValue::create(int length)
{
    valueid_t value_id = _avs->allocate(_avs->get_emptiest_vat_id(), length);
    if (value_id == AVS_NOTSET_VALUE_ID)
    {
        return false;
    }
    else
    {
        this->open(value_id);
        return true;
    }
}

void cavsValue::open(valueid_t value_id, uint8_t open_mode)
{
    _avs->parse_value_id(value_id, &_vat_id, &_vat_head_address, &_tail_length);
    AVS_DEBUG_LOG("_vat_id = ", _vat_id);
    AVS_DEBUG_LOG("_vat_head_address = ", _vat_head_address);
    AVS_DEBUG_LOG("_tail_length = ", _tail_length);

    _offset = 0;
    _offset_address = _vat_head_address;
    _offset_insector_offset = 0;

    _allocated_sectors = 0;
    _length = 0;

    if (_vat_id == AVS_ROOT_VAT_ID && _vat_head_address == AVS_ROOT_VAT_ADDRESS)
    {
        AVS_DEBUG_LOG("Root Value");
        _is_root_value = true;
        _tail_length = _avs->vat_read(_vat_id, AVS_VAT_END_ID);
    }
    else
    {
        _is_root_value = false;
    }

    if (_length == 0 && _tail_length == 0)
    {
        _vat_tail_address = _vat_head_address;
        _allocated_sectors = 1;
    }
    else
    {
        uint8_t vat_address = _vat_head_address;
        uint8_t vat_elms = 1;

        while (true)
        {
            uint8_t next_vat_address = _avs->vat_read(_vat_id, vat_address);
            if (next_vat_address == AVS_VAT_END_ID || next_vat_address == AVS_VAT_FREE_ID)
            {
                _vat_tail_address = vat_address;
                break;
            }
            else
            {
                vat_elms++;
                vat_address = next_vat_address;
            }
        }

        _length = (vat_elms - 1) * AVS_DS_SECTOR_SIZE + _tail_length;
        _allocated_sectors = vat_elms;
    }

    switch (open_mode)
    {
    case AVS_VALUE_OPEN_APPEND:
        seek(_length);
        break;
    case AVS_VALUE_OPEN_DEFAULT:
    default:
        break;
    }
    AVS_DEBUG_LOG("_vat_tail_address = ", _vat_tail_address);
    AVS_DEBUG_LOG("_allocated_sectors = ", _allocated_sectors);
    AVS_DEBUG_LOG("_length = ", _length);
    AVS_DEBUG_LOG("_tail_length = ", _tail_length);
}
valueid_t cavsValue::close()
{
    _tail_length = _length % AVS_DS_SECTOR_SIZE;
    _tail_length = (_tail_length == 0 && _length >= AVS_DS_SECTOR_SIZE) ? AVS_DS_SECTOR_SIZE : _tail_length;

    AVS_DEBUG_LOG("_vat_id = ", _vat_id);
    AVS_DEBUG_LOG("_vat_head_address = ", _vat_head_address);
    AVS_DEBUG_LOG("_tail_length = ", _tail_length);

    if (_is_root_value)
    {
        _avs->vat_write(_vat_id, AVS_VAT_END_ID, (uint8_t)_tail_length);
        return AVS_ROOT_VALUE_ID;
    }
    else
    {
        return _avs->create_value_id(_vat_id, _vat_head_address, _tail_length);
    }
}

bool cavsValue::eof()
{
    return _length == _offset;
}
int cavsValue::length()
{
    return _length;
}
int cavsValue::offset()
{
    return _offset;
}

void cavsValue::seek(int offset)
{

    AVS_DEBUG_LOG("offset = ", offset);
    int delta = offset - _offset;
    //AVS_DEBUG_LOG("delta = ", delta);
    if (delta == 0)
        return;

    if (offset >= _length)
    {
        // do nothing - read & write method handle this
        //AVS_DEBUG_LOG("Do nothing");
    }
    else if (delta > 0)
    {
        // move forward optimization
        if (_offset_insector_offset + delta < AVS_DS_SECTOR_SIZE)
        {
            // offset in current sector
            //AVS_DEBUG_LOG("in sector");
            _offset_insector_offset += delta;
        }
        else
        {
            // offset in next sectors
            //AVS_DEBUG_LOG("next sectors");
            int offset_delta = delta - (AVS_DS_SECTOR_SIZE - _offset_insector_offset);
            int elms_count = offset_delta / AVS_DS_SECTOR_SIZE + 1;

            //AVS_DEBUG_LOG("offset_delta = ", offset_delta);
            //AVS_DEBUG_LOG("elms_count = ", elms_count);

            for (int i = 0; i < elms_count; i++)
            {
                _offset_address = _avs->vat_read(_vat_id, _offset_address);
            }
        }
    }
    else
    {
        // move backward (start from begining)
        _offset_address = _vat_head_address;
        for (int i = 0; i < offset / AVS_DS_SECTOR_SIZE; i++)
        {
            _offset_address = _avs->vat_read(_vat_id, _offset_address);
        }
    }

    _offset = offset;
    _offset_insector_offset = offset % AVS_DS_SECTOR_SIZE;

    // AVS_DEBUG_LOG("_offset_address = ", _offset_address);
    // AVS_DEBUG_LOG("_offset_insector_offset = ", _offset_insector_offset);
}

void cavsValue::trim(int length)
{
    AVS_DEBUG_LOG("================ TRIM ============");
    AVS_DEBUG_LOG("length = ", length);
    AVS_DEBUG_LOG("_length = ", _length);
    int new_length = length == -1 ? _offset + 1 : length;
    AVS_DEBUG_LOG("new_length = ", new_length);
    if (new_length >= _length)
        return;

    this->seek(new_length - 1);
    uint8_t _next_vat_addtess = _avs->vat_read(_vat_id, _offset_address);
    AVS_DEBUG_LOG("_next_vat_addtess = ", _next_vat_addtess);
    if (_next_vat_addtess != AVS_VAT_END_ID)
    {
        _avs->vat_write(_vat_id, _offset_address, AVS_VAT_END_ID);
        _vat_tail_address = _offset_address;
        _avs->free(_avs->create_value_id(_vat_id, _next_vat_addtess, 0));
    }
    _length = new_length;
    _tail_length = _length % AVS_DS_SECTOR_SIZE;
    _tail_length = (_tail_length == 0 && _length >= AVS_DS_SECTOR_SIZE) ? AVS_DS_SECTOR_SIZE : _tail_length;
    AVS_DEBUG_LOG("_vat_head_address = ", _vat_head_address);
    AVS_DEBUG_LOG("_vat_tail_address = ", _vat_tail_address);
    AVS_DEBUG_LOG("_tail_length = ", _tail_length);
    AVS_DEBUG_LOG("_length = ", _length);
    AVS_DEBUG_LOG("================ TRIM ============");
}

void cavsValue::read(uint8_t *data, int data_length)
{
    uint8_t val = 0;

    for (int i = 0; i < data_length; i++)
    {
        if (_offset >= _length)
        {
            val = 0;
        }
        else
        {
            val = _avs->ds_read(_vat_id, _offset_address, _offset_insector_offset);
            AVS_DEBUG_LOG(val);
        }
        *(data + i) = val;
        this->seek(_offset + 1);
    }
}
uint8_t cavsValue::read8()
{
    uint8_t data = 0;
    this->read(&data, sizeof(uint8_t));
    return data;
}

#ifdef AVS_USE_ARDUINO_STRING_METHODS
String *cavsValue::read(int data_length)
{

    String *data_str = new String("");
    if (data_length == -1)
        data_length = _length - _offset;

    for (int i = 0; i < data_length; i++)
        data_str->concat((char)this->read8());

    return data_str;
}
#endif //AVS_USE_ARDUINO_STRING_METHODS

bool cavsValue::write(uint8_t *data, int data_length)
{
    // space section (allocate new space if needed)
    if (_offset + data_length > _length)
    {
        // current length small - need more
        int needed_space = (_offset + data_length) - _length;
        AVS_DEBUG_LOG("needed_space = ", needed_space);
        if (needed_space <= AVS_DS_SECTOR_SIZE - _tail_length)
        {
            // we already have freespace in current sector
            AVS_DEBUG_LOG("Use free space from tail. _tail_length = ", _tail_length);
            _tail_length += needed_space;
            _length += needed_space;
        }
        else
        {
            // need allocate more space
            int tail_free_space = AVS_DS_SECTOR_SIZE - _tail_length;
            AVS_DEBUG_LOG("Allocate additonal space");
            valueid_t new_space_value_id = _avs->allocate(_vat_id, needed_space - tail_free_space);
            AVS_DEBUG_LOG("new_space_value_id = ", new_space_value_id);
            if (new_space_value_id == AVS_NOTSET_VALUE_ID)
            {
                // TODO: try to reallocate value to emptiest vat
                return false; //! we have no free space to write this data
            }
            uint8_t new_space_vat_head_address = 0;
            _avs->parse_value_id(new_space_value_id, &_vat_id, &new_space_vat_head_address, &_tail_length);
            _avs->vat_write(_vat_id, _vat_tail_address, new_space_vat_head_address);
            _length += needed_space;

            bool need_find_offset_address = false;
            int offset_sector_index = _offset / AVS_DS_SECTOR_SIZE; //index number of sector pointed by _offset

            if (_offset >= _allocated_sectors * AVS_DS_SECTOR_SIZE)
            {
                // means that current _offset is outbound of allocated space
                // we need find _offset_address for current _offset while finding tail_address
                // _offset_insector_offset was set in seek();
                need_find_offset_address = true;
                offset_sector_index -= _allocated_sectors;
            }

            //find new tail address
            uint8_t vat_address = new_space_vat_head_address;
            while (true)
            {
                if (need_find_offset_address && offset_sector_index == 0)
                    _offset_address = vat_address;
                offset_sector_index--;
                _allocated_sectors++;

                uint8_t next_vat_address = _avs->vat_read(_vat_id, vat_address);

                if (next_vat_address == AVS_VAT_END_ID)
                {
                    _vat_tail_address = vat_address;
                    break;
                }
                else if (next_vat_address == AVS_VAT_FREE_ID)
                {
                    // impossible scenario - allocation was corrupted
                    AVS_DEBUG_LOG("!!!!!!!!!!! next_vat_address = AVS_VAT_FREE_ID");
                    return false;
                }
                vat_address = next_vat_address;
            }
        }
    }

    // write section
    for (int i = 0; i < data_length; i++)
    {
        uint8_t val = *(data + i);
        AVS_DEBUG_LOG("_offset_address = ", _offset_address);
        AVS_DEBUG_LOG("_offset_insector_offset = ", _offset_insector_offset);
        AVS_DEBUG_LOG("ds_write byte = ", val);
        _avs->ds_write(_vat_id, _offset_address, _offset_insector_offset, val);
        this->seek(_offset + 1);
    }
    return true;
}

bool cavsValue::write8(uint8_t data)
{
    return this->write(&data, sizeof(uint8_t));
}

#ifdef AVS_USE_ARDUINO_STRING_METHODS
bool cavsValue::write(String *data_str)
{
    int data_length = data_str->length();
    for (int i = 0; i < data_length; i++)
    {
        if (!this->write8(data_str->charAt(i)))
        {
            return false;
        }
    }
    return true;
}
#endif //AVS_USE_ARDUINO_STRING_METHODS

bool cavsValue::append(uint8_t *data, int data_length)
{
    seek(_length);
    return this->write(data, data_length);
}
bool cavsValue::replace(int pos, uint8_t *data, int data_length)
{
    seek(pos);
    return this->write(data, data_length);
}
uint8_t *cavsValue::substr(int pos, int length)
{
    uint8_t *data = new uint8_t[length];
    seek(pos);
    read(data, length);
    return data;
}
