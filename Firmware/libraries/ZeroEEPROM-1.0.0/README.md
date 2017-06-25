# ZeroEEPROM
EEPROM emulation with SAMD internal flash memory

See examples folder for simple example. 

The API is very similar to the well knownArduino EEPROM.h API, but with three 
additional functions:

* init()
** initially reads "eeprom" data from flash. This has to be done once. Ideally in setup().
* isValid()
** to check if the eeprom data is valid. This is the case, if at least one the eeprom data has been written to flash. Otherwise eeprom data is "undefined".
* commit()
** store the eeprom data in flash. Use this with care: Every call writes the complete eeprom data to flash. This will reduce the remainig flash-write-cycles. Don't call this method in a loop or you will kill your flash soon.
