// FIFO1.c
// Runs on any microcontroller
// Provide functions that implement the Software FiFo Buffer
// Last Modified: December 23, 2024
// Clark Rucker and Gauri Gupta
// Last modification date: change this to the last modification date or look very silly
#include <stdint.h>


// Declare state variables for FiFo
//        size, buffer, put and get indexes
uint8_t PutI, GetI;
static char Fifo[16];
// *********** Fifo1_Init**********
// Initializes a software FIFO1 of a
// fixed size and sets up indexes for
// put and get operations
void Fifo1_Init(){
//Complete this
PutI=GetI=0; 
}

// *********** Fifo1_Put**********
// Adds an element to the FIFO1
// Input: data is character to be inserted
// Output: 1 for success, data properly saved
//         0 for failure, FIFO1 is full
uint32_t Fifo1_Put(char data){
  //Complete this routine
    if((PutI+1)%15==GetI) return 0;
    Fifo[PutI] = data;
    PutI = (PutI+1)%15;
    return 1;
}

// *********** Fifo1_Get**********
// Gets an element from the FIFO1
// Input: none
// Output: If the FIFO1 is empty return 0
//         If the FIFO1 has data, remove it, and return it
char Fifo1_Get(void){
  uint8_t data;
    if(GetI==PutI) return 0;
    data = Fifo[GetI];
    GetI = (GetI+1)%15;
    return data;
}