/*
This example shows the differnt features offered by the library to interacat with
the TSL2561 sensor.
This code tries to use as many features offered by the library as possible.
Not everything is this example is required to successfully operate the tsl2561 device.

Tested on Adafruit's TSL2561 breakout.

Connections:

Photon/Electron/Redbear Duo       tsl2561
D1                        <---->  SCL
D0                        <---->  SDA
Not connected (floating)  <---->  ADDR

the breakout board comes with its own pull-up resistors.
*/

#include "tsl2561.h"

// instanciate a TSL2561 object with I2C address 0x39
TSL2561 tsl(TSL2561_ADDR);

// sensor related vars
uint16_t integrationTime;
double illuminance;
uint32_t illuminance_int;
bool autoGainOn;

// execution control var
bool operational;


//status vars
char tsl_status[21] = "na";
char autoGain_s[4] = "na";
uint8_t error_code;
uint8_t gain_setting;


void setup()
{

  error_code = 0;
  operational = false;
  autoGainOn = false;

  // variables on the cloud
  Particle.variable("status", tsl_status);
  Particle.variable("integ_time", integrationTime);
  Particle.variable("gain", gain_setting);
  Particle.variable("auto_gain",autoGain_s);
  Particle.variable("illuminance", illuminance);
  Particle.variable("int_ill", illuminance_int);

  //function on the cloud: change sensor exposure settings (mqx 4)
  Particle.function("setExposure", setExposure);

  //connecting to light sensor device
  if (tsl.begin()) {
    strcpy(tsl_status,"tsl2561 found");
  }
  else {
    strcpy(tsl_status,"tsl 2561 not found ");
  }

  // setting the sensor: gain x1 and 101ms integration time
  if(!tsl.setTiming(false,1,integrationTime))
  {
    error_code = tsl.getError();
    strcpy(tsl_status,"setTimingError");
    return;
  }


  if (!tsl.setPowerUp())
  {
    error_code = tsl.getError();
    strcpy(tsl_status,"PowerUPError");
    return;
  }

  // device initialized
  operational = true;
  strcpy(tsl_status,"initOK");

}

void loop()
{
  uint16_t broadband, ir;

  // update exposure settings display vars
  if (tsl._gain)
    gain_setting = 16;
  else
    gain_setting = 1;

  if (autoGainOn)
    strcpy(autoGain_s,"yes");
  else
    strcpy(autoGain_s,"no");

  if (operational)
  {
    // device operational, update status vars
    strcpy(tsl_status,"OK");

    // get raw data from sensor
    if(!tsl.getData(broadband,ir,autoGainOn))
    {
      error_code = tsl.getError();
      strcpy(tsl_status,"saturated?");
      operational = false;
    }



    // compute illuminance value in lux
    if(!tsl.getLux(integrationTime,broadband,ir,illuminance))
    {
      error_code = tsl.getError();
      strcpy(tsl_status,"getLuxError");
      operational = false;
    }

    // try the integer based calculation
    if(!tsl.getLuxInt(broadband,ir,illuminance_int))
    {
      error_code = tsl.getError();
      strcpy(tsl_status,"getLuxIntError");
      operational = false;
    }

  }
  else
  // device not set correctly
  {
      strcpy(tsl_status,"OperationError");
      illuminance = -1.0;
      // trying a fix
      // power down the sensor
      tsl.setPowerDown();
      delay(100);
      // re-init the sensor
      if (tsl.begin())
      {
        // power up
        tsl.setPowerUp();
        // re-configure
        tsl.setTiming(tsl._gain,1,integrationTime);
        // try to go back normal again
        operational = true;
      }
  }

  delay(2000);
}

// cloud function to change exposure settings (gain and integration time)
int setExposure(String command)
//command is expected to be [gain={0,1,2},integrationTimeSwitch={0,1,2}]
// gain = 0:x1, 1: x16, 2: auto
// integrationTimeSwitch: 0: 14ms, 1: 101ms, 2:402ms

{
    // private vars
    char gainInput;
    uint8_t itSwitchInput;
    boolean _setTimingReturn = false;

    // extract gain as char and integrationTime swithc as byte
    gainInput = command.charAt(0);//we expect 0, 1 or 2
    itSwitchInput = command.charAt(2) - '0';//we expect 0,1 or 2

    if (itSwitchInput >= 0 && itSwitchInput < 3){
      // acceptable integration time value, now check gain value
      if (gainInput=='0'){
        _setTimingReturn = tsl.setTiming(false,itSwitchInput,integrationTime);
        autoGainOn = false;
      }
      else if (gainInput=='1') {
        _setTimingReturn = tsl.setTiming(true,itSwitchInput,integrationTime);
        autoGainOn = false;
      }
      else if (gainInput=='2') {
        autoGainOn = true;
        // when auto gain is enabled, set starting gain to x16
        _setTimingReturn = tsl.setTiming(true,itSwitchInput,integrationTime);
      }
      else{
        // no valid settings, raise error flag
        _setTimingReturn = false;
      }
    }
    else{
      _setTimingReturn = false;
    }

    // setTiming has an error
    if(!_setTimingReturn){
        // set appropriate status variables
        error_code = tsl.getError();
        strcpy(tsl_status,"CloudSettingsError");
        //disable getting illuminance value
        operational = false;
        return -1;
    }
    else {
      // all is good
      operational = true;
      return 0;
    }
}
