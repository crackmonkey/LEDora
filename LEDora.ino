#include <EEPROM.h>

#include <MMA8452Q.h>

#include <Adafruit_NeoPixel.h>

#include <Wire.h>
#include <I2Cdev.h>
#include <MPR121.h>

// EEPROM address to store the default mode
#define DEFAULTMODE 0

byte mode;
byte modeOverride = 0;
unsigned long modeOverrideTimeout = 0;

Adafruit_NeoPixel strip = Adafruit_NeoPixel(19, 15, NEO_GRB + NEO_KHZ800);
byte midpt;
uint32_t color = Adafruit_NeoPixel::Color(255,0,0);
byte r,g,b;


MPR121 touch = MPR121();

MMA8452Q accel;
int axes[3];

extern String cmdbuf;  // from the commands file

void setup() {
  Serial.begin((uint16_t)115200); // cast to silence a warning
  
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
  strip.setBrightness(64); // Dim so I don't blind myself testing
  midpt = strip.numPixels() / 2;
  
  r=g=b=0;

  touch.initialize();
  touch.setCallback(0, MPR121::TOUCHED, &modeCallback);

  accel.begin();
  accel.scale(4);
  
  cmdbuf.reserve(32);
  
  mode = EEPROM.read(DEFAULTMODE); // EEPROM 0 is the default mode
}

void modeCallback() {
    changeMode(mode +1);
}

void loop() {
  // Process touch events
  touch.serviceCallbacks();
  
  // Handle command over serial (USB or BT)
  processCommands();
  
  // handle animation
  if (modeOverrideTimeout) { // if the mode is temporarilly overridden
    if (millis() < modeOverrideTimeout) { // and it hasn't expired
      animate(modeOverride);
    } else { // clear the override
      modeOverrideTimeout = 0;
      // animate normally
      animate(mode);
    }
  } else { // animate normally
    animate(mode);
  }
}

