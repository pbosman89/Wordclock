/****************************************************************************
 *   $Id:: timer32.h 7208 2011-04-27 00:43:31Z usb00423                     $
 *   Project: NXP LPC13xx software example
 *
 *   Description:
 *     This file contains definition and prototype for 32-bit timer 
 *     configuration.
 *
 ****************************************************************************
 * Software that is described herein is for illustrative purposes only
 * which provides customers with programming information regarding the
 * products. This software is supplied "AS IS" without any warranties.
 * NXP Semiconductors assumes no responsibility or liability for the
 * use of the software, conveys no license or title under any patent,
 * copyright, or mask work right to the product. NXP Semiconductors
 * reserves the right to make changes in the software without
 * notification. NXP Semiconductors also make no representation or
 * warranty that such application will be suitable for the specified
 * use without further testing or modification.
****************************************************************************/
#ifndef __TIMER32_H 
#define __TIMER32_H

/* The test is either MAT_OUT or CAP_IN. Default is MAT_OUT. */
#define TIMER_MATCH		1

#define TIME_INTERVAL	(SystemCoreClock/100 - 1)
/* depending on the SystemFrequency and System AHB clock divider setting, 
if SystemFrequency = 60Mhz, SYSAHBCLKDIV = 4, SystemAHBFrequency = 1/4 SystemFrequency, 
10mSec = 150.000-1 counts */

void delay32Ms(uint8_t timer_num, uint32_t delayInMs);
void TIMER32_0_IRQHandler(void);
void TIMER32_1_IRQHandler(void);
void enable_timer32(uint8_t timer_num);
void disable_timer32(uint8_t timer_num);
void reset_timer32(uint8_t timer_num);
void init_timer32(uint8_t timer_num, uint32_t timerInterval);

#endif /* end __TIMER32_H */
/*****************************************************************************
**                            End Of File
******************************************************************************/
