


void InitLogger()
{
    if (digitalRead(PIN_DISKLOG) == HIGH)
        {
        char *errMsg = NULL;
        errMsg = TheLogger.Init(MyConfig.DataFileMsecBump, "Star", TheRTC, TheDisk);    // init using Star prefix
        if (errMsg)
            {
            // If the disk doesn't work we are wasting our time. Do a permanent
            // error display on the lights.
            DiskFailedLights(errMsg);
            } 
        }
    else
        {
        Serial.println("Skipping Disk Initialization (wire 22)"); 
        }
    // Check how much buffer is needed to hold data line for MaxSensors.
    // Assume 30 bytes per sensor (3 values, each 8 chars, plus 3 commas)
    // plus 30 for timestamps. Add extra 20 for fudge factor
    int bufNeeded = (MaxSensors +1)*30 + 20;
    TheLogger.MAXLOGLINELENGTH = bufNeeded;
    WriteCSVHeader();

}

/**********************
 * DiskFailedLights
 * Separate error flashing for the case of disk not initializing
 */
void DiskFailedLights(char *msg)
{
    int lights[3] = {GPS_FIX_ON, HEATER_LED, STATUS_LED};
    
    Serial.println(msg);
    while (1)
        {
        for (int i=0; i< 3; i++)
            {
            digitalWrite (lights[i], LED_ON);
            delay(50);
            digitalWrite (lights[i], LED_OFF);
            delay(50);  
            }
        delay(100);
        }
}
/***********************************************
 * WriteCSVHeader
 * 
 * Creates the next data file named StardutDatannn, where nnn is the next available number.
 * 
 * Writes the csv header information (column names) to the file
 */
void WriteCSVHeader()
{
    TheLogger.LogMsg(VERSION);      // Version info to the Error Log
    TheLogger.LogMsg(SENSOR_SET_NAME);      // Log Sensor set
    
    char csvHeader[TheLogger.MAXLOGLINELENGTH+1];
    Mstrcpy(csvHeader, "Timestamp,      Elapsed Time",TheLogger.MAXLOGLINELENGTH);
    char fieldBuf[50];                 // gets the Header piece (i.e., "FieldA"
    for (int i=0; i < MaxSensors; i++)
        {
        SensorArr[i]->GetHeader(fieldBuf);
        Mstrcat(csvHeader, ",",TheLogger.MAXLOGLINELENGTH);
        Mstrcat(csvHeader, fieldBuf,TheLogger.MAXLOGLINELENGTH);
        }
    TheLogger.WriteDataHeader(csvHeader);
}

/**********************************************            
 *  LogDisk
 *  Builds long string with all of the fields for the csv file
 *  123,456.3, etc
 */

boolean LogDisk()
{

    char fieldBuf[40];                 // gets the log piece (i.e., "1234.0"
    char logS[TheLogger.MAXLOGLINELENGTH+1];           // The entire log line "123,4325.0,1234"
              
    strcpy(logS, "");
    for (int i=0; i < MaxSensors; i++)
        {
        if(SensorArr[i]->ErrMsg != "")
            {
            TheLogger.LogMsg((char *)SensorArr[i]->ErrMsg.c_str());
            }
        SensorArr[i]->GetLogLine(fieldBuf);
        if (i > 0)
            Mstrcat(logS, ",",TheLogger.MAXLOGLINELENGTH);
        Mstrcat(logS, fieldBuf,TheLogger.MAXLOGLINELENGTH);
        }
    TheLogger.WriteDataFile(logS);
}
