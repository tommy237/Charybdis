#include <stdint.h>
#include "Headers/tm4c123gh6pm.h"
#include "Headers/Displayer.h"

#define RS 1    // BIT0 mask for reg select
#define EN 2    // BIT1 mask for E
#define BL 4    // BIT2 mask for backlight

/*INITIALIZTIONS
   ___________________________________________________
  //Functions that start up components of the board.//
 //________________________________________________//
*/
void SevenSeg_Init(void) {
    SYSCTL_RCGCGPIO_R|=0x02;  // enable clock to GPIOB
    SYSCTL_RCGCGPIO_R|=0x04;  // enable clock to GPIOC
    SYSCTL_RCGCSSI_R |=0x04;  // enable clock to SSI2
    // :: PORTB 7,4 for SSI2 TX and SCLK ::
    GPIO_PORTB_AMSEL_R&=~0x90;       // turn off analog of PORTB 7,4
    GPIO_PORTB_AFSEL_R |=0x90;       // PORTB 7,4 for alternate function
    GPIO_PORTB_PCTL_R &=~0xF00F0000; // clear functions for PORTB 7,4
    GPIO_PORTB_PCTL_R  |=0x20020000; // PORTB 7,4 for SSI2 function
    GPIO_PORTB_DEN_R   |=0x90;       // PORTB 7,4 as digital pins
    // :: PORTC 7 for SSI2 slave select ::
    GPIO_PORTC_AMSEL_R&=~0x80;     // disable analog of PORTC 7
    GPIO_PORTC_DATA_R  |=0x80;     // set PORTC 7 idle high
    GPIO_PORTC_DIR_R   |=0x80;     // set PORTC 7 as output for SS
    GPIO_PORTC_DEN_R   |=0x80;     // set PORTC 7 as digital pin
    // :: SS12 :: //
    SSI2_CR1_R =0;             // turn off SSI2 during configuration
    SSI2_CC_R  =0;             // use system clock
    SSI2_CPSR_R=16;            // clock prescaler divide by 16 gets 1 MHz clock
    SSI2_CR0_R =0x0007;        // clock rate div by 1,phase/polarity 0 0,mode freescale,data size 8
    SSI2_CR1_R =2;             // enable SSI2 as master
}

// initialize SSI2 then initialize LCD controller
void LCD_Init(void){
    SYSCTL_RCGCSSI_R |=0x04; // enable clock to SSI2
    SYSCTL_RCGCGPIO_R|=0x02; // enable clock to GPIOB
    SYSCTL_RCGCGPIO_R|=0x04; // enable clock to GPIOC
    // PORTB 7, 4 for SSI2 TX and SCLK
    GPIO_PORTB_AMSEL_R&=~0x90;       // turn off analog of PORTB 7, 4
    GPIO_PORTB_AFSEL_R|= 0x90;       // PORTB 7, 4 for alternate function
    GPIO_PORTB_PCTL_R &=~0xF00F0000; // clear functions for PORTB 7, 4
    GPIO_PORTB_PCTL_R |= 0x20020000; // PORTB 7, 4 for SSI2 function
    GPIO_PORTB_DEN_R  |= 0x90;       // PORTB 7, 4 as digital pins
    // PORTC 6 for SSI2 slave select
    GPIO_PORTC_AMSEL_R&=~0x40; // disable analog
    GPIO_PORTC_DATA_R |= 0x40; // set PORTC6 idle high
    GPIO_PORTC_DIR_R  |= 0x40; // set PORTC6 as output for CS
    GPIO_PORTC_DEN_R  |= 0x40; // set PORTC6 as digital pins
    SSI2_CR1_R =0;             // make it master
    SSI2_CC_R  =0;             // use system clock
    SSI2_CPSR_R=16;            // clock prescaler divide by 16 gets 1 MHz clock
    SSI2_CR0_R =0x0007;        // clock rate div by 1, phase/polarity 0 0, mode freescale, data size 8
    SSI2_CR1_R =2;             // enable SSI2

    Delay_MS(20); // LCD controller reset sequence
    LCD_Nib(0x30,0);Delay_MS(5);
    LCD_Nib(0x30,0);Delay_MS(1);
    LCD_Nib(0x30,0);Delay_MS(1);

    LCD_Nib(0x20, 0); // use 4-bit data mode
    Delay_MS(1);
    LCD_Command(0x28); //set 4-bit data, 2-line, 5x7 font
    LCD_Command(ENTRY_MODE); //entry mode
    LCD_Command(CLEAR); //clear screen, move cursor to home
    LCD_Command(DISPLAY_ON); //turn on display, cursor blinking
}

void Speak_Init(void) {
    SYSCTL_RCGCGPIO_R  |=0x04; // enable clock to GPIOC
    GPIO_PORTC_AMSEL_R&=~0x10; // turn off analog of PORTC 4
    GPIO_PORTC_DIR_R   |=0x10; // set PORTC 4 as output pins
    GPIO_PORTC_DEN_R   |=0x10; // set PORTC 4 as digital pins
}

/*MODIFIERS
   ___________________________________________________
  //Functions that display data to the Tiva-C board.//
 //________________________________________________//
*/
void SSI2_Write(unsigned char data,uint32_t sel){
    GPIO_PORTC_DATA_R&=~sel;       // assert chip select   +++
    SSI2_DR_R=data;            // write data
    while(SSI2_SR_R&0x10);   // wait for transmit done
    GPIO_PORTC_DATA_R|=sel;        // deassert chip select +++
} //0x40 : Seven_Seg select
  //0x80 : LCD select

void LCD_Nib(char data,unsigned char control){
    data&=0xF0;       // clear lower nibble for control
    control&=0x0F;    // clear upper nibble for data
    SSI2_Write(data|control|BL,LCD_MODE);           // RS = 0, R/W = 0
    SSI2_Write(data|control|EN|BL,LCD_MODE);      // pulse E
    //Delay_MS(5);
    SSI2_Write(data|BL,LCD_MODE);
    SSI2_Write(BL,LCD_MODE);
}

void LCD_Command(unsigned char command){
    LCD_Nib(command&0xF0,0); // upper nibble first
    LCD_Nib(command<<4,0);   // then lower nibble
    if(command<4) Delay_MS(2);        // command 1 and 2 needs up to 1.64ms
    else Delay_MS(1);                  // all others 40 us
}

void LCD_Create(uint8_t location,uint8_t charmap[]) {
    location&=0x7; // Only 8 locations (0-7)
    LCD_Command(0x40|(location<<3)); // Set CGRAM address
    int i; for(i=0;i<8;i++) {
        LCD_Data(charmap[i]);
    } // Send each row of the custom char
}

void LCD_Data(char data){
    LCD_Nib(data&0xF0,RS); // upper nibble first
    LCD_Nib(data<<4,RS);   // then lower nibble
    Delay_MS(1);
}

void LCD_Text(uint8_t string[],int delay){
    uint8_t i;
    for(i=0;string[i];i++){
        LCD_Data(string[i]);
        if (delay>0) {
            Delay_MS(delay);
        }
    }
}

void LCD_ShiftRight(int i){
    int l;
    for(l=0;l<i;l++){
        LCD_Command(MOVE_RIGHT);
    }
}

// delay n milliseconds (16 MHz CPU clock)
void Delay_MS(int n){
    int i,j; for(i=0;i<n;i++)
        for(j=0;j<3180;j++){}  // do nothing for 1 ms
}

// delay n microseconds (16 MHz CPU clock)
void Delay_US(int n){
    int i,j; for(i=0;i<n;i++)
        for(j=0;j<3;j++){}  // do nothing for 1 us
}
