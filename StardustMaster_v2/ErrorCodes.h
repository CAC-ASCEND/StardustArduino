#ifndef ERRORCODES_H
#define ERRORCODES_H

//PINS for LED indicators (error codes)
#define RED                   41    // red led indicator
#define GREEN                 43    // green led indicator
#define YELLOW                35    // yellow led indicator
#define BLUE                  5     // blue led indicator

// Error codes for flashing the Heartbeat LED
//OK will be solid green
#define DISK_NOT_FOUND    1 //solid red
#define GPS_NOT_FOUND     2 //flashing red
#define DHT_NOT_FOUND     3 //solid blue
#define BMP388_NOT_FOUND  4 //flashing blue
#define CO2_NOT_FOUND     5 //solid yellow
#define UV_NOT_FOUND      6 //flashing yellow
#define DS18B_NOT_FOUND   7 //flash green


extern void FlashStatusError(int i, char *s);
void FlashErrors(int numTimes);
void FlashPulses(int numPulses,int *pattern);
void FlashErrNumber(int errnum, int numFlashes);
void DisplayError(int errorCode);
void Blink(int lightToBlink);


// Pulse times for FlashStatusError
#define LONGPULSE       1000
#define SHORTPULSE      200
#define BREAKMSEC       300         // time between pulses
