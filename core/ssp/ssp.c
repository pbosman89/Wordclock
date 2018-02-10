#include "LPC13xx.h"			/* LPC13xx Peripheral Registers */
#include "gpio.h"
#include "ssp.h"

//SOFTWARE SPI USING BIT-BANG!!

void SSPInit(void){

	//Set ports to I/O Mode
	LPC_IOCON->PIO0_2  = 0x00;				//SSEL
	LPC_IOCON->PIO0_9  = 0x00;				//MOSI
	LPC_IOCON->JTAG_TCK_PIO0_10  = 0x81;	//SCK
	LPC_IOCON->JTAG_TDI_PIO0_11  = 0x81;	//MISO

	//Set port direction
	GPIO_SetDir(PORT0, 2, 1);
	GPIO_SetDir(PORT0, 9, 1);
	GPIO_SetDir(PORT0, 10, 1);
	GPIO_SetDir(PORT0, 11, 0);

	SSPSSEL();

	//Continuous read mode reset
	SSPSend(0xFF);
	SSPSend(0xFF);

	SSPDSEL();

	SSPSSEL();
	SSPDSEL();
}

void SSPSSEL(void){
	GPIO_ResetBits(PORT0, 2);
}

void SSPDSEL(void){
	GPIO_SetBits(PORT0, 2);
}

unsigned char SSPSend(unsigned char Data){

	unsigned int i;

	for(i = 0; i<8; i++){
		if(Data & 0x80) // MOSI
			GPIO_SetBits(PORT0, 9);
		else
			GPIO_ResetBits(PORT0, 9);

		Data = Data <<1;

		//SCK one clock
		GPIO_ResetBits(PORT0, 10);

		if(LPC_GPIO0->DATA & 1<<11)//MISO
			Data++;

		GPIO_SetBits(PORT0, 10);
	}

	return Data;
}

