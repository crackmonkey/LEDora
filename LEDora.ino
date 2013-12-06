#include <MMA8452Q.h>

#include <Adafruit_NeoPixel.h>

#include <Wire.h>
#include <I2Cdev.h>
#include <MPR121.h>

enum {
  FLICKER = 0,
  CYLON = 1, 
  STTOS = 2
};
#define MAXMODE 2
byte mode = 0;


Adafruit_NeoPixel strip = Adafruit_NeoPixel(19, 15, NEO_GRB + NEO_KHZ800);
byte midpt;
uint32_t color = Adafruit_NeoPixel::Color(255,0,0);
byte r,g,b;


MPR121 touch = MPR121();

MMA8452Q accel;
int axes[3];

void setup() {
  Serial.begin(115200);
  
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
  midpt = strip.numPixels() / 2;
  
  r=g=b=0;

  touch.initialize();
      touch.setCallback(0, MPR121::TOUCHED, modeCallback);
  if (touch.testConnection()) {

  } else {
    Serial.println("Failed to connect to MPR121");
  }

  accel.begin();
  accel.scale(4);
}

void modeCallback() {
    mode++;
    if (mode > MAXMODE) mode=0;
    Serial.print("Mode:");
    Serial.println(mode);
}

void loop() {
  touch.serviceCallbacks();
  switch (mode) {
    case FLICKER:
      flicker();
      break;
    case CYLON:
      cylon();
      break;
    case STTOS:
      sttos();
      break;
    default:
      flicker();
  }
}

// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint8_t wait) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, c);
      strip.show();
      delay(wait);
  }
  strip.show();
}

void flicker() {

 for(uint16_t i=0; i<strip.numPixels(); i++) {
      accel.axes(axes);
      /*    
      Serial.print("x: ");
      Serial.print(axes[0]);
      Serial.print(", y: ");
      Serial.print(axes[1]);
      Serial.print(", z: ");
      Serial.println(axes[2]);    
      */
  
      r = abs(axes[0]) / 6;
      b = abs(axes[1]) / 6;
      g = abs(axes[2]) / 6;
  
      strip.setPixelColor(i, r, g, b);
      strip.show();
      //delay(25);
    }
}

#define SPREAD 4
void cylon() {
  static short pos = strip.numPixels() / 2;
  static boolean direction = true;
  
  // Clear the previous values
  for (short i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, 0,0,0);
  }
 
  strip.setPixelColor(pos-2, 64,0,0);
  strip.setPixelColor(pos-1, 128,0,0);
  strip.setPixelColor(pos, 255,0,0);
  strip.setPixelColor(pos+1, 128,0,0);
  strip.setPixelColor(pos+2, 64,0,0);
  strip.show();
  
  pos += direction ? 1 : -1;
  
  if (pos > 13) {
    direction = false;
  }
  if (pos < 7) {
    direction = true;
  }
  
  delay(150);
}

void sttos() {
  static byte pos = 0;
  
    // Clear the previous values
  for (short i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, 0,0,0);
  }
  
  strip.setPixelColor(midpt+pos, color);
  strip.setPixelColor(midpt-pos, color);
  
  strip.show();
  
  pos++;
  if (pos > midpt) pos=0;
  
  Serial.print("pos:");
  Serial.println(pos);
  
  delay(150);
}

