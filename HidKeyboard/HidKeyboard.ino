/*
  HID Keyboard example


  created 2020
  by Deqing Sun for use with CH55xduino

  This example code is in the public domain.

  modified by @pakleni

*/
//键位定义
// #define KeyPad1 KEY_F1
// #define KeyPad2 KEY_F2
// #define KeyPad3 KEY_F3
// #define KeyPad4 KEY_F4
// #define KeyPad5 KEY_F5
// #define KeyPad6 KEY_F6
// #define KeyPad7 KEY_F7
// #define KeyPad8 KEY_UP_ARROW
// #define KeyPad9 KEY_DOWN_ARROW

// #define KeyPad1 KEY_F13
// #define KeyPad2 KEY_F14
// #define KeyPad3 KEY_F15
// #define KeyPad4 KEY_F16
// #define KeyPad5 KEY_F17
// #define KeyPad6 KEY_F18
// #define KeyPad7 KEY_F19
// #define KeyPad8 KEY_F20
// #define KeyPad9 KEY_F21

#define KeyPad1 KEY_LEFT_ALT
#define KeyPad2 'w'
#define KeyPad3 'f'
#define KeyPad4 'a'
#define KeyPad5 's'
#define KeyPad6 'd'

// These are done via HIDConsumer
#define KeyPad7 MEDIA_VOLUME_MUTE
#define KeyPad8 MEDIA_VOLUME_UP
#define KeyPad9 MEDIA_VOLUME_DOWN

#define BRIGHTNESS 127
//For windows user, if you ever played with other HID device with the same PID C55D
//You may need to uninstall the previous driver completely        


#define TX(LedColor) {\
  if (((LedColor)&0x80)==0) {\
       XdigitalWriteFast(3,4,HIGH);\
       (LedColor)=(LedColor)<<1;\
       XdigitalWriteFast(3,4,LOW);\
       XdigitalWriteFast(3,4,LOW);\
       XdigitalWriteFast(3,4,LOW);\
   }else{\
       XdigitalWriteFast(3,4,HIGH);\
       XdigitalWriteFast(3,4,HIGH);\
       XdigitalWriteFast(3,4,HIGH);\
       XdigitalWriteFast(3,4,HIGH);\
       XdigitalWriteFast(3,4,HIGH);\
       XdigitalWriteFast(3,4,HIGH);\
       XdigitalWriteFast(3,4,HIGH);\
       (LedColor)=(LedColor)<<1;\
       XdigitalWriteFast(3,4,LOW);\
       XdigitalWriteFast(3,4,LOW);\
       XdigitalWriteFast(3,4,LOW);\
       XdigitalWriteFast(3,4,LOW);\
       XdigitalWriteFast(3,4,LOW);\
       }\
  }
byte RValue,GValue,BValue;

#ifndef USER_USB_RAM
#error "This example needs to be compiled with a USER USB setting"
#endif

#include "USBHIDMediaKeyboard.h"

// This block sends 24 bits to the LED
// Each LED accepts 24 bits and forwards the rest
// after the delay, a new block will need to be sent 
//Send Green value from Bit7 to 0
//Send Red value from Bit7 to 0
//Send Blue value from Bit7 to 0
#define sendColor() {\
  TX(GValue);TX(GValue);TX(GValue);TX(GValue);TX(GValue);TX(GValue);TX(GValue);TX(GValue);\
  TX(RValue);TX(RValue);TX(RValue);TX(RValue);TX(RValue);TX(RValue);TX(RValue);TX(RValue);\
  TX(BValue);TX(BValue);TX(BValue);TX(BValue);TX(BValue);TX(BValue);TX(BValue);TX(BValue);\
}


byte mode;

#define setupCycleColor() {\
  mode = 0;\
  GValue=0x00;\
  RValue=0x00;\
  BValue=BRIGHTNESS;\
}

void cycleColor() {
  switch(mode) {
    case 0:
      if (RValue == BRIGHTNESS || BValue == 0) {
        mode = 1;
        return cycleColor();
      }
      RValue++;
      BValue--;
      break;
    case 1:
      if (GValue == BRIGHTNESS || RValue == 0) {
        mode = 2;
        return cycleColor();
      }
      GValue++;
      RValue--;
      break;
    case 2:
      if (BValue == BRIGHTNESS || GValue == 0) {
        mode = 0;
        return cycleColor();
      }
      BValue++;
      GValue--;
      break;
  }
}

void buttonPress (bool * pressPrev, int pin, int keycode) {
  bool press = !digitalRead(pin);
  if ((*pressPrev) != press) {
    (*pressPrev) = press;
    if (press) {
      Keyboard_press(keycode);
    } else {
      Keyboard_release(keycode);
    }
    delay(50);  //naive debouncing
  }
}

//// PIN = CODE
// ------
// 1 = 32 -> BUTTON6
// 2 = 14 -> BUTTON5
// 3 = 15 -> BUTTON4
// 4 = 16 -> BUTTON3
// 5 = 17 -> BUTTON2
// 6 = RST
// 7 = 31 -> ?
// 8 = 30
// 9 = 11 -> BUTTON1
// 10 = 33 -> ?
// 11 = 34
// 12 = 36 -> SW1
// 13 = 37
// 14 = GND -> all diodes upper right
// 15 = VCC -> all diodes down left
// 16 = V33
// diodes are daisy chained
// I believe that the diodes are SK6812

#define BUTTON1_PIN 11
#define BUTTON2_PIN 17
#define BUTTON3_PIN 16
#define BUTTON4_PIN 15
#define BUTTON5_PIN 14
#define BUTTON6_PIN 32
#define EC11D_PIN 33
#define EC11A_PIN 31
#define EC11B_PIN 30
#define LEDPIN 34
volatile int     mRotaryEncoderPulse        = 0;
volatile uint8_t mLastestRotaryEncoderPinAB = 0; // last last pin value of A and B
volatile uint8_t mLastRotaryEncoderPinAB    = 0; // last pin value of A and B

bool button1PressPrev = false;
bool button2PressPrev = false;
bool button3PressPrev = false;
bool button4PressPrev = false;
bool button5PressPrev = false;
bool button6PressPrev = false;
bool button7PressPrev = false;
bool button8PressPrev = false;
bool button9PressPrev = false;

void setup() {
  USBInit();
  pinMode(BUTTON1_PIN, INPUT_PULLUP);
  pinMode(BUTTON2_PIN, INPUT_PULLUP);
  pinMode(BUTTON3_PIN, INPUT_PULLUP);
  pinMode(BUTTON4_PIN, INPUT_PULLUP);
  pinMode(BUTTON5_PIN, INPUT_PULLUP);
  pinMode(BUTTON6_PIN, INPUT_PULLUP);
  pinMode(EC11D_PIN, INPUT_PULLUP);
  pinMode(EC11A_PIN, INPUT_PULLUP);
  pinMode(EC11B_PIN, INPUT_PULLUP);

  pinMode(LEDPIN,OUTPUT);
  digitalWrite(LEDPIN,LOW); 

  setupCycleColor();
}

void loop() {

  buttonPress(&button1PressPrev, BUTTON1_PIN, KeyPad1);
  buttonPress(&button2PressPrev, BUTTON2_PIN, KeyPad2);
  buttonPress(&button3PressPrev, BUTTON3_PIN, KeyPad3);
  buttonPress(&button4PressPrev, BUTTON4_PIN, KeyPad4);
  buttonPress(&button5PressPrev, BUTTON5_PIN, KeyPad5);
  buttonPress(&button6PressPrev, BUTTON6_PIN, KeyPad6);

  bool button7Press = !digitalRead(EC11D_PIN);
  if (button7PressPrev != button7Press) {
    button7PressPrev = button7Press;
    if (button7Press) {
      Consumer_press(KeyPad7);
    } else {
      Consumer_release(KeyPad7);
    }
    delay(50);  //naive debouncing
  }

  uint8_t currentPin = digitalRead(EC11A_PIN) * 10 + digitalRead(EC11B_PIN);
  if (currentPin != mLastRotaryEncoderPinAB) {
        if (mLastRotaryEncoderPinAB == 00) {
          if (mLastestRotaryEncoderPinAB == 10 && currentPin == 01) {
            Consumer_press(KeyPad8);
            delay(10);
            Consumer_release(KeyPad8);
          }
          else if (mLastestRotaryEncoderPinAB == 01 && currentPin == 10) {
            Consumer_press(KeyPad9);
            delay(10);
            Consumer_release(KeyPad9);
          }
        }
        mLastestRotaryEncoderPinAB = mLastRotaryEncoderPinAB;
        mLastRotaryEncoderPinAB = currentPin;       
  }
  delay(5);  //naive debouncing

  sendColor();
  sendColor();
  sendColor();
  sendColor();
  sendColor();
  sendColor();
  cycleColor();
}
