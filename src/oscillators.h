/***********************************************************************
 * @file oscillators.h
 * @version 0.0.1
 * @brief Function header file for setting up CMU Oscillators
 *
 * @author Sriya Garde, sriya.garde@colorado.edu
 * @date Feb 01, 2024
 *
 * @institution University of Colorado Boulder (UCB)
 * @course ECEN 5823-001: IoT Embedded Firmware (Fall 2020)
 * @instructor David Sluiter
 *
 * @assignment ecen5823-assignment2-sriyagarde
 * @due Feb 01, 2024
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

#ifndef SRC_OSCILLATORS_H_
#define SRC_OSCILLATORS_H_


//function prototype for initializing the CMU Oscillator for LETIMER0
void init_oscillator();


#endif /* SRC_OSCILLATORS_H_ */
