#ifndef SYSTEMPARAMETERS_H
#define SYSTEMPARAMETERS_H

/********************************************
 * Notes:
 * The various pins used by the system are listed here
 * Some caveats:
 * 
 * 1. The I2C runs through the I2C shield. This shield expects
 * SDA/SCLK to be on A4/A5. On the Mega these are on 21/21, so 
 * we run a pair of jumper wires from 20/21 to A4/A5.
 * =====> this mean we can not use A4 and A5 for analog data!
 * 
 * 2. The GPS card uses pins 7/8 (Rcv/Tx) to talk to the Arduino. 
 * Again, these are located elsewhere on the Mega (18/19).
 * We need to run jumpers from 18/19: 18 -> 7, 19 ->8
 * 
 * The GPS has a small switch; it should be set to Software
 * 
 * 3. The I2C addresses in use are listed below. If additional
 * I2C sensors are added, add them to the list. 
 * If another sensor has the same address, check whether one of 
 * the conflicting sensors can have its address changed (some 
 * sensors allow jumper soldering across pads to change the address).
 * If you cannot change the address you will need a Mux card from 
 * sparkfun. This card allows devices to be daisy chained from separate 
 * ports. Devices off of one port do not "see" devices on a differnt
 * port.
 * 
 *
 */

/*************************************************
 * Need to input today's sea level pressure here,
 * then recompile and reload code into the Mega
 * This is needed to get better precision on altitude calculations
 * based on pressure
 */


#include <Arduino.h>
#include <CACBoardDiff.h>

/******* I2C addresses
 *  0x10  VEML6075 (UV Sensor)
 *  0x61  SCD30 (CO2 sensor)
 *  0x77  BMP388 (Pressure/Altitude)
 */
/*************** Defining pins used *****************/
#define PIN_DISKLOG 22      // debug tool - if low, do not log to disk - serial print instead

#define PINVOLT9              A0    // AD Pin for 9 Volt Battery reading    
#define PINVOLT37             A1    // AD Pin for 3.7 Volt Battery reading    
#define CH4_PIN               A2

#ifdef REDBOARD_TURBO
#define O3LOW_PIN             A3    // Turbo only has 6 A/D?
#define O3HIGH_PIN            A4
#else
#define O3LOW_PIN             A8    
#define O3HIGH_PIN            A9
#endif

// GPS uses pins 7,8 for Software Serial to GPS chip
#define GPS_RECIEVE_PIN       7
#define GPS_TRANSMIT_PIN      8
#ifdef ARDUINO_SAMD_ZERO
#define GPS_FIX_ON            13    // turns blue LED on when Fix is made
#else
#define GPS_FIX_ON            45    // turns blue LED on when Fix is made
#endif

#define HEATER_LOW_LIMIT      4    // degrees C. Below this, turns on heater
#define HEATER_HIGH_LIMIT     5    // degrees C. Below this, turns on heater

#define EXTERNTEMP_PIN        39    // DHT22 one wire read
#define INTERNTEMP_PIN        37    // DS18B20 one wire read
#define OUTDS18BTEMP_PIN      47    // DS18B20 one wire read

#define CO2SENSOR_ADDRESS     0x61

// General constants
#define LED_ON      HIGH    // for turning LEDs on/off
#define LED_OFF     LOW

extern void Mstrcpy(char *target, char *src, int tarLim);
extern void Mstrcat(char *target, char *src, int tarLim);

#endif
