//**************************************************************//
//  Name    : iFidget Firmware
//  Author  : Sean Hodgins
//  Date    : 23/06/2017
//  Notes: Firmware for the POV Fidget Spinner
//  https://github.com/IdleHandsProject/POV_Fidget
//  https://hackaday.io/project/25538-pov-fidget-spinner
//****************************************************************
#include <Arduino.h>

#include "font.h"
#include "ZeroEEPROM.h"
#include "LowPower.h"

int latchPin = 5;
int clockPin = 6;
int dataPin = 4;
int clr = 11;
int chargePin = 8;

#define BUTT1 7
#define BUTT2 10
#define BENC1 13
#define BENC2 12

byte flag1 = 0;
byte flag2 = 0;
int encoderValueB = 0;
int lastEncodedB = 0;
int newB = 0;
int sum = 0;
byte flagB = 0;
byte state1 = 0;
byte state2 = 0;

unsigned int starttime = 0;
unsigned int stoptime = 0;
unsigned int RPM = 0;
unsigned int SpinCount = 0;

byte currentMenu = 0x01;
int menuFlag = 0;
int sleepFlag = 0;
int wakeFlag = 0;

// output tracking vars
int  iWrdIndex;
int  iMsgIndex;
int  iCharIndex;
int  iCharLen;
int wordChange = 0;
int wordWrite = 1;


void setup() {
  //set pins to output so you can control the shift register
  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin, OUTPUT);
  pinMode(BENC1, INPUT);
  pinMode(BENC2, INPUT);
  pinMode(BUTT1, INPUT);
  pinMode(BUTT2, INPUT);
  attachInterrupt(BENC1, trig1, CHANGE);
  //attachInterrupt(BENC2, trig2, CHANGE);
  attachInterrupt(BUTT1, WakeUpEmpty, HIGH);
  attachInterrupt(BUTT2, menuChange, RISING);
  SerialUSB.begin(115200);
  digitalWrite(clr, HIGH);

  /////LOW POWER MODE//////
  //pinMode(6, INPUT_PULLUP);
  pinMode(chargePin, INPUT);
  pinMode(3, INPUT_PULLUP);
  pinMode(2, INPUT_PULLUP);
  pinMode(1, INPUT_PULLUP);
  pinMode(0, INPUT_PULLUP);
  pinMode(A5, INPUT_PULLUP);
  pinMode(A4, INPUT_PULLUP);
  pinMode(A3, INPUT_PULLUP);
  pinMode(A2, INPUT_PULLUP);
  pinMode(A1, INPUT_PULLUP);
  pinMode(A0, INPUT_PULLUP);
  //pinMode(13, INPUT_PULLUP);
  pinMode(38, INPUT_PULLUP);
  pinMode(30, INPUT_PULLUP);
  pinMode(31, INPUT_PULLUP);


  sendByteOut(currentMenu);
  delay(500);
  sendByteOut(0x00);
}

void sendByteOut(int value) {
  digitalWrite(latchPin, LOW);                     // set the strobe LOW
  shiftOut(dataPin, clockPin, LSBFIRST, value);
  digitalWrite(latchPin, HIGH);
  delayMicroseconds(100);
  digitalWrite(latchPin, LOW);

}

void sendCharOut(int character, int row) {
  digitalWrite(latchPin, LOW);                     // set the strobe LOW
  shiftOut(dataPin, clockPin, LSBFIRST, font[character - 32][row]);
  digitalWrite(latchPin, HIGH);
  delayMicroseconds(100);
  digitalWrite(latchPin, LOW);

}

void displayChar(int character) {
  for (int x = 0; x < 2; x++) {
    sendCharOut(' ', x);
    delay(1);
  }
  for (int x = 0; x < 5; x++) {
    sendCharOut(character, x);
    delay(1);
  }
  for (int x = 0; x < 2; x++) {
    sendCharOut(' ', x);
    delay(1);
  }
}

void displayWord(String input) {
  char buff[12];
  input.toCharArray(buff, 12);
  int stringlength = input.length();
  for (int x = 0; x < stringlength; x++) {
    displayChar(buff[x]);
  }
}

void loop() {

  int chargeState = digitalRead(chargePin);
  byte chargeLEDs = 0;
  byte dir = 0;
  while (chargeState == HIGH) {
    chargeState = digitalRead(chargePin);

    sendByteOut(chargeLEDs);

    if (chargeLEDs > 0x80) {
      dir = 1;
    }
    if (chargeLEDs <= 0) {
      dir = 0;
    }
    if (dir == 0) {
      chargeLEDs = chargeLEDs  << 1;
    }
    else {
      chargeLEDs =  chargeLEDs >> 1;
    }
    delay(100);
  }


  if (sleepFlag == 1) {
    digitalWrite(clr, LOW);
    goingtoSleep();
  }
  sleepFlag = 0;
  if (wakeFlag == 1) {
    byte ledstate = 1;
    //attachInterrupt(BUTT1, WakeUpEmpty, HIGH);
    for (int x = 0; x < 15; x++) {
      sendByteOut(ledstate);
      ledstate = ledstate << 1;
      if (ledstate > 0x80) {
        ledstate = ledstate >> 1;
      }
      if (ledstate == 0) {
        ledstate = 0x01;
      }
      delay(10);
    }
    sendByteOut(0x00);
    wakeFlag = 0;
  }

  if (menuFlag == 1) {
    starttime = millis();
    sendByteOut(currentMenu);
    int butt2State = digitalRead(BUTT2);
    int startTime = millis();
    int menuTime = 0;
    while (butt2State == HIGH && menuTime < 1000) {
      int menuWait = millis();
      menuTime = menuWait - startTime;
      butt2State = digitalRead(BUTT2);
    }
    if (butt2State == HIGH) {
      if (currentMenu <= 0x80) {
        currentMenu = currentMenu << 1;
      }
      if (currentMenu == 0) {
        currentMenu = currentMenu + 1;
      }
    }
    sendByteOut(currentMenu);
    delay(500);
    sendByteOut(0x00);
    menuFlag = 0;
  }

  String SRPM = "";

  if (flag1 == 1) {

    starttime = millis();
    unsigned int difTime = starttime - stoptime;
    RPM = difTime * 2;
    RPM = 60000 / RPM;
    SRPM = String(RPM);
    stoptime = starttime;


    if (currentMenu == 0x01) {
      displayWord(SRPM);
    }

    if (currentMenu == 0x02) {
      if (wordChange < 20) {
        displayWord("HELLO");
      }
      if (20 < wordChange && wordChange < 40) {
        displayWord("WORLD");
      }
      wordChange++;
      if (wordChange > 40) {
        wordChange = 0;
      }

    }

    if (currentMenu == 0x04) {
      String SCOUNT = String(SpinCount);
      //SerialUSB.println(SCOUNT);
      displayWord(SCOUNT);
    }

    if (currentMenu == 0x08) {
      if (wordChange < 10) {
        displayWord("DIY");
      }
      if (10 < wordChange && wordChange < 20) {
        displayWord("ARDUINO");
      }
      if (20 < wordChange && wordChange < 30) {
        displayWord("FIDGET");
      }
      if (30 < wordChange && wordChange < 40) {
        displayWord("SPINNER");
      }
      wordChange++;
      if (wordChange > 40) {
        wordChange = 0;
      }
    }

    if (currentMenu == 0x10) {
      if (wordChange < 10) {
        displayWord("POV");
      }
      if (10 < wordChange && wordChange < 20) {
        displayWord("ARDUINO");
      }
      if (20 < wordChange && wordChange < 30) {
        displayWord("FIDGET");
      }
      if (30 < wordChange && wordChange < 40) {
        displayWord("SPINNER");
      }
      wordChange++;
      if (wordChange > 40) {
        wordChange = 0;
      }
    }
    if (currentMenu == 0x20) {
      if (wordChange < 10) {
        displayWord("HELLO");
      }
      if (10 < wordChange && wordChange < 20) {
        displayWord("WORLD");
      }
      if (20 < wordChange && wordChange < 30) {
        displayWord("AND");
      }
      if (30 < wordChange && wordChange < 40) {
        displayWord("HaD :)");
      }
      wordChange++;
      if (wordChange > 40) {
        wordChange = 0;
      }
    }
if (currentMenu == 0x40) {
      if (wordChange < 10) {
        displayWord("HELLO");
      }
      if (10 < wordChange && wordChange < 20) {
        displayWord("REDDIT");
      }
      if (20 < wordChange && wordChange < 30) {
        displayWord("I");
      }
      if (30 < wordChange && wordChange < 40) {
        displayWord("HaD :)");
      }
      wordChange++;
      if (wordChange > 40) {
        wordChange = 0;
      }
    }


    int stretchDelay = 500;
    if (RPM < 1000) {

      stretchDelay = 1000 - RPM;
    }
    delayMicroseconds(stretchDelay);
    flag1 = 0;
  }
  int timeoutSleeptime = millis();
  unsigned long timeoutInt = 25000;
  unsigned long timeoutSleep = timeoutSleeptime - starttime;
  byte pluggedIn = 0;
  pluggedIn = digitalRead(chargePin);
  if (timeoutSleep > timeoutInt) {
    byte ledstate = 1;
    //attachInterrupt(BUTT1, WakeUpEmpty, HIGH);
    for (int x = 0; x < 15; x++) {
      sendByteOut(ledstate);
      ledstate = ledstate << 1;
      if (ledstate > 0x80) {
        ledstate = ledstate >> 1;
      }
      if (ledstate == 0) {
        ledstate = 0x01;
      }
      delay(10);
    }
    sendByteOut(0x00);
    delay(50);
    USBDevice.detach();
    LowPower.standby();
  }
}


void updateEncoderB() {
  int MSB = digitalRead(BENC1); //MSB = most significant bit
  int LSB = digitalRead(BENC2); //LSB = least significant bit

  int encoded = (MSB << 1) | LSB; //converting the 2 pin value to single number
  sum  = (lastEncodedB << 2) | encoded; //adding it to the previous encoded value

  if (sum == 0b0010 || sum == 0b1101 || sum == 0b1011 || sum == 0b0100) encoderValueB ++;
  if (sum == 0b1110 || sum == 0b1000 || sum == 0b0001 || sum == 0b0111) encoderValueB --;

  lastEncodedB = encoded; //store this value for next time
  flagB = 1;
}

int Lstate1 = 0;
int Lstate2 = 0;

void trig1() {
  state1 = digitalRead(BENC1);
  state2 = digitalRead(BENC2);
  if (state1 == 1 && state2 == 1) {
    flag1 = 1;
    SpinCount++;
    int Lstate1 = state1;
    int Lstate2 = state2;
  }
}

void trig2() {
  state2 = digitalRead(BENC2);
  flag2 = 1;
}


void sleepTime() {
  sleepFlag = 1;

}

void goingtoSleep() {
  unsigned long pM = 0;
  const long interval = 100;
  int butt1State = digitalRead(BUTT1);
  int startTime = millis();
  int menuTime = 0;
  byte ledstate = 1;
  while (butt1State == HIGH && menuTime < 4000) {
    int menuWait = millis();
    menuTime = menuWait - startTime;
    butt1State = digitalRead(BUTT1);

    unsigned long cM = millis();
    if (cM - pM >= interval) {
      // save the last time you blinked the LED
      pM = cM;

      sendByteOut(ledstate);
      ledstate = ledstate << 1;
      if (ledstate > 0x80) {
        ledstate = ledstate >> 1;
      }
      if (ledstate == 0) {
        ledstate = 0x01;
      }
    }
  }
  sendByteOut(0x00);
  if (butt1State == HIGH) {

    delay(50);
    detachInterrupt(BENC1);
    detachInterrupt(BENC2);
    //detachInterrupt(BUTT2);
    detachInterrupt(BUTT1);
    attachInterrupt(BUTT1, WakeUp, RISING);
    delay(50);
    USBDevice.detach();
    LowPower.standby();
    detachInterrupt(BUTT1);
    pinMode(BUTT1, OUTPUT);
    delay(10);
    pinMode(BUTT1, INPUT);
  }

}

void menuChange() {
  menuFlag = 1;
}

void WakeUp() {
  wakeFlag = 1;
  USBDevice.attach();
}

void WakeUpEmpty() {
  USBDevice.attach();
  starttime = millis();
  wakeFlag = 1;
}

