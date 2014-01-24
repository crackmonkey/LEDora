String cmdbuf = "";

typedef void (*CommandPtrType)(String arg);

struct commandHandler {
  const char *name;
  CommandPtrType handler;
};

commandHandler commands[] = {
  {"color", cmdColor},
  {"brightness", cmdBrightness},
  {"animations", cmdAnimations},
  {"animation", cmdAnimation},
  {"defaultanimation", cmdDefaultAnimation},
  {"ring", cmdRing},
//  {"fade", cmdFade},
  {"rn52", cmdRn52},
  {NULL, NULL}
};

void processCommands() {
  char chr = 0;
  String cmd = NULL,arg=NULL;

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
    if (cmd.length() == 0) return; // empty command, ignore it
    AnySerial.println("invalid command:"+cmd); 
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

void cmdBrightness(String arg) {
  if (arg == NULL) { // report current color
    AnySerial.print("brightness=");
    // Serial.print does not include padding 0's, FYI
    AnySerial.println(brightness, HEX);
    return;
  }
  brightness = strtol(arg.c_str(), NULL, 16); // parse it as hex
  strip.setBrightness(brightness);
  cmdBrightness(NULL); // report
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
    AnySerial.print("defaultanimation=");
    AnySerial.println(animations[EEPROM.read(EE_DEFAULT_ANIMATION)].name);
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
  modeOverride = 9; // hard coded index in the animations[] array
  modeOverrideTimeout = millis() + 1500;
  
  if (arg.length() == 0) {
    // default ring colors
    ringColors[0] = 0xff0000;
    ringColors[1] = 0xffffff;
  } else {
    char *next;
    ringColors[0] = strtol(arg.c_str(), &next, 16);
    if (next != NULL && *next == ',') {
      // second color specified
      ringColors[1] = strtol(next+1, NULL, 16);
    } else {
      ringColors[1] = 0xffffff; // default to white
    }
  }

  AnySerial.println("ringing");
}

void cmdFade(String arg) {
  modeOverride = 8; // hard coded index in the animations[] array
  modeOverrideTimeout = millis() + 7000;
}

// Send a command to the RN52 module
void cmdRn52(String arg) {
  btmodule.sendCommand(arg);
}

