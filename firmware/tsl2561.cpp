/*
	This library provides functions to access the TAOS TSL2561 Illumination Sensor.
	This is a library port for Particle Photon platform based on original contribution
	by SparkFun Electronics and Adafruit.

	Jaafar BenAbdallah, July 2015
	contributions:
	1- modified SparkFun's TSL2561 arduino library to work on Photon
	2- added autogain feature implemented in Adafruit's Arduino library in the getData function
	3- added Adafruit's integer based illuminance value (lux) calculating
	4- adopted Adafruit begin(void) function to check device is connected using returned  ID

	Originally developed by SparkFun and Adafruit:

	Mike Grusin, SparkFun Electronics version 1.0 2013/09/20
	K. Townsend, microBuilder.eu / adafruit.com version 1.0 2010
*/

#include "tsl2561.h"
#include "math.h" //to use pow(x,a)


TSL2561::TSL2561(uint8_t i2c_address){
	_i2c_address = i2c_address;
	_gain = false; //default gain x1
	_it = 1; // default integration time = 101 ms
}


boolean TSL2561::begin(void)
{
	uint8_t ID;
	// start I2C
	Wire.begin();
	// read device ID
	if (readByte(TSL2561_REG_ID,ID) && ID==0x50)
	{
		return true;
	}
	else
	{
		return false;
	}
}


boolean TSL2561::setPowerUp(void)
{
	// Write 0x03 to command byte (power on)
	return(writeByte(TSL2561_REG_CONTROL,0x03));
}


boolean TSL2561::setPowerDown(void)
	// Turn off TSL2561
	// Returns true (1) if successful, false (0) if there was an I2C error
	// (Also see getError() below)
{
	// Clear command byte (power off)
	return(writeByte(TSL2561_REG_CONTROL,0x00));
}


boolean TSL2561::setTiming(boolean gain, uint8_t it_switch, uint16_t &ms)
	// If gain = false (0), device is set to low gain (1X)
	// If gain = high (1), device is set to high gain (16X)
	// If time = 0, integration will be 13.7ms
	// If time = 1, integration will be 101ms
	// If time = 2, integration will be 402ms
	// If time = 3, use manual start / stop
	// ms will be set to integration time in ms
	// Returns true (1) if successful, false (0) if there was an I2C error
	// (Also see getError() below)
{
	uint8_t timing;

	// define integration time in ms for user
	switch (it_switch)
	{
		case 0: ms = 14; break;
		case 1: ms = 101; break;
		case 2: ms = 402; break;
		default: ms = 0;
	}

	// Get timing byte
	if (readByte(TSL2561_REG_TIMING,timing))
	{
		// Set gain (0 or 1)
		if (gain)
			timing |= 0x10;
		else
			timing &= ~0x10;

		// Set integration time (0 to 3)
		timing &= ~0x03;
		timing |= (it_switch & 0x03);

		// Write modified timing byte back to device
		if (writeByte(TSL2561_REG_TIMING,timing))
		{
			// update settings
			_gain = gain;
			_it = it_switch;
			return(true);
		}

	}
	return(false);
}

boolean TSL2561::manualStart(void)
	// Starts a manual integration period
	// After running this command, you must manually stop integration with manualStop()
	// Internally sets integration time to 3 for manual integration (gain is unchanged)
	// Returns true (1) if successful, false (0) if there was an I2C error
	// (Also see getError() below)
{
	uint8_t timing;

	// Get timing byte
	if (readByte(TSL2561_REG_TIMING,timing))
	{
		// Set integration time to 3 (manual integration)
		timing |= 0x03;

		if (writeByte(TSL2561_REG_TIMING,timing))
		{
			// Begin manual integration
			timing |= 0x08;

			// Write modified timing byte back to device
			if (writeByte(TSL2561_REG_TIMING,timing))
				return(true);
		}
	}
	return(false);
}


boolean TSL2561::manualStop(void)
	// Stops a manual integration period
	// Returns true (1) if successful, false (0) if there was an I2C error
	// (Also see getError() below)
{
	uint8_t timing;

	// Get timing byte
	if (readByte(TSL2561_REG_TIMING,timing))
	{
		// Stop manual integration
		timing &= ~0x08;

		// Write modified timing byte back to device
		if (writeByte(TSL2561_REG_TIMING,timing))
			return(true);
	}
	return(false);
}


boolean TSL2561::getData(uint16_t &data0, uint16_t &data1, bool autoGain)
	// Retrieve raw integration results
	// data0 and data1 will be set to integration results
	// Returns true (1) if successful, false (0) if there was an I2C error
	// (Also see getError() below)
{
	uint16_t it_ms;
	// Get data0 and data1 out of result registers, if error: exit
	if (!readUInt(TSL2561_REG_DATA_0,data0) || !readUInt(TSL2561_REG_DATA_1,data1))
		return false;

	if (!autoGain)
		return true;// auto gain disabled, just return raw data
	else
	{
		uint16_t _hi, _lo;
		// Get the hi/low threshold for the current integration time
		switch(_it)
		{
			case 0:
			_hi = TSL2561_AGC_THI_13MS;
			_lo = TSL2561_AGC_TLO_13MS;
			break;
			case 1:
			_hi = TSL2561_AGC_THI_101MS;
			_lo = TSL2561_AGC_TLO_101MS;
			break;
			default:
			_hi = TSL2561_AGC_THI_402MS;
			_lo = TSL2561_AGC_TLO_402MS;
			break;
		}

		// check if gain needs to be adjusted
		if ((data0 < _lo) && !_gain)
		// broadband value below low_thr and gain = x1
		{
			/* Increase the gain and try again */
			setTiming(true, _it, it_ms);
			/* update data, if error, exit right away */
			if (readUInt(TSL2561_REG_DATA_0,data0) && readUInt(TSL2561_REG_DATA_1,data1))
				return true;//gain adjusted, new values read: done!
			else
				return false;
		}
		else if ((data0 > _hi) && _gain)
		// broadband value above high_thr and gain = x16
		{
			/* Drop gain to 1x and try again */
			setTiming(false, _it, it_ms);
			/* update data, if error, exit right away */
			if (readUInt(TSL2561_REG_DATA_0,data0) && readUInt(TSL2561_REG_DATA_1,data1))
				return true;//gain adjusted, new values read: done!
			else
				return false;
		}
		else
		{
			/*Reading is either valid, or we're already at the chips limits */
			return true;
		}
	}
}


boolean TSL2561::getLux(uint16_t ms, uint16_t CH0, uint16_t CH1, double &lux)
	// Convert raw data to lux
	// ms: integration time in ms, from setTiming() or from manual integration
	// CH0, CH1: results from getData()
	// lux will be set to resulting lux calculation
	// returns true (1) if calculation was successful
	// RETURNS false (0) AND lux = 0.0 IF EITHER SENSOR WAS SATURATED (0XFFFF)
{
	double ratio, d0, d1;

	// Determine if either sensor saturated (0xFFFF)
	// If so, abandon ship (calculation will not be accurate)
	if ((CH0 == 0xFFFF) || (CH1 == 0xFFFF))
	{
		lux = 0.0;
		return(false);
	}

	// Convert from unsigned integer to floating point
	d0 = CH0; d1 = CH1;

	// We will need the ratio for subsequent calculations
	ratio = d1 / d0;

	// Normalize for integration time
	d0 *= (402.0/ms);
	d1 *= (402.0/ms);

	// Normalize for gain
	if (!_gain)
	{
		d0 *= 16;
		d1 *= 16;
	}

	// Determine lux per datasheet equations:

	if (ratio < 0.5)
	{
		lux = 0.0304 * d0 - 0.062 * d0 * pow(ratio,1.4);
		return(true);
	}

	if (ratio < 0.61)
	{
		lux = 0.0224 * d0 - 0.031 * d1;
		return(true);
	}

	if (ratio < 0.80)
	{
		lux = 0.0128 * d0 - 0.0153 * d1;
		return(true);
	}

	if (ratio < 1.30)
	{
		lux = 0.00146 * d0 - 0.00112 * d1;
		return(true);
	}

	// if (ratio > 1.30)
	lux = 0.0;
	return(true);
}

// alternate int based illuminance calculation
boolean TSL2561::getLuxInt(uint16_t CH0, uint16_t CH1, uint32_t &lux)
// Convert raw data to lux as integer
// this is not available for custom integration time
// this function uses integer based approximate calculation but accuracy compared...
// ...to getLux:float is very good (max 2%, probably smaller than sensor accuracy)
// CH0, CH1: results from getData()
// lux will be set to illuminance value in lux
// returns true (1) if calculation was successful
// RETURNS false (0) AND lux = 0 IF EITHER SENSOR WAS SATURATED (0XFFFF)
{
  unsigned long chScale;
  unsigned long channel1;
  unsigned long channel0;

  /* Make sure the sensor isn't saturated! */
  uint16_t clipThreshold;
  switch (_it)
  {
    case 0:
      clipThreshold = TSL2561_CLIPPING_13MS;
      break;
    case 1:
      clipThreshold = TSL2561_CLIPPING_101MS;
      break;
    default:
      clipThreshold = TSL2561_CLIPPING_402MS;
      break;
  }

  /* Return false and lux=0 if the sensor is saturated */
  if ((CH0 > clipThreshold) || (CH1> clipThreshold))
  {
		lux = 0;
    return false;
  }

  /* Get the correct scale depending on the intergration time */

  switch (_it)
  {
    case 0:
      chScale = TSL2561_LUX_CHSCALE_TINT0;
      break;
    case 1:
      chScale = TSL2561_LUX_CHSCALE_TINT1;
      break;
    default: // No scaling ... integration time = 402ms
      chScale = (1 << TSL2561_LUX_CHSCALE);
      break;
  }

  // Scale for gain (1x or 16x)
  if (!_gain) chScale = chScale << 4;

  // scale the channel values
  channel0 = (CH0 * chScale) >> TSL2561_LUX_CHSCALE;
  channel1 = (CH1 * chScale) >> TSL2561_LUX_CHSCALE;

  // find the ratio of the channel values (Channel1/Channel0)
  unsigned long ratio1 = 0;
  if (channel0 != 0) ratio1 = (channel1 << (TSL2561_LUX_RATIOSCALE+1)) / channel0;

  // round the ratio value
  unsigned long ratio = (ratio1 + 1) >> 1;

  unsigned int b, m;


  if ((ratio >= 0) && (ratio <= TSL2561_LUX_K1T))
  {b=TSL2561_LUX_B1T; m=TSL2561_LUX_M1T;}
  else if (ratio <= TSL2561_LUX_K2T)
  {b=TSL2561_LUX_B2T; m=TSL2561_LUX_M2T;}
  else if (ratio <= TSL2561_LUX_K3T)
  {b=TSL2561_LUX_B3T; m=TSL2561_LUX_M3T;}
  else if (ratio <= TSL2561_LUX_K4T)
  {b=TSL2561_LUX_B4T; m=TSL2561_LUX_M4T;}
  else if (ratio <= TSL2561_LUX_K5T)
  {b=TSL2561_LUX_B5T; m=TSL2561_LUX_M5T;}
  else if (ratio <= TSL2561_LUX_K6T)
  {b=TSL2561_LUX_B6T; m=TSL2561_LUX_M6T;}
  else if (ratio <= TSL2561_LUX_K7T)
  {b=TSL2561_LUX_B7T; m=TSL2561_LUX_M7T;}
  else if (ratio > TSL2561_LUX_K8T)
  {b=TSL2561_LUX_B8T; m=TSL2561_LUX_M8T;}

  unsigned long temp;
  temp = ((channel0 * b) - (channel1 * m));

  // do not allow negative lux value
  if (temp < 0) temp = 0;

  // round lsb (2^(LUX_SCALE-1))
  temp += (1 << (TSL2561_LUX_LUXSCALE-1));

  // strip off fractional portion
  lux = temp >> TSL2561_LUX_LUXSCALE;

  // Signal I2C had no errors
  return true;
}

boolean TSL2561::setInterruptControl(uint8_t control, uint8_t persist)
	// Sets up interrupt operations
	// If control = 0, interrupt output disabled
	// If control = 1, use level interrupt, see setInterruptThreshold()
	// If persist = 0, every integration cycle generates an interrupt
	// If persist = 1, any value outside of threshold generates an interrupt
	// If persist = 2 to 15, value must be outside of threshold for 2 to 15 integration cycles
	// Returns true (1) if successful, false (0) if there was an I2C error
	// (Also see getError() below)
{
	// Place control and persist bits into proper location in interrupt control register
	if (writeByte(TSL2561_REG_INTCTL,((control | 0B00000011) << 4) & (persist | 0B00001111)))
		return(true);

	return(false);
}


boolean TSL2561::setInterruptThreshold(uint16_t low, uint16_t high)
	// Set interrupt thresholds (channel 0 only)
	// low, high: 16-bit threshold values
	// Returns true (1) if successful, false (0) if there was an I2C error
	// (Also see getError() below)
{
	// Write low and high threshold values
	if (writeUInt(TSL2561_REG_THRESH_L,low) && writeUInt(TSL2561_REG_THRESH_H,high))
		return(true);

	return(false);
}


boolean TSL2561::clearInterrupt(void)
	// Clears an active interrupt
	// Returns true (1) if successful, false (0) if there was an I2C error
	// (Also see getError() below)
{
	// Set up command byte for interrupt clear
	Wire.beginTransmission(_i2c_address);
	Wire.write(TSL2561_CMD_CLEAR);
	_error = Wire.endTransmission();
	if (_error == 0)
		return(true);

	return(false);
}


boolean TSL2561::getID(uint8_t &ID)
	// Retrieves part and revision code from TSL2561
	// Sets ID to part ID (see datasheet)
	// Returns true (1) if successful, false (0) if there was an I2C error
	// (Also see getError() below)
{
	// Get ID byte from ID register
	if (readByte(TSL2561_REG_ID,ID))
		return(true);

	return(false);
}


uint8_t TSL2561::getError(void)
	// If any library command fails, you can retrieve an extended
	// error code using this command. Errors are from the wire library:
	// 0 = Success
	// 1 = Data too long to fit in transmit buffer
	// 2 = Received NACK on transmit of address
	// 3 = Received NACK on transmit of data
	// 4 = Other error
{
	return(_error);
}

// Private functions:

boolean TSL2561::readByte(uint8_t address, uint8_t &value)
	// Reads a byte from a TSL2561 address
	// Address: TSL2561 address (0 to 15)
	// Value will be set to stored byte
	// Returns true (1) if successful, false (0) if there was an I2C error
	// (Also see getError() above)
{
	// Set up command byte for read
	Wire.beginTransmission(_i2c_address);
	Wire.write((address & 0x0F) | TSL2561_CMD);
	_error = Wire.endTransmission();

	// Read requested byte
	if (_error == 0)
	{
		Wire.requestFrom(_i2c_address,1);
		if (Wire.available() == 1)
		{
			value = Wire.read();
			return(true);
		}
	}
	return(false);
}


boolean TSL2561::writeByte(uint8_t address, uint8_t value)
	// Write a byte to a TSL2561 address
	// Address: TSL2561 address (0 to 15)
	// Value: byte to write to address
	// Returns true (1) if successful, false (0) if there was an I2C error
	// (Also see getError() above)
{
	// Set up command byte for write
	Wire.beginTransmission(_i2c_address);
	Wire.write((address & 0x0F) | TSL2561_CMD);
	// Write byte
	Wire.write(value);
	_error = Wire.endTransmission();
	if (_error == 0)
		return(true);

	return(false);
}


boolean TSL2561::readUInt(uint8_t address, uint16_t &value)
	// Reads an unsigned integer (16 bits) from a TSL2561 address (low byte first)
	// Address: TSL2561 address (0 to 15), low byte first
	// Value will be set to stored unsigned integer
	// Returns true (1) if successful, false (0) if there was an I2C error
	// (Also see getError() above)
{
	uint8_t high, low;

	// Set up command byte for read
	Wire.beginTransmission(_i2c_address);
	Wire.write((address & 0x0F) | TSL2561_CMD);
	_error = Wire.endTransmission();

	// Read two bytes (low and high)
	if (_error == 0)
	{
		Wire.requestFrom(_i2c_address,2);
		if (Wire.available() == 2)
		{
			low = Wire.read();
			high = Wire.read();
			// Combine bytes into unsigned int
			value = (high << 8) | low;
			return(true);
		}
	}
	return(false);
}


boolean TSL2561::writeUInt(uint8_t address, uint16_t value)
	// Write an unsigned integer (16 bits) to a TSL2561 address (low byte first)
	// Address: TSL2561 address (0 to 15), low byte first
	// Value: unsigned int to write to address
	// Returns true (1) if successful, false (0) if there was an I2C error
	// (Also see getError() above)
{
	// Split int into lower and upper bytes, write each byte
	if (writeByte(address,(value & 0xFF))
		&& writeByte(address + 1,(value >> 8)))
		return(true);

	return(false);
}
