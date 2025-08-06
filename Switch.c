/*
 * Switch.c
 *
 *  Created on: Nov 5, 2023
 *      Author:
 */
#include <ti/devices/msp/msp.h>
#include "../inc/LaunchPad.h"
#include "Switch.h"
// LaunchPad.h defines all the indices into the PINCM table
void Switch_Init(void){
  
  //Initialize inputs
  //================================================================================================
  IOMUX->SECCFG.PINCM[PA28INDEX] = 0x00040081; // UP
  IOMUX->SECCFG.PINCM[PA27INDEX] = 0x00040081; // LEFT
  IOMUX->SECCFG.PINCM[PA26INDEX] = 0x00040081; // DOWN
  IOMUX->SECCFG.PINCM[PA25INDEX] = 0x00040081; // RIGHT
  IOMUX->SECCFG.PINCM[PA24INDEX] = 0x00040081; // ACTION
  IOMUX->SECCFG.PINCM[PB13INDEX] = 0x00040081; // ACTION2
  //================================================================================================
}


//variables
//================================================================================================
buttons_t Buttons;
//================================================================================================

//Find the current state of all buttons, return struct
buttons_t Keys_In(void){

  //Shift DIN and isolate
  //================================================================================================
  Buttons.UP        =(GPIOA->DIN31_0>>28)&0x1;
  Buttons.LEFT      =(GPIOA->DIN31_0>>27)&0x1;
  Buttons.DOWN      =(GPIOA->DIN31_0>>26)&0x1;
  Buttons.RIGHT     =(GPIOA->DIN31_0>>25)&0x1;
  Buttons.ACTION    =(GPIOA->DIN31_0>>24)&0x1;
  Buttons.ACTION2   =(GPIOB->DIN31_0>>13)&0x1;
  //================================================================================================

  //state of all buttons for misc tasks
  //================================================================================================
  Buttons.ALL       =(GPIOA->DIN31_0>>23)|((GPIOB->DIN31_0>>13)&0x1);
  //================================================================================================

  return Buttons;
}