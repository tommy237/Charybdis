#ifndef HEADERS_DISPLAYER_H_
#define HEADERS_DISPLAYER_H_

/* DISPLAYER.H contains the following components:
 *
 * LCD.h
 * Seven_Semgent.h
 *
 * These are components that
 * will show on the Tiva-C board,
 * especially the lights.
 * */

/* D E V
 * P R O P E R T I E S
________________________________________________________
These variables are defined for the developer to use. //
Do not touch, or else it will ruin the display. Refer//
to SETTINGS on main.c to play around with. This is  //
for grown ups! >:(                                 //
__________________________________________________//
*/
#define LCD_MODE 0x40
#define SEG_MODE 0x80
const static unsigned char SEG[]={0xC0,
                                 0xF9,
                                0xA4,
                              0xB0,
                            0x99,
                          0x92,
                       0x82,
                   0xF8,
               0x80,
          0x90,
    0xFF,};

//LCD COMMANDS
#define CLEAR 0x01
#define ENTRY_MODE 0x06
#define DISPLAY_ON 0x0F
#define MOVE_RIGHT 0x14
#define TO_ROW1 0x80
#define TO_ROW2 0xC0

/* F U N C T I O N S
________________________________________________________
These functions are defined for the developer to use. //
Do not touch, or else it will ruin the program. Refer//
to SETTINGS on main.c to play around with. This is  //
for grown ups! >:(                                 //
__________________________________________________//
*/
void LCD_Init(void);
void SevenSeg_Init(void);
void Speak_Init(void);

void LCD_Create(uint8_t location,uint8_t charmap[]);
void LCD_Nib(char data, unsigned char control);
void LCD_Command(unsigned char command);
void LCD_Data(char data);
void SSI2_Write(unsigned char data,uint32_t sel);
void LCD_Text(uint8_t string[], int delay);
void LCD_ShiftRight(int i);

void Delay_MS(int n);
void Delay_US(int n);

#endif
