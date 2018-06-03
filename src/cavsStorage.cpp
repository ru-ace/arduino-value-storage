// Copyright (c) 2018 ace (https://github.com/ru-ace)
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "cavsStorage.h"
#include "avsDebug.h"

cavsStorage::cavsStorage(cavsProvider *vatProvider, cavsProvider *dsProvider)
{
    _pvat = vatProvider;
    _pds = dsProvider;
    if (!start())
    {
        AVS_DEBUG_LOG("Format");
        format();
    }
    else
    {
        AVS_DEBUG_LOG("Storage seems Ok");
    }
}

valueid_t cavsStorage::create_value_id(vatid_t vat_id, uint8_t vat_address, vtail_t tail_length)
{
    return (valueid_t)vat_address + (valueid_t)(vat_id << AVS_VALUE_ID_VAT_ID_SHIFT) + (valueid_t)(tail_length << AVS_VALUE_ID_TAIL_LENGTH_SHIFT);
}
void cavsStorage::parse_value_id(valueid_t value_id, vatid_t *vat_id, uint8_t *vat_address, vtail_t *tail_length)
{
    *vat_id = (vatid_t)((value_id & AVS_VALUE_ID_VAT_ID_MASK) >> AVS_VALUE_ID_VAT_ID_SHIFT);
    *vat_address = (uint8_t)(value_id & AVS_VALUE_ID_VAT_ADDRESS_MASK);
    *tail_length = (vtail_t)((value_id & AVS_VALUE_ID_TAIL_LENGTH_MASK) >> AVS_VALUE_ID_TAIL_LENGTH_SHIFT);
}
uint8_t cavsStorage::vat_read(vatid_t vat_id, uint8_t vat_address)
{
    return _pvat->read(AVS_VAT_OFFSET + vat_id * 256 + vat_address);
};
void cavsStorage::vat_write(vatid_t vat_id, uint8_t vat_address, uint8_t data)
{
    _pvat->write(AVS_VAT_OFFSET + vat_id * 256 + vat_address, data);
};

uint8_t cavsStorage::ds_read(vatid_t vat_id, uint8_t vat_address, vtail_t offset)
{
    return _pds->read(AVS_DS_OFFSET + (vat_id * 256 + vat_address) * AVS_DS_SECTOR_SIZE + offset);
};
void cavsStorage::ds_write(vatid_t vat_id, uint8_t vat_address, vtail_t offset, uint8_t data)
{
    _pds->write(AVS_DS_OFFSET + (vat_id * 256 + vat_address) * AVS_DS_SECTOR_SIZE + offset, data);
};

cavsValue *cavsStorage::value_get(valueid_t value_id)
{
    return new cavsValue(this, value_id);
}
cavsValue *cavsStorage::value_add()
{
    return new cavsValue(this, allocate(get_emptiest_vat_id(), 0));
}

valueid_t cavsStorage::insert(uint8_t *data, int data_length)
{
    return update(allocate(get_emptiest_vat_id(), data_length), data, data_length);
}
bool cavsStorage::start()
{
    bool ok = true;
    for (vatid_t vat_id = 0; vat_id < AVS_VAT_COUNT; vat_id++)
    {
        if (ds_read(vat_id, AVS_VAT_FREE_ID, 0) != 'V' || ds_read(vat_id, AVS_VAT_END_ID, 0) != 'S')
        {
            ok = false;
            break;
        }
        else
        {
            vat_freespace[vat_id] = vat_read(vat_id, AVS_VAT_FREE_ID);
        }
    }
    return ok;
}

void cavsStorage::format()
{
    //AVS_DEBUG_LOG("Start");
    for (vatid_t vat_id = 0; vat_id < AVS_VAT_COUNT; vat_id++)
    {
        vat_freespace[vat_id] = vat_id == AVS_ROOT_VAT_ID ? 253 : 254;
        ds_write(vat_id, AVS_VAT_FREE_ID, 0, 'V');
        ds_write(vat_id, AVS_VAT_END_ID, 0, 'S');
        int val = 0;
        for (int i = 0; i < 256; i++)
        {
            //AVS_DEBUG_LOG(i);
            switch (i)
            {
            case AVS_VAT_FREE_ID:
                val = vat_freespace[vat_id];
                break;
            case AVS_ROOT_VALUE_ID:
                val = vat_id == AVS_ROOT_VAT_ID ? AVS_VAT_END_ID : AVS_VAT_FREE_ID;
                break;
            default:
                val = AVS_VAT_FREE_ID;
                break;
            }
            vat_write(vat_id, i, val);
        }
    }
    //AVS_DEBUG_LOG("Finish");
}

valueid_t cavsStorage::allocate(vatid_t vat_id, int length)
{
    AVS_DEBUG_LOG("vat_id = ", vat_id);
    uint8_t count = length / AVS_DS_SECTOR_SIZE;
    if (count > 254)
        return AVS_NOTSET_VALUE_ID;

    vtail_t tail_length = length % AVS_DS_SECTOR_SIZE;
    tail_length = (tail_length == 0 && count > 0) ? AVS_DS_SECTOR_SIZE : tail_length;
    uint8_t vat_elements = (count + 1 - (tail_length == AVS_DS_SECTOR_SIZE ? 1 : 0));
    if (vat_elements > vat_freespace[vat_id])
        return AVS_NOTSET_VALUE_ID;

    uint8_t value_id_vat_address = AVS_VAT_FREE_ID;
    uint8_t vat_address = AVS_VAT_FREE_ID;
    uint8_t prev_vat_address = AVS_VAT_FREE_ID;

    AVS_DEBUG_LOG("vat_elements = ", vat_elements);

    for (int i = 1; i <= vat_elements; i++)
    {

        vat_address = get_free_vat_address(vat_id);
        if (vat_address == AVS_VAT_FREE_ID) //bad(and unreal) senario - no more space/something went wrong
        {
            if (prev_vat_address != AVS_VAT_FREE_ID && value_id_vat_address != prev_vat_address) //we already wrote in vat
            {
                vat_write(vat_id, prev_vat_address, AVS_VAT_END_ID);    //close allocated block
                vat_freespace[vat_id] -= (i - 1);                       //lets know for free() real freespace(assume =0)
                free(create_value_id(vat_id, value_id_vat_address, 0)); //free block
            }
            return AVS_NOTSET_VALUE_ID;
        }

        vat_write(vat_id, vat_address, AVS_VAT_END_ID);
        if (i == 1)
            value_id_vat_address = vat_address;
        else
            vat_write(vat_id, prev_vat_address, vat_address);
        prev_vat_address = vat_address;
    }
    AVS_DEBUG_LOG("vat_freespace = ", vat_freespace[vat_id]);
    vat_freespace[vat_id] -= vat_elements;
    update_vat_freespace(vat_id);
    AVS_DEBUG_LOG("tail_length = ", tail_length);

    return create_value_id(vat_id, value_id_vat_address, tail_length);
}
valueid_t cavsStorage::move(valueid_t src_value_id, vatid_t dst_vat_id)
{
    cavsValue *src_val = new cavsValue(this, src_value_id);
    int length = src_val->length();
    valueid_t dst_value_id = allocate(dst_vat_id, length);
    if (dst_value_id == 0) //no space on dst_vat
    {
        delete src_val;
        return AVS_NOTSET_VALUE_ID;
    }
    cavsValue *dst_val = new cavsValue(this, dst_value_id);
    for (int i = 0; i < length; i++)
    {
        dst_val->write8(src_val->read8());
    }
    delete src_val;
    free(src_value_id);
    dst_value_id = dst_val->close();
    delete dst_val;
    return dst_value_id;
}
valueid_t cavsStorage::update(valueid_t value_id, uint8_t *data, int data_length)
{
    cavsValue *val = new cavsValue(this, value_id);
    int old_length = val->length();
    AVS_DEBUG_LOG("old_length = ", old_length);
    AVS_DEBUG_LOG("data_length = ", data_length);
    val->write(data, data_length);
    if (old_length > data_length)
        val->trim(data_length);

    valueid_t new_value_id = val->close();
    AVS_DEBUG_LOG("new_value_id = ", new_value_id);
    delete val;
    return new_value_id;
}

valueid_t cavsStorage::replace(valueid_t value_id, int pos, uint8_t *data, int data_length)
{
    cavsValue *val = new cavsValue(this, value_id);
    val->seek(pos);
    val->write(data, data_length);
    valueid_t new_value_id = val->close();
    delete val;
    return new_value_id;
}

void cavsStorage::free(valueid_t value_id)
{
    vatid_t vat_id;
    uint8_t vat_address;
    vtail_t tail_length;
    parse_value_id(value_id, &vat_id, &vat_address, &tail_length);
    while (true)
    {
        uint8_t next_vat_address = vat_read(vat_id, vat_address);
        vat_write(vat_id, vat_address, AVS_VAT_FREE_ID);
        vat_freespace[value_id]++;
        if (next_vat_address == AVS_VAT_END_ID || next_vat_address == AVS_VAT_FREE_ID)
        {
            //next_vat_address == AVS_VAT_FREE_ID - bad scenario, means unfinished write in past
            break;
        }
        else
        {
            vat_address = next_vat_address;
        }
    }
    //update vat freespace
    update_vat_freespace(vat_id);
}
uint8_t *cavsStorage::select(valueid_t value_id, int *length)
{
    cavsValue *val = new cavsValue(this, value_id);
    *(length) = val->length();
    uint8_t *data = new uint8_t[*(length)];
    val->read(data, *(length));
    delete val;
    return data;
}

uint8_t *cavsStorage::substr(valueid_t value_id, int pos, int length)
{
    cavsValue *val = new cavsValue(this, value_id);
    uint8_t *data = new uint8_t[length];
    val->seek(pos);
    val->read(data, length);
    delete val;
    return data;
}

uint8_t cavsStorage::get_free_vat_address(vatid_t vat_id)
{

    for (int i = 0; i < 256; i++)
        if (i != AVS_VAT_FREE_ID && i != AVS_VAT_END_ID)
            if (vat_read(vat_id, i) == AVS_VAT_FREE_ID)
            {
                AVS_DEBUG_LOG("free address = ", i);
                return i;
            }

    return AVS_VAT_FREE_ID;
}
vatid_t cavsStorage::get_emptiest_vat_id()
{

    vatid_t emptiest_vat_id = 0;
    uint8_t emptiest_vat_freespace = vat_freespace[emptiest_vat_id];
    for (vatid_t vat_id = 1; vat_id < AVS_VAT_COUNT; vat_id++)
    {
        if (emptiest_vat_freespace < vat_freespace[vat_id])
        {
            emptiest_vat_id = vat_id;
            emptiest_vat_freespace = vat_freespace[emptiest_vat_id];
        }
    }
    return emptiest_vat_id;
}

void cavsStorage::update_vat_freespace(vatid_t vat_id)
{
    vat_write(vat_id, AVS_VAT_FREE_ID, vat_freespace[vat_id]);
}
