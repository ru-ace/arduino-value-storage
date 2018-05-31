#include "cvdbStorage.h"
#include "vdbDebug.h"

cvdbStorage::cvdbStorage(cvdbProvider *vatProvider, cvdbProvider *dsProvider)
{
    _pvat = vatProvider;
    _pds = dsProvider;
    if (!start())
    {
        VDB_DEBUG_LOG("Format");
        format();
    }
    else
    {
        VDB_DEBUG_LOG("Storage seems Ok");
    }
}

vdbid_t cvdbStorage::create_vdb_id(vatid_t vat_id, uint8_t vat_address, vtail_t tail_length)
{
    return (vdbid_t)vat_address + (vdbid_t)(vat_id << VDBS_ID_VAT_ID_SHIFT) + (vdbid_t)(tail_length << VDBS_ID_TAIL_LENGTH_SHIFT);
}
void cvdbStorage::parse_vdb_id(vdbid_t vdb_id, vatid_t *vat_id, uint8_t *vat_address, vtail_t *tail_length)
{
    *vat_id = (vatid_t)((vdb_id & VDBS_ID_VAT_ID_MASK) >> VDBS_ID_VAT_ID_SHIFT);
    *vat_address = (uint8_t)(vdb_id & VDBS_ID_VAT_ADDRESS_MASK);
    *tail_length = (vtail_t)((vdb_id & VDBS_ID_TAIL_LENGTH_MASK) >> VDBS_ID_TAIL_LENGTH_SHIFT);
}
uint8_t cvdbStorage::vat_read(vatid_t vat_id, uint8_t vat_address)
{
    return _pvat->read(VDBS_VAT_OFFSET + vat_id * 256 + vat_address);
};
void cvdbStorage::vat_write(vatid_t vat_id, uint8_t vat_address, uint8_t data)
{
    _pvat->write(VDBS_VAT_OFFSET + vat_id * 256 + vat_address, data);
};

uint8_t cvdbStorage::ds_read(vatid_t vat_id, uint8_t vat_address, vtail_t offset)
{
    return _pds->read(VDBS_DS_OFFSET + (vat_id * 256 + vat_address) * VDBS_DS_SECTOR_SIZE + offset);
};
void cvdbStorage::ds_write(vatid_t vat_id, uint8_t vat_address, vtail_t offset, uint8_t data)
{
    _pds->write(VDBS_DS_OFFSET + (vat_id * 256 + vat_address) * VDBS_DS_SECTOR_SIZE + offset, data);
};

cvdbValue *cvdbStorage::value_get(vdbid_t vdb_id)
{
    return new cvdbValue(this, vdb_id);
}
cvdbValue *cvdbStorage::value_add()
{
    return new cvdbValue(this, allocate(get_emptiest_vat_id(), 0));
}

vdbid_t cvdbStorage::insert(uint8_t *data, int data_length)
{
    return update(allocate(get_emptiest_vat_id(), data_length), data, data_length);
}
bool cvdbStorage::start()
{
    bool ok = true;
    for (vatid_t vat_id = 0; vat_id < VDBS_VAT_COUNT; vat_id++)
    {
        if (ds_read(vat_id, VDBS_VAT_FREE_ID, 0) != 'V' || ds_read(vat_id, VDBS_VAT_END_ID, 0) != 'S')
        {
            ok = false;
            break;
        }
        else
        {
            vat_freespace[vat_id] = vat_read(vat_id, VDBS_VAT_FREE_ID);
        }
    }
    return ok;
}

void cvdbStorage::format()
{
    //VDB_DEBUG_LOG("Start");
    for (vatid_t vat_id = 0; vat_id < VDBS_VAT_COUNT; vat_id++)
    {
        vat_freespace[vat_id] = vat_id == VDBS_ROOT_VAT_ID ? 253 : 254;
        ds_write(vat_id, VDBS_VAT_FREE_ID, 0, 'V');
        ds_write(vat_id, VDBS_VAT_END_ID, 0, 'S');
        int val = 0;
        for (int i = 0; i < 256; i++)
        {
            //VDB_DEBUG_LOG(i);
            switch (i)
            {
            case VDBS_VAT_FREE_ID:
                val = vat_freespace[vat_id];
                break;
            case VDBS_ROOT_VDBID:
                val = vat_id == VDBS_ROOT_VAT_ID ? VDBS_VAT_END_ID : VDBS_VAT_FREE_ID;
                break;
            default:
                val = VDBS_VAT_FREE_ID;
                break;
            }
            vat_write(vat_id, i, val);
        }
    }
    //VDB_DEBUG_LOG("Finish");
}

vdbid_t cvdbStorage::allocate(vatid_t vat_id, int length)
{
    VDB_DEBUG_LOG("vat_id = ", vat_id);
    uint8_t count = length / VDBS_DS_SECTOR_SIZE;
    if (count > 254)
        return VDBS_NOTSET_VDBID;

    vtail_t tail_length = length % VDBS_DS_SECTOR_SIZE;
    tail_length = (tail_length == 0 && count > 0) ? VDBS_DS_SECTOR_SIZE : tail_length;
    uint8_t vat_elements = (count + 1 - (tail_length == VDBS_DS_SECTOR_SIZE ? 1 : 0));
    if (vat_elements > vat_freespace[vat_id])
        return VDBS_NOTSET_VDBID;

    uint8_t vdb_id_vat_address = VDBS_VAT_FREE_ID;
    uint8_t vat_address = VDBS_VAT_FREE_ID;
    uint8_t prev_vat_address = VDBS_VAT_FREE_ID;

    VDB_DEBUG_LOG("vat_elements = ", vat_elements);

    for (int i = 1; i <= vat_elements; i++)
    {

        vat_address = get_free_vat_address(vat_id);
        if (vat_address == VDBS_VAT_FREE_ID) //bad(and unreal) senario - no more space/something went wrong
        {
            if (prev_vat_address != VDBS_VAT_FREE_ID && vdb_id_vat_address != prev_vat_address) //we already wrote in vat
            {
                vat_write(vat_id, prev_vat_address, VDBS_VAT_END_ID); //close allocated block
                vat_freespace[vat_id] -= (i - 1);                     //lets know for free() real freespace(assume =0)
                free(create_vdb_id(vat_id, vdb_id_vat_address, 0));   //free block
            }
            return VDBS_NOTSET_VDBID;
        }

        vat_write(vat_id, vat_address, VDBS_VAT_END_ID);
        if (i == 1)
            vdb_id_vat_address = vat_address;
        else
            vat_write(vat_id, prev_vat_address, vat_address);
        prev_vat_address = vat_address;
    }
    VDB_DEBUG_LOG("vat_freespace = ", vat_freespace[vat_id]);
    vat_freespace[vat_id] -= vat_elements;
    update_vat_freespace(vat_id);
    VDB_DEBUG_LOG("tail_length = ", tail_length);

    return create_vdb_id(vat_id, vdb_id_vat_address, tail_length);
}
vdbid_t cvdbStorage::move(vdbid_t src_vdb_id, vatid_t dst_vat_id)
{
    cvdbValue *src_val = new cvdbValue(this, src_vdb_id);
    int length = src_val->length();
    vdbid_t dst_vdb_id = allocate(dst_vat_id, length);
    if (dst_vdb_id == 0) //no space on dst_vat
    {
        delete src_val;
        return VDBS_NOTSET_VDBID;
    }
    cvdbValue *dst_val = new cvdbValue(this, dst_vdb_id);
    for (int i = 0; i < length; i++)
    {
        dst_val->write8(src_val->read8());
    }
    delete src_val;
    free(src_vdb_id);
    dst_vdb_id = dst_val->close();
    delete dst_val;
    return dst_vdb_id;
}
vdbid_t cvdbStorage::update(vdbid_t vdb_id, uint8_t *data, int data_length)
{
    cvdbValue *val = new cvdbValue(this, vdb_id);
    int old_length = val->length();
    VDB_DEBUG_LOG("old_length = ", old_length);
    VDB_DEBUG_LOG("data_length = ", data_length);
    val->write(data, data_length);
    if (old_length > data_length)
        val->trim(data_length);

    vdbid_t new_vdb_id = val->close();
    VDB_DEBUG_LOG("new_vdb_id = ", new_vdb_id);
    delete val;
    return new_vdb_id;
}

vdbid_t cvdbStorage::replace(vdbid_t vdb_id, int pos, uint8_t *data, int data_length)
{
    cvdbValue *val = new cvdbValue(this, vdb_id);
    val->seek(pos);
    val->write(data, data_length);
    vdbid_t new_vdb_id = val->close();
    delete val;
    return new_vdb_id;
}

void cvdbStorage::free(vdbid_t vdb_id)
{
    vatid_t vat_id;
    uint8_t vat_address;
    vtail_t tail_length;
    parse_vdb_id(vdb_id, &vat_id, &vat_address, &tail_length);
    while (true)
    {
        uint8_t next_vat_address = vat_read(vat_id, vat_address);
        vat_write(vat_id, vat_address, VDBS_VAT_FREE_ID);
        vat_freespace[vdb_id]++;
        if (next_vat_address == VDBS_VAT_END_ID || next_vat_address == VDBS_VAT_FREE_ID)
        {
            //next_vat_address == VDBS_VAT_FREE_ID - bad scenario, means unfinished write in past
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
uint8_t *cvdbStorage::select(vdbid_t vdb_id, int *length)
{
    cvdbValue *val = new cvdbValue(this, vdb_id);
    *(length) = val->length();
    uint8_t *data = new uint8_t[*(length)];
    val->read(data, *(length));
    delete val;
    return data;
}

uint8_t *cvdbStorage::substr(vdbid_t vdb_id, int pos, int length)
{
    cvdbValue *val = new cvdbValue(this, vdb_id);
    uint8_t *data = new uint8_t[length];
    val->seek(pos);
    val->read(data, length);
    delete val;
    return data;
}

uint8_t cvdbStorage::get_free_vat_address(vatid_t vat_id)
{

    for (int i = 0; i < 256; i++)
        if (i != VDBS_VAT_FREE_ID && i != VDBS_VAT_END_ID)
            if (vat_read(vat_id, i) == VDBS_VAT_FREE_ID)
            {
                VDB_DEBUG_LOG("free address = ", i);
                return i;
            }

    return VDBS_VAT_FREE_ID;
}
vatid_t cvdbStorage::get_emptiest_vat_id()
{

    vatid_t emptiest_vat_id = 0;
    uint8_t emptiest_vat_freespace = vat_freespace[emptiest_vat_id];
    for (vatid_t vat_id = 1; vat_id < VDBS_VAT_COUNT; vat_id++)
    {
        if (emptiest_vat_freespace < vat_freespace[vat_id])
        {
            emptiest_vat_id = vat_id;
            emptiest_vat_freespace = vat_freespace[emptiest_vat_id];
        }
    }
    return emptiest_vat_id;
}

void cvdbStorage::update_vat_freespace(vatid_t vat_id)
{
    vat_write(vat_id, VDBS_VAT_FREE_ID, vat_freespace[vat_id]);
}
