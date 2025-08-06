/*
 * LED.c
 *
 *  Created on: Nov 5, 2023
 *      Author:
 */
#include <ti/devices/msp/msp.h>
#include "../inc/LaunchPad.h"
#include "LED.h"
// LaunchPad.h defines all the indices into the PINCM table

//LED init
void LED_Init(void){

    //initialize pins
    //================================================================================================
    IOMUX->SECCFG.PINCM[PA15INDEX] = 0x81;//red      pb8
    IOMUX->SECCFG.PINCM[PA16INDEX] = 0x81;//yellow   pb7
    IOMUX->SECCFG.PINCM[PA17INDEX] = 0x81;//green    pb6
    //================================================================================================


    //GPIO Output Enable
    //================================================================================================
    GPIOA->DOE31_0 |= 0x00038000;
    //================================================================================================
}

//variables
//====================================================================================================
uint8_t shift_light;
//====================================================================================================

//Change the state of the desired LED.
void LED(lights_t color, lightstate_t state){

    //how many bits do you need to shift the data?
    //================================================================================================
    if(color==red)          shift_light=15; //corresponds to what pin the LED is on 
    else if(color==yellow)  shift_light=16;
    else if(color==green)   shift_light=17;
    //================================================================================================


    //set, clear, or toggle the correct bit
    //================================================================================================
    if(state==on)           GPIOA->DOUTSET31_0 = (1<<shift_light);  //modify desired bit in desired
    else if(state==off)     GPIOA->DOUTCLR31_0 = (1<<shift_light);  //way 
    else if(state==toggle)  GPIOA->DOUTTGL31_0 = (1<<shift_light);
    //================================================================================================
}

