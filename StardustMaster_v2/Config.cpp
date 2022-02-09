/**************************************
 * Implementation of CStardustConfig
 * 
 * NOTE - the SD.begin() should have already been done
 * in InitDisk before this is called
 */

#include <SPI.h>
#include <SdFat.h>

#include "Config.h"

#define DEFAULTSEALEVELPRESSURE_HPA (1013.25)   // if we can't get from Config.txt
#define DEFAULTDATAFILEMSECBUMP (600000)        // if we can't get from Config.txt
#define CONFIGFILE "StardustConfig.txt"         // file name on SD disk


void CStardustConfig::Init(CBrewmicroSD TheDisk)
{
    SetDefaults();

    // load the file
    char *errMsg = TheDisk.OpenFile(CONFIGFILE);
    if (errMsg)
        {
        TheLogger.LogMsg(errMsg);
        }
    else
        { // open OK, read the file
        char line[201];
        TheLogger.LogMsg("Loading StardustConfig.txt file");
        while (TheDisk.ReadLine(line, 200))   // line is trimmed
            {
            LoadThisLine(line);
            }
        }
}

void CStardustConfig::SetDefaults()
{
    SeaLevelPressure = DEFAULTSEALEVELPRESSURE_HPA;
    DataFileMsecBump = DEFAULTDATAFILEMSECBUMP;
}


void CStardustConfig::LoadThisLine(char *buf)
{
    char logBuf[100];
    if (strlen(buf) == 0)
        return;
    if (buf[0] == '#')
        return;
    
    // OK, we might have a line like SeaLevelPressure  =  101.34
    String d = strtok(buf, "=");
    d.trim();
    d.toUpperCase();
    //debug("d is ",d);
    if (d == SEALEVELPRESSURE) 
        {
        d = strtok(NULL, "=");
        SeaLevelPressure = d.toFloat();

        // display/log message
        strcpy(logBuf,"Config:  Loaded ");
        strcat(logBuf, SEALEVELPRESSURE);
        strcat(logBuf, " with ");
        char tempbuf[15];
        dtostrf(SeaLevelPressure,8,2,tempbuf);
        strcat(logBuf, tempbuf);
        TheLogger.LogMsg(logBuf);
        }
    // OK, we might have a line like DataFileMSecBump  =  600000
    else if (d == DATAFILEMSECBUMP) 
        {
        d = strtok(NULL, "=");
        DataFileMsecBump = d.toInt();

        // display/log message
        strcpy(logBuf,"Config:  Loaded ");
        strcat(logBuf, DATAFILEMSECBUMP);
        strcat(logBuf, " with ");
        char tempbuf[15];
        ltoa(DataFileMsecBump,tempbuf,15);
        strcat(logBuf, tempbuf);
        TheLogger.LogMsg(logBuf);
        }
}



CStardustConfig MyConfig;
