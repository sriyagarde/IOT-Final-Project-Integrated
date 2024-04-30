/***********************************************************************
 * @file oscillators.c
 * @version 0.0.1
 * @brief Function source file for setting up CMU Oscillators
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

#include "oscillators.h"
#include "stdbool.h"
#include "gpio.h"
#include "em_letimer.h"
#include "em_core.h"
#include "em_cmu.h"
#include "app.h"

#define INCLUDE_LOG_DEBUG 1
#include "src/log.h"

/*
Initializes CMU Oscillator clocks LFXO / ULFRCO according to energy modes used
*LFXO - EM0,EM1,EM2
*ULFRCO - EM3
* @param null
* @return null
*/
void init_oscillator()
{
    if(LOWEST_ENERGY_MODE < 3)
      {
        //Enable LFXO Clock
        CMU_OscillatorEnable(cmuOsc_LFXO, true, true);

        //Configure the oscillator to the peripheral clock Branch
        CMU_ClockSelectSet(cmuClock_LFA,cmuSelect_LFXO);

        //Set prescaler value as 4 for LFXO Clock
        CMU_ClockDivSet(cmuClock_LETIMER0, cmuClkDiv_4);
      }

    else if(LOWEST_ENERGY_MODE == 3)
      {
        //Enable ULFRCO Clock
        CMU_OscillatorEnable(cmuOsc_ULFRCO, true, true);

        //Configure the oscillator to the peripheral clock Branch
        CMU_ClockSelectSet(cmuClock_LFA,cmuSelect_ULFRCO);

        CMU_ClockDivSet(cmuClock_LETIMER0, cmuClkDiv_1);
      }

        // CLOCK Enable for CLock tree
        CMU_ClockEnable(cmuClock_LFA, true);

        // CLOCK Enable for Core LE.
       // CMU_ClockEnable (cmuClock_CORE, true);

        // CLOCK Enable for LETIMER0.
        CMU_ClockEnable (cmuClock_LETIMER0, true);
        CMU_ClockEnable(cmuClock_GPIO, true);

        // CLOCK Enable for GPIO
       // CMU_ClockEnable(cmuClock_GPIO, true);
       // uint32_t freq = CMU_ClockFreqGet(cmuClock_LETIMER0);


}//init_oscillator()
