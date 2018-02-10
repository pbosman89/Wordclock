/************************************************************************* 
    @file     DS1337.h
    @author   ldima55
    @date     30 March 2011
    @version  0.10

   

 
*************************************************************************/

#ifndef _DS1337_H_
#define _DS1337_H_

#include "projectconfig.h"
#include <stdint.h>

#define DS1337_ADDR_WRITE    0xD0          // 11010000
#define DS1337_ADDR_READ   0xD1          // 11010001
#define DS1337_RW      0x01
#define DS1337_READBIT 0x01
#define DS1337_MAXADDR 0x0F

typedef enum
{
  DS1337_ERROR_OK = 0,               // Everything executed normally
  DS1337_ERROR_I2CINIT,              // Unable to initialise I2C
  DS1337_ERROR_I2CBUSY,              // I2C already in use
  DS1337_ERROR_ADDRERR,              // Address out of range
  DS1337_ERROR_BUFFEROVERFLOW,       // Max 8 bytes can be read/written in one operation
  DS1337_ERROR_LAST
}
DS1337_Error_e;

DS1337_Error_e DS1337_Init (void);
DS1337_Error_e DS1337_ReadBuffer (uint8_t address, uint8_t *buffer, uint32_t bufferLength);
DS1337_Error_e DS1337_WriteBuffer (uint8_t address, uint8_t *buffer, uint32_t bufferLength);
DS1337_Error_e DS1337_WriteByte (uint8_t address, uint8_t byte);

#endif

