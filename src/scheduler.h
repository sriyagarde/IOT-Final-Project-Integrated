/***********************************************************************
 * @file scheduler.c
 * @version 0.0.1
 * @brief Function source file for setting up Scheduler
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
 * @resources IoTEF Lecture Slides 05,06,07

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

#ifndef SRC_SCHEDULER_H_
#define SRC_SCHEDULER_H_

#include "src/timers.h"
#include "src/oscillators.h"
#include "src/gpio.h"
#include "app.h"
#include "sl_bt_api.h"
#include "ble.h"

#include "em_letimer.h"

//Enum to define all states used in the scheduler
typedef enum{
  eventDefault = 0x00,
  eventTimerUFflag = 0x01,
  eventTimerCOMP1flag = 0x02,
  //eventI2CTransferComplete = 0x04

} scheduler_events;

void schedulerSetEventUFFlag();
void schedulerSetEventCOMP1Flag();
void schedulerSetEventI2CTransferComplete();
uint32_t getNextEvent();
//void temperature_state_machine(sl_bt_msg_t *evt);
void light_state_machine(sl_bt_msg_t *evt);
void discovery_state_machine(sl_bt_msg_t *evt);
#endif /* SRC_SCHEDULER_H_ */
