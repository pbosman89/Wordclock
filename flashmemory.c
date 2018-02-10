
#include "gpio.h"
#include "ssp.h"
#include "flashmemory.h"
#include "projectconfig.h"
#include "type.h"
#include "lpc_types.h"
#include "cdcuser.h"

//https://github.com/BleepLabs/S25FLx/blob/master/S25FLx.cpp

const int SoundNumbers[12] = {34, 40, 46, 52, 58, 64, 70, 76, 82, 88, 94, 100};
const int SoundHET = 10;
const int SoundIS = 16;
const int SoundHALF = 22;
const int SoundUUR = 106;



volatile unsigned long CurrAddress=0;

void Flash_WriteInit(unsigned long Address){
	CurrAddress = Address;

	Flash_WriteBegin(Address);
}

void Flash_WriteBegin(unsigned long Address){
	Flash_WriteEnable();
	Flash_WaitForIt();
	SSPSSEL();
	SSPSend(0x02); //Page program
	SSPSend(Address >>16);
	SSPSend(Address >>8);
	SSPSend(Address & 0xFF);

	//Writing is ready!
}

void Flash_WriteByte(unsigned char Byte){
	SSPSend(Byte);
	CurrAddress++;

	//Check if new page needs to be opened.
	if((CurrAddress>>8) > ((CurrAddress-1)>>8)){
		Flash_WriteEnd();				//End current page
		Flash_WriteBegin(CurrAddress);	//Start new page
	}
}

void Flash_WriteEnd(void){
	SSPDSEL();
	Flash_WaitForIt();
}


void Flash_WaitForIt(void){//VERIFIED

	int ret=0x01; //Don't have to reload 0x05 continuously! Check datasheet...

	while(ret & 0x01){
		SSPSSEL();
		SSPSend(0x05);
		ret = SSPSend(0);
		SSPDSEL();
	}
}

//Enable writing to the device
void Flash_WriteEnable(void){//VERIFIED
	SSPSSEL();
	SSPSend(0x06); //Write enable
	SSPDSEL();
	Flash_WaitForIt();
}

//Erase the complete flash
//Very slow process!! Takes 10 seconds to erase everything
void Flash_EraseAll(void){//VERIFIED
	Flash_WriteEnable();
	SSPSSEL();
	SSPSend(0xC7);	//Chip erase
	SSPDSEL();
	Flash_WaitForIt();
}


//Erase flash based on the amount of bytes
void Flash_EraseBytes(unsigned long Bytes){
	unsigned long i=0;
	unsigned long CurrEraseLoc=0;

	//Erase rounded number of pages plus one extra
	for(i=0; i<((Bytes/4096)+1); i++){
		Flash_WriteEnable();

		SSPSSEL();
		SSPSend(0x20); //Sector erase command

		//Location of flash to erase
		CurrEraseLoc = i*4096;
		SSPSend(CurrEraseLoc >>16);
		SSPSend(CurrEraseLoc >>8);
		SSPSend(CurrEraseLoc & 0xFF);

		SSPDSEL();			//Start execution
		Flash_WaitForIt();	//Wait until done
	}
}

//"struct Front FrontPanel" contains first page address location of corresponding WAV.

//Setup WAV Data and return song length
unsigned long SetupWAVData(unsigned long songLoc){
	unsigned long songStartLoc;
	unsigned long songSize;

	//Deselect (to be sure)
	SSPDSEL();

	//Select chip
	SSPSSEL();

	//Gather start location and byte size of the WAV clip
	//value songLoc points to the location where you'll find the start location and length of the clip.
	//<StartHigh><StartMid><StartLow><WAVHigh><WAVMid><WAVLow>

	//Set chip in read mode & at read location
	SSPSend(0x03); //Read mode
	SSPSend(songLoc >>16); //location high byte
	SSPSend(songLoc >>8); //location mid byte
	SSPSend(songLoc & 0xFF); //location low byte

	//Get starting location of the WAV
	songStartLoc = SSPSend(0) <<16;
	songStartLoc |= SSPSend(0) <<8;
	songStartLoc |= SSPSend(0);

	//Get the size of the song in 8-bit bytes
	songSize = SSPSend(0) <<16;
	songSize |= SSPSend(0) <<8;
	songSize |= SSPSend(0);

	//release chip
	SSPDSEL();

	//Now set chip to reading location of the values just received!

	//select chip
	SSPSSEL();

	//Set chip in read mode & at read location
	SSPSend(0x03); //Read mode
	SSPSend(songStartLoc >>16); //location high byte
	SSPSend(songStartLoc >>8); //location mid byte
	SSPSend(songStartLoc & 0xFF); //location low byte

	//Return the song size
	return songSize;
}

unsigned int ReadNextFromWAV(void){
	unsigned long ReadByte;

	//Read next byte in the song!
	ReadByte = SSPSend(0); //High byte
	ReadByte |= SSPSend(0) <<8; //Low byte

	//return the read byte
	return ReadByte;
}

void EndWAVData(void){
	//Release chip from duty
	SSPDSEL();
}
