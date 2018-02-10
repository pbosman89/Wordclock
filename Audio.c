#include "gpio.h"

#include <type.h>
#include <stdbool.h>

#include "projectconfig.h"
#include "CapacitiveTouch.h"
#include "flashmemory.h"
#include "timer16.h"

//Indicates if audio is being played or waiting for next job
volatile bool PlayerIdle=TRUE;

//Length of current audio file
volatile int AudioLength=0;

//Indicate if audio is enabled
volatile bool AudioEnabled=FALSE;

void init_Audio(void){
	//Enable system clock to timer clock
	LPC_SYSCON->SYSAHBCLKCTRL |= (1<<8);

	//Set as GPIO output function
	LPC_IOCON->PIO1_9 = (1<<0);
	LPC_IOCON->PIO1_10 = (1<<0);

	// 72e6 / ( 15 * (2^8) )= 18.75 kHz, 8 bit
	LPC_TMR16B1->PR = 17;

	//reset and interrupt on match MR3, 8-bit resolution
	LPC_TMR16B1->MR3 = 255;
	LPC_TMR16B1->MCR = (1<<10); //reset on MR3
	LPC_TMR16B1->MCR |= (1<<6); //interrupt on MR2

	//Output on MR0 and MR1
	LPC_TMR16B1->PWMC = (1<<0 | 1<<1);

	LPC_TMR16B1->MR0 = 0x7F; //50% duty cycle
	LPC_TMR16B1->MR1 = 0x7F;
	LPC_TMR16B1->MR2 = 0x7F;

	//Start timer
	LPC_TMR16B1->TCR = 1;

	// Enable the TIMER1 Interrupt
	NVIC_SetPriority(TIMER_16_1_IRQn, 0); //Set priority
	NVIC_EnableIRQ(TIMER_16_1_IRQn); //Used to load new PWM data

	//Audio enable pin
	GPIO_SetDir(PORT2,4,1);
	//Audio OFF
	GPIO_ResetBits(PORT2, 4);

}


//Check if there is wave data available in the flash
bool Check_Audio(void){
	//release chip
	SSPDSEL();
	//select chip
	SSPSSEL();

	//Set flash read mode and location at start of memory
	SSPSend(0x03); SSPSend(0); SSPSend(0); SSPSend(0);
	int byte1, byte2, byte3;
	byte1 = SSPSend(0);
	byte2 = SSPSend(0);
	byte3 = SSPSend(0);

	//release chip
	SSPDSEL();

	if((byte1 == 0xAA) && (byte2 == 0x7F) && (byte3 == 0x55))
		return true;
	else
		return false;
}


void Play_Audio(void){
	static int CurrentByte=0;

	if(!PlayerIdle){

		if(CurrentByte >= AudioLength){
			CurrentByte = 0;
			PlayerIdle = TRUE;
		}
		else{
			int Output;

			Output = ReadNextFromWAV();
			Output = (65535 - (Output ^ 0x8000))>>2;

			LPC_TMR16B1->MR0 = (Output >>8);
			LPC_TMR16B1->MR1 = (Output & 0xFF);

			CurrentByte += 2;
		}

	}
}

bool SpeakTime(int Hours, int Minutes, int Seconds, int Mode, int Notation){

	static int PrevHours=12, PrevMinutes=0;
	static bool SpeechRunning=FALSE;

	static int AudioSequence[6]={-1,-1,-1,-1,-1,-1};

	//Check if current mode is display mode, otherwise do nothing and return
	if(Mode < 100 || Mode > 200)
		return FALSE;

	//Convert from 0-23 to 1-12
	if(Hours > 12)
		Hours -= 12;
	else if(Hours == 0)
		Hours = 12;

	//Speak time every full and half hour
	if( ((Minutes == 30) || (Minutes == 0)) && ((Minutes != PrevMinutes) || (PrevHours != Hours)) )
	{
		AudioSequence[0] = SoundHET;
		AudioSequence[1] = SoundIS;

		//-------- Load new speech into speech array
		if(Minutes == 30){ //half hour
			AudioSequence[2] = SoundHALF;

			if(Hours < 12)
				AudioSequence[3] = SoundNumbers[Hours];
			else
				AudioSequence[3] = SoundNumbers[0];
		}
		else if((Minutes == 0)){ //Full hour
			AudioSequence[2] = SoundNumbers[(Hours-1)];
			AudioSequence[3] = SoundUUR;
		}
		else{ //everything else
			//Say nothing for now
		}

		//-------- Disable capacitive touch
		deinit_CapANDAudio();

		//-------- Enable speech module
		init_Audio();

		//-------- Start speech
		SpeechRunning = TRUE;
	}
	else if(SpeechRunning){
		//Check if needed to load next audio track from memory
		if(PlayerIdle){

			//Find next sound to play
			int i=0;
			for(i=0; i<6; i++){
				if(AudioSequence[i] != -1)
					break;
			}

			if(i < 6){
				AudioLength = SetupWAVData(AudioSequence[i]);

				//Turn ON audio amplifier
				GPIO_SetBits(2,4);
				AudioSequence[i] = -1;
				PlayerIdle = FALSE;
			}
			else{
				GPIO_ResetBits(2,4);	//Turn OFF audio amplifier
				SpeechRunning = FALSE;	//Indicate playing done

				//De-initialize audio
				deinit_CapANDAudio();

				//Re-initialize Touch
				init_CapTouch();
			}
		}
	}
	else{
		SpeechRunning = FALSE;
	}


	PrevMinutes = Minutes;
	PrevHours = Hours;

	return SpeechRunning;
}
