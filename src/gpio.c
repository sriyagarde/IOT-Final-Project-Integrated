/*
  gpio.c
 
   Created on: Dec 12, 2018
       Author: Dan Walkes
   Updated by Dave Sluiter Dec 31, 2020. Minor edits with #defines.

   March 17
   Dave Sluiter: Use this file to define functions that set up or control GPIOs.
   
   Jan 24, 2023
   Dave Sluiter: Cleaned up gpioInit() to make it less confusing for students regarding
                 drive strength setting. 

 *
 * @sriyagarde    Sriya Garde, sriya.garde@colorado.edu
 *
 
 */


// *****************************************************************************
// Students:
// We will be creating additional functions that configure and manipulate GPIOs.
// For any new GPIO function you create, place that function in this file.
// *****************************************************************************

#include <stdbool.h>
#include "em_gpio.h"
#include <string.h>
#include "ble.h"
#include "gpio.h"
#include "app.h"
#include "ws281b.h"

#define INCLUDE_LOG_DEBUG 1
#include "src/log.h"

  // Number of LEDs in the series
    uint8_t colors[NUM_LEDS][3] =
    {
        {255, 0 ,0},   // Red
        {0, 255, 0},   // Red
        {0, 0, 255},   // Red
    };

#define LED_port   (5)
#define LED0_pin   (4) //PF4
#define LED1_pin   (5) //PF5 IN ACTIVE HIGH
#define Sensor_port (3)
#define Sensor_pin (15)
#define LCD_port   (3)
#define LCD_pin    (13)

#define Light_Sensor_VCC_port gpioPortF
#define Light_Sensor_VCC_pin 4
#define Smoke_Sensor_VCC_port gpioPortC
#define Smoke_Sensor_VCC_pin 11

//#if DEVICE_IS_BLE_SERVER == 0



//#endif


// Set GPIO drive strengths and modes of operation
void gpioInit()
{
    // Student Edit:

    // Set the port's drive strength. In this MCU implementation, all GPIO cells
    // in a "Port" share the same drive strength setting.
  GPIO_DriveStrengthSet(LED_port, gpioDriveStrengthStrongAlternateStrong); // Strong, 10mA
  //GPIO_DriveStrengthSet(LED_port, gpioDriveStrengthWeakAlternateWeak); // Weak, 1mA

  // Set the 2 GPIOs mode of operation
  GPIO_PinModeSet(LED_port, LED0_pin, gpioModePushPull, false);
 // GPIO_PinModeSet(LED_port, LED1_pin, gpioModePushPull, false);
  GPIO_PinModeSet(Sensor_port, Sensor_pin, gpioModePushPull, false);

  GPIO_PinModeSet(Light_Sensor_VCC_port,Light_Sensor_VCC_pin, gpioModePushPull, false);

  GPIO_PinModeSet(Smoke_Sensor_VCC_port,Smoke_Sensor_VCC_pin, gpioModePushPull, false);

//#if DEVICE_IS_BLE_SERVER == 0

  GPIO_PinModeSet(LED_PORT, LED_PIN, gpioModePushPull, 0);
  GPIO_PinModeSet(POWER_PORT, POWER_PIN, gpioModePushPull, 0);
  GPIO_PinModeSet(POWER_PORT, BUZZER_PIN, gpioModePushPull, 0);
//#endif
} // gpioInit()


void gpioLed0SetOn()
{
  GPIO_PinOutSet(LED_port, LED0_pin);
}


void gpioLed0SetOff()
{
  GPIO_PinOutClear(LED_port, LED0_pin);
}


void gpioLed1SetOn()
{
  GPIO_PinOutSet(LED_port, LED1_pin);
}


void gpioLed1SetOff()
{
  GPIO_PinOutClear(LED_port, LED1_pin);
}

void sensor_enable()
{
  GPIO_PinOutSet(Sensor_port, Sensor_pin);
}

void sensor_disable()
{
  GPIO_PinOutClear(Sensor_port, Sensor_pin);
}

void light_sensor_enable()
{
  LOG_INFO("Enable light sensor \r\n");
  GPIO_PinOutSet(Light_Sensor_VCC_port,Light_Sensor_VCC_pin);
}

void light_sensor_disable()
{
  LOG_INFO("Disable light sensor \r\n");
  GPIO_PinOutClear(Light_Sensor_VCC_port,Light_Sensor_VCC_pin);
}

void smoke_sensor_enable()
{
  //LOG_INFO("Enable light sensor \r\n");
  GPIO_PinOutSet(Smoke_Sensor_VCC_port,Smoke_Sensor_VCC_pin);
}

void smoke_sensor_disable()
{
  LOG_INFO("Disable light sensor \r\n");
  GPIO_PinOutClear(Smoke_Sensor_VCC_port,Smoke_Sensor_VCC_pin);
}

void buzzer_enable()
{
  LOG_INFO("Enable light sensor \r\n");
  GPIO_PinOutSet(POWER_PORT,BUZZER_PIN);
}

void buzzer_disable()
{
  LOG_INFO("Enable light sensor \r\n");
  GPIO_PinOutSet(POWER_PORT,BUZZER_PIN);
}

void  wsb_enable()
{
  GPIO_PinOutSet(POWER_PORT, POWER_PIN);
  LOG_INFO("Starting LED sequence\r\n");
  sendColors(colors);
  delayMs(5000);
}

void wsb_disable()
{
  GPIO_PinOutClear(POWER_PORT, POWER_PIN);
  LOG_INFO("Completed LED sequence\r\n");
  delayMs(5000);

}

void gpioSetDisplayExtcomin(bool value)
{
  if(value == true)
    {
      GPIO_PinOutSet(LCD_port, LCD_pin);
    }
  else
    {
      GPIO_PinOutClear(LCD_port, LCD_pin);
    }
}

