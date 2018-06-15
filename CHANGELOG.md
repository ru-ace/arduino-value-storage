<!---
 Copyright (c) 2018 ace (https://github.com/ru-ace)
 
 This software is released under the MIT License.
 https://opensource.org/licenses/MIT
-->
# Changelog 
This file contains changelog of [Arduino Value Storage Library](https://github.com/ru-ace/arduino-value-storage/) project


## [Unreleased]
### Added
- Storage-provider class cavsProviderAT24CX  
- [cavsValue]::[substr, replace]
- Methods using [Arduino String](https://www.arduino.cc/reference/en/language/variables/data-types/stringobject/):
  - [cavsStorage]::[insert, select, update] 
  - [cavsValue]::[read, write]
### Changed
- Remake storage-config-selector section in avsConfig.h
- [cavsValue]::[open, cavsValue] - add open_mode (AVS_VALUE_OPEN_DEFAULT | AVS_VALUE_OPEN_APPEND) 
- [cavsStorage]::free - catch value_id = AVS_NOTSET_VALUE_ID
### Removed
- [cavsValue]::[write16, write32, read16, read32] 

## 0.9.0-alpha - 2018-06-03
- First public release (extracted from inner project)


[Unreleased]: https://github.com/ru-ace/arduino-value-storage/compare/v0.9.0-alpha...HEAD


[cavsStorage]: ./src/cavsStorage.h
[cavsValue]: ./src/cavsValue.h
