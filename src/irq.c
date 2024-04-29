/***********************************************************************
 * @file irq.c
 * @version 0.0.2
 * @brief Function source file for setting up IRQ routine for LETIMER0 interrupt
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
#define INCLUDE_LOG_DEBUG 1
#include "src/log.h"

#include "timers.h"
#include "gpio.h"
#include "i2c.h"
#include "scheduler.h"
#include "em_letimer.h"
#include "em_core.h"
#include "em_cmu.h"
#include "em_i2c.h"
#include "em_adc.h"
#include "irq.h"
#include "log.h"
#include "i2c.h"

//Definitions to enclose critical sections of the ISR which shouldn't be modified by other peripheral interrupts
#define CORE_DECLARE_IRQ_STATE CORE_irqState_t irqState
#define CORE_ENTER_CRITICAL() irqState = CORE_EnterCritical()
#define CORE_EXIT_CRITICAL() CORE_ExitCritical(irqState)

int rollover_count = 0;

/*
 *Function to return the current millisec elapsed in the program since start

 * @param null
 * @return null
 */
uint32_t letimerMilliseconds()
{
  uint32_t time_elapsed;
  time_elapsed = (rollover_count*3000) + (PERIOD_VALUE - LETIMER_CounterGet(LETIMER0));
  return time_elapsed;
}

/*
 *IRQ routine for LETIMER0 Interrupt.
 *IRQ It checks if the COMP1 / UF flag is set and accordingly creates the reuired delay.
 * @param null
 * @return null
 */
void LETIMER0_IRQHandler (void)
{
  uint32_t flags = LETIMER_IntGetEnabled (LETIMER0);

  LETIMER_IntClear (LETIMER0, flags);
  if (flags & LETIMER_IF_UF)
    {
      rollover_count++;
     // LOG_INFO("UF flag \r\n");
      schedulerSetEventUFFlag();

    }

  if (flags & LETIMER_IF_COMP1)
    {

      LETIMER_IntDisable(LETIMER0, LETIMER_IEN_COMP1);
     // LOG_INFO("Comp1 flag \r\n");
      schedulerSetEventCOMP1Flag();
      ADC_Start(ADC0, adcStartSingle);

    }

}

///*
// *IRQ routine for I2C0 interrupt to signify that I2C transfer is done.
//
// * @param null
// * @return null
// */
//void I2C0_IRQHandler(void) {
//
// // this can be locally defined
// I2C_TransferReturn_TypeDef transferStatus;
//
// transferStatus = I2C_Transfer(I2C0);
// if (transferStatus == i2cTransferDone) {
//     schedulerSetEventI2CTransferComplete();
// }
//
// if (transferStatus < 0) {
// LOG_ERROR("Transfer status - Transfer not done%d\r\n", transferStatus);
// }
//} // I2C0_IRQHandler()
