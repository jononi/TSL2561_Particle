SparkFun TSL2561 Arduino Library ported for Particle Photon Platform
========================================

Originally developed by SparkFun

Adapted by Jaafar Ben-Abdallah - July 2015

Next features to add
================================
From adafruit library, pick:
- the begin(address) function that retrieves ID to ensure connection is established with the sensor
- the auto gain feature keeping the same thresholds
- the setTiming function (sparkfun version is kinda messy)
- if it's giving similar reading, adapt adafruit's illuminance value (lux) computing that use only integers as opposed to double and get rid of pow(.,.) function

This is a Library for the AMS/TAOS TSL2561 Luminiosity sensor.
This illumination sensor has a flat response across most of the visible spectrum and has an adjustable integration time. 
It communicates via I2C and runs at 3.3V. 

Documentation
--------------
* **[Hookup Guide](https://learn.sparkfun.com/tutorials/tsl2561-luminosity-sensor-hookup-guide)** - Basic hookup guide for the TSL2561 Luminosity Sensor.

License Information
-------------------

This product is _**open source**_! 

The **code** is beerware; if you see me (or any other SparkFun employee) at the local, and you've found our code helpful, please buy us a round!

Please use, reuse, and modify these files as you see fit. Please maintain attribution to SparkFun Electronics and release anything derivative under the same license.

Distributed as-is; no warranty is given.

- Your friends at SparkFun.
