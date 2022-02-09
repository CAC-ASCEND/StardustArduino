
#include "MySensor.h"

/***************** SCD30 stuff *******************/
Adafruit_SCD30  scd30;            // I2C 0x61
#define CO2READY                // when sensor is installed

/***************** VEML6075 stuff *******************/
//Adafruit_VEML6075 uv = Adafruit_VEML6075();     // at address 0x10
VEML6075 uv;


/*********************************** 
 * CO2 Sensor - SCD30
 *  Gives CO2 in ppm
 *  Also provides temperature (external temperature) and Relative Humidity
 *************************************/  
void CCO2Sensor::InitSensor()
{
    if (MuxPort != NO_MUX)
        EnableMuxPort(MuxPort);
        
    // CO2SENSOR_ADDRESS  is 61
    if (!scd30.begin()) 
        {
        ErrMsg = "SCD30 CO2 sensor failed to begin";
        FailSensor(CO2_NOT_FOUND);
        return;
        }
    // Can set longer measurement interval (default 2 sec) to save some power
     if (!scd30.setMeasurementInterval(5)){
         Serial.println("SCD30 Failed to set measurement interval");
    //   while(1){ delay(10);}
     }
    //Serial.print("Measurement Interval: "); 
    //Serial.print(scd30.getMeasurementInterval()); 
    //Serial.println(" seconds");    

    if (MuxPort != NO_MUX)
        DisableMuxPort(MuxPort);
}
    
bool CCO2Sensor::ReadSensor()
{
    bool readOK = true;

    if (!SensorAvailable) return(readOK);
    
    if (MuxPort != NO_MUX)
        EnableMuxPort(MuxPort);
        
    ErrMsg="";
    if (scd30.dataReady())       
        { //Data available!
        if (!scd30.read())          
            { 
            ErrMsg = "Error reading SCD30 CO2 sensor data"; 
            readOK = false; 
            }
        else
            { // data should be good
            scd30Temp = scd30.temperature;    // deg C
            scd30RH = scd30.relative_humidity;    // %
            Value =  scd30.CO2;        // in ppm
            } 
        }
    else 
        {   // no data. Leave data as prev read
        
        }

    if (MuxPort != NO_MUX)
        DisableMuxPort(MuxPort);
    return (readOK);
}

void CCO2Sensor::GetHeader(char *buf)
{
    strcpy(buf, "  CO2ppm, SCDTemp,   SCDRH");
}

void CCO2Sensor::GetLogLine(char *buf)
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
        dtostrf(scd30Temp,8,2,buf2);
        Mstrcat(buf, buf2,TheLogger.MAXLOGLINELENGTH);
        Mstrcat(buf,",",TheLogger.MAXLOGLINELENGTH);
        dtostrf(scd30RH,8,2,buf2);
        Mstrcat(buf, buf2,TheLogger.MAXLOGLINELENGTH);
        }
}



/**************************************
 * UV Light Sensor 
 **************************************/  
void CUVSensor::InitSensor()
{
    if (MuxPort != NO_MUX)
        EnableMuxPort(MuxPort);
        
    Wire.begin();
    
    if (! uv.begin()) 
        {
        ErrMsg = "VEML 6075 sensor failed to initialize";
        FailSensor(UV_NOT_FOUND);
        return;
        }
    /* Set the integration constant
     *    How do we select? Gets better precision?
        IT_50MS,
        IT_100MS,
        IT_200MS,
        IT_400MS,
        IT_800MS,
      */
    //uv.setIntegrationTime(veml6075_uv_it_t it)
    
    // Set the high dynamic mode   
    //  DYNAMIC_NORMAL,   default
    //  DYNAMIC_HIGH,
    //  HD_INVALID
    //setHighDynamic(veml6075_hd_t hd);
    
    // Set the mode
    //  AF_DISABLE,
    //  AF_ENABLE,
    //uv.setAutoForce(veml6075_af_t af);
    if (MuxPort != NO_MUX)
        DisableMuxPort(MuxPort);
}
    
bool CUVSensor::ReadSensor()
{
    bool readOK = true;

    if (!SensorAvailable) return(readOK);
    
    if (MuxPort != NO_MUX)
        EnableMuxPort(MuxPort);
        
    ErrMsg="";
    
    UVindex = uv.index();
    Value = uv.uva();
    UVB = uv.uvb();
    //Serial.print("uva,b,i is "); Serial.print(Value,2);Serial.print(UVB,2);Serial.println(UVindex,2);

    if (MuxPort != NO_MUX)
        DisableMuxPort(MuxPort);
    return (readOK);
}


void CUVSensor::GetHeader(char *buf)
{
    strcpy(buf, "     UVA,     UVB, UVindex");
}

void CUVSensor::GetLogLine(char *buf)
{
    char buf2[25];
    
    if ((strcmp(ErrMsg.c_str() , "") != 0) || !SensorAvailable)
        { // err - return empty string
        strcpy(buf, "        ,        ,        ");  
        }
    else
        { // good read
        dtostrf(Value,8,2,buf);
        Mstrcat(buf,",",TheLogger.MAXLOGLINELENGTH);
        dtostrf(UVB,8,2,buf2);
        Mstrcat(buf,buf2,TheLogger.MAXLOGLINELENGTH);
        Mstrcat(buf,",",TheLogger.MAXLOGLINELENGTH);
        dtostrf(UVindex,8,2,buf2);
        Mstrcat(buf,buf2,TheLogger.MAXLOGLINELENGTH);
        }
    
}
