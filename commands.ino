String cmdbuf = "";

typedef void (*CommandPtrType)(String arg);

struct commandHandler {
  const char *name;
  CommandPtrType handler;
};

commandHandler commands[] = {
  {"color", cmdColor},
  {"animations", cmdAnimations},
  {"animation", cmdAnimation},
  {"defaultanimation", cmdDefaultAnimation},
  {"ring", cmdRing},
  {"fade", cmdFade},
  {NULL, NULL}
};

void processCommands() {
  char chr = 0;
  String cmd = NULL,arg=NULL;
  /*
  if (Serial.available()) // USB ACM
    chr = Serial.read();
  if (Serial1.available()) // USART: BT module
    chr = Serial1.read();
    */
  if (!AnySerial.available())
    return; // nothing to do

  //if (chr == 0) return; // nothing to do
  chr=AnySerial.read();

  if (chr == '\n' or chr=='\r') { // command complete, interpret it
    int split = cmdbuf.indexOf('=');
    
    if (split == -1) { // command with arg
      cmd = cmdbuf; // simple command
    } else {
      cmd = cmdbuf.substring(0, split); 
      arg = cmdbuf.substring(split+1);
      arg.trim();
      /*
      Serial.print("cmd:");
      Serial.print(cmd);
      Serial.print(" arg:");
      Serial.println(arg);
      */
    }
    
    cmdbuf = "";
    
    for (byte i=0; commands[i].name != NULL; i++) {
      if (cmd == commands[i].name) {
        commands[i].handler(arg);
        return;
      }
    }
    AnySerial.println("invalid command"); 
  } else { // just more input, throw it on the pile until we see a \n
    cmdbuf += chr;
  }  
}

void cmdColor(String arg) {
  if (arg == NULL) { // report current color
    AnySerial.print("color=");
    // Serial.print does not include padding 0's, FYI
    AnySerial.println(color, HEX);
    return;
  }
  color = strtol(arg.c_str(), NULL, 16); // parse it as hex
  r = (color >> 16) & 0xff;
  g = (color >> 8) & 0xff;
  b = (color >> 0) & 0xff;
  cmdColor(NULL); // report
}

void cmdAnimations(String arg) {
  AnySerial.print("animations=");
  for (byte i=0; animations[i].name != NULL; i++) {
    AnySerial.print(animations[i].name);
    AnySerial.print(' ');
  }
  AnySerial.println();
}

void cmdAnimation(String arg) {
  char *endptr;
  byte newmode;
  
  if (arg == NULL) {
    AnySerial.print("animation=");
    AnySerial.println(animations[mode].name);
    return;
  }
  // use strtol instead of atoi so we can tell if it succeeds
  newmode = strtol(arg.c_str(), &endptr, 10);
  if (endptr != arg.c_str()) { // is a number
    changeAnimation(newmode);
  } else { // is not a number, try searching the names table
    for (byte i=0; i<numAnimations; i++) {
      if (arg == animations[i].name) {
        changeAnimation(i);
        return;
      }
    }
    AnySerial.println("Invalid animation");
  }
}

void cmdDefaultAnimation(String arg) {
  if (arg == NULL) {
    AnySerial.print("defaultmode=");
    AnySerial.println(animations[mode].name[EEPROM.read(EE_DEFAULT_ANIMATION)]);
    return;
  } else {
    byte prevmode = mode;
    cmdAnimation(arg); // steal the parsing from cmdAnimation
    EEPROM.write(EE_DEFAULT_ANIMATION, mode);
    mode = prevmode;
    cmdDefaultAnimation(NULL); // Report
  }
}

void cmdRing(String arg) {
  modeOverride = 4;
  modeOverrideTimeout = millis() + 1500;
  AnySerial.println("ringing");
}

void cmdFade(String arg) {
  modeOverride = 6;
  modeOverrideTimeout = millis() + 7000;
}
