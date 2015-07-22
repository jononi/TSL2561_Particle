// This is the local version to be used with Particle Dev

#include "tsl2561.h"


/* first step is to get the sensor working and reading fine
next step is to take goodies from Adafruit lib and add it
specifically:
- better constrcutor that check ID to ensure connection ok:done
- auto-gain feature: done
- fix the setTiming() mess: done
- make illuminance var an Int instead of Double:done

*/

TSL2561 tsl;

// sensor related vars
//boolean gain;
uint16_t integrationTime;
double il;
uint32_t il_int;
bool autoGainOn;

// execution control var
boolean operational;


//status vars
char tsl_sta[11] = "undef";
char error_label[21] = "NA";
char AutoGainDisplay[4] = "";
unsigned int error_code;
unsigned int gainDisplay;
unsigned int integrationTimeDisplay;

//debug vars
//unsigned int int_debug;

void setup()
{

  error_code =0;
  operational = false;
  autoGainOn = false;

  // variables on the cloud (max 10)
  //Spark.variable("errorLabel",&error_label,STRING);
  Spark.variable("integ_time",&integrationTimeDisplay,INT);
  Spark.variable("gain",&gainDisplay,INT);
  Spark.variable("auto_gain",&AutoGainDisplay,STRING);
  Spark.variable("illuminance", &il, DOUBLE);
  Spark.variable("int_ill", &il_int, INT);

  //function on the cloud: change sensor exposure settings (mqx 4)
  Spark.function("setExposure", setExposure);

  //connecting to light sensor device
  if (tsl.begin(TSL2561_ADDR)) {
    strcpy(tsl_sta,"found");
  }
  else {
    strcpy(tsl_sta,"not found");
    return;
  }

  // setting the sensor: gain x1 and 101ms integration time
  if(!tsl.setTiming(false,1,integrationTime))
  {
    error_code = tsl.getError() & 0x000000FF;
    strcpy(error_label,"setTiming");
    return;
  }


  if (!tsl.setPowerUp())
  {
    error_code = tsl.getError() & 0x000000FF;
    strcpy(error_label,"PowerUP");
    return;
  }

  // device initialized
  operational = true;
  strcpy(tsl_sta,"initOK");

}

void loop()
{
  uint16_t broadband, ir;

  // update exposure settings display vars
  if (tsl._gain)
    gainDisplay = 16;
  else
    gainDisplay = 1;

  if (autoGainOn)
    strcpy(AutoGainDisplay,"yes");
  else
    strcpy(AutoGainDisplay,"no");

  integrationTimeDisplay = integrationTime & 0x0000FFFF;

  if (operational)
  {
    // device operational, update status vars
    strcpy(error_label,"NA");
    strcpy(tsl_sta,"Ok");

    // get raw data from sensor
    if(!tsl.getData(broadband,ir,autoGainOn))
    {
      error_code = tsl.getError() & 0x000000FF;
      strcpy(error_label,"getData");
      operational = false;
    }

    // compute illuminance value in lux
    if(!tsl.getLux(integrationTime,broadband,ir,il))
    {
      error_code = tsl.getError() & 0x000000FF;
      strcpy(error_label,"getLux");
      operational = false;
    }

    // try the int based calculation
    if(!tsl.getLuxInt(broadband,ir,il_int))
    {
      error_code = tsl.getError() & 0x000000FF;
      strcpy(error_label,"getLuxInt");
      operational = false;
    }

  }
  else
  // device not set correctly
  {
      strcpy(tsl_sta,"Problem");
      il = -1.0;
      // trying a fix
      // power down the sensor
      tsl.setPowerDown();
      delay(100);
      // re-init the sensor
      if (tsl.begin(TSL2561_ADDR))
      {
        // power up
        tsl.setPowerUp();
        // re-configure
        tsl.setTiming(tsl._gain,1,integrationTime);
        // try to go back normal again
        operational = true;
      }
  }

  delay(1000);
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
        error_code = tsl.getError() & 0x000000FF;
        strcpy(error_label,"setTimingInCloud");
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
