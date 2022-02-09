
#include "MySensor.h"
#include "Config.h"

Adafruit_BMP3XX bmp;              // I2C

/***************************
 * CMySensor classes.
 * These consist of the base class CMySensor, along with
 * a separate subclass for each sensor.
 * 
 ***************************/

#define LOW_VOLTAGE_LIMIT 8.0


// Base Class
CMySensor::CMySensor(char *sensorName, int pinnum, int muxport)    // constructor
{
    Value = 0.0;
    ErrMsg = "";
    SensorName = sensorName;
    PinNum = pinnum;
    MuxPort = muxport;
    SensorAvailable = true;
}

void CMySensor::GetHeader(char *buf)
{
    strcpy(buf, SensorName.c_str());
}

void CMySensor::GetLogLine(char *buf)
{
    if ((strcmp(ErrMsg.c_str() , "") != 0) || !SensorAvailable)
        { // err - return empty string
        strcpy(buf, "        ");  
        }
    else
        {
        dtostrf(Value,8,2,buf);
        }
    
}
 
/****************************
 * FailSensor
 * 
 * inputs
 *   ErrMsg contains the basic message for the error
 *   errCode contains the number relating to this error
 *       (this controls the error lights)
 *       
 * When initialization fails, each sensor calls this function.
 * It Sets the fail code (this controls the failing light patterns),
 * and a message describing the failure.
 * This routine formats the message, sends it to the Flash routine,
 * and sets SensorAvailable to false so we 
 * a) don't waste time trying to read it,
 * b) don't fill the Error log with failure messages for each read attempt
 */
void CMySensor::FailSensor(int errcode)
{
    // format the message
    char msg[80];

    strcpy(msg, SensorName.c_str());
    Mstrcat(msg," Failure: ", 80);
    Mstrcat(msg,(char *)ErrMsg.c_str(), 80);
    FlashStatusError(errcode, msg);
    
    SensorAvailable = false;    
    
    ErrMsg = "";    // clear ErrMsg
}

// EnableMuxPort()
// If a sensor is on the mux, need 
// to enable the port to see the sensor
void CMySensor::EnableMuxPort(int muxport)
{
    if (muxport == NO_MUX) return;
    
    if (muxport > 7) muxport = 7;
  
    Wire.beginTransmission(MUX_ADDRESS);
    //Read the current mux settings
    Wire.requestFrom((int)MUX_ADDRESS, 1);
    if (!Wire.available()) return; //Error
    byte settings = Wire.read();
  
    //Set the wanted bit to enable the port
    settings |= (1 << muxport);
  
    Wire.write(settings);
    Wire.endTransmission();     
    //debug("        enabled mux port ",muxport);
}

// DisableMuxPort()
void CMySensor::DisableMuxPort(int muxport)
{
    if (muxport == NO_MUX) return;
    
    if (muxport > 7) muxport = 7;
  
    Wire.beginTransmission(MUX_ADDRESS);
    //Read the current mux settings
    Wire.requestFrom((int)MUX_ADDRESS, 1);
    if (!Wire.available()) return; //Error
    byte settings = Wire.read();
  
    //Clear the wanted bit to disable the port
    settings &= ~(1 << muxport);
  
    Wire.write(settings);
    Wire.endTransmission();     
    //debug("        disabled mux port ",muxport);
}

/**************************************
 * BMP388Sensor for Pressure, Temperature, and Altitude 
 *    Note that the temperature is inside the box, should be similar to
 *    InternTemp value.
 **************************************/  
void CBMP388Sensor::InitSensor()
{
    if (MuxPort != NO_MUX)
        EnableMuxPort(MuxPort);
        
    if (!bmp.begin_I2C()) 
        {   // hardware I2C mode, can pass in address & alt Wire
        FlashStatusError(BMP388_NOT_FOUND, "BMP388 sensor failed to initialize");
        SensorAvailable = false;
        }

    // Set up oversampling and filter initialization
    bmp.setTemperatureOversampling(BMP3_OVERSAMPLING_8X);
    bmp.setPressureOversampling(BMP3_OVERSAMPLING_4X);
    bmp.setIIRFilterCoeff(BMP3_IIR_FILTER_COEFF_3);
    bmp.setOutputDataRate(BMP3_ODR_50_HZ);

    if (MuxPort != NO_MUX)
        DisableMuxPort(MuxPort);
}
    
bool CBMP388Sensor::ReadSensor()
{
    bool readOK = true;

    if (!SensorAvailable) return(readOK);
    
    if (MuxPort != NO_MUX)
        EnableMuxPort(MuxPort);
        
    ErrMsg="";
    Value = 0.0;
    if (! bmp.performReading()) 
        {
        ErrMsg = SensorName;
        ErrMsg += " Failed to perform reading";
        readOK = false;
        }
    else
        { // good data
        Value = bmp.pressure / 100.0;         // in hpa
        bmpTemperature = bmp.temperature;     // in degC
        bmpAltitude = bmp.readAltitude(MyConfig.SeaLevelPressure);   // in meters
        }
    if (MuxPort != NO_MUX)
        DisableMuxPort(MuxPort);
    return (readOK);
}

void CBMP388Sensor::GetHeader(char *buf)
{
    strcpy(buf,"  bmpHpa,  bmpAlt, bmpTemp");
}

void CBMP388Sensor::GetLogLine(char *buf)
{
    char buf2[15];
    
    if ((strcmp(ErrMsg.c_str() , "") != 0) || !SensorAvailable)
        { // err - return empty string
        strcpy(buf, "        ,        ,        ");  
        }
    else
        { // good read
        dtostrf(Value,8,2,buf);
        Mstrcat(buf,",",TheLogger.MAXLOGLINELENGTH);
        dtostrf(bmpAltitude,8,2,buf2);
        Mstrcat(buf,buf2,TheLogger.MAXLOGLINELENGTH);
        Mstrcat(buf,",",TheLogger.MAXLOGLINELENGTH);
        dtostrf(bmpTemperature,8,2,buf2);
        Mstrcat(buf,buf2,TheLogger.MAXLOGLINELENGTH);
        }
}

  
/**************************************
 * Voltage Sensor  for reading battery voltages
 * The same class is used for both the 9 Volt and 3.7 Volt batteries
 **************************************/  
void CVoltSensor::InitSensor()
{
    pinMode (PinNum, INPUT);     // for analogRead
}
    
bool CVoltSensor::ReadSensor()
{
    bool readOK = true;

    ErrMsg="";
    int rdg = analogRead(PinNum);
    Value = rdg * 5.0 / 1024.0;
    if (SensorName == "   Volt9")
      { // The 9 Volt reading uses a voltage divider to get half the actual voltage, so the voltage
        // fits in the 0..5 Volt range of the AD. Need to double the reading to get the
        // actual voltage
      Value = 2.0 * Value;

      // If the voltage gets too low, we end up with the disk files getting
      // corrupted. If the voltage gets below 8, halt the system.
      /**** 
       *  This turned out be be a poor idead. Our battery went too low after 1 hour or so.
      if (Value < LOW_VOLTAGE_LIMIT)
          {  // too low
          digitalWrite(HEATER_LED, LED_ON);
          digitalWrite(GPS_FIX_ON, LED_ON);
          digitalWrite(STATUS_LED, LED_ON);
          while (1);
          }
          *******/
      }
    
    return (readOK);
}


// Create Sensor objects
//   Select the desired configuration in MySensor.h
#ifdef PRODUCTION_SENSORS
CGPSSensor GPSSensor("     GPS", 0, NO_MUX);    // name, pin, muxport
CCO2Sensor CO2SensorOld("  CO2Old", 0, 1);
CCO2Sensor CO2SensorNew("  CO2New", 0, 4);
//CDHTTempSensor TempSensor(" OutTemp", EXTERNTEMP_PIN, NO_MUX);
//CDS18BTempSensor InternTempSensor(" IntTemp", INTERNTEMP_PIN, NO_MUX);
//CDS18BTempSensor OutsideTempSensor("OutDSB18", OUTDS18BTEMP_PIN, NO_MUX);
//CUVSensor UVSensor1("     UV1", 0, 2);
CUVSensor UVSensor2("     UV2", 0, 7);
CBMP388Sensor BMP388Sensor("Pressure", 0, 2);
//CVoltSensor Volt9Sensor("   Volt9", PINVOLT9, NO_MUX);
//CVoltSensor Volt37Sensor("   Volt37", PINVOLT37, NO_MUX);

CMySensor *SensorArr[] = {&GPSSensor, &CO2SensorOld,&CO2SensorNew, 
//          &TempSensor, &InternTempSensor, &OutsideTempSensor,
          &BMP388Sensor, &UVSensor2};
#endif

// Sensors used in the ColdBox test setup
#ifdef COLDBOX_SENSORS
CGPSSensor GPSSensor("     GPS", 0, NO_MUX);    // name, pin, muxport
CCO2Sensor CO2SensorOld("  CO2Old", 0, 1);
CCO2Sensor CO2SensorNew("  CO2New", 0, 4);
//CDHTTempSensor TempSensor(" OutTemp", EXTERNTEMP_PIN, NO_MUX);
//CDS18BTempSensor InternTempSensor(" IntTemp", INTERNTEMP_PIN, NO_MUX);
//CDS18BTempSensor OutsideTempSensor("OutDSB18", OUTDS18BTEMP_PIN, NO_MUX);
CUVSensor UVSensor1("     UV1", 0, 2);
CUVSensor UVSensor2("     UV2", 0, 7);
//CBMP388Sensor BMP388Sensor("Pressure", 0, 2);
//CVoltSensor Volt9Sensor("   Volt9", PINVOLT9, NO_MUX);
//CVoltSensor Volt37Sensor("   Volt37", PINVOLT37, NO_MUX);

CMySensor *SensorArr[] = {&GPSSensor, &CO2SensorOld,&CO2SensorNew, 
//          &TempSensor, &InternTempSensor, &OutsideTempSensor,
          &UVSensor2, &UVSensor2};
#endif

int MaxSensors = (sizeof(SensorArr) / sizeof(int));  




/*********************************************
 * CHeaterControl
 * 
 * Logic to control the heaters based on the internal temperature
 */
CHeaterControl::CHeaterControl()   // constructor
{
}

void CHeaterControl::Init()
{
    HeaterOn = false;
    pinMode(HEATER_PIN, OUTPUT);
    pinMode(HEATER_LED, OUTPUT);
    
    digitalWrite(HEATER_PIN,LOW);     // heater off
    digitalWrite(HEATER_LED,LED_OFF); // LED off
}

         
void CHeaterControl::HeaterOnOff(double theTemp)
{
    if ((theTemp < HEATER_LOW_LIMIT) && (theTemp > -273.0))
        {
        digitalWrite(HEATER_PIN, HIGH);     // start frying  
        digitalWrite(HEATER_LED,LED_ON);  
        HeaterOn = true;
        }
    else if (theTemp > HEATER_HIGH_LIMIT)
        {
        digitalWrite(HEATER_PIN, LOW);     // stop frying  
        digitalWrite(HEATER_LED,LED_OFF);  
        HeaterOn = false;
        }
    else
        { // bad read
        digitalWrite(HEATER_PIN, LOW);     // stop frying  
        digitalWrite(HEATER_LED,LED_OFF);  
        HeaterOn = false;
        }
}

CHeaterControl HeaterControl;
