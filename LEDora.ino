#include <AnySerial.h>

#include <EEPROM.h>

#include <MMA8452Q.h>

#include <Adafruit_NeoPixel.h>

#include <Wire.h>
#include <I2Cdev.h>
#include <MPR121.h>

// pin wired to GPIO9 on the RN52, used to trigger command mode
#define BT_CMD_PIN 6
#define BT_9600_PIN 7

// EEPROM address to store the default mode
#define EE_DEFAULT_ANIMATION 0 // 1 byte
#define EE_COLOR 1 // 3 bytes

byte mode;
byte modeOverride = 0;
unsigned long modeOverrideTimeout = 0;

//Adafruit_NeoPixel strip = Adafruit_NeoPixel(19, 15, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel strip = Adafruit_NeoPixel(37, 15, NEO_GRB + NEO_KHZ800);
byte midpt;
uint32_t color;
byte r,g,b;


MPR121 touch = MPR121();

MMA8452Q accel;
int axes[3];

extern String cmdbuf;  // from the commands file

void setup() {
  AnySerial.begin(9600);
  /*
  Serial.begin((uint16_t)9600); // cast to silence a warning
  Serial1.begin((uint16_t)9600);
  */
  
  // HIGH=normal data pipe, LOW=command mode
  pinMode(BT_CMD_PIN, OUTPUT);
  digitalWrite(BT_CMD_PIN, HIGH);
  // the default 115200 gets corrupted, so run the UART in slow mode
  // HIGH=115200, LOW = 9600
  pinMode(BT_9600_PIN, OUTPUT);
  digitalWrite(BT_9600_PIN, LOW);
  
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
  strip.setBrightness(64); // Dim so I don't blind myself testing
  midpt = strip.numPixels() / 2;

  touch.initialize();
  touch.setCallback(0, MPR121::TOUCHED, &modeCallback);

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

