// Lab9Main.c
// Runs on MSPM0G3507
// Lab 9 ECE319K
// Clark Rucker & Gauri Gupta
// Last Modified: 12/26/2024

#include <stdio.h>
#include <stdint.h>
#include <ti/devices/msp/msp.h>
#include "../inc/ST7735.h"
#include "../inc/Clock.h"
#include "../inc/LaunchPad.h"
#include "../inc/TExaS.h"
#include "../inc/Timer.h"
#include "../inc/ADC1.h"
#include "../inc/DAC5.h"
#include "../inc/Arabic.h"
#include "SmallFont.h"
#include "LED.h"
#include "Switch.h"
#include "Sound.h"
#include "images/images.h"
#include "UART1.h"
#include "UART2.h"
#include "FIFO1.h"
#include "../inc/ADC1.h"


buttons_t Key,lastKey;

// ****note to ECE319K students****
// the data sheet says the ADC does not work when clock is 80 MHz
// however, the ADC seems to work on my boards at 80 MHz
// I suggest you try 80MHz, but if it doesn't work, switch to 40MHz
void PLL_Init(void){ // set phase lock loop (PLL)
  // Clock_Init40MHz(); // run this line for 40MHz
  Clock_Init80MHz(0);   // run this line for 80MHz
}


uint32_t M=1;
uint32_t Random32(void){
  M = 1664525*M+1013904223;
  return M;
}
uint32_t Random(uint32_t n){
  return (Random32()>>16)%n;
}



uint8_t TExaS_LaunchPadLogicPB27PB26(void){
  return (0x80|((GPIOB->DOUT31_0>>26)&0x03));
}

//LANGUAGES===============================================================================
typedef enum {LANGUAGE, START, READY, SCORE, HIT, MISS} phrase_t;
const char Language_English[]="English";
const char Language_French[]="Fran\x87""ais";

const char Start_English[]="Starting!";
const char Start_French[]="Commencement!";

const char Ready_English[]="Ready";
const char Ready_French[]="Pr\x88""t!";

const char Score_English[]="Score: ";
const char Score_French[]="Score: ";

const char Hit_English[]="Hit!";
const char Hit_French[]="FRAPPER";

const char Miss_English[]="Miss!";
const char Miss_French[]="MANQUER";

const char Host_English[]="Host";
const char Host_French[]="H\x93""te";

const char Client_English[]="Client";
const char Client_French[]="Client";

const char HC_English[]="Select Host/Client!";
const char HC_French[]="S\x82" "lectionner H\x93" "te/Client!";

const char Place_English[]="Place your ships!";
const char Place_French[]="Placez vos navires!";

const char Turn_English[]="YOUR TURN";
const char Turn_French[]="\x85" " votre tour";

const char Enemy_English[]="Enemy Board";
const char Enemy_French[]="Plateau Ennemi";

const char Your_English[]="Your Board";
const char Your_French[]="Votre Tableau";

const char Win_English[]="YOU WIN";
const char Win_French[]="VOUS GAGNEZ";

const char Lose_English[]="YOU LOSE!";
const char Lose_French[]="VOUS PERDEZ";

const char Y_English[]="Your";
const char Y_French[]="Votre";

const char E_English[]="Enemy";
const char E_French[]="Ennemi";

const char *Phrases[17][2]={
  {Language_English,Language_French},
  {Start_English,Start_French},
  {Ready_English,Ready_French},
  {Score_English,Score_French},
  {Hit_English,Hit_French},
  {Miss_English,Miss_French},
  {Host_English,Host_French},
  {Client_English,Client_French},
  {HC_English,HC_French},
  {Place_English,Place_French},
  {Turn_English,Turn_French},
  {Enemy_English,Enemy_French},
  {Your_English, Your_French},
  {Win_English,Win_French},
  {Lose_English,Lose_French},
  {Y_English,Y_French},
  {E_English,E_French}
};





typedef enum {MainMenu, Host_Client, ShipPlacement, Ready, MarkerPlacement, Waiting, EndGame} GameState_t;

GameState_t GameState=MainMenu;


struct sprite {
  uint8_t x,y;
  uint8_t rotation;
  uint16_t *image;
  uint8_t team;
  bool is_picked_up;
  uint8_t w,h;
};typedef struct sprite sprites_t;

struct marker{
  uint8_t x,y;
  uint16_t *image;
};typedef struct marker marker_t;



uint16_t *ShipImages[5][8]={
  {ThreeARightBlue,ThreeADownBlue,0,0,0,0,0,0},
  {ThreeARightBlue,ThreeADownBlue,0,0,0,0,0,0},
  {TwoRightBlue,TwoDownBlue,0,0,0,0,0},
  {TwoRightBlue,TwoDownBlue,0,0,0,0,0},
  {OneUpBlue,OneUpBlue,OneUpBlue},

};

sprites_t ships[5]={
  {21,40,1,ThreeARightBlue,1,0,32,10},
  {21,84,1,ThreeADownBlue,1,0,10,32},
  {32,73,1,TwoDownBlue,1,0,10,21},
  {21,51,1,TwoRightBlue,1,0,21,10},
  {43,62,1,OneUpBlue,1,0,10,10},
};

uint8_t scoreToDisplayA =0x30;
uint8_t scoreToDisplayB =0x30;
void scoreToString (uint8_t intScore) {
    scoreToDisplayA=(intScore%10)+0x30; //isolate last of initial 3 digit number, add 0x30 to make ASCII  (100s place)
    intScore/=10;  //remove last digit
    scoreToDisplayB=(intScore%10)+0x30; //isolate new last digit, add 0x30 to make ASCII                  (10s place)
}

sprites_t CURSOR={98,40,0,cursor,0,0,10,10};


marker_t markers[64];
marker_t ReceivedMarkers[64];

uint8_t Marker_Index=0,Marker_Index_In=0;

uint8_t YourHits,OpponentsHits,GameScore;

uint8_t LYourHits,LOpponentsHits,LGameScore;

bool printHit=false,printMiss=false;

uint8_t CURSOR_last[2]={55,51};

bool BoardState[8][8]={
  {{},{},{},{},{},{},{},{}},
  {{},{},{},{},{},{},{},{}},
  {{},{},{},{},{},{},{},{}},
  {{},{},{},{},{},{},{},{}},
  {{},{},{},{},{},{},{},{}},
  {{},{},{},{},{},{},{},{}},
  {{},{},{},{},{},{},{},{}},
  {{},{},{},{},{},{},{},{}},
};




bool printMyMarkers=true, printEnemyMarkers;
bool flag, readyFlag, drawLang, drawShipInit, drawCursorInit, drawHC,DrawWait=true; 
bool drawCursorADC, drawStart;
uint8_t ChooseHC = 5;
bool isHost=false;
uint8_t langFlag = 5;
uint8_t clip_count;


bool DeleteShipImages,DrawShipImages,DeleteCursor,redrawyourturn;
bool DrawHitScreen,DrawMissScreen,DrawEHitScreen,DrawEMissScreen;
bool YouWin,EnemyWin;

uint32_t inCH,inCHtokill;


uint16_t ADCval;
sprites_t SelectSc = {98,40,0,select,0,0,10,10};


void DisplayBothScores(void){
  scoreToString(YourHits);
  ST7735_SetCursor(19, 3);
  ST7735_OutStringTransparent(Phrases[15][langFlag]);
  ST7735_SetCursor(19, 4);
  ST7735_OutStringTransparent("Score:");
  ST7735_SetCursor(19, 5);
  ST7735_OutCharTransparent(scoreToDisplayB);
  ST7735_SetCursor(20, 5);
  ST7735_OutCharTransparent(scoreToDisplayA);

  scoreToString(OpponentsHits);
  ST7735_SetCursor(19, 7);
  ST7735_OutStringTransparent(Phrases[16][langFlag]);
  ST7735_SetCursor(19, 8);
  ST7735_OutStringTransparent("Score:");
  ST7735_SetCursor(19, 9);
  ST7735_OutCharTransparent(scoreToDisplayB);
  ST7735_SetCursor(20, 9);
  ST7735_OutCharTransparent(scoreToDisplayA);
}

void Grid_Init(){

  //Vertical
  //================================================================================================
  ST7735_DrawFastVLine(20, 30, 88, 0xFFFF);
  ST7735_DrawFastVLine(31, 30, 88, 0xFFFF);
  ST7735_DrawFastVLine(42, 30, 88, 0xFFFF);
  ST7735_DrawFastVLine(53, 30, 88, 0xFFFF);
  ST7735_DrawFastVLine(64, 30, 88, 0xFFFF);
  ST7735_DrawFastVLine(75, 30, 88, 0xFFFF);
  ST7735_DrawFastVLine(86, 30, 88, 0xFFFF);
  ST7735_DrawFastVLine(97, 30, 88, 0xFFFF);
  ST7735_DrawFastVLine(108, 30, 88, 0xFFFF);
  //================================================================================================

  //Horizontal 
  //================================================================================================
  ST7735_DrawFastHLine(20, 30, 88, 0xFFFF);
  ST7735_DrawFastHLine(20, 41, 88, 0xFFFF);
  ST7735_DrawFastHLine(20, 52, 88, 0xFFFF);
  ST7735_DrawFastHLine(20, 63, 88, 0xFFFF);
  ST7735_DrawFastHLine(20, 74, 88, 0xFFFF);
  ST7735_DrawFastHLine(20, 85, 88, 0xFFFF); 
  ST7735_DrawFastHLine(20, 96, 88, 0xFFFF); 
  ST7735_DrawFastHLine(20, 107, 88, 0xFFFF); 
  ST7735_DrawFastHLine(20, 118, 88, 0xFFFF);
  //================================================================================================
}

bool ShipClipCheck(void){
      //Ship Clip Check 
    //================================================================================================
    for(uint8_t i=0;i<=4;i++){
      if(ships[i].is_picked_up==true){

      //Compute the bound of your picked up ship
      //================================================================================================
      uint8_t Xmin=ships[i].x;
      uint8_t Xmax=ships[i].x+ships[i].w-1;
      
      uint8_t Ymax=ships[i].y;
      uint8_t Ymin=ships[i].y-ships[i].h+1;
      //================================================================================================

      //Does your X bound intersect with the x bound of any other ship?
      //================================================================================================
      for(uint8_t j=0;j<=4;j++){
        if(i==j)continue;
        //Compute the bound of ship to compare
        //================================================================================================
        uint8_t Xminj=ships[j].x;
        uint8_t Xmaxj=ships[j].x+ships[j].w-1;
        
        uint8_t Yminj=ships[j].y;
        uint8_t Ymaxj=ships[j].y-ships[j].h+1;
        //================================================================================================

        if(Xmin>=Xminj&&Xmin<=Xmaxj){//we know we are within the x range, compare ys
          if(!(((Ymin<Yminj) && (Ymax<Yminj)) || ((Ymin>Yminj) && (Ymax>Yminj)))){//we know that we are within the y range 
          return true;
          }
        }
      }
      //================================================================================================

      //Does your Y bound intersect with the Y bound of any other ship?
      //================================================================================================
      for(uint8_t j=0;j<=4;j++){
        if(i==j)continue;
        //Compute the bound of ship to compare
        //================================================================================================
        uint8_t Xminj=ships[j].x;
        uint8_t Xmaxj=ships[j].x+ships[j].w-1;
        
        uint8_t Yminj=ships[j].y;
        uint8_t Ymaxj=ships[j].y-ships[j].h+1;
        //================================================================================================

        if(Ymin<=Yminj&&Ymin>=Ymaxj){//we know we are within the y range, compare xs
          if(!(((Xmin<Xminj) && (Xmax<Xminj)) || ((Xmin>Xminj) && (Xmax>Xminj)))){//we know that we are within the x range 
          return true;
          }
        }
      }
      //================================================================================================



      }
  
    }
    return false;
    //================================================================================================
}


void TIMG12_IRQHandler(void){uint32_t pos,msg;
  if((TIMG12->CPU_INT.IIDX) == 1){ // this will acknowledge
    GPIOB->DOUTTGL31_0 = GREEN; // toggle PB27 (minimally intrusive debugging)
    GPIOB->DOUTTGL31_0 = GREEN; // toggle PB27 (minimally intrusive debugging)
// game engine goes here

    //Cursor Logic
    //================================================================================================
    Key=Keys_In();
    ADCval=ADCin();

    if(CURSOR_last[0]!=CURSOR.x)    CURSOR_last[0]=CURSOR.x;
    if(CURSOR_last[1]!=CURSOR.y-9)  CURSOR_last[1]=CURSOR.y-9;

    if(GameState == MainMenu) {
      if((ADCval >= 0) && (ADCval <= 2047)) {
        langFlag = 0;
        drawCursorADC = 0;
      } else {
        langFlag = 1;
        drawCursorADC = 1;
      }
    if(lastKey.ALL!=Key.ALL && Key.ACTION==1){ //english 0 french 1
      drawStart=true;
    }

    }
    else{
    if(lastKey.ALL!=Key.ALL && Key.RIGHT==1){
      if(((CURSOR.x+11) <= 108) && ((CURSOR.x + 11) >=21)) {
      CURSOR.x += 11;
        }
      }
    else if(lastKey.ALL!=Key.ALL && Key.LEFT==1){
          if(((CURSOR.x-11) <= 108) && ((CURSOR.x - 11) >=21)) {
      CURSOR.x -= 11;
        }
      }
    else if(lastKey.ALL!=Key.ALL && Key.UP==1){
            if(((CURSOR.y-11) <= 118) && ((CURSOR.y - 11) >=31)) {
      CURSOR.y -= 11;
        }
      }
    
    else if(lastKey.ALL!=Key.ALL && Key.DOWN==1){
        if(((CURSOR.y+11) <= 118) && ((CURSOR.y + 11) >=31)) {
      CURSOR.y += 11;
        }
      
    }
    }
    //================================================================================================

    //Ship Placement GameState
    //================================================================================================
    if(GameState==ShipPlacement){
      

    //Ship Movement Logic
    //================================================================================================
    if (lastKey.ALL!=Key.ALL && Key.ACTION==1) {

    //pick up and set down ships
    //================================================================================================
    for(uint8_t i=0;i<=4;i++){

    if(ShipClipCheck()==false){
    //Check the X row
    //================================================================================================
      for(uint8_t j=0;j<=ships[i].w;j+=11){
        if(((ships[i].x+j)==CURSOR.x)){
          if(ships[i].y==CURSOR.y){
            ships[i].is_picked_up=(!ships[i].is_picked_up);
            CURSOR.x=ships[i].x;
            CURSOR.y=ships[i].y;
            CURSOR_last[0]=ships[i].x;
            CURSOR_last[1]=ships[i].y;
            goto endshipcheck;
          }
        }
      }
    //================================================================================================

    //check the Y row
    //================================================================================================
      for(uint8_t j=0;j<=ships[i].h;j+=11){
        if(((ships[i].y-j)==CURSOR.y)){
          if(ships[i].x==CURSOR.x){
          ships[i].is_picked_up=(!ships[i].is_picked_up);
          CURSOR.x=ships[i].x;
          CURSOR.y=ships[i].y;
          CURSOR_last[0]=ships[i].x;
          CURSOR_last[1]=ships[i].y;
          goto endshipcheck;
        }
        }
      }
    //================================================================================================
      }
    }
    //================================================================================================
    }    
    endshipcheck:
  
  
    //Transition States Logic
    //================================================================================================
    if(lastKey.ALL!=Key.ALL && Key.ACTION2==1){
      DeleteShipImages=true;
      GameState=Ready;
    }
    //================================================================================================
    } 
    //================================================================================================
    
    //Wait for everyone to be reayd
    //================================================================================================
    else if(GameState==Ready){
      if(DrawWait==false){
      inCH=UART2_InChar();
      if(inCH=='#'){//if you receive '#' opponent is waiting, you should place markers
      
      GameState=MarkerPlacement;
      }
      else if(inCH=='!'){//If you receive the '!' that means opponent placed ships first.
        
        
        UART1_OutChar('#');//output '#' to indicate that you are now waiting, they can place
        UART1_OutChar('#');
        UART1_OutChar('#');
        UART1_OutChar('#');
        Clock_Delay1ms(400);
        GameState=Waiting;//you will enter waiting
      
      }
      else{
      
      UART1_OutChar('!');
      UART1_OutChar('!');
      UART1_OutChar('!');
      UART1_OutChar('!');
      Clock_Delay1ms(400);
      }
      }
    }
    //================================================================================================



    //MarkerPlacement Gamestate
    //================================================================================================
    else if(GameState==MarkerPlacement){
      
    //Marker Placement logic.
    //================================================================================================
    if((lastKey.ALL!=Key.ALL && Key.ACTION==1)&&(printEnemyMarkers==false)){
      for(uint8_t i=0; i<=Marker_Index;i++){
        if((markers[i].x==CURSOR.x)&&(markers[i].y==CURSOR.y)){
          CURSOR.x=CURSOR_last[0];
          CURSOR.y=CURSOR_last[1]+9;
          goto invalidplacement;
        }
      }
      markers[Marker_Index].x=CURSOR.x;
      markers[Marker_Index].y=CURSOR.y;
    //================================================================================================

    // output 4-frame message & increment marker index
    //================================================================================================
    UART1_OutChar('<');//output the data
    UART1_OutChar(markers[Marker_Index].x);
    UART1_OutChar(markers[Marker_Index].y);
    UART1_OutChar(GameScore);
    Clock_Delay1ms(200);
    Marker_Index++;
    //================================================================================================
    
    //transition to ready state
    //================================================================================================
    printEnemyMarkers=false;
    printMyMarkers=true;
    DeleteShipImages=true;
    DrawShipImages=false;
    DeleteCursor=true;
    GameState=Waiting;
    //================================================================================================
    }
    invalidplacement:
    //================================================================================================

    //Toggle between boards
    //================================================================================================
    if (lastKey.ALL!=Key.ALL && Key.ACTION2==1){
      if(printMyMarkers){
        printEnemyMarkers=true;
        printMyMarkers=false;
        DeleteShipImages=false;
        DrawShipImages=true;
        DeleteCursor=true;
      }else if(printEnemyMarkers){
        printEnemyMarkers=false;
        printMyMarkers=true;
        DeleteShipImages=true;
        DrawShipImages=false;
        DeleteCursor=true;
      }
    }
    //================================================================================================
    }
    //================================================================================================


    //Waiting Gamestate
    //================================================================================================
    else if(GameState==Waiting){
      //acquire InChar
      inCH = UART2_InChar();
      //================================================================================================
      if(inCH==0x3C){ //  "<" indicates that you have received marker location 

      //Update received markers array & received hits
      //================================================================================================
        ReceivedMarkers[Marker_Index_In].x = UART2_InChar();
        ReceivedMarkers[Marker_Index_In].y = UART2_InChar();
        GameScore = UART2_InChar();
        Marker_Index_In++;
      //================================================================================================

      //Check if the received marker was a hit with your ships.
      //================================================================================================
      for(uint8_t i=0;i<=4;i++){
      
      //Check the X row
      //================================================================================================
        for(uint8_t j=0;j<=ships[i].w;j+=11){
          if(((ships[i].x+j)==ReceivedMarkers[Marker_Index_In-1].x)){
            if(ships[i].y==ReceivedMarkers[Marker_Index_In-1].y){
              OpponentsHits++; 
              goto endcheck;
            }
          }
        }
      //================================================================================================

      //check the Y row
      //================================================================================================
        for(uint8_t j=0;j<=ships[i].h;j+=11){
          if(((ships[i].y-j)==ReceivedMarkers[Marker_Index_In-1].y)){
            if(ships[i].x==ReceivedMarkers[Marker_Index_In-1].x){
            OpponentsHits++; 
            goto endcheck;
          }
          }
        }
      //================================================================================================
      }
      endcheck:
      //================================================================================================

      //Transmit hits back to opponent. 
      //================================================================================================
      UART1_OutChar('>');//output the data
      UART1_OutChar(YourHits);
      UART1_OutChar(OpponentsHits);
      UART1_OutChar(GameScore);
      Clock_Delay1ms(200);
      //================================================================================================

      //Update EnemyHits array
      //================================================================================================
      if(OpponentsHits!=LOpponentsHits){
        ReceivedMarkers[Marker_Index_In-1].image=hit;
        DrawEHitScreen=true;
        DrawEMissScreen=false;
      } 
      else if(Marker_Index_In>0){
        ReceivedMarkers[Marker_Index_In-1].image=miss;
        DrawEMissScreen=true;
        DrawEHitScreen=false;
      } 
      //================================================================================================

      //You are now in marker placing mode
      //================================================================================================
      DeleteShipImages=true;
      redrawyourturn=true;
      GameState=MarkerPlacement;
      //================================================================================================
      }
      //================================================================================================
      

      //================================================================================================
      else if(inCH==0x3E){//  ">" indicates that you have received whether or not you had a hit

      //Update your hits & opponents hits 
      //================================================================================================
        LOpponentsHits = OpponentsHits;
        LYourHits = YourHits; 
        LGameScore = GameScore;

        OpponentsHits = UART2_InChar();
        YourHits      = UART2_InChar();
        GameScore     = UART2_InChar();
      //================================================================================================

      //Update YourHits array & set flag to draw status
      //================================================================================================
      if(YourHits!=LYourHits){
        markers[Marker_Index-1].image=hit;
        DrawHitScreen=true;
        DrawMissScreen=false;
      } 
      else{
        markers[Marker_Index-1].image=miss;
        DrawMissScreen=true;
        DrawHitScreen=false;
      }
      //================================================================================================
      }
      //================================================================================================

      //Toggle between boards
      //================================================================================================
      if (lastKey.ALL!=Key.ALL && Key.ACTION2==1){
        if(printMyMarkers){
          printEnemyMarkers=true;
          printMyMarkers=false;
          DeleteShipImages=false;
          DrawShipImages=true;
          DeleteCursor=true;
        }else if(printEnemyMarkers){
          printEnemyMarkers=false;
          printMyMarkers=true;
          DeleteShipImages=true;
          DrawShipImages=false;
          DeleteCursor=true;
        }
      }
      //================================================================================================

      //Checking for win condition
      //================================================================================================
      if(YourHits==11){
        YouWin=true;
        EnemyWin=false;
        GameState=EndGame;
      }else if(OpponentsHits==11){
        YouWin=false;
        EnemyWin=true;
        GameState=EndGame;
      }
      //================================================================================================
    
    }
      //================================================================================================
    
    
    flag=1;
    lastKey=Key;
    GPIOB->DOUTTGL31_0 = GREEN; // toggle PB27 (minimally intrusive debugging)
  }
}



// ALL ST7735 OUTPUT MUST OCCUR IN MAIN
int main(void){ // final main
  __disable_irq();
  PLL_Init(); // set bus speed
  LaunchPad_Init();
  ST7735_InitPrintf();
    //note: if you colors are weird, see different options for
    // ST7735_InitR(INITR_REDTAB); inside ST7735_InitPrintf()
  ST7735_SetRotation(3);
  ST7735_FillScreen(ST7735_BLUE);
  ADCinit();     //PB18 = ADC1 channel 5, slidepot
  Switch_Init(); // initialize switches
  LED_Init();    // initialize LED
  Sound_Init();  // initialize sound
  TExaS_Init(0,0,&TExaS_LaunchPadLogicPB27PB26); // PB27 and PB26
    // initialize interrupts on TimerG12 at 30 Hz
  TimerG12_IntArm(80000000/30,2);
  // initialize all data structures
  Fifo1_Init();
  UART1_Init();
  UART2_Init();
  
  __enable_irq();

Grid_Init();

for(uint8_t i = 0; i<=4; i++) {ST7735_DrawBitmap(ships[i].x,ships[i].y, ships[i].image, ships[i].w,ships[i].h); }

if(GameState!=Waiting)ST7735_DrawBitmap(CURSOR.x,CURSOR.y, CURSOR.image, CURSOR.w,CURSOR.h); // 3 wide ship 1


  while(1){
    //ST7735_DrawBitmap(CURSOR.x,CURSOR.y, CURSOR.image, CURSOR.w,CURSOR.h); // 3 wide ship 1
    // wait for semaphore

while(!flag) {};
  // clear semaphore
  flag = 0;

  if (GameState == MainMenu) {

    if (!drawLang) {
      ST7735_FillScreen(ST7735_BLACK);
      ST7735_SetCursor(8, 1);
      ST7735_OutString("BATTLESHIP!");

      ST7735_SetCursor(5, 2);
      ST7735_OutString("Select a Language");

      //english button
      ST7735_FillRect(38, 38, 82, 32, 0xFFFF);
      ST7735_FillRect(40, 40, 78, 28, 0);

      //french button
      ST7735_FillRect(38, 88, 82, 32, 0xFFFF);
      ST7735_FillRect(40, 90, 78, 28, 0);

      ST7735_SetCursor(11, 5);
      ST7735_OutString(Phrases[0][0]);

      ST7735_SetCursor(11, 10);
      ST7735_OutString(Phrases[0][1]);
      drawLang = 1;
    }

    if (drawCursorADC == 0) {
      ST7735_FillRect(45, 99, 10, 10, ST7735_BLACK);
      ST7735_DrawBitmap(45,57, SelectSc.image, SelectSc.w,SelectSc.h);
       
    } else {
      ST7735_FillRect(45, 48, 10, 10, ST7735_BLACK);
      ST7735_DrawBitmap(45,108, SelectSc.image, SelectSc.w,SelectSc.h);
    }

    if(drawStart) {
    ST7735_FillScreen(ST7735_BLACK);
    ST7735_SetCursor(8, 1);
    ST7735_OutString("BATTLESHIP!");

      ST7735_FillRect(36, 36, 86, 36, 0xFFFF);
      ST7735_FillRect(38, 38, 82, 32, 0);

    if(langFlag == 1) {
      ST7735_SetCursor(7, 5);
    } else {
      ST7735_SetCursor(9, 5);
    }

    ST7735_OutString(Phrases[1][langFlag]);


      Clock_Delay1ms(1000);
      GameState = ShipPlacement;
    }
  }

  else if(GameState == ShipPlacement) {

  if (!drawShipInit) {
    ST7735_FillScreen(ST7735_BLUE);
    ST7735_SetCursor(4, 1);
    ST7735_OutStringTransparent(Phrases[9][langFlag]);
    drawShipInit = 1;
  }
        for(uint8_t i = 0; i<=4; i++) {ST7735_DrawBitmap(ships[i].x,ships[i].y, ships[i].image, ships[i].w,ships[i].h); }
        ST7735_FillRect(CURSOR_last[0],CURSOR_last[1],10, 10, ST7735_BLUE); 
        ST7735_DrawBitmap(CURSOR.x,CURSOR.y, CURSOR.image, CURSOR.w,CURSOR.h); 
        Grid_Init();

      for (uint8_t i = 0;i<=4;i++){
        if(ships[i].is_picked_up==true){
          if(ships[i].x <= CURSOR.x) {
            if ((ships[i].x + ships[i].w) < 108) {
            while (ships[i].x != CURSOR.x) {
              (ships[i].x)++;
              ST7735_DrawBitmap(ships[i].x,ships[i].y, ships[i].image, ships[i].w,ships[i].h);
              }
            } else {
              CURSOR.x = CURSOR_last[0];
            }
          }
          if(ships[i].x > CURSOR.x) {
            while (ships[i].x != CURSOR.x) {
              (ships[i].x)--;
              ST7735_DrawBitmap(ships[i].x,ships[i].y, ships[i].image, ships[i].w,ships[i].h);
            }
          }

          if(ships[i].y <= CURSOR.y) {
            //if ((ships[i].y + ships[i].h) <= 118) {
            while (ships[i].y != CURSOR.y) {
              (ships[i].y)++;
              ST7735_DrawBitmap(ships[i].x,ships[i].y, ships[i].image, ships[i].w,ships[i].h);
            }
            //} else {
            // CURSOR.y = CURSOR_last[1];
            //}
          }

          if(ships[i].y > CURSOR.y) {
            if ((ships[i].y - ships[i].h) > 30) {
            while (ships[i].y != CURSOR.y) {
              (ships[i].y)--;
              ST7735_DrawBitmap(ships[i].x,ships[i].y, ships[i].image, ships[i].w,ships[i].h);
            }
            } else {
              CURSOR.y = CURSOR_last[1]+9;
            }
          }
        }
      }
    }
  
else if(GameState==Ready){
    if(DrawWait==true){
      
    ST7735_FillScreen(ST7735_BLACK);
    ST7735_SetCursor(8, 1);
    ST7735_OutString("BATTLESHIP!");

      ST7735_FillRect(36, 36, 86, 36, 0xFFFF);
      ST7735_FillRect(38, 38, 82, 32, 0);

    if(langFlag == 1) {
      ST7735_SetCursor(11, 5);
    } else {
      ST7735_SetCursor(11, 5);
    }

    ST7735_OutString(Phrases[2][langFlag]);
    DrawWait=false;
    }
}

  else if(GameState==MarkerPlacement){
      LED(green,on);
      LED(yellow,on);
      LED(red,off);

      if(DeleteShipImages==true){
      ST7735_FillScreen(ST7735_BLUE);
      Grid_Init();
      ST7735_SetCursor(4, 1);
      ST7735_OutStringTransparent(Phrases[10][langFlag]);
      DeleteShipImages=false;
      printMyMarkers=true;
      printEnemyMarkers=false;

    }else if(DrawShipImages==true){
      ST7735_FillScreen(ST7735_BLUE);
      Grid_Init();
      ST7735_SetCursor(4, 1);
      ST7735_OutStringTransparent(Phrases[12][langFlag]);
      for(uint8_t j = 0; j<=4; j++) ST7735_DrawBitmap(ships[j].x,ships[j].y, ships[j].image, ships[j].w,ships[j].h);
      DrawShipImages=false;
    }

     if(printMyMarkers){
      if(redrawyourturn==true){
      ST7735_FillScreen(ST7735_BLUE);
      Grid_Init();
      ST7735_SetCursor(4, 1);
      ST7735_OutStringTransparent(Phrases[10][langFlag]);
      redrawyourturn=false;
      }
      DisplayBothScores();
      for(uint8_t i=0; i<=Marker_Index_In; i++){
        ST7735_DrawBitmap(markers[i].x,markers[i].y, markers[i].image, CURSOR.w,CURSOR.h);
      }
         ST7735_FillRect(CURSOR_last[0],CURSOR_last[1],10, 10, ST7735_BLUE); 
         ST7735_DrawBitmap(CURSOR.x,CURSOR.y, CURSOR.image, CURSOR.w,CURSOR.h); //Draw the cursor

    }
    else if(printEnemyMarkers){
      DisplayBothScores();
      for(uint8_t i=0; i<=Marker_Index_In; i++){
        if(ReceivedMarkers[i].image!=0)ST7735_DrawBitmap(ReceivedMarkers[i].x,ReceivedMarkers[i].y, ReceivedMarkers[i].image, CURSOR.w,CURSOR.h);
      }
    }
  }

  else if(GameState==Waiting){
      LED(green,off);
      LED(yellow,on);
      LED(red,on);
    
    if(DrawHitScreen==true){
      ST7735_FillScreen(ST7735_GREEN);
      ST7735_FillRect(38, 38, 82, 32, 0x0);
      ST7735_FillRect(40, 40, 78, 28, 0);
      LED(green,on);
      LED(yellow,off);
      LED(red,off);
      Sound_Hit();
      if(langFlag == 1) {
        ST7735_SetCursor(10, 5);
      } else {
        ST7735_SetCursor(11, 5);
      }
      ST7735_OutString(Phrases[4][langFlag]);
      Clock_Delay1ms(1000);
      ST7735_FillScreen(ST7735_BLUE);
      Grid_Init();
      ST7735_SetCursor(4, 1);
      ST7735_OutStringTransparent(Phrases[11][langFlag]);
      LED(green, off);
      DrawHitScreen=false;
    }else if(DrawMissScreen){
      ST7735_FillScreen(ST7735_WHITE);
      ST7735_FillRect(38, 38, 82, 32, 0x0);
      ST7735_FillRect(40, 40, 78, 28, 0);
      LED(green,off);
      LED(yellow,on);
      LED(red,off);
      Sound_Miss();
      if(langFlag == 1) {
        ST7735_SetCursor(10, 5);
      } else {
        ST7735_SetCursor(11, 5);
      }

      ST7735_OutString(Phrases[5][langFlag]);
      Clock_Delay1ms(1000);
      ST7735_FillScreen(ST7735_BLUE);
      Grid_Init();
      ST7735_SetCursor(4, 1);
      ST7735_OutStringTransparent(Phrases[11][langFlag]);
      LED(yellow,off);
      DrawMissScreen=false;
    }
    

    if(DeleteCursor==true){
      ST7735_FillScreen(ST7735_BLUE);
      Grid_Init();
      DeleteCursor=false;
    }

    if(DeleteShipImages==true){
      ST7735_FillScreen(ST7735_BLUE);
      Grid_Init();
      ST7735_SetCursor(4, 1);
      ST7735_OutStringTransparent(Phrases[11][langFlag]);
      DeleteShipImages=false;

    }else if(DrawShipImages==true){
      ST7735_FillScreen(ST7735_BLUE);
      Grid_Init();
      ST7735_SetCursor(4, 1);
      ST7735_OutStringTransparent(Phrases[12][langFlag]);
      for(uint8_t j = 0; j<=4; j++) ST7735_DrawBitmap(ships[j].x,ships[j].y, ships[j].image, ships[j].w,ships[j].h);
      DrawShipImages=false;
    }

     if(printMyMarkers){
      DisplayBothScores();
      for(uint8_t i=0; i<=Marker_Index_In; i++){
        ST7735_DrawBitmap(markers[i].x,markers[i].y, markers[i].image, CURSOR.w,CURSOR.h);
      }
    }
    else if(printEnemyMarkers){
      DisplayBothScores();
      for(uint8_t i=0; i<=Marker_Index_In; i++){
        if(ReceivedMarkers[i].image!=0)ST7735_DrawBitmap(ReceivedMarkers[i].x,ReceivedMarkers[i].y, ReceivedMarkers[i].image, CURSOR.w,CURSOR.h);
      }
    }
  }

  else if(GameState==EndGame){
    if(YouWin==true){
      ST7735_FillScreen(ST7735_GREEN);
      ST7735_FillRect(38, 38, 82, 32, 0x0);
      ST7735_FillRect(40, 40, 78, 28, 0);
      LED(green,on);
      Sound_Win();
      if(langFlag == 1) {
        ST7735_SetCursor(8, 5);
      } else {
        ST7735_SetCursor(10, 5);
      }

      ST7735_OutString(Phrases[13][langFlag]);
      YouWin=false;
    }else if(EnemyWin==true){
      ST7735_FillScreen(ST7735_RED);
      ST7735_FillRect(38, 38, 82, 32, 0x0);
      ST7735_FillRect(40, 40, 78, 28, 0);
      LED(red, on);
      Sound_Lose();
      if(langFlag == 1) {
        ST7735_SetCursor(8, 5);
      } else {
        ST7735_SetCursor(9, 5);
      }

      ST7735_OutString(Phrases[14][langFlag]);
      EnemyWin=false;
    }
  }
}
}

