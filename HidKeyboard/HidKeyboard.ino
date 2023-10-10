/*
  HID Keyboard example


  created 2020
  by Deqing Sun for use with CH55xduino

  This example code is in the public domain.

  modified 2023
  by pakleni
*/
#define KeyPad1 KEY_LEFT_ALT
#define KeyPad2 'w'
#define KeyPad3 'f'
#define KeyPad4 'a'
#define KeyPad5 's'
#define KeyPad6 'd'

// These are done via HIDConsumer
#define RotaryKeypress MEDIA_VOLUME_MUTE
#define RotaryCW MEDIA_VOLUME_UP
#define RotaryCCW MEDIA_VOLUME_DOWN

#ifndef USER_USB_RAM
#error "This example needs to be compiled with a USER USB setting"
#endif

#include "src/userUsbHidMediaKeyboard/USBHIDMediaKeyboard.h"
#include <WS2812.h>

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

// brightness is from 0 to 100
double brightness = 30;
// color delay controls the speed of the color change
// higher value means slower change
// it is in millis
#define COLOR_DELAY 50
#define NUM_LEDS 6
#define NUM_BYTES (NUM_LEDS * 3)

void handleColor()
{
  static byte mode = 0;
  static byte RValue = 0x00, GValue = 0x00, BValue = 0xff;
  static __xdata uint8_t ledData[NUM_BYTES];
  static unsigned long lastColorChange = 0;

  // The color delay is used for delaying a color change
  if (millis() < lastColorChange + COLOR_DELAY)
  {
    return;
  }

  switch (mode)
  {
  case 0:
    ++RValue;
    --BValue;
    if (RValue == 0xff)
    {
      mode = 1;
    }
    break;
  case 1:
    ++GValue;
    --RValue;
    if (GValue == 0xff)
    {
      mode = 2;
    }
    break;
  case 2:
    ++BValue;
    --GValue;
    if (BValue == 0xff)
    {
      mode = 0;
    }
    break;
  }

  double pbrightness = brightness / 100.0;
  byte red = RValue * pbrightness;
  byte green = GValue * pbrightness;
  byte blue = BValue * pbrightness;
  // Send color codes to the daisy chained LEDs
  for (int i = 0; i < 6; i++)
  {
    set_pixel_for_GRB_LED(ledData, i, red, green, blue);
  }
  // This sends 24 bits for each LED
  // Each LED accepts 24 bits and forwards the rest
  // if changing the LED pin, make sure to change the
  // neopixel_show_P3_4 function as well
  neopixel_show_P3_4(ledData, NUM_BYTES);

  lastColorChange = millis();
}

// This controlls the button
// presses and releases
// for the 6 buttons
struct Button
{
  bool pressPrev;
  unsigned long lastPressed;
  byte pin;
  byte key;
};

struct Button buttons[] = {
    {false, 0, BUTTON1_PIN, KeyPad1},
    {false, 0, BUTTON2_PIN, KeyPad2},
    {false, 0, BUTTON3_PIN, KeyPad3},
    {false, 0, BUTTON4_PIN, KeyPad4},
    {false, 0, BUTTON5_PIN, KeyPad5},
    {false, 0, BUTTON6_PIN, KeyPad6},
};

void buttonPress(int i)
{
  struct Button *button = buttons + i;

  if (millis() > button->lastPressed + 50)
  {
    bool press = !digitalRead(button->pin);
    if (button->pressPrev != press)
    {
      button->pressPrev = press;
      if (press)
      {
        Keyboard_press(button->key);
      }
      else
      {
        Keyboard_release(button->key);
      }
      button->lastPressed = millis();
    }
  }
}

// This code controls the rotary encoder
// adapted from
// https://github.com/micromouseonline/BasicEncoder
#define m_pin_a EC11A_PIN
#define m_pin_b EC11B_PIN
#define m_pin_active LOW
#define m_steps_per_count 4
#define m_reversed true

int8_t m_previous_state = 0;

int8_t pin_state()
{
  int8_t state_now = 0;
  if (digitalRead(m_pin_a) == m_pin_active)
  {
    state_now |= 2;
  }
  if (digitalRead(m_pin_b) == m_pin_active)
  {
    state_now |= 1;
  }
  return state_now;
}

// Read changes frequently enough that overflows cannot happen.
// Servicing and reading can be done at different rates.
// I read the change in the main loop and service the encoder
int8_t get_change()
{
  // This part is the servicing
  static volatile int m_change = 0;

  int8_t state_now = pin_state();
  state_now ^= state_now >> 1; // two bit gray-to-binary
  int8_t difference = m_previous_state - state_now;
  // bit 1 has the direction, bit 0 is set if changeed
  if (difference & 1)
  {
    m_previous_state = state_now;
    int delta = (difference & 2) - 1;
    if (m_reversed)
    {
      delta = -delta;
    }
    m_change += delta;
  }

  // This part is the reading
  int8_t change = m_change;
  // the switch statement can make better code because only optimised
  // operations are used instead of generic division
  switch (m_steps_per_count)
  {
  case 4:
    m_change %= 4;
    change /= 4;
    break;
  case 2:
    m_change %= 2;
    change /= 2;
    break;
  default:
    m_change = 0;
    break;
  }
  return change;
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

  m_previous_state = pin_state();
}

void loop()
{
  // BUTTONS 1..6
  for (int i = 0; i < 6; i++)
  {
    buttonPress(i);
  }

  // Rotary Encoder A and B
  // button 0 + encoder should control the brightness
  // if the button is not pressed, then the encoder
  // should control the volume
  int encoder_change = get_change();
  if (encoder_change)
  {
    if (encoder_change > 0 && !buttons->pressPrev)
    {
      Consumer_write(RotaryCW);
    }
    else if (encoder_change < 0 && !buttons->pressPrev)
    {
      Consumer_write(RotaryCCW);
    }
    else if (encoder_change > 0 && buttons->pressPrev)
    {
      if (brightness < 100)
      {
        ++brightness;
      }
    }
    else if (encoder_change < 0 && buttons->pressPrev)
    {
      if (brightness > 0)
      {
        --brightness;
      }
    }
  }

  // Rotary Encoder Button
  static bool presPrevRE = false;
  static unsigned long lastPressedRE = 0;

  // debounce
  if (millis() > lastPressedRE + 50)
  {
    bool press = !digitalRead(EC11D_PIN);
    if (presPrevRE != press)
    {
      presPrevRE = press;
      if (press)
      {
        Consumer_press(RotaryKeypress);
      }
      else
      {
        Consumer_release(RotaryKeypress);
      }
      lastPressedRE = millis();
    }
  }

  // Cycle the hue of the displayed color
  handleColor();
}
