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

void FlashStatusError(int whichErr, char *msg) {
  // Add to errCodes List

    TheLogger.LogMsg(msg);
    if (MaxFlashCount < MAX_ERROR_FLASH)
        {
        errCodes[MaxFlashCount] = whichErr;
        MaxFlashCount++;
        }
 }

void FlashErrors(int numTimes) {
    for (int loopCnt=0; loopCnt < numTimes; loopCnt++) {
        for (int i = 0; i < MaxFlashCount; i++) {
            FlashErrNumber(errCodes[i], 10);
            }
        }
}

void FlashPulses(int numPulses,int *pattern) {
    for (int i=0; i < numPulses; i++) {
        digitalWrite (STATUS_LED, LED_ON);
        delay(pattern[i]);
        digitalWrite (STATUS_LED, LED_OFF);
        delay(BREAKMSEC);  
        }
    delay(BREAKMSEC*3);  
}

void FlashErrNumber(int errnum, int numFlashes) {
    // Using the 4 status LEDs, flash the errnum 0-7 by color.
    // Flash for a short time (1-2 seconds?)
    for (int i=0; i< numFlashes; i++) {
          DisplayError(int errorCode);
    }
    delay (3*BREAKMSEC);
}

void DisplayError(int errorCode){
  switch(errorCode){
    case 1: digitalWrite(RED, HIGH);
            break;
    case 2: Blink(RED);
            break;
    case 3: digitalWrite(BLUE, HIGH);
            break;
    case 4: Blink(BLUE);
            break;
    case 5: digitalWrite(YELLOW, HIGH);
            break;
    case 6: digitalWrite(YELLOW);
            break;
    case 7: Blink(GREEN);
            break;
    default: digitalWrite(GREEN, HIGH);
  }
}

void Blink(int lightToBlink){
  for(int i=0; i<3; i++){
    digitalWrite(lightToBlink, HIGH);
    delay(SHORTPULSE);
    digitalWrite(lightToBlink, LOW);
    delay(SHORTPULSE);
    digitalWrite(lightToBlink, HIGH);
    delay(SHORTPULSE);
    digitalWrite(lightToBlink, LOW);
    delay(SHORTPULSE);    
  }

}
