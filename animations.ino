
unsigned long nextMove = millis();
#define NEXTMOVE(time) nextMove = millis() + time

typedef void (*AnimationPtrType)(void);

struct animationHandler {
  const char *name;
  AnimationPtrType handler;
};

animationHandler animations[] = {
  {"flicker", flicker},
  {"cylon", cylon},
  {"sttos", sttos},
  {"christmas", christmas},
  {"ring", ring},
  {"preview", preview},
  {"fade", fade},
  {"twinkle", twinkle},
  {"rainbow", rainbow},
  {"police", police},
  {NULL, NULL}
};
byte numAnimations = (sizeof(animations)/sizeof(animationHandler)) - 1;

/*
enum {
  FLICKER = 0,
  CYLON = 1, 
  STTOS = 2,
  CHRISTMAS = 3,
  RING = 4,
  PREVIEW = 5,
  MAXMODE = 6
};
String modes[] = {
  "flicker",
  "cylon",
  "sttos",
  "christmas",
  "ring",
  "preview"
};
*/

void animate(byte anim) {
  // Don't process animations until there is something to do
  if (millis() > nextMove) {
    if (anim < numAnimations) {
    animations[anim].handler();
    } else {
      AnySerial.print("Unknown animation mode:");
      AnySerial.println(anim);
      NEXTMOVE(5000);
    }
  }
}

void changeAnimation(byte newmode) {
  if (newmode >= numAnimations) {
    mode = 0;  // wrap around
  } else {
    mode = newmode;
  }
  cmdAnimation(NULL); // Use the reporting code from cmdAnimation
  // black out the strip so the new animation starts clean
  for (byte i=0;i<strip.numPixels();i++) {
    strip.setPixelColor(i,0);
  }
  NEXTMOVE(0); // start the new animation immediatly
}

void ring() {
  static byte off = 0;
  
  for (byte i=off; i<strip.numPixels(); i += 2) {
    strip.setPixelColor(i, 0xff0000);
  }
  for (byte i=off+1; i<strip.numPixels(); i += 2) {
    strip.setPixelColor(i, 0xffffff);
  }
  strip.show();
  off = off ? 0 : 1; // flip the offset back and forth
  
  NEXTMOVE(35);
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
    }
    NEXTMOVE(25);
}

#define SPREAD 4
void cylon() {
  static uint8_t pos = strip.numPixels() / 2;
  static boolean direction = true;
  
  // Clear the previous values
  for (byte i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, 0,0,0);
  }
 
  strip.setPixelColor(pos-2, 64,0,0);
  strip.setPixelColor(pos-1, 128,0,0);
  strip.setPixelColor(pos, 255,0,0);
  strip.setPixelColor(pos+1, 128,0,0);
  strip.setPixelColor(pos+2, 64,0,0);
  strip.show();
  
  pos += direction ? 1 : -1;
  
  if (pos > strip.numPixels()-5) {
    direction = false;
  }
  if (pos < 7) {
    direction = true;
  }
  
  NEXTMOVE(150);
}

void sttos() {
  static byte pos = 0;
  
    // Clear the previous values
  for (byte i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, 0,0,0);
  }
  
  strip.setPixelColor(midpt+pos, color);
  strip.setPixelColor(midpt-pos, color);
  
  strip.show();
  
  pos++;
  if (pos > midpt) pos=0;

  NEXTMOVE(150);
}

// meant as an override, just dim everything (10/255) per tick
void fade() {
  dimall(10);
  strip.show();
  NEXTMOVE(500);
}

// Just show the color on all pixels for picking the right one.
void preview() {
  for (byte i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, color);
  }
  strip.show();
  NEXTMOVE(250);
  
  // dump accelerometer data
  accel.axes(axes);
  AnySerial.println("XYZ:");
  AnySerial.println(axes[0]);
  AnySerial.println(axes[1]);
  AnySerial.println(axes[2]);
  /*
  Serial.print(sqrt(axes[0]*axes[0] + axes[1]*axes[1]));
  Serial.print(" ");
  Serial.println(atan2(axes[0], axes[1]) * RAD_TO_DEG);
  */
}

void christmas() {
  static byte off = 0;
  for (byte i=off; i<strip.numPixels(); i += 2) {
    strip.setPixelColor(i, 0xff0000);
  }
  for (byte i=off+1; i<strip.numPixels(); i += 2) {
    strip.setPixelColor(i, 0x00ff00);
  }
  strip.show();
  
  off = off ? 0 : 1; // flip the offset back and forth

  NEXTMOVE(750);
}

#define NUM_TWINKLES 8
#define TWINKLE_STEPS 16
struct twinkle_ {
  short position = -1;
  byte step = 0;
  char dir = 1;
};

void twinkle() {
  static twinkle_ twinkles[NUM_TWINKLES];
  
  for (byte i=0; i < NUM_TWINKLES; i++ ) {
    if (twinkles[i].step == 0) {
      // black out this pixel
      strip.setPixelColor(twinkles[i].position, 0);
      // pick a new one to twinkle
      twinkles[i].position = random(0,strip.numPixels());
      // dir==1 is get brigher
      twinkles[i].dir = 1;
      twinkles[i].step = random(0,TWINKLE_STEPS/4); // stagger starting positions
    }
    byte scale = twinkles[i].step * ((256/TWINKLE_STEPS)-1);
    
    strip.setPixelColor(twinkles[i].position, highByte(r*scale),highByte(g*scale),highByte(b*scale));
    
    twinkles[i].step += twinkles[i].dir;
    if (twinkles[i].step >= TWINKLE_STEPS) {
      twinkles[i].dir = -1;
    }
    
  }
  strip.show();
  NEXTMOVE(50);
}

void rainbow() {
  uint16_t i;
  static uint8_t j = 0;

  for(i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, Wheel((i+j) & 255));
  }
  strip.show();
  j++;
  if (j>254) j=0;
  NEXTMOVE(20);

}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  if(WheelPos < 85) {
   return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  } else if(WheelPos < 170) {
   WheelPos -= 85;
   return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  } else {
   WheelPos -= 170;
   return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
}

// helper function for other animations
// fades the current color values by a certain ammount
// Pointer math, yay!
void dimall(byte dim){
  uint8_t *pixels = strip.getPixels();
  uint8_t c;
  uint16_t numBytes = strip.numPixels() * 3;
  uint16_t scale = 255-dim;

  // stolen from Adafruit_NeoPixel::setBrightness(uint8_t b)
  for(uint16_t i=0; i<numBytes; i++) {
    c      = *pixels;
    *pixels++ = (c * scale) >> 8;
  }
}

#define POLICE_SPIN_RATE 20*DEG_TO_RAD
void police() {
  static float angle;
  float pixelToRad = (2*PI) / strip.numPixels();
  
  for(byte i=0; i<strip.numPixels(); i++) {
    float pixelAngle = angle+(pixelToRad*i);
    strip.setPixelColor(i, 127+127*sin(pixelAngle), 0, 127+127*sin(-pixelAngle));
  }
  strip.show();
  angle += POLICE_SPIN_RATE;
  if (angle > (PI*2)) {
    angle -= PI*2;
  }
  NEXTMOVE(10);
}
