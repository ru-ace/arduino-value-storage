<!---
 Copyright (c) 2018 ace (https://github.com/ru-ace)
 
 This software is released under the MIT License.
 https://opensource.org/licenses/MIT
-->
# Arduino Value Storage Library

[![version][version-badge]][CHANGELOG] [![license][license-badge]][LICENSE]

This library was extracted from my inner project, where i need to store dynamic arrays and strings in EEPROM, which are setted by user via serial interface.

It's desined for very small storages such as internal 1KB EEPROM (in this case you will have 750 bytes for *values*)<br/> 
In my inner project i use internal 1KB EEPROM + 4KB I2C EEPROM chip: 4064 bytes for storing *values*

I will change state of this project from **alpha**  when i finish my inner project.<br/> 
Main cause: it was tested only via synthetic tests (see [main.cpp]).<br/>
Meanwhile, i think it can be usable in current state.<br/> 

## Description

#### Short

With this library you could store fragmented dynamic-length variables in EEPROM. 

#### Long

I assume, that you are familiar with concept of [FAT(File Allocation Table)](https://en.wikipedia.org/wiki/Design_of_the_FAT_file_system#FAT) and [Singly linked list](https://en.wikipedia.org/wiki/Linked_list#Singly_linked_list).

Storage devided into two parts: *vat_storage* and *data_storage*.<br/>
*vat_storage* - contains singly linked lists(wo data), like FAT8, but with one difference: it can contains several *vat (Value Allocation Table)* - 256 bytes each. Each element(byte) of each *vat* has corresponding sector in *data_storage*.<br/>
*data_storage* - divided by sectors of *AVS_DS_SECTOR_SIZE* bytes, which contains *values* data

There is *root_value* which you can access using *value_id = AVS_ROOT_VALUE_ID*. Assumed that you will use it as entry point for accessing your data. 

There is two linked classes:
* *[cavsStorage]* - main class, provides useful methods for works with *vat_storage* & *data_storage*, and high-level *[cavsValue]*-based operations
* *[cavsValue]* - provides file-liked operations to access and modify *values* data


## Documentation

In first place of TODO list, but if you want try this lib before documentation appears,<br/>there is one non-obvious rule:<br/>
**If you update *value* and its length was changed - you must save new *value_id* returned by:** 

* *cavsStorage::*[*update()*|*replace()*]
* *cavsValue::close()* - you must execute this method even for *root_value*, when it was updated and its length was changed (but in case of *root_value* you don't need to save it)  

**Cause:** 

* *value_id* contains part of *value*'s length
* cavsValue::close() - save part of *root_value*'s length in *vat*
* in future releases *value* may be reallocated to other *vat*, which aslo contains in *value_id* 

**Structure of *value_id* when type is uint16_t:** 

* 2 bytes, letter=bit : TTTTVVVV AAAAAAAA
* T = length of tail in last sector in bytes (for *root_value* stored in *vat[AVS_VAT_END_ID]*)
* V = id of *vat* 
* A = head address in *vat* 

Please note, that this structure may be fully reconfigured in [avsConfig.h]

Please read comments in [avsConfig.h] and change it for your storage.<br/>
Useful examples of usage you may found in [main.cpp]

## Limitations

* Maximum size of *data_storage* depends on size of *valueid_t* type.
  * Current *uint16_t* gives access to 65 535 bytes.
  * Using *uint32_t* gives access to 16 777 216 bytes.
* Max sector size of *data_storage*  *AVS_DS_SECTOR_SIZE* = 255 bytes (can be increase after little code update: tail length of *root_value* stored in *vat_storage*, but can be stored in *data_storage*)
* Theoretical max length of *value* = 254 * *AVS_DS_SECTOR_SIZE* (in case when only one *value* used *vat*)

## ToDo

* Documentation/JSDoc
* Reallocation of *value* to new *vat* when there no space in current *vat*.
* No More Space & "foolproof" checks and actions


[CHANGELOG]: ./CHANGELOG.md
[LICENSE]: ./LICENSE
[version-badge]: https://img.shields.io/badge/version-0.9.0_alpha-red.svg
[license-badge]: https://img.shields.io/badge/license-MIT-blue.svg
[avsConfig.h]: ./src/avsConfig.h
[main.cpp]: ./src/main.cpp
[cavsStorage]: ./src/cavsStorage.h
[cavsValue]: ./src/cavsValue.h
