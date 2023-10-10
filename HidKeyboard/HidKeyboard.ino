/*
  HID Keyboard example


  created 2020
  by Deqing Sun for use with CH55xduino

  This example code is in the public domain.

  modified by @pakleni

*/
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

#ifndef USER_USB_RAM
#error "This example needs to be compiled with a USER USB setting"
#endif

#include "USBHIDMediaKeyboard.h"

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
volatile int mRotaryEncoderPulse = 0;
volatile uint8_t mLastestRotaryEncoderPinAB = 0; // last last pin value of A and B
volatile uint8_t mLastRotaryEncoderPinAB = 0;    // last pin value of A and B

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

// This block sends 24 bits to the LED
// Each LED accepts 24 bits and forwards the rest
// after the delay, a new block will need to be sent
//
// Send Green value from Bit7 to 0
// Send Red value from Bit7 to 0
// Send Blue value from Bit7 to 0
//
// 24MHz is 41.666666666667ns per clock cycle
// 0 is 300ns high, 900ns low
// 1 is 600ns high, 600ns low
// 300ns is 7.2 clock cycles
// 600ns is 14.4 clock cycles
// 900ns is 21.6 clock cycles
void neopixel_show_long_P3_4(uint32_t dataAndLen)
{
  //'dpl' (LSB),'dph','b' & 'acc'
  // DPTR is the array address, B is the low byte of length
  __asm__("    mov r3, b                           \n"
          ";save EA to R6                          \n"
          "    mov c,_EA                           \n"
          "    clr a                               \n"
          "    rlc a                               \n"
          "    mov r6, a                           \n"
          ";disable interrupt                      \n"
          "    clr _EA                             \n"

          "byteLoop$:                              \n"
          "    movx  a,@dptr                       \n"
          "    inc dptr                            \n"
          "    mov r2,#8                           \n"
          "bitLoop$:                               \n"
          "    rlc a                               \n"
          "    setb _P3_4                          \n"
          "    jc bit7High$                        \n"
          "    clr _P3_4                           \n"
          "bit7High$:                              \n"
          "    mov r1,#5                           \n"
          "bitDelay$:                              \n"
          "    djnz r1,bitDelay$                   \n"
          "    clr _P3_4                           \n"
          "    djnz r2,bitLoop$                    \n"
          "    djnz r3,byteLoop$                   \n"

          ";restore EA from R6                     \n"
          "    mov a,r6                            \n"
          "    jz  skipRestoreEA_NP$               \n"
          "    setb  _EA                           \n"
          "skipRestoreEA_NP$:                      \n");
  (void)dataAndLen;
}

// brightness is from 0 to 1
#define BRIGHTNESS 0.3
#define COLOR_DELAY 1000
byte RValue = 0x00, GValue = 0x00, BValue = 0xff;
#define NUM_LEDS 6
#define NUM_BYTES (NUM_LEDS * 3)
__xdata uint8_t ledData[NUM_BYTES];

#define set_pixel_for_GRB_LED(ADDR, INDEX, R, G, B) \
  {                                                 \
    __xdata uint8_t *ptr = (ADDR) + ((INDEX)*3);    \
    ptr[0] = (G);                                   \
    ptr[1] = (R);                                   \
    ptr[2] = (B);                                   \
  };

#define neopixel_show_P3_4(ADDR, LEN)                     \
  neopixel_show_long_P3_4((((uint16_t)(ADDR)) & 0xFFFF) | \
                          (((uint32_t)(LEN)&0xFF) << 16));

void handleColor()
{
  static byte colorDelay = 0;
  static byte mode = 0;

  // The color delay is used for delaying a color change
  if (colorDelay > 0)
  {
    colorDelay--;
    return;
  }

  switch (mode)
  {
  case 0:
    RValue++;
    BValue--;
    if (RValue == 0xff)
    {
      mode = 1;
    }
    break;
  case 1:
    GValue++;
    RValue--;
    if (GValue == 0xff)
    {
      mode = 2;
    }
    break;
  case 2:
    BValue++;
    GValue--;
    if (BValue == 0xff)
    {
      mode = 0;
    }
    break;
  }

  // Send color codes to the daisy chained LEDs
  for (int i = 0; i < 6; i++)
  {
    set_pixel_for_GRB_LED(ledData, i, RValue * BRIGHTNESS, GValue * BRIGHTNESS, BValue * BRIGHTNESS);
  }
  neopixel_show_P3_4(ledData, NUM_BYTES);

  colorDelay = COLOR_DELAY;
}

void buttonPress(int i)
{
  bool press = !digitalRead(pins[i]);
  if (pressPrev[i] != press)
  {
    pressPrev[i] = press;
    if (press)
    {
      Keyboard_press(keypad[i]);
    }
    else
    {
      Keyboard_release(keypad[i]);
    }
    delays[i] = 50;
  }
}

void setup()
{
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

  pinMode(LEDPIN, OUTPUT);
  digitalWrite(LEDPIN, LOW);
}

void loop()
{
  // BUTTONS 1..6
  for (int i = 0; i < 6; i++)
  {
    if (delays[i] > 0)
    {
      delays[i]--;
      continue;
    }
    buttonPress(i);
  }

  // BUTTON 7
  static bool pressPrev7 = false;
  static byte delay7 = 0;

  if (delay7 > 0)
  {
    delay7--;
  }
  else
  {
    bool press = !digitalRead(EC11D_PIN);
    if (pressPrev7 != press)
    {
      pressPrev7 = press;
      if (press)
      {
        Consumer_press(KeyPad7);
      }
      else
      {
        Consumer_release(KeyPad7);
      }
      delay7 = 50; // naive debouncing
    }
  }

  // BUTTON 8, 9
  static byte delay89 = 0;
  if (delay89 > 0)
  {
    delay89--;
  }
  else
  {
    uint8_t currentPin = digitalRead(EC11A_PIN) * 10 + digitalRead(EC11B_PIN);
    if (currentPin != mLastRotaryEncoderPinAB)
    {
      if (mLastRotaryEncoderPinAB == 00)
      {
        if (mLastestRotaryEncoderPinAB == 10 && currentPin == 01)
        {
          Consumer_press(KeyPad8);
          delay(10);
          Consumer_release(KeyPad8);
        }
        else if (mLastestRotaryEncoderPinAB == 01 && currentPin == 10)
        {
          Consumer_press(KeyPad9);
          delay(10);
          Consumer_release(KeyPad9);
        }
      }
      mLastestRotaryEncoderPinAB = mLastRotaryEncoderPinAB;
      mLastRotaryEncoderPinAB = currentPin;
    }
    delay89 = 5;
  }
  // Cycle the hue of the displayed color
  handleColor();
}
