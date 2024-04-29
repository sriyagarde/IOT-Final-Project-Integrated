/***********************************************************************
 * @file timers.c
 * @version 0.0.2
 * @brief Function header file for defining functions for LETIMER0
 *
 * @author Sriya Garde, sriya.garde@colorado.edu
 * @date Feb 09, 2024
 *
 * @institution University of Colorado Boulder (UCB)
 * @course ECEN 5823-001: IoT Embedded Firmware (Fall 2020)
 * @instructor David Sluiter
 *
 * @assignment ecen5823-assignment2-sriyagarde
 * @due Feb 09, 2024
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
#include "gpio.h"
#include "app.h"

#include "em_letimer.h"
#ifndef SRC_TIMERS_H_
#define SRC_TIMERS_H_

//Load values for LFXO clock
#if (LOWEST_ENERGY_MODE < 3)
#define PERIOD_VALUE 24590

//Load values for ULFRCO clock
#else
#define PERIOD_VALUE 3000

#endif


void init_LETIMER0();
void timerWaitUs_polled(uint32_t us_wait);
void timerWaitUs_irq(uint32_t us_wait);
#endif /* SRC_TIMERS_H_ */
