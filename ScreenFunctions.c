#include "gpio.h"

#include "projectconfig.h"


//Global data
volatile unsigned int LiveColumnData[13] = {0x0001, 0x0002, 0x0004, 0x0008, 0x0010, 0x0020, 0x0040, 0x0080, 0x0100, 0x0200, 0x0400, 0x0800, 0x1000};
volatile unsigned int NewColumnData[13];

void XY2Screen(int X, int Y, int State){
	if(State)
		NewColumnData[X] |= (1<<Y);
	else
		NewColumnData[X] &= ~(1<<Y);
}

void Coordinates2Screen(int StartX, int StartY, int Chars){

	int i;
	for(i=StartX; i<(StartX+Chars); i++){
		NewColumnData[i] |= 1<<StartY;
	}
}

void SetScreen(void){
	unsigned char i = 0;

	for(i=0; i<13; i++)
		LiveColumnData[i] = NewColumnData[i];
}

void ClearScreen(void){
		unsigned char i = 0;

		for(i=0; i<13; i++)
			NewColumnData[i] = 0;
}
