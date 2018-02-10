#include "gpio.h"
#include "type.h"
#include "inttypes.h"

#include <stdlib.h>
#include <string.h>

/*Libraries for USB CDC*/
#include "usb.h"
#include "usbcfg.h"
#include "usbhw.h"
#include "usbcore.h"
#include "cdc.h"
#include "cdcuser.h"

#include "serial.h"

#include "projectconfig.h"
#include "globalcomm.h"
#include "CapacitiveTouch.h"
#include "DisplayModes.h"

#include "flashmemory.h"

void reverse(char s[]){
	int i, j;
	char c;

	for(i=0, j=strlen(s)-1; i<j; i++, j--){
		c = s[i];
		s[i] = s[j];
		s[j] = c;
	}
}

void itoa(int n, char s[]){
	int i, sign;

	if ((sign = n) <0)
		n = -n;
	i=0;
	do {
		s[i++] = n % 10 + '0';
	} while ((n /= 10) > 0);
	if(sign <0)
		s[i++] = '-';
	s[i] = '\0';
	reverse(s);

}


/*----------------------------------------------------------------------------
  Defines for ring buffers
 *---------------------------------------------------------------------------*/
volatile char CommBuff[COMM_BUF_SIZE];
volatile unsigned int Comm_ReadPntr=0, Comm_WritePntr=0;


/*----------------------------------------------------------------------------
  checks the serial state and initiates notification
 *---------------------------------------------------------------------------*/
void VCOM_CheckSerialState (void) {
         unsigned short temp;
  static unsigned short serialState;

  temp = CDC_GetSerialState();
  if (serialState != temp) {
     serialState = temp;
     CDC_NotificationIn();                  // send SERIAL_STATE notification
  }
}

volatile bool Command_Avail=FALSE;

void ParseCommand(void){

	//If a command is available, parse it.
	if(Command_Avail){

		char parseBuffer[32] = {}; //initialize empty
		char backupBuffer[32] = {}; //initialize empty

		//Read command from buffer
		Comm_data(&parseBuffer[0], TRUE);

		strncpy(backupBuffer, parseBuffer, 32);

		char *parts[15];

		char *last;
		char *token;
		char **item = parts;

		token = strtok_r(parseBuffer, " ", &last);
		while (token) {
			*(item++) = token;
			token = strtok_r(NULL, " ", &last);
		}
		*item = NULL;

		if(strstr(parts[0], "GET")){
			if(strstr(parts[1], "TOUCH")){
				char str[16];
				CommSendString("Left: ");
				itoa(CapSensorOutput[0], str);
				CommSendString(str);
				CommSendString(" ");
				CommSendString("Right: ");
				itoa(CapSensorOutput[1], str);
				CommSendString(str);
				CommSendString("\r\n");
			}
			else if(strstr(parts[1], "LMIN")){
				char str[16];
				CommSendString("Display Intensity Min: ");
				itoa(DisplayIntensityMin, str);
				CommSendString(str);
				CommSendString("\r\n");
			}
			else if(strstr(parts[1], "LMAX")){
				char str[16];
				CommSendString("Display Intensity Max: ");
				itoa(DisplayIntensityMax, str);
				CommSendString(str);
				CommSendString("\r\n");
			}
			else if(strstr(parts[1], "TIME")){
				int Hr, Min,Sec;
				char str[16];

				GetTime(&Hr, &Min, &Sec);

				CommSendString("Current Time: ");
				itoa(Hr, str);
				CommSendString(str);
				CommSendByte(':');
				itoa(Min, str);
				CommSendString(str);
				CommSendByte(':');
				itoa(Sec, str);
				CommSendString(str);
				CommSendString("\r\n");
			}
			else if(strstr(parts[1], "MODE")){
				char str[16];

				CommSendString("Current Mode: ");
				itoa(DisplayMode, str);
				CommSendString(str);
				CommSendString("\r\n");
			}
			else if(strstr(parts[1], "NOTAT")){
				if(DisplayNotation == 5)
					CommSendString("Notation is 5\r\n");
				else
					CommSendString("Notation is 1\r\n");
			}
			else if(strstr(parts[1], "AUDIOENA")){
				CommSendString("Audio Enabled: ");
				if(AudioEnabled == TRUE)
					CommSendString("True");
				else
					CommSendString("False");

				CommSendString("\r\n");
			}
			else if(strstr(parts[1], "DASHENA")){
				if(DashEnabled == FALSE)
					CommSendString("Dash Disabled\r\n");
				else
					CommSendString("Dash Enabled\r\n");
			}
			else if(strstr(parts[1], "LANG")){
				CommSendString("The chosen langage is ");
				if(FrontLang == 1)
					CommSendString("English");
				else
					CommSendString("Dutch");

				CommSendString("\r\n");
			}
			else{
				CommSendString("Unknown Command: ");
				CommSendString(backupBuffer);
				CommSendString("\r\n");
			}
		}
		else if(strstr(parts[0], "SET")){
			if(strstr(parts[1], "TIME")){
				int hr = (parts[2][0]-0x30) * 10 + (parts[2][1]-0x30);
				int min = (parts[2][3]-0x30) * 10 + (parts[2][4]-0x30);
				int sec = (parts[2][6]-0x30) * 10 + (parts[2][7]-0x30);

				if(hr > 24 || hr < 0)
					hr = 12;
				if(min > 59 || min < 0)
					min = 0;
				if(sec > 59 || sec < 0)
					sec = 0;

				SetTime(hr, min, sec);
				CommSendString("New time set\r\n");
			}
			else if(strstr(parts[1], "LMIN")){
				DisplayIntensityMin = atoi(parts[2]);
				if(DisplayIntensityMin < 1)
					DisplayIntensityMin = 1;

				CommSendString("New display min intensity set\r\n");
			}
			else if(strstr(parts[1], "LMAX")){
				DisplayIntensityMax = atoi(parts[2]);

				if(DisplayIntensityMax > 100)
					DisplayIntensityMax = 100;

				CommSendString("New display max intensity set\r\n");
			}
			else if(strstr(parts[1], "TRANSPD")){
				TransSpeedFactor = atoi(parts[2]);
				if(TransSpeedFactor < 1)
					TransSpeedFactor = 1;
				else if(TransSpeedFactor > 1000)
					TransSpeedFactor = 1000;

				CommSendString("Transition speed factor set\r\n");
			}
			else if(strstr(parts[1], "LANG")){
				FrontLang = atoi(parts[2]);
				if(FrontLang < 1)
					FrontLang = 0;
				else if(FrontLang > 1)
					FrontLang = 0;

				CommSendString("New language set\r\n");
			}
			else if(strstr(parts[1], "AUDIOENA")){
				if(strstr(parts[2], "TRUE")){
					AudioEnabled = TRUE;
					CommSendString("Audio Enabled\r\n");
				}
				else{
					AudioEnabled = FALSE;
					CommSendString("Audio Disabled\r\n");
				}
			}
			else if(strstr(parts[1], "DASHENA")){
				if(strstr(parts[2], "FALSE")){
					DashEnabled = FALSE;
					CommSendString("Display dash disabled\r\n");
				}
				else{
					DashEnabled = TRUE;
					CommSendString("Display dash enabled\r\n");
				}
			}
			else if(strstr(parts[1], "MODE")){
				int reqmode = atoi(parts[2]);
				if(reqmode != 110 && reqmode != 120 && reqmode != 130 && reqmode != 140 && reqmode != 150 && reqmode != 160 && reqmode != 170){
					CommSendString("Mode Unknown\r\n");
				}
				else{
					CommSendString("New mode set\r\n");
					DisplayMode = reqmode;
				}
			}
			else if(strstr(parts[1], "NOTAT")){
				int reqnotat= atoi(parts[2]);
				if(reqnotat != 1 && reqnotat != 5){
					CommSendString("Notation Unknown\r\n");
				}
				else{
					CommSendString("New notation set\r\n");
					DisplayNotation = reqnotat;
				}
			}
			else if(strstr(parts[1], "TOUCH")){
				int filter = atoi(parts[2]);
				int thresholdHigh = atoi(parts[3]);
				int thresholdLow = atoi(parts[4]);
				int shortPress = atoi(parts[5]);
				int longPress = atoi(parts[6]);
				SetSenCapTouch(filter, thresholdHigh, thresholdLow, shortPress, longPress);
				CommSendString("New Touch sensitivity set\r\n");
			}
			else{
				CommSendString("Unknown Command: ");
				CommSendString(backupBuffer);
				CommSendString("\r\n");
			}
		}
		else if(strstr(parts[0], "CLRFLASH")){
			int EraseSize = atoi(parts[1]);
			Flash_WaitForIt();
			Flash_EraseBytes(EraseSize);
			Flash_WaitForIt();

			CommSendString("Flash erased\r\n");
		}
		else if(strstr(parts[0], "UPLFLASH")){

			//Get flash write size
			UploadAudioProcessingSize = atoi(parts[1]);

			Flash_WaitForIt();
			//Erase flash
			Flash_EraseBytes(UploadAudioProcessingSize);
			Flash_WaitForIt();	//Wait until done
			Flash_WriteInit(0); //Initialize writing
			//Signal ready
			CommSendString("OK \r\n");

			UploadAudioProcessing = TRUE;
		}
		else if(strstr(parts[0], "DWNFLASH")){
			//Deselect flash
		//	SSPDSEL();
			//Select flash
		//	SSPSSEL();

			//Set flash to read mode and read start location (106 = furthest location of WAV file)
//			SSPSend(0x03); SSPSend(0); SSPSend(0); SSPSend(106);
			//int readsize = ((SSPSend(0) <<16) | (SSPSend(0) <<8) | SSPSend(0)) + ((SSPSend(0) <<16) | (SSPSend(0) <<8) | SSPSend(0));
			int readsize = atoi(parts[1]);


//			char str[16];
	//		itoa(readsize, str);
	//		CommSendString(str);

			//release chip
			SSPDSEL();
			//select chip
			SSPSSEL();

			//Set flash read mode and location at start of memory
			SSPSend(0x03); SSPSend(0); SSPSend(0); SSPSend(0);

			int i;
			int totalread=0;
			for(i=0; i<(readsize/256); i++){
				int j;
				char data[256];
				for(j=0; j<256; j++){
					data[j] = SSPSend(0);
					totalread++;
				}
				CommSend(&data[0], 256);
			}

			//Send remainder
			int remaining = readsize - totalread;
			char data[256];
			for(i=0; i<remaining; i++){
				data[i] = SSPSend(0);
			}
			CommSend(&data[0], remaining);

		}
		else{
			CommSendString("Unknown Command: ");
			CommSendString(backupBuffer);
			CommSendString("\r\n");
		}

		//Reset for next time
		Command_Avail = FALSE;
	}
}


void Comm_write(const char *buffer, int *length){

	int bytesToWrite = *length;

	while (bytesToWrite--) {

		Comm_WritePntr++;

		if(Comm_WritePntr >= COMM_BUF_SIZE)
			Comm_WritePntr = 0;

		CommBuff[Comm_WritePntr] = *buffer++;

		if(CommBuff[Comm_WritePntr] == '\n')
			Command_Avail = TRUE;
	}
}

void Comm_data(char *buffer, bool UpdatePntr){
	int bytesToRead = Comm_AvailChar();

	//set pointer at latest read + 1(newest available)
	int localpointer = Comm_ReadPntr + 1;

	if(localpointer >= COMM_BUF_SIZE)
		localpointer = 0;

	while (bytesToRead--) {
		*buffer++ = CommBuff[localpointer];

		localpointer++;

		if(localpointer >= COMM_BUF_SIZE)
			localpointer = 0;
		}

	if(UpdatePntr == TRUE){
		Comm_ReadPntr = Comm_WritePntr;
	}
}

int Comm_AvailChar(void){
	if(Comm_WritePntr >= Comm_ReadPntr)
		return (Comm_WritePntr - Comm_ReadPntr);
	else{
		return (64 - (Comm_ReadPntr - Comm_WritePntr));
	}
}


void CommSend(const char *buffer, int length){
    ser_Write (&buffer[0], &length);

    if(USBConnected){
    	/*
    	int timeout=0;
    	//Wait till buffer is empty
    	while((!CDC_DepInEmpty) && (timeout < 0xFFFFFF)){
    		timeout++;
    	};
*/
    	while(!CDC_DepInEmpty);
    	//Fill buffer with new data
    	CDC_WrInBuf (&buffer[0], &length);
    	CDC_DepInEmpty = FALSE;

    	//Trigger interrupt to start CDC_BulkIn event
		LPC_USB->DevIntSet = 1 << 4; // force endpoint 1 IN (Physical EP3) interrupt
    }
}

void CommSendString(char *data){
	int length = strlen(data);
	CommSend(data, length);
}

void CommSendByte(char Data){
	char serBuf[1] = {Data};

	CommSend(&serBuf[0], 1);
}

//---------------------------------------------------------------------------
//Check if there is data available in the serial or USB buffer and send to communication buffer
void CommRead(void){

	//Read from external devices
	char serBuf [USB_CDC_BUFSIZE];
	int  numBytesRead, numAvailByte;

	ser_AvailChar (&numAvailByte);
	if (numAvailByte > 0) {
		  numBytesRead = ser_Read (&serBuf[0], &numAvailByte);
		  Comm_write(&serBuf[0], &numBytesRead);
	}

	if(USBConnected){
		if(!CDC_DepOutEmpty){
			CDC_OutBufAvailChar (&numAvailByte);
			if (numAvailByte > 0) {
				numBytesRead = CDC_RdOutBuf (&serBuf[0], &numAvailByte);
				Comm_write(&serBuf[0], &numBytesRead);
			}

			CDC_DepOutEmpty = TRUE;
		}
		else if(CDC_DepOutWaiting){
			//Trigger interrupt to start CDC_BulkOut event
			LPC_USB->DevIntSet = 1 << 4; // force endpoint 1 IN (Physical EP3) interrupt
		}

	}

	VCOM_CheckSerialState();
}
