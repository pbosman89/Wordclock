/**************************************************************************/
/*! 
    @file     timer16.c
    @author   K. Townsend (microBuilder.eu)

    @section DESCRIPTION

    Generic code for both 16-bit timers.

    @warning  16-bit timers are limited to roughly ~0.91ms (or 910uS) on
              a system running at 72MHz since:
    @code
              1 mS = CFG_CPU_CCLK / 1000
                   = 72000000 / 1000
                   = 72000 'ticks'
    @endcode
              Meaning that 0xFFFF (65535) 'ticks' = 0.910208 milliseconds
              or 910 microseconds.

    @section Example

    @code 
    #include "/core/cpu/cpu.h"
    #include "/core/timer16/timer16.h"

    // Instantiated in timer16.h
    extern volatile uint32_t timer16_0_counter;
    ...
    cpuInit();

    // Initialise timer0 with a delay of 0xFFFF, which will cause the
    // timer interrupt to fire every 65535 ticks and increment
    // timer16_0_counter by 1
    timer16Init(0, 0xFFFF);

    // Enable the timer
    timer16Enable(0);

    // At this point timer16_0_counter should start incrementing by 1
    // every 65535 ticks
    @endcode
	
    @section LICENSE

    Software License Agreement (BSD License)

    Copyright (c) 2010, microBuilder SARL
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:
    1. Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    3. Neither the name of the copyright holders nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ''AS IS'' AND ANY
    EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
/**************************************************************************/

#include <type.h>
#include "timer16.h"

#include "Peripherals.h"
#include "ScreenFunctions.h"
#include "CapacitiveTouch.h"

volatile uint32_t timer16_0_counter = 0;
volatile uint32_t timer16_1_counter = 0;

/**************************************************************************/
/*! 
    @brief Interrupt handler for 16-bit timer 0
*/
/**************************************************************************/
void TIMER16_0_IRQHandler(void)
{
	if ( LPC_TMR16B0->IR & (1<<1) )
	{
			//Update screen
			static int CurrColumn = 0;

			//Column
			if(CurrColumn++ <13)
				HC595ShiftOut(0x00, 1);	//Shift register one column
			else{
				HC595ShiftOut(0x01, 1);
				CurrColumn = 0;
			}

			TLC5925ShiftOut(LiveColumnData[CurrColumn], 16);

			//Disable driver
			TLC5925Output(FALSE);

			//Latch outputs at the same time
			TLC5925Latch();
			HC595Latch();

			//Enable driver
			TLC5925Output(TRUE);

		LPC_TMR16B0->IR = 1<<1;			// clear interrupt flag for MR1
	}

  if ( LPC_TMR16B0->IR & (0x1<<4) )
  {
	LPC_TMR16B0->IR = 0x1<<4;		// clear interrupt flag
  }

  return;
}

void TIMER16_1_IRQHandler(void)
{
	if ( LPC_TMR16B1->IR & 0x01 )
	{
		LPC_TMR16B1->IR = 1;			/* clear interrupt flag */
	}

  	if ( LPC_TMR16B1->IR & (1<<2) )
  	{
  		Play_Audio();
  		LPC_TMR16B1->IR = 1<<2;			/* clear interrupt flag */
  	}

	//Capacitive touch capture interrupt routine
	if ( LPC_TMR16B1->IR & (0x1<<4) )
	{
		//Process channel 0 data
		CaptureCapTouch(1);

		//Clear interrupt flag
		LPC_TMR16B1->IR = 1<<4;
	}

	return;
}
