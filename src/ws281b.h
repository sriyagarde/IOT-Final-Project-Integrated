/*
 * ws281b.h
 *
 *  Created on: 29-Apr-2024
 *      Author: SRIYA
 */

#ifndef SRC_WS281B_H_
#define SRC_WS281B_H_
#include "stdbool.h"
#define NUM_LEDS 5

void sendBit(bool bitVal);
void sendByte(uint8_t byte);
void sendColor(uint8_t red, uint8_t green, uint8_t blue);
void sendColors(uint8_t ledColors[][3]);
void delayMs(int milliseconds);
void send_colours(void);

#endif /* SRC_WS281B_H_ */
