#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>

#include "Headers/Displayer.h"
#include "Headers/tm4c123gh6pm.h"
#include "Headers/Logo.h"

/* S E T T I N G S
________________________________
Play with some properties here//
to make the program more fun!//
____________________________//
*/
#define MINUTES 15      // Must be under 99 minutes. Less than 1 if you're a tryhard.
#define SECONDS 0       // Must be under 59 seconds.
#define QUESTIONS 7     // Must be under 99 questions.
#define ENABLE_SFX 0    // 1 for sound, 0 for no sound.

/*Stalls the program
  for a millisecond, the original
  function was "Timer0A_wait1ms()".

  Using TimerA0*/
void wait(int miliseconds) {
    int l;
    SYSCTL_RCGCTIMER_R |=1;          /* enable clock to Timer Block 0 */
    TIMER0_CTL_R        =0;         /* disable Timer before initialization */
    TIMER0_CFG_R        =0x04;     /* 16-bit option */
    TIMER0_TAMR_R       =0x02;    /* periodic mode and down-counter */
    TIMER0_TAILR_R      =16e3-1; /* Timer A interval load value register*/
    TIMER0_ICR_R        =0x1;   /* clear the TimerA timeout flag*/
    TIMER0_CTL_R       |=0x01; /* enable Timer A after initialization*/
    for (l=0;l<miliseconds;l++){
        while ((TIMER0_RIS_R&0x1)==0) ; /* wait for TimerA timeout flag */
        TIMER0_ICR_R=0x1; /* clear the TimerA timeout flag */
    }
}

/*The board speaks in high-pitch, due
  to losing the game or time running out.
  The only way to stop this is restarting
  the program.*/
void siren(int seconds,int freq){
    int l,k,g;
    for (l=0;l<seconds;l++) {
        for (g=0;g<freq;g++) {
            for (k=0;k<25;k+=2) {
                GPIO_PORTC_DATA_R=0x10; wait(1); // delay 1 milisecond
                GPIO_PORTC_DATA_R=0x00; wait(1); // delay 1 milisecond
            }
            wait(ceil(800/freq));
        }
    }
}

/*Regular function that returns
  the number within the min-max
  range.*/
int random(int min,int max){
    return rand()%(max-min+1)+min;
}

/*Creates a problem for the user to solve
  dependent on categories:
     ________________________
    // SOLVE THIS EQUATION //______________________
   // Random assortments of letters are given,   //
  // users decode alphabets within the equation //
 // and solve for the numerical answer.        //
//____________________________________________//
     ________________________
    // DECODE THE VARIABLE //________________________
   // A random expression and answer is given,     //
  // users must find the solution associated with //
 // the assigned letter to validate the answer.  //
//______________________________________________//
      __________________________
     // CRACK THE CODE (FPGA) //_______________________
    // When connected to Scylla, users must identify //
   // the letter's position from the alphabet using //
  // the switches on the Nexys-A7 50T. If, and     //
 // only if, the Tiva-C is connected, of course.  //
//_______________________________________________//
*/
//void new_problem(){
//    if (random(1,3)==1) {
//        // decode the letters
//    } else {
//        // solve this equation
//    }
//    return
//}

#define ON  0x10
#define OFF 0x00
void beep(uint32_t b) {
    GPIO_PORTC_DATA_R=b;
}

/* W A R N I N G
__________________________________________________________________________
What you're about to witness is some extreme technical jargon that      //
includes: ternary operators, nested statements, function inefficiency, //
and more. If you are uncomfortable about these topics, please refer to//
your nearest C API reference manual immediately before proceeding    //
further to view incomprehensible language. You have been warned.    //
And most importantly, viewer discretion is advised.                //
__________________________________________________________________//
*/
void shiftCursor(int i,int k){
    int j;
    LCD_Command(i%2==0?TO_ROW1:TO_ROW2);  // set position to top or bottom
    LCD_Command(MOVE_RIGHT);             // genius offset to center letters
    for(j=0;j<floor(i/2)-k;j++){
        LCD_Command(MOVE_RIGHT);
    }
}

/*Applying the logo
  animation initial
  program startup.*/
void intro(void) {
    // Logo.h is just the letters CHARYBDIS in cool format.
    int i;
    LCD_Command(ENTRY_MODE); // entry mode
    for(i=0;i<36;i++){
        /* --WHY THIS IS DONE DUE TO SEVERAL REASONS:
            _______________________________________________________________
           // character creation stops at i=28.                          //
          // LCD must delete all characters after that.                 //
         // Iteration must interchange between deleting & adding chars.//
        //____________________________________________________________//
         * */
        if(i<28){ //Generating characters//
            LCD_Create(i%8,i%2==0?top[i/2]:btm[i/2]);
            shiftCursor(i,0);
            LCD_Data(i%8);
        }
        if(i>=8){ //Erasing previous characters from behind//
            shiftCursor(i,4);
            LCD_Data(' ');
        }
    }
    LCD_Command(TO_ROW1);
}

void loadingScreen(int tics,int delay){
    int i;
    for(i=0;i<tics;i++){
        LCD_Command(TO_ROW2);
        LCD_ShiftRight(i%16);
        LCD_Data(0xFF); //Filled character//
        if(i>7){
            LCD_Command(TO_ROW2);
            LCD_ShiftRight((i-8)%16);
            LCD_Data(' ');
        } wait(delay);
    }
}

/*All initializations
  organized into
  one function.*/
void setup(void) {
    Speak_Init();
    LCD_Init();
    SevenSeg_Init();
}

// Primary function for all action.
int main(void) {
    //new_problem();
    setup();
    intro();

    LCD_ShiftRight(2);
    LCD_Text("READY OR NOT",10);
    loadingScreen(100,30);
    LCD_Command(CLEAR);
    LCD_ShiftRight(3);
    // TODO: add section to determine problem number. //

    // end TODO //
    LCD_Text("Problem  1",10);

    /*Any absurd value will be
      limited to where SevenSeg
      can handle.*/
    int m=MINUTES>99?99:MINUTES;
    int s=SECONDS>59?59:SECONDS;
    int paused=0;

    // Timer to traverse through.
    while((m>=0&&s>=0)){
        if(paused==1)continue;
        // tick depicts amount of milliseconds elapsed during the loop.
        // it should total 1 second per loop.
        int tick=0;
        while(tick<1e3){ // 1e3=10^3=1,000
            // Each waiting millisecond attributes to displaying SevenSeg.
            int f=(m/10)%10==0?10:(m/10)%10; // Minute should stop displaying if it's unnecessary
            int g=(m%10==0)&&f==10?10:m%10; // Same here
            int h=((s/10)%10==0)&&g==10?10:(s/10)%10; // And here (but it's seconds so who cares)
            int mac=tick<1e3/10&&ENABLE_SFX==1?1:0; // Annoy some people by beeping out the Tiva-C's swearing.
            SSI2_Write(SEG[f],SEG_MODE);SSI2_Write(8,SEG_MODE);if(mac==1){beep(ON);}wait(1);
            SSI2_Write(SEG[g],SEG_MODE);SSI2_Write(4,SEG_MODE);beep(OFF);wait(1);
            SSI2_Write(SEG[h],SEG_MODE);SSI2_Write(2,SEG_MODE);if(mac==1){beep(ON);}wait(1);
            SSI2_Write(SEG[s%10],SEG_MODE);SSI2_Write(1,SEG_MODE);beep(OFF);wait(1);
            tick+=4;} // We wasted 4 milliseconds already
        tick=0;
        if(s<=0&&m>0){ // If there are minutes left remaining.
            m-=1;
            s=59;
        }else{
            s-=1;
        }
    }
}
