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
  {NULL, NULL}
};

void processCommands() {
  char chr = 0;
  String cmd = NULL,arg=NULL;
  if (Serial.available()) // USB ACM
    chr = Serial.read();
  if (Serial1.available()) // USART
    chr = Serial1.read();

  if (chr == 0) return; // nothing to do

  if (chr == '\n') { // command complete, interpret it
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
    Serial.println("invalid command"); 
  } else { // just more input, throw it on the pile until we see a \n
    cmdbuf += chr;
  }  
}

void cmdColor(String arg) {
  if (arg == NULL) { // report current color
    Serial.print("color=");
    // Serial.print does not include padding 0's, FYI
    Serial.println(color, HEX);
    return;
  }
  color = strtol(arg.c_str(), NULL, 16); // parse it as hex
  cmdColor(NULL); // report
}

void cmdAnimations(String arg) {
  Serial.print("animations=");
  for (byte i=0; animations[i].name != NULL; i++) {
    Serial.print(animations[i].name);
    Serial.print(' ');
  }
  Serial.println();
}

void cmdAnimation(String arg) {
  char *endptr;
  byte newmode;
  
  if (arg == NULL) {
    Serial.print("animation=");
    Serial.println(animations[mode].name);
    return;
  }
  // use strtol instead of atoi so we can tell if it succeeds
  newmode = strtol(arg.c_str(), &endptr, 10);
  if (endptr != arg.c_str()) { // is a number
    mode = newmode;
  } else { // is not a number, try searching the names table
    for (byte i=0; i<ANIMATIONS; i++) {
      if (arg == animations[i].name) {
        mode = i;
        cmdAnimation(NULL); // report
        return;
      }
    }
    Serial.println("Invalid animation");
  }
}

void cmdDefaultAnimation(String arg) {
  if (arg == NULL) {
    Serial.print("defaultmode=");
    Serial.println(animations[mode].name[EEPROM.read(DEFAULTMODE)]);
    return;
  } else {
    byte prevmode = mode;
    cmdAnimation(arg); // steal the parsing from cmdAnimation
    EEPROM.write(DEFAULTMODE, mode);
    mode = prevmode;
    cmdDefaultAnimation(NULL); // Report
  }
}

void cmdRing(String arg) {
  modeOverride = 4;
  modeOverrideTimeout = millis() + 1500;
  Serial.println("ringing");
}
