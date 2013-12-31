#include <AnySerial.h>

#include <EEPROM.h>

#include <MMA8452Q.h>

#include <Adafruit_NeoPixel.h>

#include <Wire.h>
#include <I2Cdev.h>
#include <MPR121.h>
#include <rn52.h>

// pin wired to GPIO9 on the RN52, used to trigger command mode
#define BT_CMD_PIN 6
#define BT_9600_PIN 7

// EEPROM address to store the default mode
#define EE_DEFAULT_ANIMATION 0 // 1 byte
#define EE_COLOR 1 // 3 bytes

// back right
#define BTN_ANIMATION 5
// back left
#define BTN_BRIGHTNESS 10
// front
#define BTN_FRONT 1
// front center
#define BTN_PLAY 6
// front left
#define BTN_PREVTRACK 7
// front right
#define BTN_NEXTTRACK 0
// top
#define BTN_ANSWER 11
// left brim
#define BTN_VOLUP 2
// right brim
#define BTN_VOLDOWN 8
// left dimple
#define BTN_LEFT 9
// right dimple
#define BTN_RIGHT 3
// inside
#define BTN_WEAR 4

byte mode;
byte modeOverride = 0;
unsigned long modeOverrideTimeout = 0;

//Adafruit_NeoPixel strip = Adafruit_NeoPixel(19, 15, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel strip = Adafruit_NeoPixel(37, 15, NEO_GRB + NEO_KHZ800);
byte midpt;
uint32_t color;
uint32_t ringColors[2]; // the colors to flash in "ring" mode, set by cmdRing
byte r,g,b;

byte brightness;

MPR121 touch = MPR121();

MMA8452Q accel;
int axes[3];

extern String cmdbuf;  // from the commands file

RN52 btmodule = RN52(6, Serial1);

void setup() {
  AnySerial.begin(9600);
  
  // the default 115200 gets corrupted, so run the UART in slow mode
  // HIGH=115200, LOW = 9600
  pinMode(BT_9600_PIN, OUTPUT);
  digitalWrite(BT_9600_PIN, LOW);
  
  btmodule.begin();
  
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
  strip.setBrightness(64); // Dim so I don't blind myself testing
  midpt = strip.numPixels() / 2;

  touch.initialize();
  touch.setCallback(BTN_ANIMATION, MPR121::TOUCHED, &modeCallback);
  touch.setCallback(BTN_BRIGHTNESS, MPR121::TOUCHED, &brightnessCallback);
  touch.setCallback(BTN_ANSWER, MPR121::TOUCHED, &answerCallback);
  
  touch.setCallback(BTN_WEAR, MPR121::TOUCHED, &wearCallback);
  touch.setCallback(BTN_WEAR, MPR121::RELEASED, &removeCallback);
  
  touch.setCallback(BTN_VOLUP, MPR121::TOUCHED, &volUpCallback);
  touch.setCallback(BTN_VOLDOWN, MPR121::TOUCHED, &volDownCallback);
  
  touch.setCallback(BTN_NEXTTRACK, MPR121::TOUCHED, &nextTrackCallback);
  touch.setCallback(BTN_PREVTRACK, MPR121::TOUCHED, &prevTrackCallback);
  touch.setCallback(BTN_PLAY, MPR121::TOUCHED, &playCallback);
  
  touch.setCallback(BTN_LEFT, MPR121::TOUCHED, &leftCallback);
  touch.setCallback(BTN_RIGHT, MPR121::TOUCHED, &rightCallback);
  touch.setCallback(BTN_FRONT, MPR121::TOUCHED, &frontCallback);

  accel.begin();
  accel.scale(4);
  
  randomSeed(analogRead(A0)); // seed the RNG with noise from an unconnected ADC
  
  // pre-allocate a big-enough string buffer so the String class doesn't end up reallocating the string for every byte that comes in
  cmdbuf.reserve(32);
  
  mode = EEPROM.read(EE_DEFAULT_ANIMATION); // EEPROM 0 is the default mode
  /*
  r = EEPROM.read(EE_COLOR);
  g = EEPROM.read(EE_COLOR+1);
  b = EEPROM.read(EE_COLOR+2);
  color = Adafruit_NeoPixel::Color(r,g,b);
  */
  cmdColor("00407f");
}

void modeCallback() {
    changeAnimation(mode +1);
}

void brightnessCallback() {
  brightness += 64;
  cmdBrightness(String(brightness, HEX));
}

// doesn't work unless the HFP/HSP profile is connected
// which won't happen because it disabled because I don't have the MIC input wired properly
void answerCallback() {
  AnySerial.println("answer");
  btmodule.answer(); 
}

void volUpCallback() {
  AnySerial.println("volUp");
  btmodule.volUp();
}

void volDownCallback() {
  AnySerial.println("volDown");
  btmodule.volDown();
}

void nextTrackCallback() {
  AnySerial.println("nextTrack");
  btmodule.nextTrack();
}

void prevTrackCallback() {
  AnySerial.println("prevTrack");
  btmodule.prevTrack();
}

void playCallback() {
  AnySerial.println("play");
  btmodule.play();
}

void leftCallback() {
  AnySerial.println("left");
}

void rightCallback() {
  AnySerial.println("right");
}

void frontCallback() {
  AnySerial.println("front");
}

void wearCallback() {
  AnySerial.println("hat=on");
}

void removeCallback() {
  AnySerial.println("hat=off");
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

