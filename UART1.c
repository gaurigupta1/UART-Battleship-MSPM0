/* UART1.c
 * Clark Rucker and Gauri Gupta
 * Date:
 * PA8 UART1 Tx to other microcontroller PA22 UART2 Rx
 */


#include <ti/devices/msp/msp.h>
#include "UART1.h"
#include "../inc/Clock.h"
#define PA8INDEX  18 // UART1_TX  SPI0_CS0  UART0_RTS TIMA0_C0  TIMA1_C0N


// power Domain PD0
// for 80MHz bus clock, UART clock is ULPCLK 40MHz
// initialize UART1 for 2000 baud rate
// blind, no synchronization on transmit
void UART1_Init(void){
    // do not reset or activate PortA, already done in LaunchPad_Init

   UART1->GPRCM.RSTCTL = 0xB1000003; // resets UART 
   UART1->GPRCM.PWREN = 0x26000001;  // enables power to the UART
   Clock_Delay(24); // Gives a small delay to allow time for UART to initialize 

   IOMUX->SECCFG.PINCM[PA8INDEX] = 0x00000082;//PA8 Initialization as TX (BIT 1), Bit 7 connects pc, INENA enable

   UART1->CLKSEL = 0x08; //Bus clock speed
   UART1->CLKDIV = 0x00; //No clock divide
   UART1->CTL0 &= ~0x01; //Disables UART
   UART1->CTL0 = 0x00020018; // enable fifos, tx and rx

// 40000000/16 = 2,500,000, 2,500,000/9600 = 260.4166666666667

   UART1->IBRD = 260; // divider = 21+45/64 = 21.703125
   UART1->FBRD = 27;
   UART1->LCRH = 0x00000030; // 8bit, 1 stop, no parity
   UART1->CTL0 |= 0x01; // enable UART1

}

//------------UART1_OutChar------------
// Output 8-bit to serial port
// blind synchronization
// 10 bit frame, 2000 baud, 5ms per frame
// Input: letter is an 8-bit ASCII character to be transferred
// Output: none
void UART1_OutChar(char data){
// simply output data to transmitter without waiting or checking status
  UART1->TXDATA = data;
}
