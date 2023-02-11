
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __LCD_H
#define __LCD_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"


void LCD_Init ();
void LCD_SendByte (uint8_t data);
void LCD_WriteCommand (uint8_t cmd);
void LCD_WriteChar (char data);
void LCD_WriteString (char *text);
void LCD_Clear ();


#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
