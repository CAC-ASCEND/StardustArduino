#ifndef STARDUSTCONFIG_H
#define STARDUSTCONFIG_H

#include <CACLogger.h>
extern CLogger TheLogger;
#include <CACBoardDiff.h>

/*************************************************
 * Config class
 *  
 *  This holds various configuration parameters for the
 *  system. These parameters are loaded from the file
 *  Config.txt on the micro-SD disk. If this is missing,
 *  defaults are used.
 */

// define Config tokens here
// Need to be Upper case
#define SEALEVELPRESSURE "SEALEVELPRESSURE"
#define DATAFILEMSECBUMP "DATAFILEMSECBUMP"


class CStardustConfig
{
    public:
      void Init(CBrewmicroSD TheDisk);     // loads from the file

      double SeaLevelPressure;
      long DataFileMsecBump;

    private:
      void SetDefaults();           // Set defaults before loading file
      void LoadThisLine(char *buf);
};

extern CStardustConfig MyConfig;
#endif
