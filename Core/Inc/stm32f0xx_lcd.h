/*
 * stm32f0xx_lcd.h
 *
 *  Created on: Nov 10, 2021
 *      Author: jamian vieira
 */
#include <stdint.h>
#ifndef INC_STM32F0XX_LCD_H_
#define INC_STM32F0XX_LCD_H_

//Ports and pins
#define D4_Pin GPIO_PIN_5
#define D4_GPIO_Port GPIOB
#define D5_Pin GPIO_PIN_4
#define D5_GPIO_Port GPIOB
#define D6_Pin GPIO_PIN_10
#define D6_GPIO_Port GPIOB
#define D7_Pin GPIO_PIN_8
#define D7_GPIO_Port GPIOA
#define RS_Pin GPIO_PIN_9
#define RS_GPIO_Port GPIOA
#define EN_Pin GPIO_PIN_7
#define EN_GPIO_Port GPIOC


// commands
#define LCD_CLEARDISPLAY 0x01
#define LCD_RETURNHOME 0x02
#define LCD_ENTRYMODESET 0x04
#define LCD_DISPLAYCONTROL 0x08
#define LCD_CURSORSHIFT 0x10
#define LCD_FUNCTIONSET 0x20
#define LCD_SETCGRAMADDR 0x40
#define LCD_SETDDRAMADDR 0x80

// flags for display entry mode
#define LCD_ENTRYRIGHT 0x00
#define LCD_ENTRYLEFT 0x02
#define LCD_ENTRYSHIFTINCREMENT 0x01
#define LCD_ENTRYSHIFTDECREMENT 0x00

// flags for display on/off control
#define LCD_DISPLAYON 0x04
#define LCD_DISPLAYOFF 0x00
#define LCD_CURSORON 0x02
#define LCD_CURSOROFF 0x00
#define LCD_BLINKON 0x01
#define LCD_BLINKOFF 0x00

// flags for display/cursor shift
#define LCD_DISPLAYMOVE 0x08
#define LCD_CURSORMOVE 0x00
#define LCD_MOVERIGHT 0x04
#define LCD_MOVELEFT 0x00

// flags for function set
#define LCD_8BITMODE 0x10
#define LCD_4BITMODE 0x00
#define LCD_2LINE 0x08
#define LCD_1LINE 0x00
#define LCD_5x10DOTS 0x04
#define LCD_5x8DOTS 0x00

void begin(uint8_t cols, uint8_t rows);

void writeDisplay(char *str);
void clear();
void home();
void noDisplay();
void display();
void noBlink();
void blink();
void noCursor();
void cursor();
void scrollDisplayLeft();
void scrollDisplayRight();
void leftToRight();
void rightToLeft();
void autoscroll();
void noAutoscroll();
void setRowOffsets(int row1, int row2, int row3, int row4);
void createChar(uint8_t, uint8_t[]);
void setCursor(uint8_t, uint8_t);
void write(uint8_t);
void command(uint8_t);

void send(uint8_t, uint8_t);
void write4bits(uint8_t);
void write8bits(uint8_t);
void pulseEnable();

#endif /* INC_STM32F0XX_LCD_H_ */
