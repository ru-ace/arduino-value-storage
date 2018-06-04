// Copyright (c) 2018 ace (https://github.com/ru-ace)
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include <Arduino.h>

#define AVS_DEBUG
#include "avsDebug.h"

#include "cavsProviderEEPROM.h"
#include "cavsProviderAT24CX.h"
#include "cavsStorage.h"

cavsProviderEEPROM avsProviderEEPROM;
#ifdef AVS_CONFIG_CURRENT_PROJECT
cavsProviderAT24CX avsProviderAT24CX;
#endif //AVS_CONFIG_CURRENT_PROJECT
cavsStorage *avs;

void avs_erase_eeprom();

void avs_test_1_simple();
void avs_test_2_variable_list_of_values();
void avs_test_3_seek();
void avs_test_4_multi_vat();

void setup()
{

    AVS_DEBUG_START;
    //avs_erase_eeprom();
    AVS_DEBUG_LOG_FREEMEM;
#ifdef AVS_CONFIG_CURRENT_PROJECT
    avs = new cavsStorage(&avsProviderEEPROM, &avsProviderAT24CX);
#else
    avs = new cavsStorage(&avsProviderEEPROM, &avsProviderEEPROM);
#endif //AVS_CONFIG_CURRENT_PROJECT
    //AVS_DEBUG_DUMP_EEPROM;
    AVS_DEBUG_LOG_FREEMEM;
    //avs_test_1_simple();
    avs_test_2_variable_list_of_values();
    //avs_test_3_seek();
    //avs_test_4_multi_vat();
    AVS_DEBUG_LOG_FREEMEM;
    AVS_DEBUG_LOG("That's all folks!");
}
void avs_test_4_multi_vat()
{
    avs->format();

    uint8_t string_vat0[] = "0123456";
    AVS_DEBUG_LOG(" ================= Insert new val [0123456]=================");
    valueid_t new_val_id = avs->insert(string_vat0, sizeof(string_vat0));
    AVS_DEBUG_LOG("new_val_id = ", new_val_id);
    AVS_DEBUG_DUMP_EEPROM;
    uint8_t string_vat1[] = "ABCDEFG";
    AVS_DEBUG_LOG(" ================= Insert new val [ABCDEFG]=================");
    new_val_id = avs->insert(string_vat1, sizeof(string_vat1));
    AVS_DEBUG_LOG("new_val_id = ", new_val_id);

    AVS_DEBUG_DUMP_EEPROM;
}

void avs_test_3_seek()
{
    cavsValue *root;
    int i;
    avs->format();
    AVS_DEBUG_LOG("==== Create root value obj ===");
    root = new cavsValue(avs, AVS_ROOT_VALUE_ID);
    /*
    AVS_DEBUG_LOG("==== Fill with 'x'");

    for (i = 0; i < 61; i++)
        root->write8('x');

    root->close();
    */
    AVS_DEBUG_LOG("==== SEEK  10  ==================================");
    root->seek(10);
    for (i = 0; i < 8; i++)
        root->write8('X');
    root->close();

    AVS_DEBUG_LOG("==== BACKWARD SEEK  ==================================");
    root->seek(9);
    root->write8(0);
    for (i = 8; i > 1; i--)
    {
        AVS_DEBUG_LOG("seek ", i);
        root->seek(i);
        root->write8('0' + i);
    }

    root->close();
    delete root;

    int val_len = 0;
    uint8_t *val;
    AVS_DEBUG_LOG("==== substr(root, 2,8) ===================================");
    val = avs->substr(AVS_ROOT_VALUE_ID, 2, 8);
    AVS_DEBUG_LOG("len = ", val_len);
    AVS_DEBUG_LOG("val = ", (char *)val);
    delete val;

    uint8_t substring[] = "abcdefg";
    AVS_DEBUG_LOG("==== replace(root, 2, 'abcdefg', 8) ===================================");
    avs->replace(AVS_ROOT_VALUE_ID, 2, substring, sizeof(substring));

    AVS_DEBUG_LOG("final lenght = ", root->length());
    AVS_DEBUG_DUMP_EEPROM;
}
void avs_test_2_variable_list_of_values()
{
    valueid_t record_value_id;
    cavsValue *root;
    cavsValue *record;

    avs->format();
    AVS_DEBUG_LOG("==== Create root value obj ===");
    root = new cavsValue(avs, AVS_ROOT_VALUE_ID);
    AVS_DEBUG_LOG("==== Create record value obj ===");
    record = new cavsValue(avs);
    AVS_DEBUG_LOG("==== WRITE SECTION ===================================");
    for (int i = 0; i < 10; i++)
    {
        AVS_DEBUG_LOG("===== Record ", i);
        record->create();

        for (int j = 0; j < i + 1; j++)
        {
            AVS_DEBUG_LOG("====== write char ", i);
            record->write8('0' + i);
        }
        record->write8(0);
        record_value_id = record->close();
        AVS_DEBUG_LOG(" add to root record_value_id = ", record_value_id);
        root->write((uint8_t *)&record_value_id, sizeof(record_value_id));
    }
    root->close();

    delete root;
    delete record;

    AVS_DEBUG_DUMP_EEPROM;
    AVS_DEBUG_LOG("==== READ SECTION ===================================");
    int val_len = 0;
    uint8_t *val;

    AVS_DEBUG_LOG("==== Create root value obj ===");
    root = new cavsValue(avs, AVS_ROOT_VALUE_ID);
    while (!root->eof())
    {
        AVS_DEBUG_LOG("Read next record_value_id from root");
        root->read((uint8_t *)&record_value_id, sizeof(record_value_id));
        AVS_DEBUG_LOG("Select record with record_value_id = ", record_value_id);
        val = avs->select(record_value_id, &val_len);
        AVS_DEBUG_LOG("len = ", val_len);
        AVS_DEBUG_LOG("val = ", (char *)val);
        delete val;
    }

    delete root;
}

void avs_test_1_simple()
{
    int val_len = 0;
    uint8_t *val;
    avs->format();
    AVS_DEBUG_LOG(" ================= Update Root with [ABCD] =================");
    uint8_t string1[] = "ABCD";
    avs->update(AVS_ROOT_VALUE_ID, string1, sizeof(string1));
    AVS_DEBUG_DUMP_EEPROM;

    uint8_t string2[] = "0123";
    AVS_DEBUG_LOG(" ================= Insert new val [0123]=================");
    valueid_t new_val_id = avs->insert(string2, sizeof(string2));
    AVS_DEBUG_LOG("new_val_id = ", new_val_id);
    AVS_DEBUG_DUMP_EEPROM;
    uint8_t string3[] = "ABCDEFG";
    AVS_DEBUG_LOG(" ================= Update Root with [ABCDEFG] =================");
    avs->update(AVS_ROOT_VALUE_ID, string3, sizeof(string3));
    AVS_DEBUG_DUMP_EEPROM;

    AVS_DEBUG_LOG(" ================= Read inserted val =================");
    val = avs->select(new_val_id, &val_len);
    AVS_DEBUG_LOG("len = ", val_len);
    AVS_DEBUG_LOG("val = ", (char *)val);
    delete val;

    AVS_DEBUG_LOG(" ================= Read Root val =================");
    val = avs->select(AVS_ROOT_VALUE_ID, &val_len);
    AVS_DEBUG_LOG("len = ", val_len);
    AVS_DEBUG_LOG("val = ", (char *)val);
    delete val;
}

void avs_erase_eeprom()
{
    AVS_DEBUG_LOG("Erease EEPROM with 0xEE");
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
