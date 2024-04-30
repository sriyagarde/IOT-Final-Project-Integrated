/*
 * ws281b.c
 *
 *  Created on: 29-Apr-2024
 *      Author: SRIYA
 */

#include "em_device.h"
#include "em_chip.h"
#include "em_gpio.h"
#include "em_cmu.h"
#include "app.h"
#include "src/gpio.h"
#include "ws281b.h"

#define INCLUDE_LOG_DEBUG 1
#include "src/log.h"

#include <stdint.h>


void sendBit(bool bitVal)
{
    if (bitVal) {
        GPIO_PinOutSet(LED_PORT, LED_PIN);
        __NOP(); __NOP(); __NOP();
        GPIO_PinOutClear(LED_PORT, LED_PIN);
    } else {
        GPIO_PinOutSet(LED_PORT, LED_PIN);
        GPIO_PinOutClear(LED_PORT, LED_PIN);
        __NOP(); __NOP(); __NOP();
    }
}

void sendByte(uint8_t byte)
{
    for (int i = 7; i >= 0; i--)
    {
        sendBit(byte & (1 << i));
    }
}

void sendColor(uint8_t red, uint8_t green, uint8_t blue)
{
    sendByte(green);
    sendByte(red);
    sendByte(blue);
}

void sendColors(uint8_t ledColors[][3])
{
    for (int led = 0; led < NUM_LEDS; led++)
    {
        sendColor(ledColors[led][0], ledColors[led][1], ledColors[led][2]);
    }
    LOG_INFO("All colors sent.\r\n");
}

void delayMs(int milliseconds)
{
    for (int i = 0; i < milliseconds * 1000; i++)
    {
        __NOP();
    }
}






