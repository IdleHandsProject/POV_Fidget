#include "ZeroEEPROM.h"

void setup() {
  // initialize digital pin 13 as an output.
  pinMode(13, OUTPUT);

  SerialUSB.begin(9600);
  while (!SerialUSB) { }

  SerialUSB.println("Zero running ...");
  
  

  SerialUSB.println("Reading data");
  long start = micros();
  ZeroEEPROM.init();
  long ende = micros();
  SerialUSB.print("Reading data *done* duration=");
  SerialUSB.print(ende-start);
  SerialUSB.println("us");

  // If this is the first run the "valid" value should be "false"...
  if (!ZeroEEPROM.isValid()) {


    SerialUSB.println("EEPROM in factory. WRITING!");
    
    // set "valid" to true, so the next time we know that we
    // have valid data inside
    for(int i=0;i<1024;i++){
      ZeroEEPROM.write(i, 0xC0);  
    }
    
    
 
    long start = micros();    
    ZeroEEPROM.commit();
    long ende = micros();
    SerialUSB.print("Writing data *done* duration=");
    SerialUSB.print(ende-start);
    SerialUSB.println("us");

    for(int i=0;i<1024;i++){
      
      SerialUSB.print("after write: addr=");
      SerialUSB.print(i);
      SerialUSB.print(" -> value=");
      SerialUSB.println(ZeroEEPROM.read(i));
    }
    SerialUSB.print("after write: valid? -> ");
    SerialUSB.println(ZeroEEPROM.isValid());
   

  } else {

    SerialUSB.println("EEPROM already valid.");
    
    for(int i=0;i<1024;i++){
      
      SerialUSB.print("addr=");
      SerialUSB.print(i);
      SerialUSB.print(" -> value=");
      SerialUSB.println(ZeroEEPROM.read(i));
    }
        SerialUSB.print("valid? -> ");
    SerialUSB.println(ZeroEEPROM.isValid());

  }
  
}

// the loop function runs over and over again forever
void loop() {
  digitalWrite(13, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(300);              // wait for a second
  digitalWrite(13, LOW);    // turn the LED off by making the voltage LOW
  delay(300);              // wait for a second
}
