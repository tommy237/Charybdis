#include <stdint.h>
#include "Headers/tm4c123gh6pm.h"
#include "Headers/Threads.h"

/*
d888888P oo                              d8888b.  .d888888
   88                                        `88 d8'    88
   88    dP 88d8b.d8b. .d8888b. 88d888b. .aaadP' 88aaaaa88a
   88    88 88'`88'`88 88ooood8 88'  `88 88'     88     88
   88    88 88  88  88 88.  ... 88       88.     88     88
   dP    dP dP  dP  dP `88888P' dP       Y88888P 88     88
   _________________________________________________________
  //Functions from Timer2A that allow to run interrupts to//
 //gather information and deliver it to the main program.//
//______________________________________________________//
 * */
void Timer2A_Init(void){
  SYSCTL_RCGCTIMER_R|=0x00000004;  // Activate Timer2
  SYSCTL_RCGCGPIO_R |=0x00000002; // Activate Port B

  GPIO_PORTB_DEN_R  |=0x01;                                        // Enable digital I/O on PB0
  GPIO_PORTB_AFSEL_R|=0x01;                                       // Enable alternate function on PB0
  GPIO_PORTB_PCTL_R  =(GPIO_PORTB_PCTL_R&0xFFFFFFF0)|0x00000007; // Enable T2CCP0

  TIMER2_CTL_R &=~TIMER_CTL_TAEN;                        // Disable Timer2A during setup
  TIMER2_CFG_R  =TIMER_CFG_16_BIT;                      // Configure for 16-bit timer mode
  TIMER2_TAMR_R =TIMER_TAMR_TACMR|TIMER_TAMR_TAMR_CAP; // Configure for capture mode
  TIMER2_CTL_R &=~(TIMER_CTL_TAEVENT_POS|0xC);        // Configure for rising-edge event
  TIMER2_TAILR_R=0x0000FFFF;                         // Start value
  TIMER2_IMR_R |=TIMER_IMR_CAEIM;                   // Enable capture match interrupt
  TIMER2_ICR_R  =TIMER_ICR_CAECINT;                // Clear Timer2A capture match flag
  TIMER2_CTL_R |=TIMER_CTL_TAEN;                  // Enable Timer2A

  NVIC_PRI5_R=(NVIC_PRI5_R&0x00FFFFFF)|0x40000000; // Timer2A = Priority 2
  NVIC_EN0_R =0x00800000;                         // Enable interrupt 23 in NVIC
  //EnableInterrupts();
  return;
}

void Timer2A_Handler(void){
  TIMER2_ICR_R=TIMER_ICR_CAECINT; // Acknowledge Timer2A capture
  // TODO: ADD SOMETHING BELOW

  // END SECTION
  return;
}




