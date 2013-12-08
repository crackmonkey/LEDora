
unsigned long nextMove = millis();
#define NEXTMOVE(time) nextMove = millis() + time

typedef void (*AnimationPtrType)(void);

struct animationHandler {
  const char *name;
  AnimationPtrType handler;
};

#define ANIMATIONS 6
animationHandler animations[] = {
  {"flicker", flicker},
  {"cylon", cylon},
  {"sttos", sttos},
  {"christmas", christmas},
  {"ring", ring},
  {"preview", preview},
  {NULL, NULL}
};

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
    if (anim < ANIMATIONS) {
    animations[anim].handler();
    } else {
      Serial.print("Unknown animation mode:");
      Serial.println(anim);
      NEXTMOVE(5000);
    }
  }
}

void changeMode(byte newmode) {

  if (newmode >= ANIMATIONS) {
    mode = 0;  // wrap around
  } else {
    mode = newmode;
  }
  cmdAnimation(NULL); // Use the reporting code from cmdAnimation
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
  static short pos = strip.numPixels() / 2;
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
  
  if (pos > 13) {
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
  
  Serial.print("pos:");
  Serial.println(pos);
  
  NEXTMOVE(150);
}

// Just show the color on all pixels for picking the right one.
void preview() {
  for (byte i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, color);
  }
  strip.show();
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
