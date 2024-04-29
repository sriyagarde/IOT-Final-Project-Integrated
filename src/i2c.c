/***********************************************************************
 * @file i2c.c
 * @version 0.0.1
 * @brief Function source file for setting up I2C
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

#define INCLUDE_LOG_DEBUG 1
#include "src/log.h"

#include "i2c.h"
#include "em_i2c.h"
#include "em_core.h"
#include "em_cmu.h"
#include "stdbool.h"
#include "em_letimer.h"
#include "em_gpio.h"
#include "sl_i2cspm.h"
#include "app.h"
#include "src/gpio.h"
#include "src/timers.h"
#include "src/oscillators.h"

//Device slave address to communicate with
#define SI7021_DEVICE_ADDR 0x40

I2C_TransferSeq_TypeDef transferSequence; // this one can be local
uint8_t cmd_data; // make this global for IRQs in A4
uint8_t read_data[2]; // make this global for IRQs in A4

/*
*Initializes I2C and defines transfer Sequence parameters for read and write i2c functions

* @param null
* @return null
*/
void i2c_init()
{
  //uint32_t i2c_bus_frequency;
//Setting up I2C configuration for SDA and SDL required pins and port
I2CSPM_Init_TypeDef I2C_Config = {
    .port = I2C0,
    .sclPort = gpioPortC,
    .sclPin =  10,
    .sdaPort = gpioPortC,
    .sdaPin = 11,
    .portLocationScl = 14,
    .portLocationSda = 16,
    .i2cRefFreq = 0,
    .i2cMaxFreq = I2C_FREQ_STANDARD_MAX,
    .i2cClhr = i2cClockHLRStandard
};
I2CSPM_Init(&I2C_Config); //Initialize I2C communication

//i2c_bus_frequency = I2C_BusFreqGet(I2C0);
}


/*
*I2C write function to send data to Temperature sensor

* @param null
* @return null
*/
void i2c_write()
{
  I2C_TransferReturn_TypeDef transferStatus;
i2c_init();
cmd_data = 0xF3; //to write data
transferSequence.addr = SI7021_DEVICE_ADDR << 1; // shift device address left
transferSequence.flags = I2C_FLAG_WRITE;
transferSequence.buf[0].data = &cmd_data; // pointer to data to write
transferSequence.buf[0].len = sizeof(cmd_data);

NVIC_EnableIRQ(I2C0_IRQn); //Have I disabled this in state machine

transferStatus = I2C_TransferInit (I2C0, &transferSequence);

if (transferStatus < 0)  //If transfer fails
    {
      LOG_ERROR("I2CSPM_Transfer: I2C bus write of cmd failed\n\r");
}

}//


/*
*I2C read function to receive data from Temperature sensor

* @param null
* @return null
*/
void i2c_read()
{
 I2C_TransferReturn_TypeDef transferStatus;
  i2c_init();
transferSequence.addr = SI7021_DEVICE_ADDR << 1; // shift device address left
transferSequence.flags = I2C_FLAG_READ;
transferSequence.buf[0].data = &read_data[0]; // pointer to data to write
transferSequence.buf[0].len = sizeof(read_data);

NVIC_EnableIRQ(I2C0_IRQn); //Have I disabled this in state machine

transferStatus = I2C_TransferInit (I2C0, &transferSequence);

if (transferStatus < 0)  //If transfer fails
    {
      LOG_ERROR("I2CSPM_Transfer: I2C bus read of cmd failed\n\r");
}
}


/*
*Function to convert the received value of temperature to degree celsius
*Referring to the datasheet for formula
* @param null
* @return temp value in degree celsius
*/
float convert_temp()
{
  float temp;
  uint16_t tempcelcius;
  tempcelcius = (read_data[0]<<8);
  tempcelcius += (read_data[1]);
  temp = tempcelcius * 175.72;
  temp /= 65536;
  temp -= 46.85;
  return temp;
}//convert_temp()

/*
*Function to perform reading of temperature data every 3 sec from the sensor & displaying it on the terminal in degree celsius
*Referring to the datasheet for wait values
* @param null
* @return null
*/
//void read_temp_sensor()
//{
//  float get_value;
//  sensor_enable();
//  I2CSPM_Init(&I2C_Config);
// // timerWaitUs_polled(85000);
//  i2c_write();
// // timerWaitUs_polled(12000);
//  i2c_read();
//  sensor_disable();
//  get_value = convert_temp();
//  //LOG_INFO("Temperature in celcius is : %fC\r\n",get_value);
//} //read_temp_sensor()


