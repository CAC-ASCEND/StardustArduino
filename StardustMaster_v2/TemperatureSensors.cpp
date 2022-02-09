/**************************************************
 * TemperatureSensors
 * 
 * Subclasses based on CMySensor for the temperature Sensors
 * 
 * Currently two sensors:
 * CDHTTempSensor for the DHT22
 * CDS18BTempSensor for the DS18B20 sensor
 *************************************************/
 
#include "MySensor.h"

/***************** DHT22 stuff *******************/
#define DHTTYPE DHT22
       

/***************** DS18B temperature stuff *******************/


/********************************************** 
 * DS18B20 Temperature Sensor 
 ***********************************************/  
void CDS18BTempSensor::InitSensor()
{
    UseForHeaterControl = false;
    
    pinMode (PinNum, INPUT);      // for AD pin analogRead

    // Create the objects
    // Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs) 
    oneWire = new OneWire(PinNum); 

    // Pass our oneWire reference to Dallas Temperature.  
    sensors = new DallasTemperature(oneWire); 

    // arrays to hold device address 
    //DeviceAddress insideThermometer; 

    // locate devices on the bus 
    //Serial.print("Locating devices..."); 
    sensors->begin(); 
    //Serial.print("Found "); 
    //Serial.print(sensors->getDeviceCount(), DEC); 
    //Serial.println(" devices."); 

    if (sensors->getDeviceCount() == 0)
        {
        ErrMsg = "No Devices found on DS18B bus";
        FailSensor(DS18B_NOT_FOUND);
        return;
        }
        
    // report parasite power requirements 
    //Serial.print("Parasite power is: ");  
    //if (sensors->isParasitePowerMode()) Serial.println("ON"); 
    //else Serial.println("OFF"); 

    // Method 1: 
    // Search for devices on the bus and assign based on an index. Ideally, 
    // you would do this to initially discover addresses on the bus and then  
    // use those addresses and manually assign them (see above) once you know  
    // the devices on your bus (and assuming they don't change). 
    if (!sensors->getAddress(insideThermometer, 0)) Serial.println("Unable to find address for Device 0");  
    
  // show the addresses we found on the bus 
  //Serial.print("Device 0 Address: "); 
  //printAddress(insideThermometer); 
  //Serial.println(); 

  // set the resolution to 9 bit (Each Dallas/Maxim device is capable of several different resolutions) 
  sensors->setResolution(insideThermometer, 12);    // we want 12?
  
  //Serial.print("Device 0 Resolution: "); 
  //Serial.print(sensors->getResolution(insideThermometer), DEC);  
  //Serial.println(); 

  sensors->requestTemperatures(); // Start the first read  
    
}

// function to print a device address 
void CDS18BTempSensor::printAddress(DeviceAddress deviceAddress) 
{ 
  for (uint8_t i = 0; i < 8; i++) 
  { 
    if (deviceAddress[i] < 16) Serial.print("0"); 
    Serial.print(deviceAddress[i], HEX); 
  } 
}

bool CDS18BTempSensor::ReadSensor()
{
    bool readOK = true;

    if (!SensorAvailable) return(readOK);

    ErrMsg="";

    // Need to reverse these for better response? The first request is doen in init
    
    Value = sensors->getTempC(insideThermometer);
    sensors->requestTemperatures(); // Start the next read temperatures 

    if (readOK && UseForHeaterControl)
        {
        HeaterControl.HeaterOnOff(Value); 
        }
    return (readOK);  
}

void CDS18BTempSensor::GetHeader(char *buf)
{
    strcpy(buf, SensorName.c_str());
    Mstrcat(buf,",HeaterOn",TheLogger.MAXLOGLINELENGTH);
}

void CDS18BTempSensor::GetLogLine(char *buf)    
{
    // buf receives this field piece, like
    //  "25.45,Off"
    
    if ((strcmp(ErrMsg.c_str() , "") != 0) || !SensorAvailable)
        { // err - return empty string
        strcpy(buf, "        ,        ");  
        }
    else
        { // good read
        char locBuf[15];
        dtostrf(Value,8,2,buf);
        Mstrcat(buf,",",TheLogger.MAXLOGLINELENGTH);
        if (UseForHeaterControl)  
            {
            if (HeaterOn)
                Mstrcat(buf, "On      ",TheLogger.MAXLOGLINELENGTH);
            else
                Mstrcat(buf, "Off     ",TheLogger.MAXLOGLINELENGTH);
            }
        else
            Mstrcat(buf,"        ",TheLogger.MAXLOGLINELENGTH);
        }
}




/********************************************** 
 * DHT22 Temperature Sensor with option for heater control
 * If Temperature falls below 10C, turn on heater
 * If Temperature rises above 15, turn off heater
 * Set up for DHT22
 * Needs 2 sec delay between reads
 ***********************************************/  
void CDHTTempSensor::InitSensor()
{
    dht = new DHT(PinNum, DHTTYPE);  
    dht->begin();     
    
    UseForHeaterControl = false;

    // did it work? Try to read temperature
    double testVal = dht->readTemperature();
    if ( isnan(testVal) ) 
        {
        ErrMsg = "DHT sensor not found";
        FailSensor(DHT_NOT_FOUND);
        return;
        }

}
    
bool CDHTTempSensor::ReadSensor()
{
    bool readOK = true;

    if (!SensorAvailable) return(readOK);

    ErrMsg="";
    // For now, return hard-coded values. Need to read the values
    Value = dht->readTemperature(); // reads degC by default
    Humidity = dht->readHumidity();

    if ( isnan(Value) ) 
        {
        //ErrMsg = "Failed to read from DHT sensor!";
        Value = -273.0;
        readOK = false;
        }

    if (UseForHeaterControl)
        {
        HeaterControl.HeaterOnOff(Value);
        }
    return (readOK);  
}

void CDHTTempSensor::GetHeader(char *buf)
{
    strcpy(buf, SensorName.c_str());
    Mstrcat(buf,",HeaterOn",TheLogger.MAXLOGLINELENGTH);
    Mstrcat(buf,",DHTHumid",TheLogger.MAXLOGLINELENGTH);
}

void CDHTTempSensor::GetLogLine(char *buf)
{
    char buf2[12];
    if ((strcmp(ErrMsg.c_str() , "") != 0) || !SensorAvailable)
        { // err - return empty string
        strcpy(buf, "        ,        ,        ");  
        }
    else
        { // good read
        dtostrf(Value,8,2,buf);
        Mstrcat(buf,",",TheLogger.MAXLOGLINELENGTH);
        if (UseForHeaterControl)  
            {
            if (HeaterOn)
                Mstrcat(buf, "On      ",TheLogger.MAXLOGLINELENGTH);
            else
                Mstrcat(buf, "Off     ",TheLogger.MAXLOGLINELENGTH);
            }
        else
            Mstrcat(buf,"        ",TheLogger.MAXLOGLINELENGTH);
        dtostrf(Humidity,8,2,buf2);
        Mstrcat(buf,",",TheLogger.MAXLOGLINELENGTH);
        Mstrcat(buf,buf2,TheLogger.MAXLOGLINELENGTH);
        }
}
