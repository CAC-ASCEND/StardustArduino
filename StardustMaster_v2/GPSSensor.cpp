/***************** GPS stuff *******************
    Using the Sparkfun u-blox SAM-M8Q
    Qwiic connection

    NOTE - the GPS breakout does not work through the mux. It needs interrupts
    / callback to run, and this doesn't work when it's mux port is disabled
***/

#include "MySensor.h"
#include <Wire.h>
#include <SparkFun_u-blox_GNSS_Arduino_Library.h> //http://librarymanager/All#SparkFun_u-blox_GNSS
#include <MicroNMEA.h> //http://librarymanager/All#MicroNMEA

SFE_UBLOX_GNSS myGNSS;
char nmeaBuffer[100];
MicroNMEA nmea(nmeaBuffer, sizeof(nmeaBuffer));

//This function gets called from the SparkFun u-blox Arduino Library
//As each NMEA character comes in you can specify what to do with it
//Useful for passing to other libraries like tinyGPS, MicroNMEA, or even
//a buffer, radio, etc.
void SFE_UBLOX_GNSS::processNMEA(char incoming)
{
  //Take the incoming char from the u-blox I2C port and pass it on to the MicroNMEA lib
  //for sentence cracking
  nmea.process(incoming);
}

/**************************************
 * GPS Sensor 
 **************************************/  
void CGPSSensor::InitSensor()
{
    //EnableMuxPort(MuxPort);
        
    pinMode(GPS_FIX_ON, OUTPUT);      // LED for indicating a fix has been established
    digitalWrite(GPS_FIX_ON, LOW);
    
    Wire.begin();
    
    if (myGNSS.begin() == false)
        {
        ErrMsg = "u-blox GNSS not detected at default I2C address 0x42. Please check wiring.";
        FailSensor(GPS_NOT_FOUND);
        //DisableMuxPort(MuxPort);
        return;  
        }

    myGNSS.setI2COutput(COM_TYPE_UBX | COM_TYPE_NMEA); //Set the I2C port to output both NMEA and UBX messages
    myGNSS.saveConfigSelective(VAL_CFG_SUBSEC_IOPORT); //Save (only) the communications port settings to flash and BBR
  
    myGNSS.setProcessNMEAMask(SFE_UBLOX_FILTER_NMEA_ALL); // Make sure the library is passing all NMEA messages to processNMEA
    //myGNSS.setProcessNMEAMask(SFE_UBLOX_FILTER_NMEA_GGA); // Or, we can be kind to MicroNMEA and _only_ pass the GGA messages to it

    // Set the Dynamic Model? to typical 2g Airborne model rather than Portable 2D model low acceleration
    //bool myGNSS.setDynamicModel(DYN_MODEL_AIRBORNE2g);
    
    GPS_fix = false;
    startFixTime = millis();

    Value = 0.0;    // Value is altitude      
    Latitude = 0.0;
    Longitude = 0.0;
 
    //DisableMuxPort(MuxPort);
}

/********************************
 * CGPSSensor::ReadSensor()
 * 
 * Read the next message from the GPS and parse it.
 */
bool CGPSSensor::ReadSensor()
{
    bool readOK = true;
    
    if (!SensorAvailable) return(readOK);
    
    //EnableMuxPort(MuxPort);
        
    ErrMsg="";
    // checking timing to get a fix
    double fixMinutes = 0.0;    // num minutes to find or lose fix
    char logMsg[TheLogger.MAXLOGLINELENGTH+1];       // use for logging to error log
    char buf[15];           // buffer for number conversions
                    
    myGNSS.checkUblox(); //See if new data is available. Process bytes as they come in.
    if(nmea.isValid() == true)      // is fix valid
        {
        digitalWrite(GPS_FIX_ON, HIGH);
        if ((!GPS_fix) && (myGNSS.getFixType() > 0))
            {  
            GPS_fix = true;
            strcpy(logMsg, "Established fix in"); 
            fixMinutes = (millis() - startFixTime) / 1000.0 / 60.0;     // convert msec to minutes
            dtostrf(fixMinutes,8,1,buf);
            strcat(logMsg, buf);
            strcat(logMsg," minutes");
            TheLogger.LogMsg(logMsg);
            startFixTime = millis();
            }
        Latitude = nmea.getLatitude() / 1000000.;    // in millionths of deg
        Longitude = nmea.getLongitude() / 1000000.;
        long alt_temp = 0;
        if (nmea.getAltitude(alt_temp))      // in mm
            { // altitude was no good for some reason
            Value = alt_temp / 1000.0;
            }
        }
    else if (myGNSS.getFixType() == 0)
        {   // no fix
        digitalWrite(GPS_FIX_ON, LOW);
        GPS_fix = false;
        strcpy(logMsg, "Lost fix in"); 
        fixMinutes = (millis() - startFixTime) / 1000.0 / 60.0;
        dtostrf(fixMinutes,8,1,buf);
        strcat(logMsg, buf);
        strcat(logMsg," minutes");
        TheLogger.LogMsg(logMsg);
        startFixTime = millis();
        }

    //DisableMuxPort(MuxPort);
    return (readOK);
}

void CGPSSensor::GetHeader(char *buf)
{
    strcpy(buf, "Altitude,  Latitude, Longitude");
}

void CGPSSensor::GetLogLine(char *buf)
{
    char buf2[15];
    
    if ((strcmp(ErrMsg.c_str() , "") != 0) || !SensorAvailable)
        { // err - return empty string
        strcpy(buf, "        ,          ,          ");  
        }
    else
        { // good read
        dtostrf(Value,8,1,buf);
        Mstrcat(buf,",",TheLogger.MAXLOGLINELENGTH);
        dtostrf(Latitude,10,6,buf2);
        Mstrcat(buf, buf2,TheLogger.MAXLOGLINELENGTH);
        Mstrcat(buf,",",TheLogger.MAXLOGLINELENGTH);
        dtostrf(Longitude,10,6,buf2);
        Mstrcat(buf, buf2,TheLogger.MAXLOGLINELENGTH);
        }
}
