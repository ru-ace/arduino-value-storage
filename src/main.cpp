// Copyright (c) 2018 ace (https://github.com/ru-ace)
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include <Arduino.h>
#include "vdbDebug.h"
#include "cvdbProviderEEPROM.h"
#include "cvdbStorage.h"

cvdbProviderEEPROM vdbProviderEEPROM;
cvdbStorage *vdb;

void vdb_erase_eeprom();

void vdb_test_1_simple();
void vdb_test_2_variable_list_of_values();
void vdb_test_3_seek();
void vdb_test_4_multi_vat();

void setup()
{

    VDB_DEBUG_START;
    vdb_erase_eeprom();
    VDB_DEBUG_LOG_FREEMEM;

    vdb = new cvdbStorage(&vdbProviderEEPROM, &vdbProviderEEPROM);
    //VDB_DEBUG_DUMP_EEPROM;
    VDB_DEBUG_LOG_FREEMEM;
    //vdb_test_1_simple();
    vdb_test_2_variable_list_of_values();
    //vdb_test_3_seek();
    //vdb_test_4_multi_vat();
    VDB_DEBUG_LOG_FREEMEM;
    VDB_DEBUG_LOG("That's all folks!");
}
void vdb_test_4_multi_vat()
{
    vdb->format();

    uint8_t string_vat0[] = "0123456";
    VDB_DEBUG_LOG(" ================= Insert new val [0123456]=================");
    vdbid_t new_val_id = vdb->insert(string_vat0, sizeof(string_vat0));
    VDB_DEBUG_LOG("new_val_id = ", new_val_id);
    VDB_DEBUG_DUMP_EEPROM;
    uint8_t string_vat1[] = "ABCDEFG";
    VDB_DEBUG_LOG(" ================= Insert new val [ABCDEFG]=================");
    new_val_id = vdb->insert(string_vat1, sizeof(string_vat1));
    VDB_DEBUG_LOG("new_val_id = ", new_val_id);

    VDB_DEBUG_DUMP_EEPROM;
}

void vdb_test_3_seek()
{
    cvdbValue *root;
    int i;
    vdb->format();
    VDB_DEBUG_LOG("==== Create root value obj ===");
    root = new cvdbValue(vdb, VDBS_ROOT_VDBID);
    /*
    VDB_DEBUG_LOG("==== Fill with 'x'");

    for (i = 0; i < 61; i++)
        root->write8('x');

    root->close();
    */
    VDB_DEBUG_LOG("==== SEEK  10  ==================================");
    root->seek(10);
    for (i = 0; i < 8; i++)
        root->write8('X');
    root->close();

    VDB_DEBUG_LOG("==== BACKWARD SEEK  ==================================");
    root->seek(9);
    root->write8(0);
    for (i = 8; i > 1; i--)
    {
        VDB_DEBUG_LOG("seek ", i);
        root->seek(i);
        root->write8('0' + i);
    }

    root->close();
    delete root;

    int val_len = 0;
    uint8_t *val;
    VDB_DEBUG_LOG("==== substr(root, 2,8) ===================================");
    val = vdb->substr(VDBS_ROOT_VDBID, 2, 8);
    VDB_DEBUG_LOG("len = ", val_len);
    VDB_DEBUG_LOG("val = ", (char *)val);
    delete val;

    uint8_t substring[] = "abcdefg";
    VDB_DEBUG_LOG("==== replace(root, 2, 'abcdefg', 8) ===================================");
    vdb->replace(VDBS_ROOT_VDBID, 2, substring, sizeof(substring));

    VDB_DEBUG_LOG("final lenght = ", root->length());
    VDB_DEBUG_DUMP_EEPROM;
}
void vdb_test_2_variable_list_of_values()
{
    vdbid_t record_vdbid;
    cvdbValue *root;
    cvdbValue *record;

    vdb->format();
    VDB_DEBUG_LOG("==== Create root value obj ===");
    root = new cvdbValue(vdb, VDBS_ROOT_VDBID);
    VDB_DEBUG_LOG("==== Create record value obj ===");
    record = new cvdbValue(vdb);
    VDB_DEBUG_LOG("==== WRITE SECTION ===================================");
    for (int i = 0; i < 10; i++)
    {
        VDB_DEBUG_LOG("===== Record ", i);
        record->create();

        for (int j = 0; j < i + 1; j++)
        {
            VDB_DEBUG_LOG("====== write char ", i);
            record->write8('0' + i);
        }
        record->write8(0);
        record_vdbid = record->close();
        VDB_DEBUG_LOG(" add to root record_vdbid = ", record_vdbid);
        root->write((uint8_t *)&record_vdbid, sizeof(record_vdbid));
    }
    root->close();

    delete root;
    delete record;

    VDB_DEBUG_DUMP_EEPROM;
    VDB_DEBUG_LOG("==== READ SECTION ===================================");
    int val_len = 0;
    uint8_t *val;

    VDB_DEBUG_LOG("==== Create root value obj ===");
    root = new cvdbValue(vdb, VDBS_ROOT_VDBID);
    while (!root->eof())
    {
        VDB_DEBUG_LOG("Read next record_vdbid from root");
        root->read((uint8_t *)&record_vdbid, sizeof(record_vdbid));
        VDB_DEBUG_LOG("Select record with record_vdbid = ", record_vdbid);
        val = vdb->select(record_vdbid, &val_len);
        VDB_DEBUG_LOG("len = ", val_len);
        VDB_DEBUG_LOG("val = ", (char *)val);
        delete val;
    }

    delete root;
}

void vdb_test_1_simple()
{
    int val_len = 0;
    uint8_t *val;
    vdb->format();
    VDB_DEBUG_LOG(" ================= Update Root with [ABCD] =================");
    uint8_t string1[] = "ABCD";
    vdb->update(VDBS_ROOT_VDBID, string1, sizeof(string1));
    VDB_DEBUG_DUMP_EEPROM;
    /*
    uint8_t string2[] = "0123";
    VDB_DEBUG_LOG(" ================= Insert new val [0123]=================");
    vdbid_t new_val_id = vdb->insert(string2, sizeof(string2));
    VDB_DEBUG_LOG("new_val_id = ", new_val_id);
    VDB_DEBUG_DUMP_EEPROM;
    uint8_t string3[] = "ABCDEFG";
    VDB_DEBUG_LOG(" ================= Update Root with [ABCDEFG] =================");
    vdb->update(VDBS_ROOT_VDBID, string3, sizeof(string3));
    VDB_DEBUG_DUMP_EEPROM;
    */
    /*
    VDB_DEBUG_LOG(" ================= Read inserted val =================");
    val = vdb->select(new_val_id, &val_len);
    VDB_DEBUG_LOG("len = ", val_len);
    VDB_DEBUG_LOG("val = ", (char *)val);
    delete val;

    VDB_DEBUG_LOG(" ================= Read Root val =================");
    val = vdb->select(VDBS_ROOT_VDBID, &val_len);
    VDB_DEBUG_LOG("len = ", val_len);
    VDB_DEBUG_LOG("val = ", (char *)val);
    delete val;
*/
}

void vdb_erase_eeprom()
{
    VDB_DEBUG_LOG("Erease EEPROM with 0xEE");
    for (int address = 0; address <= E2END; address++)
        EEPROM.write(address, 0xee);
}

int led_val = 0;
int led_dir = 1;
int led_count = 10;

void loop()
{
    if (led_count == 0)
        return;
    led_val += led_dir;
    bool switch_dir = led_val == 3 || led_val == 0;
    led_dir = switch_dir ? -led_dir : led_dir;
    led_count -= switch_dir ? 1 : 0;
    analogWrite(LED_BUILTIN, led_val);
    delay(100);
}
