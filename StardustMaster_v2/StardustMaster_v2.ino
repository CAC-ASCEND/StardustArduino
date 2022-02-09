/********************************************************
 * StardustMaster
 * 
 * This sketch collects data from each sensor (expect 3-4?) and writes
 * the results to the MiscroSD card.
 * 
 * Wiring Notes:
 * 
 * - the GPS module expects the Arduino to talk to it 
 * through pins 7&8. However, the Mega does not use 7&8.
 * To make this work, I run jumpers from 18->7 and 19->8
 * (Tx/Rcv are switched so they can talk). The GPS switch 
 * is set to SoftSerial, but the setup uses Hardware Serial1
 * set to 
 * 
 * - The I2C runs through the I2C shield. This shield expects
 * SDA/SCLK to be on A4/A5. On the Mega these are on 21/21, so 
 * we run a pair of jumper wires from 20/21 to A4/A5.
 * =====> this mean we can not use A4 and A5 for analog data!
 * 
 * Changes:
 * 
 * 7/31/21 Brew Initial version 1.0
 * 12/9/21 Brew Replace LogDisk with an Object CLogger, using CRTC as the
 *              RTC clock object. CLogger and CRTC are in the library CACLogger
 * 
 *  TODO:  Test CLogger with date stamps
 *         Check if CardDetect works on the microSD card. If so, implement
 *         error check to verify card is present at startup.
 *         Change sensor code to check "pin" to use alternate I2C address
 */

// Update this when changes are made so you can verify the current version is
// running
#define VERSION "Stardust Master controller V 2.0"

// Set up array of sensor objects
#include "MySensor.h"
#include "Config.h"
#include <CACBoardDiff.h>
#include <MemoryFree.h>         // checking for memory leaks
unsigned int startFreeMemory = 0;
unsigned int curMemory = 0;

#include <BrewmicroSD.h>
#include <CACLogger.h>
CRTC TheRTC;
CBrewmicroSD TheDisk;
CLogger TheLogger;

// FreeMemory stuff to verify we do not have memory leak
//#define CHECK_FREE_MEMORY

/*********************************
 * setup()
 * 
 * This code runs once at startup of the Arduino.
 * This happens when a) the Arduino is powered on, or
 * b) when the Arduino Reset button is pressed
 */
void setup() {
    Serial.begin(115200);   // initialize Serial Console for debugging info
    Serial.println("************************");
    Serial.println(VERSION);
    
    pinMode(STATUS_LED, OUTPUT);
    pinMode(HEATER_LED, OUTPUT);
    pinMode(GPS_FIX_ON, OUTPUT);
    digitalWrite (STATUS_LED, LED_ON);      
    pinMode(PIN_DISKLOG, INPUT_PULLUP);
    
    char *errMsg = TheRTC.Init();
    if (errMsg)
        {
        Serial.print("RTC init err "); Serial.println(errMsg);
        }
    
    if (digitalRead(PIN_DISKLOG) == HIGH)
        {
        char *errMsg = NULL;
        errMsg = TheDisk.Init();    
        if (errMsg)
            {
            // If the disk doesn't work we are wasting our time. Do a permanent
            // error display on the lights.
            DiskFailedLights(errMsg);
            } 
        }
        
    MyConfig.Init(TheDisk);    // load config settings
    
    InitLogger();
    HeaterControl.Init();
    debug("SeaLevelPressure is ", MyConfig.SeaLevelPressure);
    debug("DataFileMsecBump is ", MyConfig.DataFileMsecBump);
    
    
    for (int i=0; i < MaxSensors; i++)
        {
        //Serial.print("Sensor Init ");Serial.println(SensorArr[i]->SensorName);
        SensorArr[i]->InitSensor();
        }
    // If a temperature sensor is used for heater control, uncomment the next line
    //InternTempSensor.UseForHeaterControl = true;

    // Flash any error messages
    FlashErrors(2);     // flash 2 times

#ifdef CHECK_FREE_MEMORY
#ifndef ARDUINO_SAMD_ZERO
    startFreeMemory = freeMemory();    // initial memory
    Serial.print("Starting Free Memory ");Serial.println(startFreeMemory);
#endif
#endif
}

void loop() {
    digitalWrite (STATUS_LED, LED_ON);      
    for (int i=0; i < MaxSensors; i++)
        {
        //Serial.print("Read Sensor ");Serial.println(SensorArr[i]->SensorName);
        SensorArr[i]->ReadSensor();
        }
    LogDisk();
    digitalWrite (STATUS_LED, LED_OFF);      
    
    // Checking for memory leaks
#ifdef CHECK_FREE_MEMORY
#ifndef ARDUINO_SAMD_ZERO
    curMemory = freeMemory();
    if (curMemory != startFreeMemory)
        {
        Serial.print("Free Memory ");Serial.println(curMemory);
        }
#endif    
#endif    
    
    delay(2000);
}



/*****************************
 * FlashStatusError
 * Utility function to flash the heartbeat LED in a pattern
 * when an error occurs. This would typically be something like 
 * a sensor is not found, or the disk didn't initialize right
 * The user selects the desired pattern when calling this function
 * from the list above
 */

// Hold a list of error code to flash
#define MAX_ERROR_FLASH 10            // max number of errors which can flash
int errCodes[MAX_ERROR_FLASH];
int MaxFlashCount = 0;

void FlashStatusError(int whichErr, char *msg)
 {
  // Add to errCodes List

    TheLogger.LogMsg(msg);
    if (MaxFlashCount < MAX_ERROR_FLASH)
        {
        errCodes[MaxFlashCount] = whichErr;
        MaxFlashCount++;
        }
 }

void FlashErrors(int numTimes)
{
    for (int loopCnt=0; loopCnt < numTimes; loopCnt++)
        {
        for (int i = 0; i < MaxFlashCount; i++)
            {
            FlashErrNumber(errCodes[i], 10);
            }
        }
}

void FlashPulses(int numPulses,int *pattern)
{
    for (int i=0; i < numPulses; i++)
        {
        digitalWrite (STATUS_LED, LED_ON);
        delay(pattern[i]);
        digitalWrite (STATUS_LED, LED_OFF);
        delay(BREAKMSEC);  
        }
    delay(BREAKMSEC*3);  
}

void FlashErrNumber(int errnum, int numFlashes)
{
    // Using the 3 status LEDs, rapidly flash the errnum 1-7 in binary.
    // Flash for a short time (1-2 seconds?) 
    for (int i=0; i< numFlashes; i++)
        {
        digitalWrite(GPS_FIX_ON, errnum & 1? HIGH : LOW);  // Blue Light  
        digitalWrite(HEATER_LED, errnum & 2? HIGH : LOW);  // Red Light 
        digitalWrite(STATUS_LED, errnum & 4? HIGH : LOW);  // Yellow Light 
        delay(50);
        digitalWrite(GPS_FIX_ON, LOW);  // Blue Light  
        digitalWrite(HEATER_LED, LOW);  // Red Light 
        digitalWrite(STATUS_LED, LOW);
        delay(50);
        }
    delay (3*BREAKMSEC);
}


/**************************************
 * String functions
 * I keep running into problems where strcpy, strcat
 * overrun buffers
 * These are functions to avoid that
 *******************************/

// lim should be max length of target
void Mstrcpy(char *target, char *src, int tarLim)
{
    if (strlen(src) < tarLim)          
        { // OK to copy
        strcpy (target, src);
        }
    else
        {
        Serial.println("    >>> Mstrcpy: target is not large enough to hold src");
        Serial.print("    >>> src is "); Serial.print(src);Serial.print(" Limit is ");Serial.println(tarLim);
        }
}

void Mstrcat(char *target, char *src, int tarLim)
{
    if ((strlen(target) + strlen(src)) < tarLim)          
        { // OK to cat
        strcat (target, src);
        }
    else
        {
        Serial.println("    >>> Mstrcat: target is not large enough to hold src");
        Serial.print("    >>> target is "); Serial.print(target);
        Serial.print("    >>> src is "); Serial.println(src);
        Serial.print("    >>> Limit is ");Serial.println(tarLim);
        }
  
}
