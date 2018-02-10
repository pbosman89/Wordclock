#include <type.h>
#include <stdbool.h>
#include "DisplayModes.h"
#include "ScreenFunctions.h"
#include "projectconfig.h"
#include "DisplayGraphics.h"

bool DisplayTime(int Hr, int Min, int Sec, int Mode, int Notation){

	static int PrevSec, PrevMin, PrevHour;
	static int PrevMode;
	static bool TransitionRunning=FALSE;
	static bool LoadFromStart=TRUE;

	//Check if current mode is display mode, otherwise do nothing and return
	if(Mode < 100 || Mode > 200)
		return FALSE;

	//Is there is a transaction running, wait until it is finished before going to the next.
	if(TransitionRunning == TRUE)
		Mode = PrevMode;

	//Set AM or PM
	if(Hr >= 12){
		AmPm = 1;
	}
	else{
		AmPm=0;
	}
	//Convert from 0-23 to 1-12
	if(Hr > 12){
		Hr = Hr - 12;
	}
	else if(Hr == 0){
		Hr = 12;
	}

	//Start animation if the mode changed, or time requires updating
	if((TransitionRunning == TRUE) ||
	  (PrevHour != Hr) ||
	  (Mode != PrevMode) ||
	  ((Notation == 1) && (Min != PrevMin)) ||
	  ((Notation == 5) && ((PrevMin/5) != (Min/5)))){

		static int ShowMode;
		if((Mode == PrevMode) && (Mode == 170)){
			if(!TransitionRunning){
				ShowMode = (xorshift16() / 10922 + 11) * 10;
				CommSendString("Random Mode: ");
				char str[3];
				itoa(ShowMode, str);
				CommSendString(str);
				CommSendString("\r\n");
			}
		}
		else {
			ShowMode = Mode;
		}

		switch(ShowMode){
			case 110:
				TransitionRunning = FrontpanelMode_Basic(Hr, Min, Sec, Notation);
				break;
			case 120:
				TransitionRunning = FrontpanelMode_FallingStars(Hr, Min, Sec, Notation);
				break;
			case 130:
				TransitionRunning = FrontpanelMode_HorizontalSlideLeft(Hr, Min, Sec, Notation);
				break;
			case 140:
				TransitionRunning = FrontpanelMode_HorizontalSlideRight(Hr, Min, Sec, Notation);
				break;
			case 150:
				TransitionRunning = FrontpanelMode_DissolveOutIn(Hr, Min, Sec, Notation);
				break;
			case 160:
				TransitionRunning = FrontpanelMode_DissolveCross(Hr, Min, Sec, Notation);
				break;
			case 170:
				//Random mode introduction screen
				TransitionRunning = FrontpanelMode_RandomIntro(Hr, Min, Sec, Notation);
				break;
			default:
				TransitionRunning = FrontpanelMode_Basic(Hr, Min, Sec, Notation);
				break;
		}
	}

	//Update just the DOTS
	else if((Sec != PrevSec && (Sec == 0 || Sec == 15 || Sec == 30 || Sec == 45)) || (Min != PrevMin)){
		ClearScreen();
		Frontpanel_DefaultText(Hr, Min, Sec, Notation, FALSE, FALSE);
		Frontpanel_DOTS(Hr, Min, Sec, Notation);
		SetScreen();
	}

	PrevSec = Sec;
	PrevMin = Min;
	PrevHour = Hr;
	PrevMode = Mode;


	if(LoadFromStart == TRUE && TransitionRunning == FALSE){
		LoadFromStart = FALSE;
		DisplayMode = DISPLAYSTARTUPMODE;
		PrevMode = DISPLAYSTARTUPMODE;
	}

	//If transition running then notify calling function(so other actions can be blocked)
	if(TransitionRunning)
		return TRUE;
	else
		return FALSE;
}
