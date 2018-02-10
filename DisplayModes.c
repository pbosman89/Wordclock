#include "gpio.h"
#include "projectconfig.h"
#include "ScreenFunctions.h"
#include "DisplayModes.h"
#include "cdcuser.h"
#include "inttypes.h"
#include "flashmemory.h"

#include <type.h>
#include <stdbool.h>

volatile int TransSpeedFactor = 100;

struct Front FrontPanel[2] = {
		{ //------>>> NEDERLANDS
			.Text.HET.X = 0, 11, 3,
			.Text.IS.X = 4, 11, 2,
			.Text.KWART.X = 0, 6, 5,
			.Text.VOOR.X = 9, 5, 4,
			.Text.OVER.X = 0, 4, 4,
			.Text.UUR.X = 10, 0, 3,
			.Text.HALF.X = 5, 4, 4,

			.Minute[0].X = 0, 0, 0,		//OFF
			.Minute[1].X = 7, 11, 3,	//een
			.Minute[2].X = 0, 10, 4,	//twee
			.Minute[3].X = 4, 10, 4,
			.Minute[4].X = 9, 10, 4,
			.Minute[5].X = 0, 9, 4,
			.Minute[6].X = 4, 9, 3,
			.Minute[7].X = 7, 9, 5,
			.Minute[8].X = 0, 8, 4,
			.Minute[9].X = 7, 8, 5,
			.Minute[10].X = 0, 7, 4,
			.Minute[11].X = 4, 7, 3,
			.Minute[12].X = 7, 7, 6,
			.Minute[13].X = 6, 6, 7,	//dertien
			.Minute[14].X = 0, 5, 8,	//veertien

			.Hour[0].X = 0, 0, 0,		//OFF
			.Hour[1].X = 10, 4, 3,		//een
			.Hour[2].X = 0, 3, 4,		//twee
			.Hour[3].X = 5, 3, 4,
			.Hour[4].X = 9, 3, 4,
			.Hour[5].X = 0, 2, 4,
			.Hour[6].X = 4, 2, 3,
			.Hour[7].X = 8, 2, 5,
			.Hour[8].X = 0, 1, 4,
			.Hour[9].X = 4, 1, 5,
			.Hour[10].X = 9, 1, 4,
			.Hour[11].X = 0, 0, 3,		//elf
			.Hour[12].X = 3, 0, 6,		//twaalf
		},
		{ //----->>> ENGELS
			.Text.HET.X = 0, 11, 2,		//HIT
			.Text.IS.X = 3, 11, 2,
			.Text.A.X = 0, 5, 1,
			.Text.KWART.X = 2, 5, 7,	//QUARTER
			.Text.VOOR.X = 10, 5, 2,	//TO
			.Text.OVER.X = 0, 4, 4,		//PAST
			.Text.UUR.X = 4, 0, 6,		//O'CLOCK
			.Text.HALF.X = 9, 6, 4,
			.Text.AM.X = 11, 0, 1,
			.Text.PM.X = 12, 0, 1,
			.Text.DASH1.X = 4, 6, 1,	//-(TEEN)
			.Text.TEEN.X = 5, 6, 4,		//TEEN
			.Text.TWENTY.X = 0, 10, 6,	//TWENTY
			.Text.DASH2.X = 6, 10, 1,	//(TWENTY)-

			.Minute[0].X = 0, 0, 0,		//OFF
			.Minute[1].X = 7, 10, 3,	//ONE
			.Minute[2].X = 4, 9, 3,		//TWO
			.Minute[3].X = 5, 7, 5,
			.Minute[4].X = 5, 8, 4,
			.Minute[5].X = 0, 9, 4,
			.Minute[6].X = 10, 7, 3,
			.Minute[7].X = 0, 8, 5,
			.Minute[8].X = 1, 7, 5,
			.Minute[9].X = 9, 8, 4,
			.Minute[10].X = 10, 10, 3,
			.Minute[11].X = 6, 11, 6,
			.Minute[12].X = 7, 9, 6,
			.Minute[13].X = 0, 6, 4,	//THIR
			.Minute[14].X = 5, 8, 4,	//FOUR

			.Hour[0].X = 0, 0, 0,		//OFF
			.Hour[1].X = 10, 2, 3,		//ONE
			.Hour[2].X = 0, 0, 3,		//TWO
			.Hour[3].X = 3, 1, 5,
			.Hour[4].X = 9, 3, 4,
			.Hour[5].X = 9, 4, 4,
			.Hour[6].X = 0, 1, 3,
			.Hour[7].X = 8, 1, 5,
			.Hour[8].X = 5, 2, 5,
			.Hour[9].X = 5, 4, 4,
			.Hour[10].X = 6, 3, 3,
			.Hour[11].X = 0, 3, 6,		//ELEVEN
			.Hour[12].X = 0, 2, 6,		//TWELVE
		}
};


// returns values from 1 to 65535 inclusive, period is 65535
uint16_t xorshift16(void) {
	static uint16_t y16 = 1;

	y16 ^= (y16 << 13);
    y16 ^= (y16 >> 9);
    return y16 ^= (y16 << 7);
}

uint8_t xorshift8(void) {
	static uint8_t y8 = 1;

    y8 ^= (y8 << 7);
    y8 ^= (y8 >> 5);
    return y8 ^= (y8 << 3);
}

//TODO: General DOT mode function
//TODO: Add DOT mode which blinks the corresponding DOT the last 5 seconds before switch
void Frontpanel_DOTS(int Hours, int Minutes, int Seconds, int Notation){
	int HighDot=0;

	//0x1000 = left bottom
	//0x2000 = left top
	//0x4000 = right top
	//0x8000 = right bottom

	//Add Dots in corners
	if(Notation < 5){
		if(Seconds < 15){HighDot = 0x2000;}	else
		if(Seconds < 30){HighDot = 0x1000;} else
		if(Seconds < 45){HighDot = 0x8000;} else
		if(Seconds < 60){HighDot = 0x4000;}
	}
	else{
		int temp;
		temp = (Minutes/5); //19minutes / 5 = 3
		temp = (Minutes * 10 / 5) - temp * 10;

		//Temp now contains: 0, 2, 4, 6, 8
		if(temp == 2){HighDot = 0x2000;} else
		if(temp == 4){HighDot = 0x1000;} else
		if(temp == 6){HighDot = 0x8000;} else
		if(temp == 8){HighDot = 0x4000;}
	}

	//Add DOT to data
	//Only add to one row to keep same intensity as other LED's
	NewColumnData[12] |= HighDot;
}

void Frontpanel_DefaultText(int Hours, int Minutes, int Seconds, int Notation, bool MinutesOFF, bool HoursOFF){

	//Check if 5 minute notation
	if(Notation >= 5){
		if(Minutes <5) {Minutes = 0;} else
		if(Minutes <10) {Minutes = 5;} else
		if(Minutes <15) {Minutes = 10;} else
		if(Minutes <20) {Minutes = 15;} else
		if(Minutes <25) {Minutes = 20;} else
		if(Minutes <30) {Minutes = 25;} else
		if(Minutes <35) {Minutes = 30;} else
		if(Minutes <40) {Minutes = 35;} else
		if(Minutes <45) {Minutes = 40;} else
		if(Minutes <50) {Minutes = 45;} else
		if(Minutes <55) {Minutes = 50;} else
		if(Minutes <60) {Minutes = 55;}
	}

	//Add HET and IS
	Coordinates2Screen(FrontPanel[FrontLang].Text.HET.X, FrontPanel[FrontLang].Text.HET.Y, FrontPanel[FrontLang].Text.HET.Chars);
	Coordinates2Screen(FrontPanel[FrontLang].Text.IS.X, FrontPanel[FrontLang].Text.IS.Y, FrontPanel[FrontLang].Text.IS.Chars);

	//Add Minutes to screen
	if(Minutes == 0 && !MinutesOFF)
		Coordinates2Screen(FrontPanel[FrontLang].Text.UUR.X, FrontPanel[FrontLang].Text.UUR.Y, FrontPanel[FrontLang].Text.UUR.Chars);

	if(!MinutesOFF){
		if((Minutes > 0) && (Minutes < 15)){
			Coordinates2Screen(FrontPanel[FrontLang].Minute[(Minutes)].X, FrontPanel[FrontLang].Minute[(Minutes)].Y, FrontPanel[FrontLang].Minute[(Minutes)].Chars);
			Coordinates2Screen(FrontPanel[FrontLang].Text.OVER.X, FrontPanel[FrontLang].Text.OVER.Y, FrontPanel[FrontLang].Text.OVER.Chars);
			if((Minutes == 13 || Minutes == 14) && FrontLang==1){ //13 or 14 over
				Coordinates2Screen(FrontPanel[FrontLang].Text.TEEN.X, FrontPanel[FrontLang].Text.TEEN.Y, FrontPanel[FrontLang].Text.TEEN.Chars);
				if(Minutes != 40 && DashEnabled){
					Coordinates2Screen(FrontPanel[FrontLang].Text.DASH1.X, FrontPanel[FrontLang].Text.DASH1.Y, FrontPanel[FrontLang].Text.DASH1.Chars);
				}
			}
		}

		else if(Minutes > 15 && Minutes < 30 && FrontLang==0){ //xx voor half
			Coordinates2Screen(FrontPanel[FrontLang].Minute[(30-Minutes)].X, FrontPanel[FrontLang].Minute[(30-Minutes)].Y, FrontPanel[FrontLang].Minute[(30-Minutes)].Chars);
			Coordinates2Screen(FrontPanel[FrontLang].Text.VOOR.X, FrontPanel[FrontLang].Text.VOOR.Y, FrontPanel[FrontLang].Text.VOOR.Chars);
			Coordinates2Screen(FrontPanel[FrontLang].Text.HALF.X, FrontPanel[FrontLang].Text.HALF.Y, FrontPanel[FrontLang].Text.HALF.Chars);			
		}

		else if(Minutes > 15 && Minutes < 20 && FrontLang==1){ //1x over
			Coordinates2Screen(FrontPanel[FrontLang].Minute[(Minutes-10)].X, FrontPanel[FrontLang].Minute[(Minutes-10)].Y, FrontPanel[FrontLang].Minute[(Minutes-10)].Chars);
			Coordinates2Screen(FrontPanel[FrontLang].Text.TEEN.X, FrontPanel[FrontLang].Text.TEEN.Y, FrontPanel[FrontLang].Text.TEEN.Chars);
			if(Minutes != 40 && DashEnabled){
				Coordinates2Screen(FrontPanel[FrontLang].Text.DASH1.X, FrontPanel[FrontLang].Text.DASH1.Y, FrontPanel[FrontLang].Text.DASH1.Chars);
			}
			Coordinates2Screen(FrontPanel[FrontLang].Text.OVER.X, FrontPanel[FrontLang].Text.OVER.Y, FrontPanel[FrontLang].Text.OVER.Chars);
		}
		
		else if(Minutes >= 20 && Minutes < 30 && FrontLang==1){ //2x over
			Coordinates2Screen(FrontPanel[FrontLang].Minute[(Minutes-20)].X, FrontPanel[FrontLang].Minute[(Minutes-20)].Y, FrontPanel[FrontLang].Minute[(Minutes-20)].Chars);
			Coordinates2Screen(FrontPanel[FrontLang].Text.TWENTY.X, FrontPanel[FrontLang].Text.TWENTY.Y, FrontPanel[FrontLang].Text.TWENTY.Chars);
			if(Minutes != 20 && DashEnabled){
				Coordinates2Screen(FrontPanel[FrontLang].Text.DASH2.X, FrontPanel[FrontLang].Text.DASH2.Y, FrontPanel[FrontLang].Text.DASH2.Chars);
			}
			Coordinates2Screen(FrontPanel[FrontLang].Text.OVER.X, FrontPanel[FrontLang].Text.OVER.Y, FrontPanel[FrontLang].Text.OVER.Chars);
		}
		
		else if(Minutes > 30 && Minutes < 45 && FrontLang==0){
			Coordinates2Screen(FrontPanel[FrontLang].Minute[(Minutes-30)].X, FrontPanel[FrontLang].Minute[(Minutes-30)].Y, FrontPanel[FrontLang].Minute[(Minutes-30)].Chars);
			Coordinates2Screen(FrontPanel[FrontLang].Text.OVER.X, FrontPanel[FrontLang].Text.OVER.Y, FrontPanel[FrontLang].Text.OVER.Chars);
			Coordinates2Screen(FrontPanel[FrontLang].Text.HALF.X, FrontPanel[FrontLang].Text.HALF.Y, FrontPanel[FrontLang].Text.HALF.Chars);
		}

		else if(Minutes > 30 && Minutes <= 40 && FrontLang==1){ //2x voor
			Coordinates2Screen(FrontPanel[FrontLang].Text.TWENTY.X, FrontPanel[FrontLang].Text.TWENTY.Y, FrontPanel[FrontLang].Text.TWENTY.Chars);
			if(Minutes != 40 && DashEnabled){
				Coordinates2Screen(FrontPanel[FrontLang].Text.DASH2.X, FrontPanel[FrontLang].Text.DASH2.Y, FrontPanel[FrontLang].Text.DASH2.Chars);
			}
			Coordinates2Screen(FrontPanel[FrontLang].Minute[(40-Minutes)].X, FrontPanel[FrontLang].Minute[(40-Minutes)].Y, FrontPanel[FrontLang].Minute[(40-Minutes)].Chars);
			Coordinates2Screen(FrontPanel[FrontLang].Text.VOOR.X, FrontPanel[FrontLang].Text.VOOR.Y, FrontPanel[FrontLang].Text.VOOR.Chars);
		}
		
		else if(Minutes > 40 && Minutes < 45 && FrontLang==1){ //1x voor
			Coordinates2Screen(FrontPanel[FrontLang].Minute[(50-Minutes)].X, FrontPanel[FrontLang].Minute[(50-Minutes)].Y, FrontPanel[FrontLang].Minute[(50-Minutes)].Chars);
			Coordinates2Screen(FrontPanel[FrontLang].Text.TEEN.X, FrontPanel[FrontLang].Text.TEEN.Y, FrontPanel[FrontLang].Text.TEEN.Chars);
			if(DashEnabled){
				Coordinates2Screen(FrontPanel[FrontLang].Text.DASH1.X, FrontPanel[FrontLang].Text.DASH1.Y, FrontPanel[FrontLang].Text.DASH1.Chars);
			}
			Coordinates2Screen(FrontPanel[FrontLang].Text.VOOR.X, FrontPanel[FrontLang].Text.VOOR.Y, FrontPanel[FrontLang].Text.VOOR.Chars);
		}		

		else if(Minutes > 45){ //xx voor heel
			Coordinates2Screen(FrontPanel[FrontLang].Minute[(60-Minutes)].X, FrontPanel[FrontLang].Minute[(60-Minutes)].Y, FrontPanel[FrontLang].Minute[(60-Minutes)].Chars);
			Coordinates2Screen(FrontPanel[FrontLang].Text.VOOR.X, FrontPanel[FrontLang].Text.VOOR.Y, FrontPanel[FrontLang].Text.VOOR.Chars);
			if((Minutes == 46 || Minutes == 47) && FrontLang==1){ //46 or 47 over
				Coordinates2Screen(FrontPanel[FrontLang].Text.TEEN.X, FrontPanel[FrontLang].Text.TEEN.Y, FrontPanel[FrontLang].Text.TEEN.Chars);
				if(DashEnabled){
					Coordinates2Screen(FrontPanel[FrontLang].Text.DASH1.X, FrontPanel[FrontLang].Text.DASH1.Y, FrontPanel[FrontLang].Text.DASH1.Chars);
				}
			}
		}

		else if(Minutes == 15){ //Kwart over
			if(FrontLang == 1){
				Coordinates2Screen(FrontPanel[FrontLang].Text.A.X, FrontPanel[FrontLang].Text.A.Y, FrontPanel[FrontLang].Text.A.Chars);
			}
			Coordinates2Screen(FrontPanel[FrontLang].Text.KWART.X, FrontPanel[FrontLang].Text.KWART.Y, FrontPanel[FrontLang].Text.KWART.Chars);
			Coordinates2Screen(FrontPanel[FrontLang].Text.OVER.X, FrontPanel[FrontLang].Text.OVER.Y, FrontPanel[FrontLang].Text.OVER.Chars);
		}

		else if(Minutes == 30){ //Half
			Coordinates2Screen(FrontPanel[FrontLang].Text.HALF.X, FrontPanel[FrontLang].Text.HALF.Y, FrontPanel[FrontLang].Text.HALF.Chars);
			if(FrontLang == 1){
				Coordinates2Screen(FrontPanel[FrontLang].Text.OVER.X, FrontPanel[FrontLang].Text.OVER.Y, FrontPanel[FrontLang].Text.OVER.Chars);
			}
		}

		else if(Minutes == 45){ //Kwart voor
			if(FrontLang == 1){
				Coordinates2Screen(FrontPanel[FrontLang].Text.A.X, FrontPanel[FrontLang].Text.A.Y, FrontPanel[FrontLang].Text.A.Chars);
			}
			Coordinates2Screen(FrontPanel[FrontLang].Text.KWART.X, FrontPanel[FrontLang].Text.KWART.Y, FrontPanel[FrontLang].Text.KWART.Chars);
			Coordinates2Screen(FrontPanel[FrontLang].Text.VOOR.X, FrontPanel[FrontLang].Text.VOOR.Y, FrontPanel[FrontLang].Text.VOOR.Chars);
		}
	}

	if(!HoursOFF){
		int NL_ENG = 15;
		if(FrontLang == 1){
			NL_ENG = 30;
		}
		
		//Add hours to screen
		if(Minutes <=NL_ENG){ //Display current hour
			Coordinates2Screen(FrontPanel[FrontLang].Hour[(Hours)].X, FrontPanel[FrontLang].Hour[(Hours)].Y, FrontPanel[FrontLang].Hour[(Hours)].Chars);
		}
		else if(Hours < 12 && Minutes > NL_ENG){ //Display next hour
			Coordinates2Screen(FrontPanel[FrontLang].Hour[(Hours+1)].X, FrontPanel[FrontLang].Hour[(Hours+1)].Y, FrontPanel[FrontLang].Hour[(Hours+1)].Chars);
		}
		else if(Hours == 12 && Minutes >NL_ENG){ //Display 1
			Coordinates2Screen(FrontPanel[FrontLang].Hour[1].X, FrontPanel[FrontLang].Hour[1].Y, FrontPanel[FrontLang].Hour[1].Chars);
		}
		if(FrontLang == 1){
			if(AmPm == 1){
				Coordinates2Screen(FrontPanel[FrontLang].Text.PM.X, FrontPanel[FrontLang].Text.PM.Y, FrontPanel[FrontLang].Text.PM.Chars);
			}
			else{
				Coordinates2Screen(FrontPanel[FrontLang].Text.AM.X, FrontPanel[FrontLang].Text.AM.Y, FrontPanel[FrontLang].Text.AM.Chars);
			}
		}
	}
}


bool FrontpanelMode_FallingStars(int Hours, int Minutes, int Seconds, int Notation){

	static int RunTransition;
	static int ColumnOverlay[13];
	static int BaseColumnData[13];

	static int TransitionDelay = 0;

	int i;

	if(RunTransition == 0){
		RunTransition = 24; //12 rows, run 2 times
		TransitionDelay = 4000 * TransSpeedFactor / 100;

		//Store original to use as base for first overlay transition
		for(i=0; i<13; i++){
			BaseColumnData[i] = LiveColumnData[i] & 0x0FFF;
			ColumnOverlay[i] = 0;
		}

	}
	else{
		if(TransitionDelay-- > 0)
			return TRUE;
		else
			TransitionDelay = 4000 * TransSpeedFactor / 100;
	}


	RunTransition--;
	ClearScreen();

	//Fill screen(scroll down) with random points
	if(RunTransition > 12){

		for(i=0; i<13; i++){
			int Temp = (xorshift16() & 0x01) + (xorshift16() & 0x01);
			if(Temp == 0x02)
				ColumnOverlay[i] |= 1<<11;

			//Empty top row when touched by a LED
			if(ColumnOverlay[i] & (1<<(RunTransition-12)))
				BaseColumnData[i] &= ~(1<<(RunTransition-12));

			//Combine the two images
			NewColumnData[i] = ColumnOverlay[i] | BaseColumnData[i];

			//Shift one down to prepare for next
			ColumnOverlay[i] = ColumnOverlay[i] >>1;
		}
	}
	//Reveal new time(scroll falling stars off screen)
	else if(RunTransition > 0){

		//Get new screen data
		Frontpanel_DefaultText(Hours, Minutes, Seconds, Notation, FALSE, FALSE);

		for(i=0; i<13; i++){
			//Hide all but new row (right shift the bytes gone and shift back)
			NewColumnData[i] = (NewColumnData[i]>>(RunTransition-1)) <<(RunTransition-1);

			//Combine the two images
			NewColumnData[i] |= ColumnOverlay[i];

			//Shift column overlay once
			ColumnOverlay[i] = ColumnOverlay[i] >>1;
		}

	}
	else if(RunTransition == 0){
		Frontpanel_DefaultText(Hours, Minutes, Seconds, Notation, FALSE, FALSE);
	}

	//Add dots
	Frontpanel_DOTS(Hours, Minutes, Seconds, Notation);

	SetScreen();

	//Notify if transition state
	if(RunTransition)
		return TRUE;
	else
		return FALSE;
}

//Slide out old time and slide in new time right to left
bool FrontpanelMode_HorizontalSlideLeft(int Hours, int Minutes, int Seconds, int Notation){

	static int RunTransition;
	static int BaseColumnData[26];

	static int TransitionDelay = 0;

	int i=0;

	if(RunTransition == 0){
		RunTransition = 13; //13 columns shift
		TransitionDelay = 8000 * TransSpeedFactor / 100;

		//Retrieve new data -> into NewColumnData
		ClearScreen();
		Frontpanel_DefaultText(Hours, Minutes, Seconds, Notation, FALSE, FALSE);

		//Store old and new display data together
		for(i=0; i<13; i++){
			BaseColumnData[i] = LiveColumnData[i] & 0x0FFF;
			BaseColumnData[i+13] = NewColumnData[i];
		}

	}

	//Add transition delay
	if(RunTransition > 0){
		if(TransitionDelay-- > 0)
			return TRUE; //still running
		else{
			if(RunTransition > 7) //Speed up transition
				TransitionDelay = 8000/((13-RunTransition+1)*75/100) * TransSpeedFactor / 100;
			else //Slow down transition
				TransitionDelay = 8000/((RunTransition)*75/100) * TransSpeedFactor / 100;
		}

		RunTransition--;

		//Now start shifting

		//Shift all data to the right, minus the last one
		//Remove dots
		for(i=0; i<25; i++){
			BaseColumnData[i] = BaseColumnData[i+1] & 0x0FFF;
		}

		for(i=0; i<13; i++){
			NewColumnData[i] = BaseColumnData[i];
		}
	}
	else{
		Frontpanel_DefaultText(Hours, Minutes, Seconds, Notation, FALSE, FALSE);
	}

	//Add dots
	Frontpanel_DOTS(Hours, Minutes, Seconds, Notation);

	SetScreen();

	//Notify if transition state
	if(RunTransition)
		return TRUE;
	else
		return FALSE;
}


//Slide out old time and slide in new time right to left
bool FrontpanelMode_HorizontalSlideRight(int Hours, int Minutes, int Seconds, int Notation){

	static int RunTransition;
	static int BaseColumnData[26];

	static int TransitionDelay = 0;

	int i=0;

	if(RunTransition == 0){
		RunTransition = 13; //13 columns shift
		TransitionDelay = 8000 * TransSpeedFactor / 100;

		//Clear the screen
		ClearScreen();

		//Retrieve new data -> into NewColumnData
		Frontpanel_DefaultText(Hours, Minutes, Seconds, Notation, FALSE, FALSE);

		//Store old and new display data together
		//Remove dots
		for(i=0; i<13; i++){
			BaseColumnData[i] = LiveColumnData[i] & 0x0FFF;
			BaseColumnData[i+13] = NewColumnData[i];
		}
	}

	//Add transition delay
	if(RunTransition > 0){
		if(TransitionDelay-- > 0)
			return TRUE; //still running
		else{
			if(RunTransition > 7) //Speed up transition
				TransitionDelay = 8000/((13-RunTransition+1)*75/100) * TransSpeedFactor / 100;
			else //Slow down transition
				TransitionDelay = 8000/(RunTransition*75/100) * TransSpeedFactor / 100;
		}


		RunTransition--;

		//Now start shifting
		ClearScreen(); //TODO: probably not needed to clear screen.

		//Temp store last row
		int TempRow = BaseColumnData[25];

		//Shift all data to the right, minus the last one
		//Remove dots
		for(i=25; i>0; i--){
			BaseColumnData[i] = BaseColumnData[i-1] & 0x0FFF;
		}

		//Place last row back in array
		BaseColumnData[0] = TempRow;

		//Fill new temp data to display
		for(i=0; i<13; i++){
			NewColumnData[i] = BaseColumnData[i];
		}



	}
	else{
		Frontpanel_DefaultText(Hours, Minutes, Seconds, Notation, FALSE, FALSE);
	}

	Frontpanel_DOTS(Hours, Minutes, Seconds, Notation);
	SetScreen();

	//Notify if transition state
	if(RunTransition)
		return TRUE;
	else
		return FALSE;
}

//Remove old pixels at random, when all gone, show new pixel at random
bool FrontpanelMode_DissolveOutIn(int Hours, int Minutes, int Seconds, int Notation){

	#define TRANSPEED	3500

	static int DissInColumnData[13];

	static int TransitionDelay = 0;
	static int CurrentStep = 0;

	int i=0;

	if(CurrentStep == 0){
		TransitionDelay = TRANSPEED * TransSpeedFactor / 100;
		CurrentStep = 1;

		ClearScreen();  //empty last transition

		//Retrieve new data -> into NewColumnData
		Frontpanel_DefaultText(Hours, Minutes, Seconds, Notation, FALSE, FALSE);

		//Store old and new display data together
		for(i=0; i<13; i++){
			//Get new data
			DissInColumnData[i] = NewColumnData[i] & 0x0FFF;

			//Get live data
			NewColumnData[i] = LiveColumnData[i] & 0x0FFF;
		}

	}


	if(CurrentStep > 0){

		if(TransitionDelay-- > 0)
			return TRUE; //still running
		else{

			//Set delay for next time
			TransitionDelay = TRANSPEED * TransSpeedFactor / 100;

			if(CurrentStep == 1){								//Step 1: Dissolve current screen

				//Check if dissolve is complete
				unsigned int temp=0;

				for(i=0; i<13; i++){
					//Remove dot
					NewColumnData[i] = NewColumnData[i] & 0x0FFF;

					//Sum values
					temp += NewColumnData[i];
				}

				if(temp == 0){									//if nothing to do, set to next mode and exit
					CurrentStep = 2;
					return TRUE; //still running
				}
				else{	//Dissolve one more LED

					//1. Select random row (Loop until non-empty row is found
					int column = ((xorshift8() * 13) >> 8);

					while(NewColumnData[column] == 0){
						column = ((xorshift8() * 13) >> 8);
					}

					//2. Select random LED
					int row = ((xorshift8() * 12) >> 8);

					while((NewColumnData[column] & 1<<row) == 0){
						row = ((xorshift8() * 12) >> 8);
					}

					NewColumnData[column] &= ~(1<<row); //turn LED OFF
				}
			}

			else if(CurrentStep == 2){							//Step 2: show new screen

				unsigned int temp=0;
				for(i=0; i<13; i++){
					temp += DissInColumnData[i];
				}

				if(temp == 0){									//if nothing to do, exit transition
					CurrentStep = 0;
					return FALSE; //done with the transition!
				}
				else{	//Show one more LED

					//1. Select random row (Loop until non-empty row is found
					int column = ((xorshift8() * 13) >> 8);

					while(DissInColumnData[column] == 0){
						column = ((xorshift8() * 13) >> 8);
					}

					//2. Select random LED
					int row = ((xorshift8() * 12) >> 8);

					while((DissInColumnData[column] & 1<<row) == 0){
						row = ((xorshift8() * 12) >> 8);
					}

					DissInColumnData[column] &= ~(1<<row);	//turn LED OFF in buffer (not show it again)
					NewColumnData[column] |= (1<<row);		//turn LED ON to screen
				}
			}

		}
	}

	//Add dots
	Frontpanel_DOTS(Hours, Minutes, Seconds, Notation);

	//Set new data into live data
	SetScreen();

	return TRUE;
}


//Similar to dissolveoutin but when old display is dissolving out, the new one is directly shown
bool FrontpanelMode_DissolveCross(int Hours, int Minutes, int Seconds, int Notation){

	static int DissInColumnData[13], DissOutColumnData[13], StaticColumnData[13];

	static int TransitionDelay = 0;
	static int CurrentStep = 0;

	int i=0;

	if(CurrentStep == 0){
		TransitionDelay = 4000 * TransSpeedFactor / 100;
		CurrentStep = 1;

		ClearScreen();  //empty last transition

		//Retrieve new data -> into NewColumnData
		Frontpanel_DefaultText(Hours, Minutes, Seconds, Notation, FALSE, FALSE);

		//Store old and new display data together

		bool OldNewMatch = TRUE;
		for(i=0; i<13; i++){

			//Store the new screen in the static buffer
			StaticColumnData[i] = NewColumnData[i];

			//Store the new screen in the active count buffer
			DissInColumnData[i] = NewColumnData[i];

			//Store the old screen in the active count buffer (minus dots)
			DissOutColumnData[i] = LiveColumnData[i] & 0x0FFF;

			//Store the old screen in the active newscreen (minus dots)
			NewColumnData[i] = LiveColumnData[i] & 0x0FFF;

			//Check if screen is the same(if so, new transition will be forced)
			if(DissInColumnData[i] != DissOutColumnData[i])
				OldNewMatch = FALSE;
		}

		//If a transition is forced, then dissolve ON both minutes and hours
		//Minutes and hours are turned off and dissolve back IN
		if(OldNewMatch){
			ClearScreen();
			Frontpanel_DefaultText(Hours, Minutes, Seconds, Notation, TRUE, TRUE);
			Frontpanel_DOTS(Hours, Minutes, Seconds, Notation);
			SetScreen();
			CurrentStep = 0;
			return TRUE; //still running
		}
	}


	if(CurrentStep > 0){

		if(TransitionDelay-- > 0)
			return TRUE; //still running
		else{

			//Set delay for next time
			TransitionDelay = 4000 * TransSpeedFactor / 100;

			//Check if there is work to do
			int DissOut=0, DissIn=0;

			for(i=0; i<13; i++){
				DissOut += DissOutColumnData[i];
				DissIn += DissInColumnData[i];
			}

			//if no work then quit
			if((DissOut == 0) && (DissIn == 0)){
				CurrentStep = 0;	//Done!
				return FALSE; //Done running!
			}

			//Perform dissolve out task
			if(DissOut != 0){

				//1. Select random row (Loop until non-empty row is found)
				int column = ((xorshift8() * 13) >> 8);
				int row = ((xorshift8() * 12) >> 8);

				//Check if the new screen contains the old screen led
				while(1){
					while(DissOutColumnData[column] == 0){
						column = ((xorshift8() * 13) >> 8);
					}

					while((DissOutColumnData[column] & 1<<row) == 0){
						row = ((xorshift8() * 12) >> 8);
					}

					DissOutColumnData[column] &= ~(1<<row);

					DissOut = 0;

					//Check if still work to do
					for(i=0; i<13; i++){
						DissOut += DissOutColumnData[i];
					}

					//if not in static buffer, then action is required, skip the rest
					if(!(StaticColumnData[column] & 1<<row) || DissOut == 0)
						break;
				}

				//if not in new frame, then hide
				if(!(StaticColumnData[column] & 1<<row))
					NewColumnData[column] &= ~(1<<row);
			}


			//Perform dissolve out task
			if(DissIn != 0){

				//1. Select random row (Loop until non-empty row is found)
				int column = ((xorshift8() * 13) >> 8);
				int row = ((xorshift8() * 12) >> 8);

				//Check if the new screen contains the old screen led
				while(1){
					while(DissInColumnData[column] == 0){
						column = ((xorshift8() * 13) >> 8);
					}

					while((DissInColumnData[column] & 1<<row) == 0){
						row = ((xorshift8() * 12) >> 8);
					}

					DissInColumnData[column] &= ~(1<<row);

					DissIn = 0;

					//Check if still work to do
					for(i=0; i<13; i++){
						DissIn += DissInColumnData[i];
					}

					//if not in static buffer, then action is required, skip the rest
					if(!(NewColumnData[column] & 1<<row) || DissIn == 0)
						break;
				}

				//if not yet in new frame, then show
				if(!(NewColumnData[column] & 1<<row))
					NewColumnData[column] |= (1<<row);
			}
		}

	}
	else{	//Default operation
		Frontpanel_DefaultText(Hours, Minutes, Seconds, Notation, FALSE, FALSE);
	}

	//Show dots
	Frontpanel_DOTS(Hours, Minutes, Seconds, Notation);

	//Set new data into live data
	SetScreen();

	return TRUE;
}


bool FrontpanelMode_RandomIntro(int Hours, int Minutes, int Seconds, int Notation){
	static int TransitionDelay = 0;

	if(TransitionDelay <= 0){
		TransitionDelay = 25000 * TransSpeedFactor / 100;
	}

	if((--TransitionDelay) > 0){
		//Blink screen at transition
		int i;
		for(i=0; i<13; i++)
			NewColumnData[i] = 0b111111111111;
		SetScreen();
		return TRUE; //transition still running
	}

	ClearScreen();
	Frontpanel_DefaultText(Hours, Minutes, Seconds, Notation, FALSE, FALSE);
	Frontpanel_DOTS(Hours, Minutes, Seconds, Notation);
	SetScreen();

	return FALSE; //transition done
}


bool FrontpanelMode_Basic(int Hours, int Minutes, int Seconds, int Notation){

	static int TransitionDelay = 0;

	if(TransitionDelay <= 0){
		TransitionDelay = 10000 * TransSpeedFactor / 100;
	}

	if((--TransitionDelay) > 0){
		//Blink screen at transition
		ClearScreen();
		SetScreen();
		return TRUE; //transition still running
	}

	ClearScreen();
	Frontpanel_DefaultText(Hours, Minutes, Seconds, Notation, FALSE, FALSE);
	Frontpanel_DOTS(Hours, Minutes, Seconds, Notation);
	SetScreen();

	return FALSE; //transition done
}
