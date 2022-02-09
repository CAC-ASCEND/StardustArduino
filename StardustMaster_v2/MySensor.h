#ifndef MYSENSOR_H
#define MYSENSOR_H

#include <Arduino.h>
#include "SystemParameters.h"
#include <CACLogger.h>

//if sensor is direct, not going through the I2C mux, use this
const int NO_MUX =    -1;
#define MUX_ADDRESS   0x70      // I2C address of the mux

/***************** BMP380 stuff *******************/
#include <Adafruit_BMP3XX.h>      // BMP388 Pressure/Temperature sensor
#include <bmp3.h>
#include <bmp3_defs.h>

/***************** SCD30 stuff *******************/
#include <Adafruit_SCD30.h>

/***************** DHT22 stuff *******************/
#include <DHT.h>

/***************** VEML6075 stuff *******************/
//#include "Adafruit_VEML6075.h"
#include <SparkFun_VEML6075_Arduino_Library.h> 

/***************** GPS stuff *******************/
#include <Adafruit_GPS.h>
// Uncomment to use sneaky parsing of messages
//#define USE_SNEAKY_GPS_MESSAGES

/***************** DS18B temperature stuff *******************/
#include <OneWire.h> 
#include <DallasTemperature.h> 

/***************** Ozone & Methane stuff *******************/
#include <CACMQ131.h> 
#include <CACMQ9B.h> 

#include <CACLogger.h> 
extern CLogger TheLogger;

// Select the desired set of sensors.
// See near the end of MySensor.cpp for the sensor setup
#define PRODUCTION_SENSORS
#define SENSOR_SET_NAME "Production Sensors"
//#define COLDBOX_SENSORS
//#define SENSOR_SET_NAME "Cold Box Sensors"


/*********************************************
 * CHeaterControl
 * 
 * Logic to control the heaters based on the internal temperature
 */
class CHeaterControl
{
public:
  CHeaterControl();    // constructor
  
  void Init();         // code for setup() initialization  0=> pure virtual?
  void HeaterOnOff(double intTemp);     // read the sensor
  
  bool HeaterOn;        // tracking whether the heater is on
  
private:
};


/***************************************
 * CMySensor
 * 
 * Base class for Sensors in general. This class is
 * subclassed to create the actual sensors.
 */
class CMySensor
{
public:
  // muxport = -1 if not connected to a mux port
  CMySensor(char *sensorName, int pin, int muxport);    // constructor
  
  virtual void InitSensor() = 0;         // code for setup() initialization  0=> pure virtual?
  virtual bool ReadSensor() = 0;         // read the sensor
  virtual void GetHeader(char *buf);     // csv field header, like Temperature
  virtual void GetLogLine(char *buf);    // csv field string, like 1234.0
  void FailSensor(int errcode);          // Logs Initialization failure message

  void HeaterOnOff();           // used by Temperature sensor to turn on heaters
  bool HeaterOn;

  void EnableMuxPort(int muxport);   
  void DisableMuxPort(int muxport);   
  
  double Value;                  // current data value of ReadSensor
  String ErrMsg;                 // err msg in case read fails
  String SensorName;
  int    PinNum;                 // Arduino pin for reading the sensor, if needed
                                 // If I2C sensor, holds alternate I2C address
  int    MuxPort;                // NO_MUX if not on the mux

  bool SensorAvailable;         // true allows operation. False skips readings
private:
};

/********************************************************
 * CO2 sensor - SCD30
 *     also gives temperature (outside) and Relative Humidity
 *     I2C connection
 */
class CCO2Sensor: public CMySensor
{
public:
    CCO2Sensor(char *name, int pin, int muxport) : CMySensor(name, pin, muxport){}
    void InitSensor();
    bool ReadSensor();
    void GetHeader(char *buf);      // Use base routine
    void GetLogLine(char *buf);    

    // Value is CO2 in ppm
    double scd30Temp;               // temp in degC
    double scd30RH;                 // Relative Humidity in %
};

/********************************************************
 * GPS sensor - sparkfun u-blox SAM-M8Q breakout
 */
class CGPSSensor: public CMySensor
{
public:
  CGPSSensor(char *name, int pin, int muxport) : CMySensor(name, pin, muxport){}
  void InitSensor();
  bool ReadSensor();    // returns altitude
  void GetHeader(char *buf);     // override base
  void GetLogLine(char *buf);    // csv field string, like 1234.0

  // GPS-only variables
  // Value is altitude
  double Latitude;
  double Longitude;

  bool GPS_fix;     // true - we have a fix

private:

    // Used to measure time to get a fix
    uint32_t startFixTime = millis();   // tracking how long to get a fix
};

class CDHTTempSensor: public CMySensor
{
public:
  CDHTTempSensor(char *name, int pin, int muxport) : CMySensor(name, pin, muxport){}
  void InitSensor();
  bool ReadSensor();
  void GetHeader(char *buf);     // override base
  void GetLogLine(char *buf);    // csv field string, like 1234.0

  bool UseForHeaterControl;     // default false
  double Humidity;
  DHT *dht;
};

class CDS18BTempSensor: public CMySensor
{
public:
    CDS18BTempSensor(char *name, int pin, int muxport) : CMySensor(name, pin, muxport){}
    void InitSensor();
    bool ReadSensor();
    void GetHeader(char *buf);     // override base
    void GetLogLine(char *buf);    // csv field string, like 1234.0
  
    void printAddress(DeviceAddress deviceAddress);
    bool UseForHeaterControl;     // default false

private:
    OneWire *oneWire; 
    
    // Pass our oneWire reference to Dallas Temperature.  
    DallasTemperature *sensors; 
    
    // arrays to hold device address 
    DeviceAddress insideThermometer; 

};

class CUVSensor: public CMySensor
{
public:
  CUVSensor(char *name, int pin, int muxport) : CMySensor(name, pin, muxport){}
  void InitSensor();
  bool ReadSensor();
  void GetHeader(char *buf);     // csv field header, like Temperature
  void GetLogLine(char *buf);    // csv field string, like 1234.0

  // UV-only variables
  double UVB;            // UVA is in Value
  double UVindex;        // UV index
};

/****** replace this if we find one that works
class CCH4Sensor: public CMySensor
{
public:
  CCH4Sensor(char *name, int pin, int muxport) : CMySensor(name, pin, muxport){}
  void InitSensor();
  bool ReadSensor();    // returns altitude
  void GetHeader(char *buf);     // override base
  void GetLogLine(char *buf);    // csv field string, like 1234.0

  // CH4-only variables   
  double Voltage;   // the raw voltage used to calculate ppm
};
*******/
/****** replace this if we find one that works
class COzoneSensor: public CMySensor
{
public:
    COzoneSensor(char *name, int pin, int muxport) : CMySensor(name, pin, muxport){}
    void InitSensor();
    bool ReadSensor();    // returns altitude
    void GetHeader(char *buf);     // override base
    void GetLogLine(char *buf);    // csv field string, like 1234.0
  
    // Oxone-only variables   Value is OzoneLow 
    double OzoneHigh ;
    double VoltageHigh;   // the raw voltage used to calculate ppm
    double VoltageLow;   // the raw voltage used to calculate ppm

};
*****/

class CBMP388Sensor: public CMySensor
{
public:
  CBMP388Sensor(char *name, int pin, int muxport) : CMySensor(name, pin, muxport){}
  void InitSensor();
  bool ReadSensor();
  void GetHeader(char *buf);     // csv field header, like Temperature
  void GetLogLine(char *buf);    // csv field string, like 1234.0

  // Value is the pressure
  // Also reads Altitude and temperature
  double bmpAltitude;            // in meters
  double bmpTemperature;         // in degC
};


class CVoltSensor: public CMySensor
{
public:
  CVoltSensor(char *name, int pin, int muxport) : CMySensor(name, pin, muxport){}
  void InitSensor();
  bool ReadSensor();    // returns altitude
};

extern CGPSSensor GPSSensor;
extern CCO2Sensor CO2Sensor;
extern CDS18BTempSensor InternTempSensor;
extern CDHTTempSensor TempSensor;
extern CUVSensor UVSensor;
//extern COzoneSensor O3Sensor;
extern CVoltSensor CVolt9Sensor;
extern CVoltSensor CVolt37Sensor;
extern CBMP388Sensor BMP388Sensor;
//extern CCH4Sensor CH4Sensor;

extern CMySensor *SensorArr[];
extern int MaxSensors;

extern CHeaterControl HeaterControl;


#endif
