#include "cvdbValue.h"
#include "vdbDebug.h"

cvdbValue::cvdbValue(cvdbStorage *vdbStorage)
{
    this->_vdbs = vdbStorage;
}

cvdbValue::cvdbValue(cvdbStorage *vdbStorage, vdbid_t vdb_id)
{
    this->_vdbs = vdbStorage;
    this->open(vdb_id);
}

bool cvdbValue::create(int length)
{
    vdbid_t vdb_id = _vdbs->allocate(_vdbs->get_emptiest_vat_id(), length);
    if (vdb_id == VDBS_NOTSET_VDBID)
    {
        return false;
    }
    else
    {
        this->open(vdb_id);
        return true;
    }
}

void cvdbValue::open(vdbid_t vdb_id)
{
    _vdbs->parse_vdb_id(vdb_id, &_vat_id, &_vat_head_address, &_tail_length);

    _offset = 0;
    _offset_vat_address = _vat_head_address;
    _offset_vat_offset = 0;

    _allocated_sectors = 0;
    _length = 0;

    if (_vat_id == VDBS_ROOT_VAT_ID && _vat_head_address == VDBS_ROOT_VAT_ADDRESS)
    {
        VDB_DEBUG_LOG("Root Value");
        _is_root_value = true;
        _tail_length = _vdbs->vat_read(_vat_id, VDBS_VAT_END_ID);
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
            uint8_t next_vat_address = _vdbs->vat_read(_vat_id, vat_address);
            if (next_vat_address == VDBS_VAT_END_ID || next_vat_address == VDBS_VAT_FREE_ID)
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

        _length = (vat_elms - 1) * VDBS_DS_SECTOR_SIZE + _tail_length;
        _allocated_sectors = vat_elms;
    }

    VDB_DEBUG_LOG("_vat_id = ", _vat_id);
    VDB_DEBUG_LOG("_vat_head_address = ", _vat_head_address);
    VDB_DEBUG_LOG("_vat_tail_address = ", _vat_tail_address);
    VDB_DEBUG_LOG("_allocated_sectors = ", _allocated_sectors);
    VDB_DEBUG_LOG("_length = ", _length);
    VDB_DEBUG_LOG("_tail_length = ", _tail_length);
}
vdbid_t cvdbValue::close()
{
    _tail_length = _length % VDBS_DS_SECTOR_SIZE;
    _tail_length = (_tail_length == 0 && _length >= VDBS_DS_SECTOR_SIZE) ? VDBS_DS_SECTOR_SIZE : _tail_length;

    if (_is_root_value)
    {
        _vdbs->vat_write(_vat_id, VDBS_VAT_END_ID, (uint8_t)_tail_length);
        return VDBS_ROOT_VDBID;
    }
    else
    {
        return _vdbs->create_vdb_id(_vat_id, _vat_head_address, _tail_length);
    }
}

bool cvdbValue::eof()
{
    return _length == _offset;
}
int cvdbValue::length()
{
    return _length;
}
int cvdbValue::offset()
{
    return _offset;
}

void cvdbValue::seek(int offset)
{

    //VDB_DEBUG_LOG("offset = ", offset);
    int delta = offset - _offset;
    //VDB_DEBUG_LOG("delta = ", delta);
    if (delta == 0)
        return;

    if (offset >= _length)
    {
        // do nothing - read & write method handle this
        //VDB_DEBUG_LOG("Do nothing");
    }
    else if (delta > 0)
    {
        // move forward optimization
        if (_offset_vat_offset + delta < VDBS_DS_SECTOR_SIZE)
        {
            // offset in current sector
            //VDB_DEBUG_LOG("in sector");
            _offset_vat_offset += delta;
        }
        else
        {
            // offset in next sectors
            //VDB_DEBUG_LOG("next sectors");
            int offset_delta = delta - (VDBS_DS_SECTOR_SIZE - _offset_vat_offset);
            int elms_count = offset_delta / VDBS_DS_SECTOR_SIZE + 1;

            //VDB_DEBUG_LOG("offset_delta = ", offset_delta);
            //VDB_DEBUG_LOG("elms_count = ", elms_count);

            for (int i = 0; i < elms_count; i++)
            {
                _offset_vat_address = _vdbs->vat_read(_vat_id, _offset_vat_address);
            }
        }
    }
    else
    {
        // move backward (start from begining)
        _offset_vat_address = _vat_head_address;
        for (int i = 0; i < offset / VDBS_DS_SECTOR_SIZE; i++)
        {
            _offset_vat_address = _vdbs->vat_read(_vat_id, _offset_vat_address);
        }
    }

    _offset = offset;
    _offset_vat_offset = offset % VDBS_DS_SECTOR_SIZE;

    // VDB_DEBUG_LOG("_offset_vat_address = ", _offset_vat_address);
    // VDB_DEBUG_LOG("_offset_vat_offset = ", _offset_vat_offset);
}

void cvdbValue::trim(int length)
{
    length = length == -1 ? _offset + 1 : length;
    if (length >= _length)
        return;

    this->seek(length - 1);
    uint8_t _next_vat_addtess = _vdbs->vat_read(_vat_id, _offset_vat_address);
    if (_next_vat_addtess != VDBS_VAT_END_ID)
    {
        _vdbs->vat_write(_vat_id, _offset_vat_address, VDBS_VAT_END_ID);
        _vat_tail_address = _offset_vat_address;
        _vdbs->free(_vdbs->create_vdb_id(_vat_id, _next_vat_addtess, 0));
    }

    _length = length;
}

void cvdbValue::read(uint8_t *data, int data_length)
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
            val = _vdbs->ds_read(_vat_id, _offset_vat_address, _offset_vat_offset);
            VDB_DEBUG_LOG(val);
        }
        *(data + i) = val;
        this->seek(_offset + 1);
    }
}
bool cvdbValue::write(uint8_t *data, int data_length)
{
    // space section (allocate new space if needed)
    if (_offset + data_length > _length)
    {
        // current length small - need more
        int needed_space = (_offset + data_length) - _length;
        VDB_DEBUG_LOG("needed_space = ", needed_space);
        if (needed_space <= VDBS_DS_SECTOR_SIZE - _tail_length)
        {
            // we already have freespace in current sector
            VDB_DEBUG_LOG("Use free space from tail. _tail_length = ", _tail_length);
            _tail_length += needed_space;
            _length += needed_space;
        }
        else
        {
            // need allocate more space
            int tail_free_space = VDBS_DS_SECTOR_SIZE - _tail_length;
            VDB_DEBUG_LOG("Allocate additonal space");
            vdbid_t new_space_vdb_id = _vdbs->allocate(_vat_id, needed_space - tail_free_space);
            VDB_DEBUG_LOG("new_space_vdb_id = ", new_space_vdb_id);
            if (new_space_vdb_id == VDBS_NOTSET_VDBID)
            {
                // TODO: try to reallocate value to emptiest vat
                return false; //! we have no free space to write this data
            }
            uint8_t new_space_vat_head_address = 0;
            _vdbs->parse_vdb_id(new_space_vdb_id, &_vat_id, &new_space_vat_head_address, &_tail_length);
            _vdbs->vat_write(_vat_id, _vat_tail_address, new_space_vat_head_address);
            _length += needed_space;

            bool need_find_offset_vat_address = false;
            int offset_sector_index = _offset / VDBS_DS_SECTOR_SIZE; //index number of sector pointed by _offset

            if (_offset >= _allocated_sectors * VDBS_DS_SECTOR_SIZE)
            {
                // means that current _offset is outbound of allocated space
                // we need find _offset_vat_address for current _offset while finding tail_address
                // _offset_vat_offset was set in seek();
                need_find_offset_vat_address = true;
                offset_sector_index -= _allocated_sectors;
            }

            //find new tail address
            uint8_t vat_address = new_space_vat_head_address;
            while (true)
            {
                if (need_find_offset_vat_address && offset_sector_index == 0)
                    _offset_vat_address = vat_address;
                offset_sector_index--;
                _allocated_sectors++;

                uint8_t next_vat_address = _vdbs->vat_read(_vat_id, vat_address);

                if (next_vat_address == VDBS_VAT_END_ID)
                {
                    _vat_tail_address = vat_address;
                    break;
                }
                else if (next_vat_address == VDBS_VAT_FREE_ID)
                {
                    // impossible scenario - allocation was corrupted
                    VDB_DEBUG_LOG("!!!!!!!!!!! next_vat_address = VDBS_VAT_FREE_ID");
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
        VDB_DEBUG_LOG("_offset_vat_address = ", _offset_vat_address);
        VDB_DEBUG_LOG("_offset_vat_offset = ", _offset_vat_offset);
        VDB_DEBUG_LOG("ds_write byte = ", val);
        _vdbs->ds_write(_vat_id, _offset_vat_address, _offset_vat_offset, val);
        this->seek(_offset + 1);
    }
    return true;
}
uint8_t cvdbValue::read8()
{
    uint8_t data = 0;
    this->read(&data, sizeof(uint8_t));
    return data;
}
uint16_t cvdbValue::read16()
{
    uint16_t data = 0;
    this->read((uint8_t *)data, sizeof(uint16_t));
    return data;
}
uint32_t cvdbValue::read32()
{
    uint32_t data = 0;
    this->read((uint8_t *)data, sizeof(uint32_t));
    return data;
}

bool cvdbValue::write8(uint8_t data)
{
    return this->write(&data, sizeof(uint8_t));
}
bool cvdbValue::write16(uint16_t data)
{
    return this->write((uint8_t *)&data, sizeof(uint16_t));
}
bool cvdbValue::write32(uint32_t data)
{
    return this->write((uint8_t *)&data, sizeof(uint32_t));
}
