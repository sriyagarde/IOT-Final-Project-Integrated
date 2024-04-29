/*
 * adc.c
 *
 *  Created on: 23-Apr-2024
 *      Author: SRIYA
 */


/***************************************************************************//**
 * @file main_s0.c
 * @brief Use the ADC to take nonblocking single-ended measurements
 *******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * SPDX-License-Identifier: Zlib
 *
 * The licensor of this software is Silicon Laboratories Inc.
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 *******************************************************************************
 * # Evaluation Quality
 * This code has been minimally tested to ensure that it builds and is suitable
 * as a demonstration for evaluation purposes only. This code will be maintained
 * at the sole discretion of Silicon Labs.
 ******************************************************************************/

#include <stdio.h>
#include "em_device.h"
#include "em_chip.h"
#include "em_cmu.h"
#include "em_adc.h"
#include "lcd.h"
#include "timers.h"


#define INCLUDE_LOG_DEBUG 1
#include "src/log.h"

#define adcFreq   16000000

 uint32_t sample_light;
 uint32_t millivolts_light;

 uint32_t sample_smoke;
 uint32_t millivolts_smoke;

 uint32_t light_threshold = 1000;

 uint8_t channel = 0;
void initADC (void)
{
  // Enable ADC0 clock
  CMU_ClockEnable(cmuClock_ADC0, true);
  CMU_ClockEnable(cmuClock_GPIO, true);
  LOG_INFO("ADC0 and GPIO clocks enabled.\r\n");

  GPIO_PinModeSet(gpioPortD, 10, gpioModePushPull, 0);
  LOG_INFO("GPIO PD10 configured as Push-Pull for the buzzer.\r\n");

//  LOG_INFO("ADC0 clock enabled.\r\n");

  // Declare init structs
  ADC_Init_TypeDef init = ADC_INIT_DEFAULT;
  ADC_InitSingle_TypeDef initSingle = ADC_INITSINGLE_DEFAULT;

  // Modify init structs and initialize
  init.prescale = ADC_PrescaleCalc(adcFreq, 0); // Init to max ADC clock for Series 0
  init.timebase = ADC_TimebaseCalc(0);

  initSingle.diff       = false;        // single ended
  initSingle.reference  = adcRef2V5;    // internal 2.5V reference
  initSingle.resolution = adcRes12Bit;  // 12-bit resolution

  // Select ADC input. See README for corresponding EXP header pin.
  if(channel == 0)
    {
      initSingle.posSel = adcPosSelAPORT2XCH9;
    }

  else
    {
      initSingle.posSel = adcPosSelAPORT2XCH7;
    }

  ADC_Init(ADC0, &init);
  ADC_InitSingle(ADC0, &initSingle);
  LOG_INFO("ADC initialized for single-ended operation with 12-bit resolution.\r\n");

  // Enable ADC Single Conversion Complete interrupt
  ADC_IntEnable(ADC0, ADC_IEN_SINGLE);

  // Enable ADC interrupts
  NVIC_ClearPendingIRQ(ADC0_IRQn);
  NVIC_EnableIRQ(ADC0_IRQn);
}

/**************************************************************************//**
 * @brief  ADC Handler
 *****************************************************************************/
void ADC0_IRQHandler(void)
{
  LOG_INFO("ADC IRQ Handler triggered.\r\n");
  // Clear the interrupt flag
  ADC_IntClear(ADC0, ADC_IF_SINGLE);

  //delayApprox(300000);
  timerWaitUs_irq(300000);

  if(channel == 0)
    {
  // Get ADC result
  sample_light = ADC_DataSingleGet(ADC0);
  channel = 1;
    }

  else
    {
  sample_smoke = ADC_DataSingleGet(ADC0);
  channel = 0;
    }
  //LOG_INFO("Sample from ADC = %d",sample);
  // Calculate input voltage in mV

}

int convert_light()
{
  LOG_INFO("Convert light called\r\n");
  //displayPrintf(DISPLAY_ROW_11, "Convert light");
  millivolts_light = (sample_light * 2500) / 4096;

  // Print
  LOG_INFO("ADC data received: %u mV\r\n", millivolts_light);
  displayPrintf(DISPLAY_ROW_11, "Light value = %d lux",millivolts_light);

  if (millivolts_light < light_threshold)
  {

      GPIO_PinOutSet(gpioPortD, 10);
      LOG_INFO("Buzzer ON due to low light intensity.\r\n");
      return 1;
  }
  else
  {

      GPIO_PinOutClear(gpioPortD, 10);
      LOG_INFO("Buzzer OFF due to sufficient light intensity.\r\n");
      return 0;
  }

  // Start next ADC conversion
 // ADC_Start(ADC0, adcStartSingle);
}

int convert_smoke()
{
  LOG_INFO("Convert light called\r\n");
  //displayPrintf(DISPLAY_ROW_11, "Convert light");
  millivolts_smoke = (sample_smoke * 2500) / 4096;

  // Print
  LOG_INFO("ADC data received: %u mV\r\n", millivolts_smoke);
  displayPrintf(DISPLAY_ROW_10, "Smoke value = %d lux",millivolts_smoke);

  if (millivolts_smoke < light_threshold)
  {

      //GPIO_PinOutSet(gpioPortD, 10);
      LOG_INFO("LED OFF due to low smoke intensity.\r\n");
      return 1;
  }
  else
  {

      //GPIO_PinOutClear(gpioPortD, 10);
      LOG_INFO("LED ON due to high smoke intensity.\r\n");
      return 0;
  }

  // Start next ADC conversion
 // ADC_Start(ADC0, adcStartSingle);
}
