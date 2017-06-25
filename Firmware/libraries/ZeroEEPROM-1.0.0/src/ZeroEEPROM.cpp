/*
  Copyright (c) 2015 Arduino LLC.  All right reserved.
  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.
  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the GNU Lesser General Public License for more details.
  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA


  EEPROM like API that uses Arduino Zero's flash memory. 
  Requirements: cmaglie's Flash Storage: https://github.com/cmaglie/FlashStorage
  (c) 2016, A. Christian
*/

#include "Arduino.h"
#include "ZeroEEPROM.h"

FlashStorage(my_flash_ptr, EEPROM_EMULATION); 

ZeroEEPROMClass::ZeroEEPROMClass(void) {
  _dirty = false;
  _eeprom.valid = false;  

  
  _flash = &my_flash_ptr;
}

uint8_t ZeroEEPROMClass::read(int address)
{
  return _eeprom.data[address]; 
}

void ZeroEEPROMClass::write(int address, uint8_t value)
{
  _dirty = true;
  _eeprom.data[address] = value; 
}

void ZeroEEPROMClass::update(int address, uint8_t value)
{
  _eeprom.data[address] = value; 
}

void ZeroEEPROMClass::init()
{
  _eeprom = _flash->read();
}

bool ZeroEEPROMClass::isValid()
{
  return _eeprom.valid;
}

void ZeroEEPROMClass::commit()
{
  if (_dirty) {
    _eeprom.valid=true;
    _flash->write(_eeprom);
  }
}

ZeroEEPROMClass ZeroEEPROM;
