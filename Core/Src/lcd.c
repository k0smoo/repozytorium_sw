/**
  ******************************************************************************
  * @file           : lcd.c
  * @brief          : Biblioteka LCD
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
#include "lcd.h"

/*!
 * Funkcja LCD_SendByte() wysyła na magistrale 8-bitową daną. Na początku ustawiany
jest pin LCD_E_Pin, następnie ze względu na ograniczenia, bity od D0 do D7 wysyłane są po kolei.
Ostatecznie pin LCD_E_Pin jest zerowany.
 */
void LCD_SendByte (uint8_t data) {
  HAL_GPIO_WritePin (LCD_E_GPIO_Port, LCD_E_Pin, 1);
  HAL_GPIO_WritePin (LCD_D0_GPIO_Port, LCD_D0_Pin, (data & 0b00000001));
  HAL_GPIO_WritePin (LCD_D1_GPIO_Port, LCD_D1_Pin, (data & 0b00000010));
  HAL_GPIO_WritePin (LCD_D2_GPIO_Port, LCD_D2_Pin, (data & 0b00000100));
  HAL_GPIO_WritePin (LCD_D3_GPIO_Port, LCD_D3_Pin, (data & 0b00001000));
  HAL_GPIO_WritePin (LCD_D4_GPIO_Port, LCD_D4_Pin, (data & 0b00010000));
  HAL_GPIO_WritePin (LCD_D5_GPIO_Port, LCD_D5_Pin, (data & 0b00100000));
  HAL_GPIO_WritePin (LCD_D6_GPIO_Port, LCD_D6_Pin, (data & 0b01000000));
  HAL_GPIO_WritePin (LCD_D7_GPIO_Port, LCD_D7_Pin, (data & 0b10000000));
  HAL_GPIO_WritePin (LCD_E_GPIO_Port, LCD_E_Pin, 0);
  HAL_Delay (60);
}

/*!
 * Funkcja LCD_WriteCommand() wystawia na pin LCD_RS_Pin zero co informuje wyświetlacz, że od tego momentu przez magistrale będą przesyłane polecenia. Następnie wywoływana jest funkcja LCD_SendByte() przesyłająca bajt do wyświetlacza.
 */
void LCD_WriteCommand (uint8_t cmd) {
  HAL_GPIO_WritePin (LCD_RS_GPIO_Port, LCD_RS_Pin, 0);
  LCD_SendByte (cmd);
}

/*!
 * Funkcja LCD_WriteChar() wystawia na pin LCD_RS_Pin jedynka co informuje wyświetlacz, że od tego momentu przez magistrale będą przesyłane dane. Następnie wywoływana jest funkcja LCD_SendByte() przesyłająca bajt do wyświetlacza.
 */
void LCD_WriteChar (char data) {
  HAL_GPIO_WritePin (LCD_RS_GPIO_Port, LCD_RS_Pin, 1);
  LCD_SendByte (data);
}

/*!
 * Funkcja LDC_WriteString() służy do przesyłania łańcucha znaków na wyświetlacz. W pętli while inkrementowany jest adres zmiennej „text” i wysyłany do funkcji LCD_WriteChar().
 */
void LCD_WriteString (char *text) {
  while (*text)
  LCD_WriteChar (*text++);
}

/*!
 * Funkcja LCD_Clear() służy do wyczyszczenia ekranu z wyświetlanej zawartości
 */
void LCD_Clear () {
  LCD_WriteCommand (LCD_CLEAR);
  HAL_Delay (5);
  LCD_WriteCommand (LCDC_ENTRY_MODE|LCD_EM_SHIFT_CURSOR|LCD_EM_RIGHT);
}

/*!
 * Funkcja LCD_init() przeprowadzana inicjalizacje wyświetlacza HD44780. Następnie konfigurowane są tryby pracy wyświetlacza. Komendywysyłane przy pomocy funkcji LCD_WriteCommand(). W tym przypadku ustawiana zostaje komunikacja 8-bitowa, wyświetlanie na dwóch liniach i wielkość czcionki na 5x7.
 */
void LCD_Init () {
  HAL_GPIO_WritePin (GPIOC,LCD_D0_Pin,0);
  HAL_GPIO_WritePin (GPIOC,LCD_D1_Pin,0);
  HAL_GPIO_WritePin (GPIOC,LCD_D2_Pin,0);
  HAL_GPIO_WritePin (GPIOC,LCD_D3_Pin,0);
  HAL_GPIO_WritePin (GPIOC,LCD_D4_Pin,0);
  HAL_GPIO_WritePin (GPIOC,LCD_D5_Pin,0);
  HAL_GPIO_WritePin (GPIOC,LCD_D6_Pin,0);
  HAL_GPIO_WritePin (GPIOC,LCD_D7_Pin,0);

  HAL_Delay (15);
  HAL_GPIO_WritePin (LCD_E_GPIO_Port, LCD_E_Pin, 0);
  HAL_GPIO_WritePin (LCD_RS_GPIO_Port, LCD_RS_Pin, 0);
  HAL_Delay (4);
  HAL_Delay (100);
  LCD_SendByte (0b00110000);
  HAL_Delay (5);
  LCD_SendByte (0b00110000);
  HAL_Delay (100);
  LCD_SendByte (0b00110000);
  HAL_Delay (100);

  LCD_WriteCommand (LCD_FUNC|LCD_8_BIT|LCDC_TWO_LINE|LCDC_FONT_5x7);
  LCD_WriteCommand (LCD_ONOFF|LCD_DISP_ON);
  LCD_WriteCommand (LCD_CLEAR);
  HAL_Delay (5);
  LCD_WriteCommand (LCDC_ENTRY_MODE|LCD_EM_SHIFT_CURSOR|LCD_EM_RIGHT);
}
