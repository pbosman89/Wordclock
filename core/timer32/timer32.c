/****************************************************************************
 *   $Id:: timer32.c 7207 2011-04-27 00:42:41Z usb00423                     $
 *   Project: NXP LPC13xx 32-bit timer example
 *
 *   Description:
 *     This file contains 32-bit timer code example which include timer 
 *     initialization, timer interrupt handler, and related APIs for 
 *     timer setup.
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
#include "LPC13xx.h"
#include "timer32.h"
#include "lpc_types.h"
#include "type.h"

#include "CapacitiveTouch.h"


/******************************************************************************
** Function name:		TIMER32_0_IRQHandler
**
** Descriptions:		Timer/Counter 0 interrupt handler
**						executes each 10ms @ 60 MHz CPU Clock
**
** parameters:			None
** Returned value:		None
** 
******************************************************************************/
void TIMER32_0_IRQHandler(void)
{
//	static int UpdateTime_Timer = 0;

	if ( LPC_TMR32B0->IR & 0x01 ){
		LPC_TMR32B0->IR = 1;				/* clear interrupt flag */
	}

	if ( LPC_TMR32B0->IR & (0x1<<4) )
	{
		LPC_TMR32B0->IR = 0x1<<4;			/* clear interrupt flag */
	}

	return;
}

/******************************************************************************
** Function name:		TIMER32_1_IRQHandler
**
** Descriptions:		Timer/Counter 1 interrupt handler
**						executes each 10ms @ 60 MHz CPU Clock
**
** parameters:			None
** Returned value:		None
** 
******************************************************************************/
void TIMER32_1_IRQHandler(void)
{
	if ( LPC_TMR32B1->IR & 0x01 )
	{
		LPC_TMR32B1->IR = 1;			/* clear interrupt flag */
	}

	//Capacitive touch capture interrupt routine
	if ( LPC_TMR32B1->IR & (0x1<<4) )
	{
		//Process channel 0 data
		CaptureCapTouch(0);

		//Clear interrupt flag
		LPC_TMR32B1->IR = 1<<4;
	}

	return;
}

/******************************************************************************
** Function name:		enable_timer
**
** Descriptions:		Enable timer
**
** parameters:			timer number: 0 or 1
** Returned value:		None
** 
******************************************************************************/
void enable_timer32(uint8_t timer_num)
{
  if ( timer_num == 0 )
  {
    LPC_TMR32B0->TCR = 1;
  }
  else
  {
    LPC_TMR32B1->TCR = 1;
  }
  return;
}

/******************************************************************************
** Function name:		disable_timer
**
** Descriptions:		Disable timer
**
** parameters:			timer number: 0 or 1
** Returned value:		None
** 
******************************************************************************/
void disable_timer32(uint8_t timer_num)
{
  if ( timer_num == 0 )
  {
    LPC_TMR32B0->TCR = 0;
  }
  else
  {
    LPC_TMR32B1->TCR = 0;
  }
  return;
}

/******************************************************************************
** Function name:		reset_timer
**
** Descriptions:		Reset timer
**
** parameters:			timer number: 0 or 1
** Returned value:		None
** 
******************************************************************************/
void reset_timer32(uint8_t timer_num)
{
  uint32_t regVal;

  if ( timer_num == 0 )
  {
    regVal = LPC_TMR32B0->TCR;
    regVal |= 0x02;
    LPC_TMR32B0->TCR = regVal;
  }
  else
  {
    regVal = LPC_TMR32B1->TCR;
    regVal |= 0x02;
    LPC_TMR32B1->TCR = regVal;
  }
  return;
}

void OneShot(int Direction){
	LPC_SYSCON->SYSAHBCLKCTRL |= (1<<10);	//Enable clock to 32-bit timer
	LPC_TMR32B1->PR = 72;					//Clock divider, system clock 72MHz / 1MHz = 72
	LPC_IOCON->ARM_SWDIO_PIO1_3 = 0b11010011;	//Connect GPIO with Timer

	LPC_TMR32B1->MCR = (1<<8); 	//Stop counter on MR2

	if(Direction)
		LPC_TMR32B1->EMR = (2<<8); 	//Set MAT2 output HIGH on match
	else
		LPC_TMR32B1->EMR = (2<<8); 	//Clear MAT2 output HIGH on match

	LPC_TMR32B1->MR2 = 255;		//Set MAT2 top value

	LPC_TMR32B1->TCR = (1<<0) | (1<<1);//Enable timer, reset counter & prescale
	LPC_TMR32B1->TCR &= ~(1<<1); //Start counting
}

/******************************************************************************
** Function name:		init_timer
**
** Descriptions:		Initialize timer, set timer interval, reset timer,
**						install timer interrupt handler
**
** parameters:			timer number and timer interval
** Returned value:		None
** 
******************************************************************************/
void init_timer32(uint8_t timer_num, uint32_t TimerInterval) 
{
	if (timer_num == 0){
		LPC_SYSCON->SYSAHBCLKCTRL |= (1<<9);	//Enable clock to 32-bit timer
		LPC_TMR32B0->EMR = 0;					//No external interrupt
		//LPC_TMR32B0->MCR = 3;					//Interrupt and reset on MR0
		//LPC_TMR32B0->PR = 72;					//Clock divider, system clock 72MHz / 1MHz = 72

		LPC_TMR32B0->PR = 0;					//as fast as possible!
		LPC_TMR32B0->MR0 = TimerInterval;

		NVIC_ClearPendingIRQ(TIMER_32_0_IRQn);
		/* Enable the TIMER0 Interrupt */
		//NVIC_EnableIRQ(TIMER_32_0_IRQn);
	}
	else{
		LPC_SYSCON->SYSAHBCLKCTRL |= (1<<10);	//Enable clock to 32-bit timer
		//nope.. timer 1
	}
}

/******************************************************************************
**                            End Of File
******************************************************************************/
