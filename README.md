# TSL2561 light sensor
_TSL2561 light sensor with I2C interface library for Particle IoT development boards_

## Typical usage

Connect a Photon/Electron/ReadBear Duo/Bluz DK to a TSL2561 breakout (Adafruit's or Sparkfun's for instance) and use the library to get the ambient visible and/or IR light levels + illuminance value in lux.

Hookup guide: pretty straightforward but you can follow this guide by Adafruit: https://learn.adafruit.com/tsl2561?view=all 
(It's using an Arduino, but the steps are the same, including the use of 3.3v supply voltage for the sensor)

Photon/Electron                  tsl2561
D1                        <---->  SCL
D0                        <---->  SDA
Not connected (floating)  <---->  ADDR

Example of usage:
```
#include "tsl2561.h"

// declare a TSL2561 object with I2C address 0x39 (ADDR pin floating)
TSL2561 tsl(TSL2561_ADDR);

// Light sensor settings vars
uint16_t integrationTime;
bool autoGainOn = True;

// define a frequency of readings every x ms
sensePeriod = 1000;

void setup() {
    tsl.begin();
    tsl.setPowerUp();
    // x1 gain, 101ms integration time
    tsl.setTiming(false,1,integrationTime);

    // set illuminance as a cloud variable
    Particle.variable("illuminance", illuminance);

    // initialise a timer
    senseTime = millis();
}

void loop() {

  if ((millis() - senseTime) > sensePeriod) {

    // get raw data
    uint16_t _broadband, _ir;
    if (tsl.getData(_broadband,_ir,autoGainOn)){

      // now get the illuminance value in lux
      tsl.getLux(integrationTime,_broadband,_ir,illuminance);      
    }
  }
}
```

Tested on Particle Photon and Adafruit breakout board: http://www.adafruit.com/products/439

This library re-uses code developed orginally by Sparkfun Electronics and Adafruit in their respective libraries to operate this device on Arduino. The merit goes all to them for making this code available.
I have also made the following contributions/modification on this library:

1- modified SparkFun's TSL2561 arduino library to work on Photon

2- changed the setTiming function for a simpler version and begin(void) to adapt to the new class public variables

3- added the autogain feature implemented in Adafruit's Arduino library in the getData function (with modifications)

4- added Adafruit's integer based illuminance value (lux) calculating (pretty close to the float based calculation

5- adopted Adafruit begin(void) function to check that the  device is connected by retrieving and checking the device ID


## Example

[**Read illuminance over Particle's cloud :**](examples/demo_tsl2561/demo_tsl2561.ino)
This example shows most of the features of this library.

Cloud Function:

`setExposure(int gain, int integrationTimeSwitch)`

gain:

0: gain x1, 1: gain x16, 2: auto gain

integrationTimeSwitch

0: 14ms, 1: 101ms, 2: 402ms

Cloud Variables:

status: OK or error code

integ_time: integration time in ms

gain: gain setting: x1 or x16

auto_gain: auto gain enabled: Yes/No

illuminance: illuminance value in lux as a double

int_ill: illuminance value in lux as an Integer

## Reference

###`TSL2561(uint8_t i2c_address);`

 Declare a TSL2561 object
 
 Parameters:
 
 TSL2561_ADDR_0 (0x29 address with '0', connected to GND)
 
 TSL2561_ADDR   (0x39 default address, pin floating)
 
 TSL2561_ADDR_1 (0x49 address with '1' connected to VIN)
 

###`boolean begin(void);`

Initialize TSL2561 library

Returns:

True if the device is detected


###`boolean setPowerUp(void);`

Turn on TSL2561, begin integration

Returns:

True(1) if successful, False (0) if there was an I2C error
(Also see getError() below)


###`boolean setPowerDown(void);`

 Turn off TSL2561
 
 Returns:
 
  True (1) if successful, False (0) if there was an I2C error
 (Also see getError() below)
 

###`boolean setTiming(boolean gain, uint8_t time);`

  Configure sensor's settings
  
  Parameters:
  
    If gain = False (0), device is set to low gain (1X)
    
    If gain = True (1), device is set to high gain (16X)
    
    If time = 0, integration will be 13.7ms
    
    If time = 1, integration will be 101ms
    
    If time = 2, integration will be 402ms
    
    If time = 3, use manual start / stop
    
 Returns true (1) if successful, false (0) if there was an I2C error
 (Also see getError() below)

###`boolean setTiming(boolean gain, uint8_t time, uint16_t &ms);`

 Identical to above command, except ms is set to selected integration time
 
 Parameters:
 
   If gain = False (0), device is set to low gain (1X)
   
   If gain = True (1), device is set to high gain (16X)
   
   If time = 0, integration will be 13.7ms
   
   If time = 1, integration will be 101ms
   
   If time = 2, integration will be 402ms
   
   If time = 3, use manual start / stop (ms = 0)
   
   ms will be set to requested integration time
   
 Returns True (1) if successful, False (0) if there was an I2C error
 (Also see getError() below)


###`boolean manualStart(void);`

 Starts a manual integration period
 
 After running this command, you must manually stop integration with manualStop()
 
 Internally sets integration time to 3 for manual integration (gain is unchanged)
 
 Returns True (1) if successful, False (0) if there was an I2C error
 (Also see getError() below)

###`boolean manualStop(void);`

 Stops a manual integration period
 
 Returns true (1) if successful, false (0) if there was an I2C error
 (Also see getError() below)

###`boolean getData(uint16_t &CH0, uint16_t &CH1, bool autoGain);`

 Retrieve raw integration results
 
 Parameters:
 
  If autoGain is True, auto gain is enabled and the gain will be adjusted according to brightness
  
 Returns:
 
  CH0 and CH1 will be set to integration results
  
  Returns True (1) if successful, false (0) if there was an I2C error
  (Also see getError() below)

###`boolean getLux(uint16_t ms, uint16_t CH0, uint16_t CH1, double &lux);`

 Convert raw data to illuminance value in lux
 
 Parameters;
 
  ms: integration time in ms, from setTiming() or from manual integration
  
  CH0, CH1: results from getData()
  
  lux will be set to resulting lux calculation
  
 Returns True (1) if calculation was successful
 
 RETURNS False (0) AND lux = 0.0 if either sensor (visible and IR) was saturated (0XFFFF)

###`boolean getLuxInt(uint16_t CH0, uint16_t CH1, uint32_t &lux);`

 Convert raw data to lux as integer
 
 This is not available for custom integration time
 
 This function uses integer based approximate calculation but accuracy compared
 
 To getLux:float is very good (max 2%, probably smaller than sensor accuracy)
 
 CH0, CH1: results from getData()
 
 lux will be set to illuminance value in lux
 
 Returns True (1) if calculation was successful
 or False (0) AND lux = 0 if either sensor was saturated(0XFFFF)

###`boolean setInterruptControl(uint8_t control, uint8_t persist);`

 Sets up interrupt operations
 
 Parameters:
 
   If control = 0, interrupt output disabled
   
   If control = 1, use level interrupt, see setInterruptThreshold()
   
   If persist = 0, every integration cycle generates an interrupt
   
   If persist = 1, any value outside of threshold generates an interrupt
   
   If persist = 2 to 15, value must be outside of threshold for 2 to 15 integration cycles
   
 Returns True (1) if successful, False (0) if there was an I2C error
 (Also see getError() below)
 

###`boolean setInterruptThreshold(uint16_t low, uint16_t high);`

 Set interrupt thresholds (channel 0 only)
 
 Parameters:
 
 low, high: 16-bit threshold values
 
 Returns True (1) if successful, False (0) if there was an I2C error
 (Also see getError() below)


###`boolean clearInterrupt(void);`

 Clears an active interrupt
 
 Returns True (1) if successful, False (0) if there was an I2C error
 (Also see getError() below)


###`boolean getID(uint8_t &ID);`

 Retrieves part and revision code from TSL2561
 
 Sets ID to part ID (see datasheet)
 
 Returns True (1) if successful, False (0) if there was an I2C error
 (Also see getError() below)


###`uint8_t getError(void);`

 If any library command fails, you can retrieve an error code using this function.
 Codes are from the wire library.
 
 Returns:
 
   0 = Success
   
   1 = Data too long to fit in transmit buffer
   
   2 = Received NACK on transmit of address
   
   3 = Received NACK on transmit of data
   
   4 = Other error
