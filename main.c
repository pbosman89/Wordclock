/**************************************************************************//**
 * @file     main.c
 * @brief    USB Virtual COM CDC - LPC1343
 * @version  V1.0
 * @date     03. DIC 2010
 *
 * @note
 * Autor: Ing. Leandro J. Aguierre
 *
 ******************************************************************************/
#include "gpio.h"

#include <stdbool.h>

#define     EN_USBREG       (1<<14)

/*Libraries for USB CDC*/
#include "usb.h"
#include "usbcfg.h"
#include "usbhw.h"
#include "usbcore.h"
#include "cdc.h"
#include "cdcuser.h"

#include "adc.h"
#include "ds1337.h"

//Program libraries
#include "Peripherals.h"
#include "globalcomm.h"
#include "projectconfig.h"
#include "CapacitiveTouch.h"
#include "ScreenFunctions.h"
#include "ssp.h"
#include "flashmemory.h"

void _delay_ms (uint16_t ms)
{
 uint16_t delay;
 volatile uint32_t i;
 for (delay = ms; delay >0 ; delay--)
//1ms loop with -Os optimisation
  {
  for (i=3500; i >0;i--){};
  }
}

volatile bool USBConnected=FALSE;

volatile
volatile int DisplayMode=150, DisplayNotation=1;
volatile int Hours, Minutes, Seconds;

volatile bool UploadAudioProcessing;
volatile int UploadAudioProcessingSize;

/*----------------------------------------------------------------------------
  Main Program
 *---------------------------------------------------------------------------*/
int main (void) {

	//Setup Display peripherals
	ADCInit(ADC_CLK);
	DS1337_Init();
	HC595Init();
	TLC5925Init();
	init_ScreenRefresh();
	SSPInit();

	//Setup Serial Port
	ser_ClosePort();
	ser_OpenPort();
	ser_InitPort(9600,8,0,0);

	//Initialize bluetooth module
	init_Bluetooth();

	//Audio enable pin
	GPIO_SetDir(PORT2,4,1);
	//Audio OFF
	GPIO_ResetBits(PORT2, 4);


	//--> Check if USB should be activated
	//Set PROGRAM button as input
	GPIO_SetDir(PORT0,1,0);

	ClearScreen();
	Coordinates2Screen(0,0,1);
	SetScreen();

	int j=0;
	for(j=0; j<2000; j++){
		if(!(LPC_GPIO0->DATA & 1<<1)){
			LPC_SYSCON->SYSAHBCLKCTRL |= (1<<16);//enable IOCON clock
			LPC_SYSCON->SYSAHBCLKCTRL |= (1<<6); //enable GPIO Clock

			USBIOClkConfig();
			LPC_SYSCON->SYSAHBCLKCTRL |= EN_USBREG;
			CDC_Init();
			USB_Init();
			USB_Connect(TRUE);                        // USB Connect
			while (!USB_Configuration);              // wait until USB is configured

			USBConnected = TRUE;
			break;
		}
		_delay_ms(2);

		if(j<500){
			XY2Screen((j/42),0,1);
			XY2Screen(0,(j/46),1);
		}
		else if(j<1000){
			XY2Screen(((j-500)/39),11,1);
			XY2Screen(12,((j-500)/41),1);
		}
		else if(j<1500){
			XY2Screen(((j-1000)/42),0,0);
			XY2Screen(0,((j-1000)/46),0);
		}
		else{
			XY2Screen(((j-1500)/39),11,0);
			XY2Screen(12,((j-1500)/41),0);
		}
		SetScreen();
	}

	LPC_SYSCON->SYSAHBCLKCTRL |= (1<<16);//enable IOCON clock
	LPC_SYSCON->SYSAHBCLKCTRL |= (1<<6); //enable GPIO Clock

	init_CapTouch();


	//Reverse screen animation to show USB is connected
	if(USBConnected){
		for(j=j; j>0; j--){
			_delay_ms(2);

			if(j<500){
				XY2Screen((j/42),0,0);
				XY2Screen(0,(j/46),0);
			}
			else if(j<1000){
				XY2Screen(((j-500)/39),11,0);
				XY2Screen(12,((j-500)/41),0);
			}
			else if(j<1500){
				XY2Screen(((j-1000)/42),0,1);
				XY2Screen(0,((j-1000)/46),1);
			}
			else{
				XY2Screen(((j-1500)/39),11,1);
				XY2Screen(12,((j-1500)/41),1);
			}
			SetScreen();
		}
	}

	ClearScreen();
	SetScreen();

	AudioEnabled = Check_Audio();

	CommSendString("WordClock Initialized");

	while(1){

		//Handle all USB and Serial communication
		CommRead();

		//Check for incoming command and execute
		ParseCommand();

		//Block everything else(except screen) while uploading audio
		while(UploadAudioProcessing){
			static int TotalReceived=0, BatchTotal=0;

			//Read new data
			CommRead();

			//Check if new data is available
			int DataAvail = Comm_AvailChar();

			if(DataAvail){
				char parseBuffer[64];
				//Read data from buffer
				Comm_data(&parseBuffer[0], TRUE);

				int i;
				for(i=0; i<DataAvail; i++)
					Flash_WriteByte(parseBuffer[i]);

				TotalReceived += DataAvail;
				BatchTotal += DataAvail;


				if(BatchTotal == 32){
					//Signal data OK
					CommSendString("OK\r\n");
					BatchTotal = 0;
				}

				//Check if all bytes are uploaded
				if(TotalReceived >= UploadAudioProcessingSize){
					Flash_WriteEnd();
					UploadAudioProcessing = FALSE;
					CommSendString("END\r\n");
					UploadAudioProcessingSize = 0;
					TotalReceived = 0;
					BatchTotal = 0;
				}
			}
		}


		//Get the intensity of the display
		GetIntensity(); //TODO:return intensity from function and add to global value instead of updating this value from function

		//Get time from RTC
		int Hr, Min, Sec;
		GetTime(&Hr, &Min, &Sec);
		Hours = Hr;
		Minutes = Min;
		Seconds = Sec;

		//Display time on screen
		bool isRunning;

		isRunning = DisplayTime(Hours, Minutes, Seconds, DisplayMode, DisplayNotation);

		if(!isRunning && AudioEnabled)
			isRunning = SpeakTime(Hours, Minutes, Seconds, DisplayMode, DisplayNotation);

		//Process capacitive touch buttons to actions
		if(!isRunning){
			//Calculate capacitive touch button values
			CalcCapTouch();
		}
	}
}
