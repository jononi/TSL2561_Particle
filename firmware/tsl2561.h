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

#ifndef TSL2561_h
#define TSL2561_h

#include "application.h"

class TSL2561
{

	public:

	char _i2c_address;
	uint8_t _error;
	bool _gain;
	uint8_t _it;

	public:
		TSL2561(uint8_t i2c_address);
		// TSL2561 object
		// TSL2561_ADDR_0 (0x29 address with '0', connected to GND)
		// TSL2561_ADDR   (0x39 default address, pin floating)
		// TSL2561_ADDR_1 (0x49 address with '1' connected to VIN)

		boolean begin(void);
		// Initialize TSL2561 library
		// returns true if device connected

		boolean setPowerUp(void);
		// Turn on TSL2561, begin integration
		// Returns true (1) if successful, false (0) if there was an I2C error
		// (Also see getError() below)

		boolean setPowerDown(void);
		// Turn off TSL2561
		// Returns true (1) if successful, false (0) if there was an I2C error
		// (Also see getError() below)

		boolean setTiming(boolean gain, uint8_t time);
		// If gain = false (0), device is set to low gain (1X)
		// If gain = high (1), device is set to high gain (16X)
		// If time = 0, integration will be 13.7ms
		// If time = 1, integration will be 101ms
		// If time = 2, integration will be 402ms
		// If time = 3, use manual start / stop
		// Returns true (1) if successful, false (0) if there was an I2C error
		// (Also see getError() below)

		boolean setTiming(boolean gain, uint8_t time, uint16_t &ms);
		// Identical to above command, except ms is set to selected integration time
		// If gain = false (0), device is set to low gain (1X)
		// If gain = high (1), device is set to high gain (16X)
		// If time = 0, integration will be 13.7ms
		// If time = 1, integration will be 101ms
		// If time = 2, integration will be 402ms
		// If time = 3, use manual start / stop (ms = 0)
		// ms will be set to requested integration time
		// Returns true (1) if successful, false (0) if there was an I2C error
		// (Also see getError() below)

		boolean manualStart(void);
		// Starts a manual integration period
		// After running this command, you must manually stop integration with manualStop()
		// Internally sets integration time to 3 for manual integration (gain is unchanged)
		// Returns true (1) if successful, false (0) if there was an I2C error
		// (Also see getError() below)

		boolean manualStop(void);
		// Stops a manual integration period
		// Returns true (1) if successful, false (0) if there was an I2C error
		// (Also see getError() below)

		boolean getData(uint16_t &CH0, uint16_t &CH1, bool autoGain);
		// Retrieve raw integration results
		// CH0 and CH1 will be set to integration results
		// if autoGain is true, autogain is enabled and the gain will be adjusted if needed
		// Returns true (1) if successful, false (0) if there was an I2C error
		// (Also see getError() below)

		boolean getLux(uint16_t ms, uint16_t CH0, uint16_t CH1, double &lux);
		// Convert raw data to lux
		// ms: integration time in ms, from setTiming() or from manual integration
		// CH0, CH1: results from getData()
		// lux will be set to resulting lux calculation
		// returns true (1) if calculation was successful
		// RETURNS false (0) AND lux = 0.0 IF EITHER SENSOR WAS SATURATED (0XFFFF)

		boolean getLuxInt(uint16_t CH0, uint16_t CH1, uint32_t &lux);
		// Convert raw data to lux as integer
		// this is not available for custom integration time
		// this function uses integer based approximate calculation but accuracy compared...
		// ...to getLux:float is very good (max 2%, probably smaller than sensor accuracy)
		// CH0, CH1: results from getData()
		// lux will be set to illuminance value in lux
		// returns true (1) if calculation was successful
		// RETURNS false (0) AND lux = 0 IF EITHER SENSOR WAS SATURATED (0XFFFF)

		boolean setInterruptControl(uint8_t control, uint8_t persist);
		// Sets up interrupt operations
		// If control = 0, interrupt output disabled
		// If control = 1, use level interrupt, see setInterruptThreshold()
		// If persist = 0, every integration cycle generates an interrupt
		// If persist = 1, any value outside of threshold generates an interrupt
		// If persist = 2 to 15, value must be outside of threshold for 2 to 15 integration cycles
		// Returns true (1) if successful, false (0) if there was an I2C error
		// (Also see getError() below)

		boolean setInterruptThreshold(uint16_t low, uint16_t high);
		// Set interrupt thresholds (channel 0 only)
		// low, high: 16-bit threshold values
		// Returns true (1) if successful, false (0) if there was an I2C error
		// (Also see getError() below)

		boolean clearInterrupt(void);
		// Clears an active interrupt
		// Returns true (1) if successful, false (0) if there was an I2C error
		// (Also see getError() below)

		boolean getID(uint8_t &ID);
		// Retrieves part and revision code from TSL2561
		// Sets ID to part ID (see datasheet)
		// Returns true (1) if successful, false (0) if there was an I2C error
		// (Also see getError() below)

		uint8_t getError(void);
		// If any library command fails, you can retrieve an extended
		// error code using this command. Errors are from the wire library:
		// 0 = Success
		// 1 = Data too long to fit in transmit buffer
		// 2 = Received NACK on transmit of address
		// 3 = Received NACK on transmit of data
		// 4 = Other error

		private:

		boolean readByte(uint8_t address, uint8_t &value);
		// Reads a byte from a TSL2561 address
		// Address: TSL2561 address (0 to 15)
		// Value will be set to stored byte
		// Returns true (1) if successful, false (0) if there was an I2C error
		// (Also see getError() above)

		boolean writeByte(uint8_t address, uint8_t value);
		// Write a byte to a TSL2561 address
		// Address: TSL2561 address (0 to 15)
		// Value: byte to write to address
		// Returns true (1) if successful, false (0) if there was an I2C error
		// (Also see getError() above)

		boolean readUInt(uint8_t address, uint16_t &value);
		// Reads an unsigned integer (16 bits) from a TSL2561 address (low byte first)
		// Address: TSL2561 address (0 to 15), low byte first
		// Value will be set to stored unsigned integer
		// Returns true (1) if successful, false (0) if there was an I2C error
		// (Also see getError() above)

		boolean writeUInt(uint8_t address, uint16_t value);
		// Write an unsigned integer (16 bits) to a TSL2561 address (low byte first)
		// Address: TSL2561 address (0 to 15), low byte first
		// Value: unsigned int to write to address
		// Returns true (1) if successful, false (0) if there was an I2C error
		// (Also see getError() above)
};

#define TSL2561_ADDR_0 0x29 // address with '0' shorted on board
#define TSL2561_ADDR   0x39 // default address
#define TSL2561_ADDR_1 0x49 // address with '1' shorted on board

// TSL2561 registers

#define TSL2561_CMD           0x80
#define TSL2561_CMD_CLEAR     0xC0
#define	TSL2561_REG_CONTROL   0x00
#define	TSL2561_REG_TIMING    0x01
#define	TSL2561_REG_THRESH_L  0x02
#define	TSL2561_REG_THRESH_H  0x04
#define	TSL2561_REG_INTCTL    0x06
#define	TSL2561_REG_ID        0x0A
#define	TSL2561_REG_DATA_0    0x0C
#define	TSL2561_REG_DATA_1    0x0E

// Int based illuminance calculation
// T, FN and CL package values
#define TSL2561_LUX_K1T           (0x0040)  // 0.125 * 2^RATIO_SCALE
#define TSL2561_LUX_B1T           (0x01f2)  // 0.0304 * 2^LUX_SCALE
#define TSL2561_LUX_M1T           (0x01be)  // 0.0272 * 2^LUX_SCALE
#define TSL2561_LUX_K2T           (0x0080)  // 0.250 * 2^RATIO_SCALE
#define TSL2561_LUX_B2T           (0x0214)  // 0.0325 * 2^LUX_SCALE
#define TSL2561_LUX_M2T           (0x02d1)  // 0.0440 * 2^LUX_SCALE
#define TSL2561_LUX_K3T           (0x00c0)  // 0.375 * 2^RATIO_SCALE
#define TSL2561_LUX_B3T           (0x023f)  // 0.0351 * 2^LUX_SCALE
#define TSL2561_LUX_M3T           (0x037b)  // 0.0544 * 2^LUX_SCALE
#define TSL2561_LUX_K4T           (0x0100)  // 0.50 * 2^RATIO_SCALE
#define TSL2561_LUX_B4T           (0x0270)  // 0.0381 * 2^LUX_SCALE
#define TSL2561_LUX_M4T           (0x03fe)  // 0.0624 * 2^LUX_SCALE
#define TSL2561_LUX_K5T           (0x0138)  // 0.61 * 2^RATIO_SCALE
#define TSL2561_LUX_B5T           (0x016f)  // 0.0224 * 2^LUX_SCALE
#define TSL2561_LUX_M5T           (0x01fc)  // 0.0310 * 2^LUX_SCALE
#define TSL2561_LUX_K6T           (0x019a)  // 0.80 * 2^RATIO_SCALE
#define TSL2561_LUX_B6T           (0x00d2)  // 0.0128 * 2^LUX_SCALE
#define TSL2561_LUX_M6T           (0x00fb)  // 0.0153 * 2^LUX_SCALE
#define TSL2561_LUX_K7T           (0x029a)  // 1.3 * 2^RATIO_SCALE
#define TSL2561_LUX_B7T           (0x0018)  // 0.00146 * 2^LUX_SCALE
#define TSL2561_LUX_M7T           (0x0012)  // 0.00112 * 2^LUX_SCALE
#define TSL2561_LUX_K8T           (0x029a)  // 1.3 * 2^RATIO_SCALE
#define TSL2561_LUX_B8T           (0x0000)  // 0.000 * 2^LUX_SCALE
#define TSL2561_LUX_M8T           (0x0000)  // 0.000 * 2^LUX_SCALE

// Auto-gain thresholds
#define TSL2561_AGC_THI_13MS      (4850)    // Max value at Ti 13ms = 5047
#define TSL2561_AGC_TLO_13MS      (100)
#define TSL2561_AGC_THI_101MS     (36000)   // Max value at Ti 101ms = 37177
#define TSL2561_AGC_TLO_101MS     (200)
#define TSL2561_AGC_THI_402MS     (63000)   // Max value at Ti 402ms = 65535
#define TSL2561_AGC_TLO_402MS     (500)
// Clipping thresholds
#define TSL2561_CLIPPING_13MS     (4900)
#define TSL2561_CLIPPING_101MS    (37000)
#define TSL2561_CLIPPING_402MS    (65000)
// scale cofficients
#define TSL2561_LUX_LUXSCALE      (14)      // Scale by 2^14
#define TSL2561_LUX_RATIOSCALE    (9)       // Scale ratio by 2^9
#define TSL2561_LUX_CHSCALE       (10)      // Scale channel values by 2^10
#define TSL2561_LUX_CHSCALE_TINT0 (0x7517)  // 322/11 * 2^TSL2561_LUX_CHSCALE
#define TSL2561_LUX_CHSCALE_TINT1 (0x0FE7)  // 322/81 * 2^TSL2561_LUX_CHSCALE


#endif
