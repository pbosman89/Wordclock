#include "gpio.h"
#include "Peripherals.h"
#include "type.h"
#include "lpc_types.h"
#include "projectconfig.h"
#include "adc.h"
#include "ds1337.h"
#include "globalcomm.h"

volatile unsigned int DisplayIntensity=0, DisplayIntensityABS=0;
volatile unsigned int DisplayIntensityMin=1, DisplayIntensityMax=100;
volatile unsigned int FrontLang=0;
volatile unsigned int AmPm=0;
volatile bool DashEnabled=TRUE;

void init_Bluetooth(void){
	//Set Name
	CommSendString("AT+NAMEWordClock\r\n");
	//Set Power
	CommSendString("AT+POWE2\r\n");
	//Restart device
	CommSendString("AT+RESET\r\n");
}

void GetTime(int *Hr, int *Min, int *Sec){
	static int LastHr, LastMin, LastSec;

	//if pin low -> one second interrupt of RTC happened
	if(!(LPC_GPIO0->DATA & 1<<7)){

		unsigned char byte[4];

		//reset seconds interrupt flag
		DS1337_WriteByte(0x0F, 0x00);
		DS1337_ReadBuffer(0x00,byte,4);

		LastSec = *Sec = (byte[0] >>4) * 10  + (byte[0] & 0x0F);
		LastMin = *Min = (byte[1] >>4) * 10  + (byte[1] & 0x0F);
		LastHr = *Hr = (((byte[2] & 0x20) >>5) * 20 + ((byte[2] & 0x10) >>4) * 10 + (byte[2] & 0x0F));
	}
	else{
		*Hr = LastHr;
		*Min = LastMin;
		*Sec = LastSec;
	}
}

void SetTime(int H, int I, int S){
	unsigned char byte[3];

	int temp, temp1 = 0;

	temp = S/10;
	temp1 = S - (temp*10);
	byte[0] = (temp<<4) | temp1;	//0000 = 10seconds, 0000 = 0 seconds

	temp = I/10;
	temp1 = I - (temp*10);
	byte[1] = (temp<<4) | temp1;	//0000 = 10minutes, 0101 = 5 minutes

	temp = H/10;
	temp1 = H - (temp*10);

	byte[2] = 0b00000000 | (temp<<4) | temp1; //0000 = 24hr notation, 1000 = 8 hours

	DS1337_WriteBuffer(0x00, byte, 3);
}


void GetIntensity(void){

	static int UpdateInterval=0;
	static unsigned int FilterAvg=0;

	//Upadte intensity once every n cycles
	if(UpdateInterval++ >= LIGHT_UPDATEINTERVAL){
		UpdateInterval = 0;

		//Volledig donker = 252
		//Volledig licht = 0

		LPC_IOCON->PIO1_4 &= ~0x8F; /* Clear bit7, change to analog mode. */
		LPC_IOCON->PIO1_4 |= 0x01; /* ADC IN5 */


		//Read light detector at channel 5 - Output 0 - 255
		unsigned int IntensityABS = (255 - (ADCRead(5) >>2)) * 400 / 255; //invert reading, multiply by maximum value, divide by maximum adc read value

		unsigned int VirtualMax = 400 * DisplayIntensityMax / 100;
		unsigned int VirtualMin = 400 * DisplayIntensityMin / 100;

		// 10% + ((100%-10%)/100) * output(0-100%) = output in percentage
		unsigned int NewIntensity = VirtualMin + (VirtualMax - VirtualMin) * IntensityABS / 400;


		//Check if comply with min/max
		if(NewIntensity > LIGHT_MAXINTENSITY)
			NewIntensity = LIGHT_MAXINTENSITY;
		else if(NewIntensity < LIGHT_MININTENSITY)
			NewIntensity = LIGHT_MININTENSITY;


		FilterAvg = FilterAvg - (FilterAvg>>LIGHT_FILTERSTRENGTH) + NewIntensity;

		//Store absolute intensity; Value between 0 and 400
		DisplayIntensity = FilterAvg >>LIGHT_FILTERSTRENGTH;

		//Wait until screen updating not running
	//	while ((LPC_TMR16B0->IR & (1<<1)));

		LPC_TMR16B0->MR0 = DisplayIntensity;

	}
}



void init_ScreenRefresh(void){
	  //Enable system clock to timer clock
	  LPC_SYSCON->SYSAHBCLKCTRL |= (1<<7);

	  LPC_TMR16B0->PR = 72; //MHZ_PRESCALE;
	  LPC_TMR16B0->MR1 = 120; //Screen refresh rate
//	  LPC_TMR16B0->MR2 = 350;

	  LPC_IOCON->PIO0_8 = 2<<0; //Connect pin to MAT0
	  LPC_TMR16B0->PWMC = (1<<0); //MR0 = PWM

	  //Setup PWM
	  LPC_TMR16B0->MR0 = LIGHT_STARTINTENSITY;

	  LPC_TMR16B0->MCR = (1<<3) | (1<<4);				/* Interrupt and Reset on MR1 */

	  /* Enable the TIMER0 Interrupt */
	  NVIC_SetPriority(TIMER_16_0_IRQn, 1); //Set lower priority than touch
	  NVIC_EnableIRQ(TIMER_16_0_IRQn);

	  //Enable the timer
	  LPC_TMR16B0->TCR = 1;
}



//**************** TLC5925 *****************************

//Initialize TLC5925 IC
void TLC5925Init(void){

	//Pins
	//ROW_OE	= PIO3_0
	//ROW_SCK	= PIO3_1
	//ROW_MOSI	= PIO3_3
	//ROW_LATCH	= PIO2_6

	//Set pin direction
	GPIO_SetDir(PORT2, 6, 1);	//Latch
	GPIO_SetDir(PORT3, 3, 1);	//MOSI
	GPIO_SetDir(PORT3, 1, 1);	//SCK
	GPIO_SetDir(PORT3, 0, 1);	//LEDENABLE

	//Disable LED Output
	GPIO_SetBits(PORT3, 0);

	//Others initial low state
	GPIO_ResetBits(PORT2, 6);
	GPIO_ResetBits(PORT3, 3);
	GPIO_ResetBits(PORT3, 1);
}

//Shift data out to TLC5925
void TLC5925ShiftOut(int Data, int Length){

	unsigned int shift;

	for(shift = 0; shift<Length; shift++){
		if(Data & 1<<(Length-1)) // MOSI
			GPIO_SetBits(PORT3, 3);
		else
			GPIO_ResetBits(PORT3, 3);

		Data = Data <<1;

		//SCK one clock
		GPIO_SetBits(PORT3, 1);
		GPIO_ResetBits(PORT3, 1);
	}
}

//Latch new data of TLC5925
void TLC5925Latch(void){
	GPIO_SetBits(PORT2, 6);
	GPIO_ResetBits(PORT2, 6);
}

void TLC5925Output(bool State){
	if(State == FALSE)
		GPIO_SetBits(PORT3, 0);
	else
		GPIO_ResetBits(PORT3, 0);
}



//**************** 74HC595 *****************************

//Pins
//COL_OE	= PIO0_8
//COL_SCK	= PIO2_10
//COL_MOSI	= PIO2_2
//COL_LATCH	= PIO2_9

//Initialize TLC5925 IC
void HC595Init(void){
	//Set pin direction
	GPIO_SetDir(PORT2, 9, 1);	//Latch
	GPIO_SetDir(PORT2, 2, 1);	//MOSI
	GPIO_SetDir(PORT2, 10, 1);	//SCK

	//Others initial low state
	GPIO_ResetBits(PORT2, 9);
	GPIO_ResetBits(PORT2, 2);
	GPIO_ResetBits(PORT2, 10);
}

//Shift data out to TLC5925
void HC595ShiftOut(int Data, int Length){

	unsigned int shift;

	for(shift = 0; shift<Length; shift++){
		if(Data & 1<<(Length-1)) // MOSI
			GPIO_SetBits(PORT2, 2);
		else
			GPIO_ResetBits(PORT2, 2);

		Data = Data <<1;

		//SCK one clock
		GPIO_SetBits(PORT2, 10);
		GPIO_ResetBits(PORT2, 10);
	}
}

//Latch new data of TLC5925
void HC595Latch(void){
	GPIO_SetBits(PORT2, 9);
	GPIO_ResetBits(PORT2, 9);
}

void HC595Output(bool State){
	if(State == FALSE)
		GPIO_SetBits(PORT0, 8);
	else
		GPIO_ResetBits(PORT0, 8);
}
