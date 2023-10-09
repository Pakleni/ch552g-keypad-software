/*
  HID Keyboard example


  created 2020
  by Deqing Sun for use with CH55xduino

  This example code is in the public domain.

  modified by @pakleni

*/
//键位定义
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


// 24MHz is 41.666666666667ns per clock cycle
// 0 is 300ns high, 900ns low
// 1 is 600ns high, 600ns low
// 300ns is 7.2 clock cycles
// 600ns is 14.4 clock cycles
// 900ns is 21.6 clock cycles
// I however have no idea how to do this
#define TX(LedColor) {\
  if (((LedColor)&0x80)==0) {\
       XdigitalWriteFast(3,4,HIGH);\
       (LedColor)=(LedColor)<<1;\
       XdigitalWriteFast(3,4,LOW);\
       delayMicroseconds(1);\
   }else{\
       XdigitalWriteFast(3,4,HIGH);\
       delayMicroseconds(1);\
       (LedColor)=(LedColor)<<1;\
       XdigitalWriteFast(3,4,LOW);\
       }\
  }

#ifndef USER_USB_RAM
#error "This example needs to be compiled with a USER USB setting"
#endif

#include "USBHIDMediaKeyboard.h"

// This block sends 24 bits to the LED
// Each LED accepts 24 bits and forwards the rest
// after the delay, a new block will need to be sent
//
// Send Green value from Bit7 to 0
// Send Red value from Bit7 to 0
// Send Blue value from Bit7 to 0
//
byte RValue,GValue,BValue;
#define sendColor() {\
  TX(GValue);TX(GValue);TX(GValue);TX(GValue);TX(GValue);TX(GValue);TX(GValue);TX(GValue);\
  TX(RValue);TX(RValue);TX(RValue);TX(RValue);TX(RValue);TX(RValue);TX(RValue);TX(RValue);\
  TX(BValue);TX(BValue);TX(BValue);TX(BValue);TX(BValue);TX(BValue);TX(BValue);TX(BValue);\
}

// brightness is from 0 to 255
#define BRIGHTNESS 255
#define COLOR_DELAY 1000

void handleColor() {
  static byte colorDelay = 0;
  static byte mode = 0;
  static byte Red = 0x00,Green = 0x00,Blue = BRIGHTNESS;

  // Send color codes to the daisy chained LEDs
  for (int i = 0; i < 6; i++) {
    RValue = Red;
    GValue = Green;
    BValue = Blue;
    sendColor();
  }

  // The color delay is used for delaying a color change
  if (colorDelay > 0) {
    colorDelay--;
    return;
  }

  switch(mode) {
    case 0:
      Red++;
      Blue--;
      if (Red == BRIGHTNESS) {
        mode = 1;
      }
      break;
    case 1:
      Green++;
      Red--;
      if (Green == BRIGHTNESS) {
        mode = 2;
      }
      break;
    case 2:
      Blue++;
      Green--;
      if (Blue == BRIGHTNESS) {
        mode = 0;
      }
      break;
  }

  colorDelay = COLOR_DELAY;
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

byte pins[] = {
  BUTTON1_PIN,
  BUTTON2_PIN,
  BUTTON3_PIN,
  BUTTON4_PIN,
  BUTTON5_PIN,
  BUTTON6_PIN,
};

byte keypad[] = {
  KeyPad1,
  KeyPad2,
  KeyPad3,
  KeyPad4,
  KeyPad5,
  KeyPad6,
};

bool pressPrev[] = {
  false,
  false,
  false,
  false,
  false,
  false,
};

byte delays[] = {
  0,
  0,
  0,
  0,
  0,
  0,
};

void buttonPress(int i) {
  bool press = !digitalRead(pins[i]);
  if (pressPrev[i] != press) {
    pressPrev[i] = press;
    if (press) {
      Keyboard_press(keypad[i]);
    } else {
      Keyboard_release(keypad[i]);
    }
    delays[i]=50;
  }
}

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
}

void loop() {
  // BUTTONS 1..6
  for(int i = 0; i <6; i++) {
    if (delays[i] > 0) {
      delays[i]--;
      continue;
    }
    buttonPress(i);
  }

  // BUTTON 7
  static bool pressPrev7 = false;
  static byte delay7 = 0;

  if (delay7 > 0) {
    delay7--;
  }
  else {
    bool press = !digitalRead(EC11D_PIN);
    if (pressPrev7 != press) {
      pressPrev7 = press;
      if (press) {
        Consumer_press(KeyPad7);
      } else {
        Consumer_release(KeyPad7);
      }
      delay7=50;  //naive debouncing
    }
  }

  // BUTTON 8, 9
  static byte delay89 = 0;
  if (delay89 > 0) {
    delay89--;
  }
  else {
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
    delay89=5;
  }
  // Cycle the hue of the displayed color
  handleColor();
}
