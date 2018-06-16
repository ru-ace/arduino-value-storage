<!---
 Copyright (c) 2018 ace (https://github.com/ru-ace)
 
 This software is released under the MIT License.
 https://opensource.org/licenses/MIT
-->
# Changelog 
This file contains changelog of [Arduino Value Storage Library](https://github.com/ru-ace/arduino-value-storage/) project


## [Unreleased]
### Changed
- Add AVS_USE_ARDUINO_STRING_METHODS in [avsConfig.h]. If defined: will declared methods, which are using [Arduino String](https://www.arduino.cc/reference/en/language/variables/data-types/stringobject/):
  - [cavsStorage]::[insert, select, update] 
  - [cavsValue]::[read, write]
- data_length param in methods [cavsStorage]::[insert, update] now optional. Use only when data is null-terminated string.   

## [0.9.1-alpha] - 2018-06-15
### Added
- cavsProviderAT24CX - Storage-provider class for Amtel 2-Wire Serial EEPROM Chips AT24CX
- [cavsValue]::[substr, replace]
- [cavsValue]::[open, cavsValue] - add param open_mode (AVS_VALUE_OPEN_DEFAULT | AVS_VALUE_OPEN_APPEND) 
- Methods using [Arduino String](https://www.arduino.cc/reference/en/language/variables/data-types/stringobject/):
  - [cavsStorage]::[insert, select, update] 
  - [cavsValue]::[read, write]
- [main.cpp] - add avs_test_5_String_methods() 
### Changed
- Remake storage-config-selector section in [avsConfig.h]
- [cavsStorage]::free(), can catch value_id = AVS_NOTSET_VALUE_ID
### Fixed
- Fix bug in [cavsStorage]::free(), which was produce randomized effects.
### Removed
- [cavsValue]::[write16, write32, read16, read32] 

## 0.9.0-alpha - 2018-06-03
- First public release (extracted from inner project)


[Unreleased]: https://github.com/ru-ace/arduino-value-storage/compare/v0.9.1-alpha...HEAD
[0.9.1-alpha]: https://github.com/ru-ace/arduino-value-storage/compare/v0.9.0-alpha...v0.9.1-alpha

[cavsStorage]: ./src/cavsStorage.h
[cavsValue]: ./src/cavsValue.h
[main.cpp]: ./src/main.cpp
[avsConfig.h]: ./src/avsConfig.h
