// Sound.c
// Runs on MSPM0
// Sound assets in sounds/sounds.h
// your name
// your data 
#include <stdint.h>
#include <ti/devices/msp/msp.h>
#include "Sound.h"
#include "sounds/sounds.h"
#include "../inc/DAC5.h"
#include "../inc/Timer.h"

#define PB0INDEX  11 // UART0_TX  SPI1_CS2  TIMA1_C0  TIMA0_C2
#define PB1INDEX  12 // UART0_RX  SPI1_CS3  TIMA1_C1  TIMA0_C2N
#define PB2INDEX  14 // UART3_TX  UART2_CTS I2C1_SCL  TIMA0_C3  UART1_CTS TIMG6_C0  TIMA1_C0
#define PB3INDEX  15 // UART3_RX  UART2_RTS I2C1_SDA  TIMA0_C3N UART1_RTS TIMG6_C1  TIMA1_C1
#define PB4INDEX  16 // UART1_TX  UART3_CTS TIMA1_C0  TIMA0_C2  TIMA1_C0N

//variables
//================================================================================================
uint8_t *snd=0;
uint32_t indexSin,sinMax,systickperiod, outdata;
//================================================================================================


//Systick arm things
void SysTick_IntArm(uint32_t period, uint32_t priority){
  
  //systick enable shenanagains 
  //================================================================================================
  systickperiod = period;

  SysTick->CTRL = 0x00;      // disable SysTick during setup

  SysTick->LOAD = 0;;  // reload value
  SCB->SHP[1] = (SCB->SHP[1]&(~0xC0000000))|(priority<<30); // priority 2
  SysTick->VAL = 0;          // any write to VAL clears COUNT and sets VAL equal to LOAD
  SysTick->CTRL = 0x07;      // enable SysTick with 80 MHz bus clock and interrupts
  //================================================================================================

}


// initialize a 11kHz SysTick, however no sound should be started
// initialize any global variables
// Initialize the 5-bit DAC
void Sound_Init(void){

  indexSin=0;

  //init 5 bit dac pb0-4
  //================================================================================================
  IOMUX->SECCFG.PINCM[PB0INDEX] = 0x81;//dac
  IOMUX->SECCFG.PINCM[PB1INDEX] = 0x81;//
  IOMUX->SECCFG.PINCM[PB2INDEX] = 0x81;//
  IOMUX->SECCFG.PINCM[PB3INDEX] = 0x81;//
  IOMUX->SECCFG.PINCM[PB4INDEX] = 0x81;//
  //================================================================================================

  //output enable
  //================================================================================================
  GPIOB->DOE31_0 |= 0x0000001F;
  //================================================================================================

  //init 11khz systick
  //================================================================================================
  SysTick_IntArm(7257, 2);
  //================================================================================================
}


void Dac_5_out(uint32_t data){

  outdata = data;
  //write to dac
  //================================================================================================
  GPIOB->DOUTCLR31_0 = 0x1F;
  GPIOB->DOUTSET31_0 = outdata;
  //================================================================================================

}

void SysTick_Handler(void){ // called at 11 kHz
    
    // output one value to DAC if a sound is active
    //================================================================================================
    Dac_5_out(snd[indexSin]);
    indexSin++;
    if (indexSin == sinMax){
    SysTick->LOAD = 0;
    }
    //================================================================================================
}



//******* Sound_Start ************
// This function does not output to the DAC. 
// Rather, it sets a pointer and counter, and then enables the SysTick interrupt.
// It starts the sound, and the SysTick ISR does the output
// feel free to change the parameters
// Sound should play once and stop
// Input: pt is a pointer to an array of DAC outputs
//        count is the length of the array
// Output: none
// special cases: as you wish to implement



void Sound_Start(const uint8_t *pt, uint32_t count){

  //systick start :-)
  //================================================================================================
  SysTick->CTRL = 0;
  SysTick->LOAD = systickperiod - 1;
  SysTick->VAL = 0;
  SysTick->CTRL = 0x07;
  //================================================================================================
  

  //set index to 0, set global pointer
  //================================================================================================
  indexSin = 0;
  sinMax=count;
  snd=pt;
  //================================================================================================
}






//================================================================================================
void Sound_Hit(void){
  Sound_Start(holymoly, 10533 );
}
void Sound_Miss(void){
  Sound_Start(Willhelm, 13257);
}
void Sound_Win(void){
  Sound_Start(hooray, 23552);
}

void Sound_Lose(void){
  Sound_Start(ufo_lowpitch, 25805);
}
void Sound_Fastinvader2(void){

}
void Sound_Fastinvader3(void){

}
void Sound_Fastinvader4(void){

}
void Sound_Highpitch(void){

}

