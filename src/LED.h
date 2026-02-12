/*
 * LED.h
 *
 *  Created on: Nov 5, 2023
 *      Author:
 */

#ifndef LED_H_
#define LED_H_

// initialize your LEDs
void LED_Init(void);

enum lightstate{on,off,toggle};
typedef enum lightstate lightstate_t;

enum lights{red,yellow,green};
typedef enum lights lights_t;



// data specifies which LED to turn on
void LED(lights_t color, lightstate_t state);

// data specifies which LED to turn off
void LED_Off(uint32_t data);

// data specifies which LED to toggle
void LED_Toggle(uint32_t data);

#endif /* LED_H_ */
