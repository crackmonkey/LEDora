LEDora, the sexiest, blinkiest hat ever concieved by man kind
==============
This is the Arduino sketch that drives the LEDora, and it's NeoPixels, and accelerometer, and capacitive touch sensors, and Bluetooth module.

The code for the Android app will be in a separate repository once it's in a less-humiliating state.

This depends on:
- the AdaFruit_NeoPixel library (https://github.com/adafruit/Adafruit_NeoPixel)
- a slightly tweaked version of the I2Cdev.com MPR121 library (http://www.i2cdevlib.com/devices/mpr121)
- a hacked-together MMA8452Q library I swiped from a GIST and tweaked (https://github.com/crackmonkey/arduinolibs/tree/master/MMA8452Q)
- My own AnySerial & rn52 libraries (https://github.com/crackmonkey/arduinolibs)
- and the stock Arduino Wire & EEPROM libraries
