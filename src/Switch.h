/*
 * Switch.h
 *
 *  Created on: Nov 5, 2023
 *      Author: jonat
 */

#ifndef SWITCH_H_
#define SWITCH_H_

// initialize your switches
void Switch_Init(void);


struct buttons{
   bool UP;
   bool LEFT;
   bool DOWN;
   bool RIGHT;
   bool ACTION;
   bool ACTION2;
   uint32_t ALL;
}; 
typedef struct buttons buttons_t;


buttons_t Keys_In(void);

#endif /* SWITCH_H_ */
