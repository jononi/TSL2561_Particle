TSL2561 light sensor with I2C library for Particle IoT development boards.

Tested on Particle Photon and Adafruit breakout board: http://www.adafruit.com/products/439
Hookup guide: pretty straightforward but you can follow this guide by Adafruit: https://learn.adafruit.com/tsl2561?view=all (For Arduino, but the steps are the same, including the use of 3.3v supply voltage for the sensor)

This library re-uses code developed orginally by Sparkfun Electronics and Adafruit in their respective libraries to operate this device on Arduino. The merit goes all to them for making this code available.
I have also made the following contributions/modification on this library:

1- modified SparkFun's TSL2561 arduino library to work on Photon
2- changed the setTiming function for a simpler version and begin(void) to adapt to the new class public variables 
3- added the autogain feature implemented in Adafruit's Arduino library in the getData function (with modifications)
4- added Adafruit's integer based illuminance value (lux) calculating (pretty close to the float based calculation
5- adopted Adafruit begin(void) function to check that the  device is connected by retrieving and checking the device ID
