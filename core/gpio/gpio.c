/***********************************************************************//**
 * @file	: lpc13xx_GPIO.c
 * @brief	: Contains all functions support for GPIO firmware library on lpc13xx
 * @version	: 1.0
 * @date	: 22. Jan. 2010
 * @author	: Coocox
 **********************************************************************/

/* Peripheral group ----------------------------------------------------------- */
/** @addtogroup GPIO
 * @{
 */

/* Includes ------------------------------------------------------------------- */
#include "gpio.h"
#include "lpc13xx_syscon.h"
#include "lpc13xx_libcfg.h"


#if _GPIO

/* Public Functions ---------------------------------------------------------- */
/** @defgroup GPIO_Public_Functions
 * @{
 */

/*********************************************************************//**
 * @brief		Get pointer to GPIO peripheral due to GPIO port
 * @param[in]	portNum		Port Number value, should be in range from 0 to 4.
 * @return		Pointer to GPIO peripheral
 **********************************************************************/
static GPIO_TypeDef *GPIO_GetPointer(uint8_t port)
{
	GPIO_TypeDef *pGPIO = NULL;

	switch (port) {
	case PORT0:
		pGPIO = LPC_GPIO0;
		break;
	case PORT1:
		pGPIO = LPC_GPIO1;
		break;
	case PORT2:
		pGPIO = LPC_GPIO2;
		break;
	case PORT3:
		pGPIO = LPC_GPIO3;
		break;
	default:
		break;
	}

	return pGPIO;
}


/********************************************************************//**
 * @brief 		Set Direction for GPIO port.
 * @param[in]	port specifies the port, it can be
 *                    -PORT0 : GPIO0
 *                    -PORT1 : GPIO1
 *                    -PORT2 : GPIO2
 *                    -PORT3 : GPIO3
 * @param[in]	pins specifies the port bits to be written (0 - 11)
 * @param[in]	dir  Direction value, should be:
 *                    -0: Input.
 *                    -1: Output.
 * @return		None
 *********************************************************************/
void GPIO_SetDir (uint8_t port, uint32_t pins, uint8_t dir)
{
    GPIO_TypeDef *pGPIO;

    CHECK_PARAM (PARAM_PORT(port));

    pGPIO = GPIO_GetPointer(port);
    
    if(dir == 0) {
        pGPIO->DIR &= (~ (1<<pins)) & GPIO_PINS_MASK;
    } else {
        pGPIO->DIR |= (1<<pins) & GPIO_PINS_MASK;
    }  
}


/********************************************************************//**
 * @brief 		Sets the selected data port bits.
 * @param[in]	specifies the port, it can be
 *               -PORT0 : GPIO0
 *               -PORT1 : GPIO1
 *               -PORT2 : GPIO2
 *               -PORT3 : GPIO3
 * @param[in]	specifies the port bits to be written (0 - 11)
 * @return		None
 *********************************************************************/
void GPIO_SetBits (uint8_t port, uint32_t pins)
{    
    GPIO_TypeDef *pGPIO;

    CHECK_PARAM (PARAM_PORT(port));

    pGPIO = GPIO_GetPointer(port);
    
    pGPIO->MASKED_ACCESS[(1<<pins) & GPIO_PINS_MASK] = (1<<pins) & GPIO_PINS_MASK;
}


/********************************************************************//**
 * @brief 		Resets the selected data port bits.
 * @param[in]	specifies the port, it can be
 *               -PORT0 : GPIO0
 *               -PORT1 : GPIO1
 *               -PORT2 : GPIO2
 *               -PORT3 : GPIO3
 * @param[in]	specifies the port bits to be written (0 - 11)
 * @return		None
 *********************************************************************/
void GPIO_ResetBits (uint8_t port, uint32_t pins)
{    
    GPIO_TypeDef *pGPIO;        
    
    CHECK_PARAM (PARAM_PORT(port));

    pGPIO = GPIO_GetPointer(port);        
    
    pGPIO->MASKED_ACCESS[(1<<pins) & GPIO_PINS_MASK] = 0;
}


/********************************************************************//**
 * @brief 		Reads the specified GPIO input data port.
 * @param[in]	specifies the port, it can be
 *               -PORT0 : GPIO0
 *               -PORT1 : GPIO1
 *               -PORT2 : GPIO2
 *               -PORT3 : GPIO3 
 * @return		GPIO input data port value
 *********************************************************************/
uint32_t GPIO_ReadInput (uint8_t port)
{  
    GPIO_TypeDef *pGPIO;
    
    CHECK_PARAM(PARAM_PORT(port));

    pGPIO = GPIO_GetPointer(port);
    
    return (pGPIO->DATA & GPIO_PINS_MASK);
}


/********************************************************************//**
 * @brief 		Reads the specified input port pin.
 * @param[in]	port specifies the port, it can be
 *               -PORT0 : GPIO0
 *               -PORT1 : GPIO1
 *               -PORT2 : GPIO2
 *               -PORT3 : GPIO3 
 * @param[in]   pin specifies the port bit (0-11) to read 
 * @return		The input port pin value
 *********************************************************************/
FlagStatus GPIO_ReadInputPin(uint8_t port, uint32_t pin)
{      
    FlagStatus bitSta = RESET;
    GPIO_TypeDef *pGPIO = GPIO_GetPointer(port);
    
    CHECK_PARAM(PARAM_PORT(port));
    
    if(pGPIO->DATA & pin) {
        bitSta = SET;
    } else {
        bitSta = RESET;
    }   
    
    return bitSta; 
}


/********************************************************************//**
 * @brief 		Initializes the port event according to the specified
 *              parameters in the Event_InitStruct.
 * @param[in]	Event_InitStruct pointer to a EVENT_InitTypeDef structure
 *
 * @return		None
 *********************************************************************/
void GPIO_EventInit(EVENT_InitTypeDef *Event_InitStruct)
{    
    GPIO_TypeDef *pGPIO;

    CHECK_PARAM(PARAM_PORT(Event_InitStruct->port));
    CHECK_PARAM(PARAM_EVENT_TYPEDEF(Event_InitStruct->EVENT_Mode));

    pGPIO = GPIO_GetPointer(Event_InitStruct->port);

    /* Select the trigger for the selected pins interrupt */
    switch(Event_InitStruct->EVENT_Mode) {
    case EVENT_HIGH_LEVEL:        
    case EVENT_LOW_LEVEL:
        pGPIO->IS |= Event_InitStruct->pins;
        if(Event_InitStruct->EVENT_Mode == EVENT_HIGH_LEVEL) {
             pGPIO->IEV |= Event_InitStruct->pins & GPIO_PINS_MASK;  
        } else {
            pGPIO->IEV &= ~Event_InitStruct->pins & GPIO_PINS_MASK;        
        }
        break;
    case EVENT_RISING_EDGE:        
    case EVENT_FALLING_EDGE:
    case EVENT_BOTH_EDGE:
        pGPIO->IS &= ~Event_InitStruct->pins & GPIO_PINS_MASK;
        if(Event_InitStruct->EVENT_Mode == EVENT_BOTH_EDGE) {
            pGPIO->IBE |= Event_InitStruct->pins & GPIO_PINS_MASK;
        } else if (Event_InitStruct->EVENT_Mode == EVENT_RISING_EDGE) {
            pGPIO->IEV |= Event_InitStruct->pins & GPIO_PINS_MASK;  
        } else {
            pGPIO->IEV &= ~Event_InitStruct->pins & GPIO_PINS_MASK;        
        }           
        break;
    };

    /* Enable or Disable the pins interrupt */
    if (Event_InitStruct->INTCmd == ENABLE) {
        pGPIO->IE |= Event_InitStruct->pins & GPIO_PINS_MASK;
    } else {
        pGPIO->IE &= ~Event_InitStruct->pins & GPIO_PINS_MASK;    
    }
}


/********************************************************************//**
 * @brief 		Get event status of the selected port  
 * @param[in]	specifies the port, it can be
 *               -PORT0 : GPIO0
 *               -PORT1 : GPIO1
 *               -PORT2 : GPIO2
 *               -PORT3 : GPIO3 
 * @return		The event status
 *********************************************************************/
uint32_t GPIO_GetEventStatus(uint8_t port) 
{
    GPIO_TypeDef *pGPIO;
    
    CHECK_PARAM(PARAM_PORT(port));

    pGPIO = GPIO_GetPointer(port);
        
    return (pGPIO->RIS & GPIO_PINS_MASK);   
}


/********************************************************************//**
 * @brief 		Get interrupt status of the selected port 
 * @param[in]	specifies the port, it can be
 *               -PORT0 : GPIO0
 *               -PORT1 : GPIO1
 *               -PORT2 : GPIO2
 *               -PORT3 : GPIO3 
 * @return		The interrupt status
 *********************************************************************/
uint32_t GPIO_GetIntStatus(uint8_t port)
{
    GPIO_TypeDef *pGPIO;

    CHECK_PARAM(PARAM_PORT(port));

    pGPIO = GPIO_GetPointer(port);
    
    return (pGPIO->MIS & GPIO_PINS_MASK);    
}


/********************************************************************//**
 * @brief 		Clear the pins interrupt status flag
 * @param[in]	port specifies the port, it can be
 *               -PORT0 : GPIO0
 *               -PORT1 : GPIO1
 *               -PORT2 : GPIO2
 *               -PORT3 : GPIO3 
 * @param[in]	pins specifie the selected pins
 * @return		None
 *********************************************************************/
void GPIO_ClearInt(uint8_t port, uint32_t pins) 
{
    GPIO_TypeDef *pGPIO;

    CHECK_PARAM(PARAM_PORT(port));

    pGPIO = GPIO_GetPointer(port);
    
    pGPIO->IC = pins & GPIO_PINS_MASK;  
}


/********************************************************************//**
 * @brief 		Enable or Disable the port interrupt in NVIC
 * @param[in]	port specifies the port, it can be
 *               -PORT0 : GPIO0
 *               -PORT1 : GPIO1
 *               -PORT2 : GPIO2
 *               -PORT3 : GPIO3 
 * @param[in]	cmd, it can be:
 *               -ENABLE  : ENABLE the selected port interrupt
 *               -DISABLE : DISABLE the selected port interrupt
 * @return		None
 *********************************************************************/
void GPIO_PortIntCmd(uint8_t port, FunctionalState cmd)
{
    IRQn_Type IRQn;

    CHECK_PARAM(PARAM_PORT(port));
    
    switch(port) {
    case PORT0:
        IRQn = EINT0_IRQn; break;
    case PORT1:
        IRQn = EINT1_IRQn; break;
    case PORT2:
        IRQn = EINT2_IRQn; break;        
    case PORT3:
        IRQn = EINT3_IRQn; break;
    default: return;
    }  

    if(cmd == DISABLE) {        
        NVIC_DisableIRQ(IRQn);        
    } else {
#include <lpc13xx.h>
        NVIC_EnableIRQ(IRQn);        
    }
}


/**
 * @}
 */

#endif   /* _GPIO */

/**
 * @}
 */
/* --------------------------------- End Of File ------------------------------ */
