/*************************************************************************
 
    @file     DS1337.c
    @author   Dima Teller
    @date     30 March 2011
    @version  0.1

    @section DESCRIPTION

    Driver for DS1337 I2C Serial Real-Time-Clock-->(RTC).
    
*************************************************************************/
#include "gpio.h"

#include "i2c.h"
#include <stdint.h>
#include "ds1337.h"
#include <stdbool.h>

#include "serial.h"
#include "projectconfig.h"


extern volatile uint8_t   I2CMasterBuffer[BUFSIZE];
extern volatile uint8_t   I2CSlaveBuffer[BUFSIZE];
extern volatile uint32_t  I2CReadLength, I2CWriteLength;

uint32_t i, timeout;

static bool DS1337_Initialised = false;

/**************************************************************************/
/*! 
    @brief  Initialises the I2C block
*/
/**************************************************************************/
DS1337_Error_e DS1337_Init()
{
  // Initialise I2C
  if (I2CInit(I2CMASTER) == false)
  {
    return DS1337_ERROR_I2CINIT;    /* Fatal error */
  }

  // Set initialisation flag
  DS1337_Initialised = true;

  //Set PIO0_7 as input
  GPIO_SetDir(0, 7, 0);
  LPC_IOCON->PIO0_2 = 0b0000010000; //input function, pull-up

  	unsigned char byte[9];

  	byte[0] = 0x80; //A1M1
	byte[1] = 0x80; //A1M2
	byte[2] = 0x80; //A1M3
	byte[3] = 0x80; //A1M4
	byte[4] = 0; //A2M2
	byte[5] = 0; //A2M3
	byte[6] = 0; //A2M4
	byte[7] = 0b00011101; //enable oscillator, global interrupt and alarm1
	byte[8] = 0b00000000; //reset all flags

	DS1337_WriteBuffer(0x07, byte, 9);

	return DS1337_ERROR_OK;
}

/*************************************************************************

    @brief Reads the specified number of bytes from the supplied address.

    This function will read one or more bytes starting at the supplied
    address.  A maximum of 7 bytes can be read in one operation.

    @param[in]  address
                The 8-bit address where the read will start.
    @param[in]  *buffer
                Pointer to the buffer that will store the read results
    @param[in]  bufferLength
                Length of the buffer

*************************************************************************/
DS1337_Error_e DS1337_ReadBuffer (uint8_t address, uint8_t *buffer, uint32_t bufferLength)
{
  if (!DS1337_Initialised) DS1337_Init();

  if (address >= DS1337_MAXADDR)
  {
    return DS1337_ERROR_ADDRERR;
  }

 
  // ToDo: Check if I2C is ready

  // Clear buffers
  for ( i = 0; i < BUFSIZE; i++ )
  {
    I2CMasterBuffer[i] = 0x00;
    I2CSlaveBuffer[i] = 0x00;
  }
  
  // Write address bits to enable random read
  I2CWriteLength = 2;
  I2CReadLength = bufferLength;
  I2CMasterBuffer[0] = DS1337_ADDR_WRITE;  // I2C device address
  I2CMasterBuffer[1] = address;         // start Address 
  I2CMasterBuffer[2] = DS1337_ADDR_READ; 

  // Transmit command
  I2CEngine();

  // Fill response buffer
  for (i = 0; i < bufferLength; i++)
  {
    buffer[i] = I2CSlaveBuffer[i];
  }

  return DS1337_ERROR_OK;
}

//Write a single byte to the RTC
DS1337_Error_e DS1337_WriteByte (uint8_t address, uint8_t byte){

	  if (!DS1337_Initialised) DS1337_Init();

	  if (address > DS1337_MAXADDR)
	  {
	    return DS1337_ERROR_ADDRERR;
	  }

	  I2CWriteLength = 3;
	  I2CReadLength = 0;
	  I2CMasterBuffer[0] = DS1337_ADDR_WRITE; // I2C device address
	  I2CMasterBuffer[1] = address;   // Address
	  I2CMasterBuffer[2] = byte;

	  // Transmit command
	  I2CEngine();

		int i, j;
		for(i=0; i<1; i++){
			for(j=0; j<10000; j++){
				__asm volatile("NOP");
			}
		}

		return DS1337_ERROR_OK;
}


/*************************************************************************
 
    @brief Writes the supplied bytes at a specified address.

    This function will write one or more bytes starting at the supplied
    address.  A maximum of 7 bytes can be written in one operation.

    @param[in]  address
                The 8-bit address where the write will start.
    @param[in]  *buffer
                Pointer to the buffer that contains the values to write.
    @param[in]  bufferLength
                Length of the buffer

*************************************************************************/
DS1337_Error_e DS1337_WriteBuffer (uint8_t address, uint8_t *buffer, uint32_t bufferLength)
{
  if (!DS1337_Initialised) DS1337_Init();

  if (address > DS1337_MAXADDR)
  {
    return DS1337_ERROR_ADDRERR;
  }

  if (bufferLength > 16)
  {
    return DS1337_ERROR_BUFFEROVERFLOW;
  }

  // ToDo: Check if I2C is ready

  // Clear write buffer
  for ( i = 0; i < BUFSIZE; i++ )
  {
    I2CMasterBuffer[i] = 0x00;
  }

  // Write address bits and data to the master buffer
  I2CWriteLength = 2+bufferLength;
  I2CReadLength = 0;
  I2CMasterBuffer[0] = DS1337_ADDR_WRITE; // I2C device address
  I2CMasterBuffer[1] = address;   // Address 
  
  for (i = 0; i < bufferLength; i++)
  {
    I2CMasterBuffer[i+2] = buffer[i];
  }

  // Transmit command
  I2CEngine();

	int i, j;
	for(i=0; i<1; i++){
		for(j=0; j<10000; j++){
			__asm volatile("NOP");
		}
	}

	return DS1337_ERROR_OK;
}
