/***********************************************************************
 * @file i2c.h
 * @version 0.0.1
 * @brief Function source file for setting up I2C
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

#ifndef SRC_I2C_H_
#define SRC_I2C_H_

void i2c_init();
void i2c_write();
void i2c_read();
float convert_temp();


#endif /* SRC_I2C_H_ */
