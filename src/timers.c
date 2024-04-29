/***********************************************************************
 * @file timers.c
 * @version 0.0.2
 * @brief Function header file for defining functions for LETIMER0
 *
 * @author Sriya Garde, sriya.garde@colorado.edu
 * @date Feb 15, 2024
 *
 * @institution University of Colorado Boulder (UCB)
 * @course ECEN 5823-001: IoT Embedded Firmware (Fall 2020)
 * @instructor David Sluiter
 *
 * @assignment ecen5823-assignment4-sriyagarde
 * @due Feb 15, 2024
 *
 * @resources IoTEF Lecture Slides 03,04,05
 * an0026-efm32-letimer Application note
 * an0012-efm32-gpio Application note
 * an0004.1-efm32-cmu Application note
 * an0007.1-efr32-efm32-series-1-energymodes Application note
 * EFR32xG1 Wireless Gecko Reference Manual
 * ug343-multinode-energy-profiler
 *
 * @copyright All rights reserved. Distribution allowed only for the
 * use of assignment grading. Use of code excerpts allowed at the
 * discretion of author. Contact for permission.
 */


#include "timers.h"
#include "em_letimer.h"
#include "em_core.h"
#include "em_cmu.h"
#include "stdbool.h"
#include "app.h"

#define INCLUDE_LOG_DEBUG 1
#include "src/log.h"

//Load values for LFXO clock
#if (LOWEST_ENERGY_MODE < 3)
#define PERIOD_VALUE 24590
#define freq 8192
//Load values for ULFRCO clock
#else
#define PERIOD_VALUE 3000
#define freq 1000
#endif

#define Min_wait 1000  ///1sec
#define Max_wait 3000000


/*
 *Initializes LETIMER0 to use as a counter and switch LED On/Off
 *LFXO - EM0,EM1,EM2
 *ULFRCO - EM3
 * @param null
 * @return null
 */
void init_LETIMER0()
{
  uint32_t temp;

  const LETIMER_Init_TypeDef letimerInitData = {
      false, // enable; don't enable when init completes, we'll enable last
      true, // debugRun; useful to have the timer running when single-stepping in the debugger
      true, // comp0Top; load COMP0 into CNT on underflow
      false, // bufTop; don't load COMP1 into COMP0 when REP0==0
      0, // out0Pol; 0 default output pin value
      0, // out1Pol; 0 default output pin value
      letimerUFOANone, // ufoa0; no underflow output action
      letimerUFOANone, // ufoa1; no underflow output action
      letimerRepeatFree, // repMode; free running mode i.e. load & go forever
      0 // COMP0(top) Value, I calculate this below
  };

  // Initialize the LETIMER0
  LETIMER_Init (LETIMER0, &letimerInitData);

  // calculate and load COMP0 (top) & COMP1 values for LFXO & ULFRCO
  LETIMER_CompareSet(LETIMER0, 0, PERIOD_VALUE);
  // LETIMER_CompareSet(LETIMER0, 1, ON_TIME);


  // Clear all IRQ flags in the LETIMER0 IF status register
  LETIMER_IntClear (LETIMER0, 0xFFFFFFFF); // punch them all down

  // Set UF and COMP1 in LETIMER0_IEN, so that the timer will generate IRQs to the NVIC.
  temp = LETIMER_IEN_UF ;

  LETIMER_IntEnable (LETIMER0, temp);
  // Enable the timer to starting counting down, set LETIMER0_CMD[START] bit, see LETIMER0_STATUS[RUNNING] bit
  LETIMER_Enable (LETIMER0, true);

  //Enable IRQ for LETIMER0
  NVIC_EnableIRQ(LETIMER0_IRQn);
  //   //Start LETIMER0
  //   LETIMER0->CMD = LETIMER_CMD_START;


} //init_LETIMER0()

/*
 *Creates a polling based wait function to generate delay based on the input value given in microseconds

 * @param us_wait value in msec to create delay
 * @return null
 */
void timerWaitUs_polled(uint32_t us_wait)
{
  uint16_t ticks, current_count, required_count;

  //Range checking to see ensure value is between max and min delays that can be generated
  if((us_wait<(uint32_t)Min_wait) | (us_wait>(uint32_t)Max_wait)) {
      //LOG_ERROR("Timer wait range is exceeding limits\n\r");

      //wait time value decided for max and min cases
      if(us_wait < (uint16_t)Min_wait) {
          us_wait = Min_wait;
      }

      else if(us_wait > (uint16_t)Max_wait) {
          us_wait = Max_wait;
      }
  }

  //values to calculate delay given the ticks and the counter value of the LETIMER0
  ticks = (us_wait / 1000); //msec

  current_count = LETIMER_CounterGet(LETIMER0);

  required_count = current_count - ticks;

  if(current_count >= ticks)
    {
      while((LETIMER_CounterGet(LETIMER0)) != required_count);
    }

  else
    {
      while((LETIMER_CounterGet(LETIMER0)) != (uint32_t)(PERIOD_VALUE-(ticks-current_count)));
    }

}//timerWaitUs(uint32_t us_wait)

/*
 *Creates a letimer0 comp1 interrupt based wait function to generate delay based on the input value given in microseconds

 * @param us_wait value in msec to create delay
 * @return null
 */
void timerWaitUs_irq(uint32_t us_wait)
{

  uint16_t ticks, current_count, required_count;

  //Range checking to see ensure value is between max and min delays that can be generated
  if((us_wait<(uint32_t)Min_wait) | (us_wait>(uint32_t)Max_wait)) {
      LOG_ERROR("Timer wait range is exceeding limits\n\r");

      //wait time value decided for max and min cases
      if(us_wait < (uint16_t)Min_wait) {
          us_wait = Min_wait;
      }

      else if(us_wait > (uint16_t)Max_wait) {
          us_wait = Max_wait;
      }
  }

  //values to calculate delay given the ticks and the counter value of the LETIMER0
  ticks = (us_wait * freq / 1000000); //msec

  current_count = LETIMER_CounterGet(LETIMER0);

  required_count = current_count - ticks;

  if(required_count < PERIOD_VALUE) //Case 1 - no overflow
    {

      LETIMER_CompareSet(LETIMER0, 1, required_count);
    }

  else //Case 2 - overflow
    {
      required_count = PERIOD_VALUE - (0XFFFF - required_count);
      LETIMER_CompareSet(LETIMER0, 1, required_count);
    }
  //enable COMP1 interrupt of timer peripheral
  LETIMER_IntClear  (LETIMER0, LETIMER_IFC_COMP1);
  LETIMER_IntEnable(LETIMER0, LETIMER_IEN_COMP1);

//  LETIMER0->IEN |= LETIMER_IEN_COMP1;

}//timerWaitUs_irq(uint32_t us_wait)
