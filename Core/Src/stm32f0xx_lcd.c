/*
 * stm32f0xx_lcd.c
 *
 *  Created on: Nov 10, 2021
 *      Author: jamia
 */
#include "stm32f0xx_lcd.h"
#include "stm32f0xx_hal.h"

#include <stdio.h>
#include <string.h>
#include <stdint.h>


// When the display powers up, it is configured as follows:
//
// 1. Display clear
// 2. Function set:
//    DL = 1; 8-bit interface data
//    N = 0; 1-line display
//    F = 0; 5x8 dot character font
// 3. Display on/off control:
//    D = 0; Display off
//    C = 0; Cursor off
//    B = 0; Blinking off
// 4. Entry mode set:
//    I/D = 1; Increment by 1
//    S = 0; No shift
//
// Note, however, that resetting the "Arduino" doesn't reset the LCD, so we
// can't assume that it's in that state when a sketch starts (and the
// LiquidCrystal constructor is called).

//uint8_t _data_pins[4] = {D4_Pin, D5_Pin, D6_Pin, D7_Pin};
//uint8_t _data_ports[4]= {D4_GPIO_Port, D5_GPIO_Port, D6_GPIO_Port, D7_GPIO_Port};

uint8_t _displayfunction = LCD_4BITMODE | LCD_1LINE | LCD_5x8DOTS;
uint8_t _displaycontrol;
uint8_t _displaymode;

uint8_t _initialized;

uint8_t _numlines;
uint8_t _row_offsets[4];

void begin(uint8_t cols, uint8_t lines, uint8_t dotsize){
	if (lines > 1){
		_displayfunction |= LCD_2LINE;
	}

	_numlines = lines;

	setRowOffsets(0x00, 0x40, 0x00 + cols, 0x40 + cols);


	// SEE PAGE 45/46 FOR INITIALIZATION SPECIFICATION!
	// according to datasheet, we need at least 40 ms after power rises above 2.7 V
	// before sending commands. Arduino can turn on way before 4.5 V so we'll wait 50
	HAL_Delay(50);
	// Now we pull both RS and R/W low to begin commands
	HAL_GPIO_WritePin(RS_GPIO_Port, RS_Pin, RESET);
	HAL_GPIO_WritePin(EN_GPIO_Port, EN_Pin, RESET);

	// this is according to the Hitachi HD44780 datasheet
	// figure 24, pg 46

	// we start in 8bit mode, try to set 4 bit mode
	write4bits(0x03);
	HAL_Delay(4.5);

	//second try
	write4bits(0x03);
	HAL_Delay(4.5);

	// third go!
	write4bits(0x03);
	HAL_Delay(150/1000);

	// finally, set to 4-bit interface
	write4bits(0x02);


	// finally, set # lines, font size, etc.
	command(LCD_FUNCTIONSET | _displayfunction);

	// turn the display on with no cursor or blinking default
	_displaycontrol = LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF;
	display();

	// clear it off
	clear();

	// Initialize to default text direction (for romance languages)
	_displaymode = LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT;
	// set the entry mode
	command(LCD_ENTRYMODESET | _displaymode);
}

void setRowOffsets(int row0, int row1, int row2, int row3){
	_row_offsets[0] = row0;
	_row_offsets[1] = row1;
	_row_offsets[2] = row2;
	_row_offsets[3] = row3;
}

/********** high level commands, for the user! */

void writeDisplay(char *str){
	int i;
	for(i = 0; str[i] != '\0'; i++)
		write(str[i]);
}

void clear(){
	command(LCD_CLEARDISPLAY); // clear display, set cursor position to zero
	HAL_Delay(2); // this command takes a long time!
}

void home(){
	command(LCD_RETURNHOME); // set cursor position to zero
	HAL_Delay(2); // this command takes a long time!
}

void setCursor(uint8_t col, uint8_t row){
	const size_t max_lines = sizeof(_row_offsets) / sizeof(*_row_offsets);
	if(row >= max_lines){
		row = max_lines - 1; // we count rows starting w/ 0
	}
	if(row >= _numlines){
		row = _numlines - 1; // we count rows starting w/ 0
	}

	command(LCD_SETDDRAMADDR |(col +  _row_offsets[row]));
}

void noDIsplay(){
	_displaycontrol &= ~LCD_DISPLAYON;
	command(LCD_DISPLAYCONTROL | _displaycontrol);
}
void display(){
	_displaycontrol &= LCD_DISPLAYON;
	command(LCD_DISPLAYCONTROL | _displaycontrol);
}

// Turns the underline cursor on/off
void noCursor() {
  _displaycontrol &= ~LCD_CURSORON;
  command(LCD_DISPLAYCONTROL | _displaycontrol);
}
void cursor() {
  _displaycontrol |= LCD_CURSORON;
  command(LCD_DISPLAYCONTROL | _displaycontrol);
}

// Turn on and off the blinking cursor
void noBlink() {
  _displaycontrol &= ~LCD_BLINKON;
  command(LCD_DISPLAYCONTROL | _displaycontrol);
}
void blink() {
  _displaycontrol |= LCD_BLINKON;
  command(LCD_DISPLAYCONTROL | _displaycontrol);
}

// These commands scroll the display without changing the RAM
void scrollDisplayLeft(void) {
  command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVELEFT);
}
void scrollDisplayRight(void) {
  command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVERIGHT);
}

// This is for text that flows Left to Right
void leftToRight(void) {
  _displaymode |= LCD_ENTRYLEFT;
  command(LCD_ENTRYMODESET | _displaymode);
}

// This is for text that flows Right to Left
void rightToLeft(void) {
  _displaymode &= ~LCD_ENTRYLEFT;
  command(LCD_ENTRYMODESET | _displaymode);
}

// This will 'right justify' text from the cursor
void autoscroll(void) {
  _displaymode |= LCD_ENTRYSHIFTINCREMENT;
  command(LCD_ENTRYMODESET | _displaymode);
}

// This will 'left justify' text from the cursor
void noAutoscroll(void) {
  _displaymode &= ~LCD_ENTRYSHIFTINCREMENT;
  command(LCD_ENTRYMODESET | _displaymode);
}

// Allows us to fill the first 8 CGRAM locations
// with custom characters
void createChar(uint8_t location, uint8_t charmap[]) {
  location &= 0x7; // we only have 8 locations 0-7
  command(LCD_SETCGRAMADDR | (location << 3));
  for (int i=0; i<8; i++) {
    write(charmap[i]);
  }
}

/*********** mid level commands, for sending data/cmds */

void command(uint8_t value){
	send(value, RESET);
}

void write(uint8_t value){
	send(value, SET);
}


/************ low level data pushing commands **********/

// write either command or data, with automatic 4/8-bit selection
void send(uint8_t value, uint8_t mode){

	HAL_GPIO_WritePin(RS_GPIO_Port, RS_Pin, mode);

	write4bits(value>>4);
	write4bits(value);
}

void pulseEnable(void){
	HAL_GPIO_WritePin(EN_GPIO_Port, EN_Pin, RESET);
	HAL_Delay(1/1000);
	HAL_GPIO_WritePin(EN_GPIO_Port, EN_Pin, SET);
	HAL_Delay(1/1000);
	HAL_GPIO_WritePin(EN_GPIO_Port, EN_Pin, RESET);
	HAL_Delay(100/1000);
}

void write4bits(uint8_t value){

	HAL_GPIO_WritePin(D4_GPIO_Port, D4_Pin, (value) & 0x01);
	HAL_GPIO_WritePin(D5_GPIO_Port, D5_Pin, (value >> 1) & 0x01);
	HAL_GPIO_WritePin(D6_GPIO_Port, D6_Pin, (value >> 2) & 0x01);
	HAL_GPIO_WritePin(D7_GPIO_Port, D7_Pin, (value >> 3) & 0x01);

	pulseEnable();
}
