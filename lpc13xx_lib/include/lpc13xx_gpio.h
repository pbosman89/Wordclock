/***********************************************************************//**
 * @file	: lpc13xx_GPIO.h
 * @brief	: Contains all macro definitions and function prototypes
 * 				support for GPIO firmware library on lpc13xx
 * @version	: 1.0
 * @date	: 25. Jan. 2010
 * @author	: Coocox
 **************************************************************************/

/* Peripheral group ----------------------------------------------------------- */
/** @defgroup GPIO
 * @ingroup lpc1300CMSIS_FwLib_Drivers
 * @{
 */

#ifndef __lpc13XX_GPIO_H_
#define __lpc13XX_GPIO_H_

/* Includes ------------------------------------------------------------------- */
#include "lpc13xx.h"
#include "lpc_types.h"


#ifdef __cplusplus
extern "C"
{
#endif


/* Public Types --------------------------------------------------------------- */
/** @defgroup GPIO_Public_Types
 * @{
 */

 /**
 * @brief GPIO interrupt trigger mode enumeration
 */
typedef enum
{
    EVENT_HIGH_LEVEL = 0x0,      /**< High level on pin trigger an interrupt */
    EVENT_LOW_LEVEL,             /**< Low level on pin trigger an interrupt */
    EVENT_RISING_EDGE,           /**< Rising edge on pin trigger an interrupt */
    EVENT_FALLING_EDGE,          /**< Falling edge on pin trigger an interrupt */
    EVENT_BOTH_EDGE,             /**< Both edge on pin trigger an interrupt */
}EVENT_TypeDef;

#define PARAM_EVENT_TYPEDEF(n)  ((n==EVENT_HIGH_LEVEL) || \
                                 (n==EVENT_LOW_LEVEL) || \
                                 (n==EVENT_RISING_EDGE) || \
                                 (n==EVENT_FALLING_EDGE) || \
                                 (n==EVENT_BOTH_EDGE)) 

/**
 * @brief GPIO Event init structure definition
 */
typedef struct
{
    uint8_t port;               /**< Specifies the GPIO port to be configured.
                                     This parameter can be PORT0,PORT1,PORT2,PORT3 */
    uint32_t pins;              /**< Specifies the GPIO pins to be configured.
                                     This parameter can be GPIO_Pin_0 - GPIO_Pin_11*/
    EVENT_TypeDef EVENT_Mode;   /**< Specifies interrupt trigger mode for the selected pins.
                                     This parameter can be a value of @EVENT_TypeDef */     
    FunctionalState INTCmd;     /**< Specifies the new interrupt state of the selected pins.
                                     This parameter can be set either to ENABLE or DISABLE */
}EVENT_InitTypeDef;

/**
 * @}
 */


/* Public Macros -------------------------------------------------------------- */
/** @defgroup GPIO_Public_Macros
 * @{
 */

#define  PORT0   0      /** GPIO port 0 */
#define  PORT1   1      /** GPIO port 1 */
#define  PORT2   2      /** GPIO port 2 */
#define  PORT3   3      /** GPIO port 3 */

#define PARAM_PORT(n)   (n==PORT0 || n==PORT1 || n==PORT2 || n==PORT3)


#define GPIO_Pin_0      ((uint32_t)1 <<  0)   /** Pin  0 selected */
#define GPIO_Pin_1      ((uint32_t)1 <<  1)   /** Pin  1 selected */
#define GPIO_Pin_2      ((uint32_t)1 <<  2)   /** Pin  2 selected */
#define GPIO_Pin_3      ((uint32_t)1 <<  3)   /** Pin  3 selected */
#define GPIO_Pin_4      ((uint32_t)1 <<  4)   /** Pin  4 selected */
#define GPIO_Pin_5      ((uint32_t)1 <<  5)   /** Pin  5 selected */
#define GPIO_Pin_6      ((uint32_t)1 <<  6)   /** Pin  6 selected */
#define GPIO_Pin_7      ((uint32_t)1 <<  7)   /** Pin  7 selected */
#define GPIO_Pin_8      ((uint32_t)1 <<  8)   /** Pin  8 selected */
#define GPIO_Pin_9      ((uint32_t)1 <<  9)   /** Pin  9 selected */
#define GPIO_Pin_10     ((uint32_t)1 << 10)   /** Pin 10 selected */
#define GPIO_Pin_11     ((uint32_t)1 << 11)   /** Pin 11 selected */

/** GPIO pins mask */
#define GPIO_PINS_MASK  ( GPIO_Pin_0 | GPIO_Pin_1  | GPIO_Pin_2 | \
                          GPIO_Pin_3 | GPIO_Pin_4  | GPIO_Pin_5 | \
                          GPIO_Pin_6 | GPIO_Pin_7  | GPIO_Pin_8 | \
                          GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11 )

#define PARAM_PINS(n)   (n & GPIO_PINS_MASK)


/**
 * @}
 */


/* Public Functions ----------------------------------------------------------- */
/** @defgroup GPIO_Public_Functions
 * @{
 */

/* GPIO style ------------------------------- */
void GPIO_SetDir (uint8_t port, uint32_t pins, uint8_t dir);
void GPIO_SetBits (uint8_t port, uint32_t pins);
void GPIO_ResetBits (uint8_t port, uint32_t pins);
uint32_t GPIO_ReadInput (uint8_t port);
FlagStatus GPIO_ReadInputPin(uint8_t port, uint32_t pin);

/* GPIO Event and Interrupt style ------------------------------- */
void GPIO_EventInit(EVENT_InitTypeDef *Event_InitStruct);
uint32_t GPIO_GetEventStatus(uint8_t port);
uint32_t GPIO_GetIntStatus(uint8_t port);
void GPIO_ClearInt(uint8_t port, uint32_t pins);
void GPIO_PortIntCmd(uint8_t port, FunctionalState cmd);



/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif /* __lpc13XX_GPIO_H_ */

/**
 * @}
 */

/* --------------------------------- End Of File ------------------------------ */
