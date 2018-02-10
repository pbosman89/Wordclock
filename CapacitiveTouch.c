#include "gpio.h"

#include "CapacitiveTouch.h"
#include <type.h>
#include <stdbool.h>
#include "projectconfig.h"
#include "globalcomm.h"
#include "DisplayGraphics.h"

//Initialize the cap touch timers
//PIO1_0 - CT32B1_CAP0 = TOUCH_SEN_A
//PIO1_8 - CT16B1_CAP0 = TOUCH_SEN_B
//PIO1_2 - TOUCH_COMM1
//PIO2_8 - TOUCH_COMM2

inline void SetChan0(int direction){
	if(direction){ //Pull high
		LPC_IOCON->JTAG_TMS_PIO1_0 = 0b10110011;
	}
	else { //Pull low
		LPC_IOCON->JTAG_TMS_PIO1_0 = 0b10101011;
	}
}
inline void SetChan1(int direction){
	if(direction){ //Pull high
		LPC_IOCON->PIO1_8 = 0b110001;
	}
	else { //Pull low
		LPC_IOCON->PIO1_8 = 0b101001;
	}
}

//de-initialize capacitive touch
void deinit_CapANDAudio(void){

	//Turn off interrupts
	NVIC_DisableIRQ(TIMER_32_1_IRQn);
	NVIC_ClearPendingIRQ(TIMER_32_1_IRQn);
	NVIC_DisableIRQ(TIMER_16_1_IRQn);
	NVIC_ClearPendingIRQ(TIMER_16_1_IRQn);

	//Reset timer
	LPC_TMR32B1->TCR = (1<<0) | (1<<1); //Reset
	LPC_TMR16B1->TCR = (1<<0) | (1<<1); //Reset

	//Set all registers to default state
	LPC_TMR32B1->PR = 0;
	LPC_TMR16B1->PR = 0;
	LPC_TMR32B1->PC = 0;
	LPC_TMR16B1->PC = 0;
	LPC_TMR32B1->MCR = 0;
	LPC_TMR16B1->MCR = 0;
	LPC_TMR32B1->CCR = 0;
	LPC_TMR16B1->CCR = 0;
	LPC_TMR32B1->EMR = 0;
	LPC_TMR16B1->EMR = 0;
	LPC_TMR32B1->CTCR = 0;
	LPC_TMR16B1->CTCR = 0;
	LPC_TMR32B1->PWMC = 0;
	LPC_TMR16B1->PWMC = 0;
	LPC_TMR32B1->TCR = 0; //Disable
	LPC_TMR16B1->TCR = 0; //Disable

	//Turn off timer clock
	LPC_SYSCON->SYSAHBCLKCTRL &= ~(1<<10);	//CT32B1
	LPC_SYSCON->SYSAHBCLKCTRL &= ~(1<<8);	//CT16B1

}


//initialize capacitive touch
void init_CapTouch(void){
	//Setup CT32B1_CAP0
		//Enable system clock to timer clock
		LPC_SYSCON->SYSAHBCLKCTRL |= (1<<10);	//CT32B1
		LPC_SYSCON->SYSAHBCLKCTRL |= (1<<8);	//CT16B1

		//Reset and enable timer (automatically disabled)
		LPC_TMR32B1->TCR = (1<<0) | (1<<1); //Reset
		LPC_TMR16B1->TCR = (1<<0) | (1<<1); //Reset

		//Set prescale
		LPC_TMR32B1->PR = 1;
		LPC_TMR16B1->PR = 1;

		//Set capture register
		//Capture on rising edge and generate interrupt
		//Value can be found in LPC_TMR32B1->CR0
		LPC_TMR32B1->CTCR = 0;
		LPC_TMR16B1->CTCR = 0;

		//No interrupt on match
		LPC_TMR32B1->MCR = 0;
		LPC_TMR16B1->MCR = 0;

		//Capture on both edges
		LPC_TMR32B1->CCR = (1<<0) | (1<<1) | (1<<2);
		LPC_TMR16B1->CCR = (1<<0) | (1<<1) | (1<<2);

		//Connect pin to capture function, enable hysteresis, enable digital mode
		LPC_IOCON->JTAG_TMS_PIO1_0 = 0b10101011;
		LPC_IOCON->PIO1_8 = 0b101001;
		GPIO_SetDir(PORT1,0,0); //input
		GPIO_SetDir(PORT1,8,0); //input

		//Set common channel GPIO, input, no pull
		LPC_IOCON->JTAG_nTRST_PIO1_2 = 0b10000001;
		LPC_IOCON->PIO2_8 = 0; //input
		GPIO_SetDir(PORT1,2,0); //Input
		GPIO_SetDir(PORT2,8,0); //Input

		//Set priority
		NVIC_SetPriority(TIMER_32_1_IRQn, 0);
		NVIC_SetPriority(TIMER_16_1_IRQn, 0);
		//Enable interrupts
		NVIC_ClearPendingIRQ(TIMER_32_1_IRQn);
		NVIC_EnableIRQ(TIMER_32_1_IRQn);
		NVIC_ClearPendingIRQ(TIMER_16_1_IRQn);
		NVIC_EnableIRQ(TIMER_16_1_IRQn);

		//Set pull-up
		SetChan0(1);
		SetChan1(1);

		//Start timer counters for measurement
		LPC_TMR32B1->TCR = 1;
		LPC_TMR16B1->TCR = 1;
}


volatile int CapRawVal[2];
volatile bool CapAvail[2] = {FALSE, FALSE};

volatile int CapFilter = CAP_FILTERSTRENGTH;
volatile int CapSensorOutput[2] = {-1,-1}; //Output after filter and base offset compensation
volatile int CapHighThr = CAP_HIGHTHRESHOLD;
volatile int CapLowThr = CAP_LOWTHRESHOLD;
volatile int CapMinTimeShort = CAP_SHORTPRESS;
volatile int CapMinTimeLong = CAP_LONGPRESS;

//Capture capacitive touch values
void CaptureCapTouch(int channel){
	if(channel == 0){
		//Stop & reset timer
		LPC_TMR32B1->TCR = 1<<1;
		//Store data
		CapRawVal[0] += LPC_TMR32B1->CR0;
		//Data ready flag
		CapAvail[0] = TRUE;
	}
	else{
		//Stop & reset timer
		LPC_TMR16B1->TCR = 1<<1;
		//Store data
		CapRawVal[1] += LPC_TMR16B1->CR0;
		//Data ready flag
		CapAvail[1] = TRUE;
	}
}

volatile bool ForceReset=FALSE;

//Calculate the final capacitive touch values
void CalcCapTouch(void){

	static int MidFilter[2]={0xFFF,0xFFF};
	static int Output[2];
	static int TotCount[2];

	static int RestartCount[2];

	bool NewDataAvail[2] = {FALSE,FALSE};

	static int baseLineDelay[2]={0,0};
	static int baseLine[2]={5000,5000};
	static int WarmUp[2] = {500,500};

	static bool StartMeas[2] = {FALSE, FALSE};

	//Force reset if filter settings changed
	if(ForceReset == TRUE){
		RestartCount[0] = 100001;
		RestartCount[1] = 100001;
		ForceReset = FALSE;
	}

	//Touch panel watchdog
	if(RestartCount[0]++ > 100000){
		CommSendString("Touch 0 Restart..\r\n");
		RestartCount[0] = 0;

		CapRawVal[0] = 0;
		CapAvail[0] = FALSE;

		WarmUp[0] = 500;

		StartMeas[0] = TRUE;
	}
	//Touch panel watchdog
	if(RestartCount[1]++ > 100000){
		CommSendString("Touch 1 Restart..\r\n");
		RestartCount[1] = 0;

		CapRawVal[1] = 0;
		CapAvail[1] = FALSE;

		WarmUp[1] = 500;

		StartMeas[1] = TRUE;
	}

	int i;
	for(i=0; i<2; i++){
		//Touch panel watchdog
		if(RestartCount[i]++ > 100000){
			CommSendString("Touch ");
			CommSendByte((i+0x30));
			CommSendString(" Restart..\r\n");
			RestartCount[i] = 0;

			CapRawVal[i] = 0;
			CapAvail[i] = FALSE;

			WarmUp[i] = 500;

			StartMeas[i] = TRUE;
		}
	}

	for(i=0; i<2; i++){
		if(CapAvail[i] == TRUE){
			//Reset watchdog
			RestartCount[i] = 0;

			//Get ready for next measurement cycle
			CapAvail[i] = FALSE;

			if(TotCount[i]++ >= 512){
						//Signal new data is available
						NewDataAvail[i] = TRUE;

						//Reset total count
						TotCount[i] = 0;
			}
			else{
				StartMeas[i] = TRUE;
			}

			break;
		}
	}

	static int NextButton = 1;

	//Process available data
	for(i=0; i<2; i++){
		if(NewDataAvail[i] == TRUE){

			//---- Smoothen input
			MidFilter[i] = MidFilter[i] - (MidFilter[i]>>CapFilter) + CapRawVal[i];
			Output[i] = (MidFilter[i]>>CapFilter) >>5;

			//---- Reset values for next measurement
			NewDataAvail[i] = FALSE;
			CapRawVal[i] = 0;		//Reset measured value
			StartMeas[i] = TRUE;

			NextButton ^= 1; //Toggle next button

			if(WarmUp[i] > 0){
				WarmUp[i]--;
				baseLine[i] = (baseLine[i] * 4 + Output[i]) / 5;

				if(WarmUp[i] <= 0){
					CommSendString("Touch ");
					CommSendByte((i+0x30));
					CommSendString(": initialized\r\n");

					baseLine[i] += 10;
				}
			}
			else{
				//---- Update baseline value
				if(baseLineDelay[i]++ >= CAP_BASELINEDELAY){
					if((baseLine[i]-5) < Output[i]){
						baseLine[i]++;
					}
					else{
						baseLine[i] = (baseLine[i]+Output[i])/2;
					}

					baseLineDelay[i] = 0;
				}

				//---- Calculate sensor relative output
				CapSensorOutput[i] = Output[i] - baseLine[i];

				if(CapSensorOutput[i] < 0)
					CapSensorOutput[i] = 0;

				//If touch time is reset by ProcessCapTouch, lift baseline to prevent false triggers
				if(ProcessCapTouch(i) == TRUE){
					baseLine[i] = baseLine[i] + 50;
				}
			}
		}
	}

	//Restart sensors for next measurement
	//Check if high
	if(StartMeas[1] == TRUE && NextButton == 1){
		NextButton = 0;
		//Check if high
		if((LPC_IOCON->PIO1_8 & 1<<4)){
			//Pull low
			SetChan1(0);
			//Restart timer
			LPC_TMR16B1->TCR = 1;
		}
		else{
			SetChan1(1);			//Pull common channel high
			LPC_TMR16B1->TCR = 1;	//Restart timer
		}
	}
	else if(StartMeas[0] == TRUE && NextButton == 0){
		NextButton = 1;
		if((LPC_IOCON->JTAG_TMS_PIO1_0 & 1<<4)){
			//Pull low
			SetChan0(0);
			//Restart timer
			LPC_TMR32B1->TCR = 1;
		}
		else{
			SetChan0(1);			//Pull common channel high
			LPC_TMR32B1->TCR = 1;	//Restart timer
		}
	}
}



//---------------------------------------------------------------------------------
//Process capacitive touch data
//---------------------------------------------------------------------------------
bool ProcessCapTouch(int Button){

	static int CapSensorTime[2];
	static int PrevOutput[2];

	bool Reset=FALSE;

		if(CapSensorOutput[Button] > CapHighThr){
			if(PrevOutput[Button] > CapHighThr){
				CapSensorTime[Button]++;

				if(CapSensorTime[Button] == CapMinTimeShort){
					CommSendString("Short Touch ");
					char str[16];
					itoa(Button, str);
					CommSendString(str);

					CommSendString(": ");
					itoa(CapSensorTime[Button], str);
					CommSendString(str);
					CommSendString("\r\n");

					//Perform short press action

					//Left button
					if(Button == 0){
						if(DisplayMode > 100 && DisplayMode < 200){

							switch(DisplayMode){
								case 110:
									DisplayMode = 160;
									break;
								case 120:
									DisplayMode = 110;
									break;
								case 130:
									DisplayMode = 120;
									break;
								case 140:
									DisplayMode = 130;
									break;
								case 150:
									DisplayMode = 140;
									break;
								case 160:
									DisplayMode = 150;
									break;
								default:
									DisplayMode = 120;
									break;
							}
						}
					}
					//Right button
					else{
						if(DisplayMode > 100 && DisplayMode < 200){

							switch(DisplayMode){
								case 110:
									DisplayMode = 120;
									break;
								case 120:
									DisplayMode = 130;
									break;
								case 130:
									DisplayMode = 140;
									break;
								case 140:
									DisplayMode = 150;
									break;
								case 150:
									DisplayMode = 160;
									break;
								case 160:
									DisplayMode = 110;
									break;
								default:
									DisplayMode = 120;
									break;
							}
						}
					}
					//Notify
					//char str[16];
					itoa(DisplayMode, str);

					CommSendString("New Mode Set: ");
					CommSendString(str);
					CommSendString("\r\n");
				}
				if(CapSensorTime[Button] > CapMinTimeLong){
					CommSendString("Long Touch ");
					char str[16];
					itoa(Button, str);
					CommSendString(str);
					CommSendString(": ");
					itoa(CapSensorTime[Button], str);
					CommSendString(str);
					CommSendString("\r\n");

					//Long press of right button
					if(Button == 1){
						DisplayMode = 170; //Set random mode!
						CommSendString("New Mode Set: Mystic!\r\n");
					}

					CapSensorTime[Button] = 0;
					Reset = TRUE;
				}
			}
			else{//triggered once signal gets above trigger
				CapSensorTime[Button] = 0;
			}

		}

		PrevOutput[Button] = CapSensorOutput[Button];

		if(Reset){
			return TRUE;
		}
		else {
			return FALSE;
		}
}

//Set the sensitivity of the cap touch buttons
void SetSenCapTouch(int filter, int thresholdHigh, int thresholdLow, int shortPress, int longPress){
	if(CapFilter != filter){
		CapFilter = filter;
		ForceReset = TRUE; //Restart initialization
	}
	CapHighThr = thresholdHigh;
	CapLowThr = thresholdLow;
	CapMinTimeShort = shortPress;
	CapMinTimeLong = longPress;
}
