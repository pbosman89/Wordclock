#include <stdbool.h>

#define SystemCoreClock 72000000

extern volatile bool USBConnected;

extern volatile unsigned int DisplayIntensity, DisplayIntensityABS;
extern volatile unsigned int DisplayIntensityMin, DisplayIntensityMax;
extern volatile unsigned int FrontLang;
extern volatile unsigned int AmPm;
extern volatile int DisplayMode, DisplayNotation;
extern void GetIntensity(void);

extern volatile unsigned int LiveColumnData[13];
extern volatile unsigned int NewColumnData[13];

extern volatile bool UploadAudioProcessing;
extern volatile int UploadAudioProcessingSize;

extern volatile bool AudioEnabled;
extern volatile bool DashEnabled;

//#define FrontLang 					0

#define LIGHT_MININTENSITY			5
#define LIGHT_MAXINTENSITY			400
#define LIGHT_FILTERSTRENGTH		4
#define LIGHT_STARTINTENSITY		100
#define LIGHT_UPDATEINTERVAL		2000

#define CAP_BASELINEDELAY			30
#define CAP_SHORTPRESS				10
#define CAP_LONGPRESS				160
#define CAP_HIGHTHRESHOLD			55
#define CAP_LOWTHRESHOLD			15
#define CAP_FILTERSTRENGTH			5

#define DISPLAYSTARTUPMODE			160
