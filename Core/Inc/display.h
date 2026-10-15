/*
 * display.h
 *
 *  Created on: 31 Dec 2022
 *      Author: ACA
 */

/*
		Display.h file is a Board Support Package file.
    Copyright (C) 2018 Nima Mohammadi
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef INC_DISPLAY_H_
#define INC_DISPLAY_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32l4xx.h"
#include "main.h"
#include "ssd1306.h"


typedef enum {
	DISPLAY_PIXEL_BLUE		= 0x00,
	DISPLAY_PIXEL_YELLOW	= 0x01
} pixel_color_t;

typedef struct {
	char* header;
	char* line_one;
	char* line_two;
	char* line_three;
	uint8_t page_index;
} page_container_t;


/*  */
void DISPLAY_Init(I2C_HandleTypeDef* i2cHandler);

/*  */
void DISPLAY_Clear(void);

/* Print String Message in OLED , In this function Before printing , display will be cleared */
void DISPLAY_Write_String(I2C_HandleTypeDef* i2cHandler, char* data , pixel_color_t color , uint8_t x , uint8_t y);

/* Print String Message in OLED , Display wont be cleared */
void DISPLAY_Write_No_Clear(I2C_HandleTypeDef* i2cHandler, char* data , pixel_color_t color , uint8_t x , uint8_t y);

/* Print a page that contains three line and header */
void DISPLAY_Draw_Page(I2C_HandleTypeDef* i2cHandler, page_container_t* container);

/* Print a three line page */
void DISPLAY_Three_Line_Page(I2C_HandleTypeDef* i2cHandler, char* line_one , char* line_two , char* line_three);

/* print a four layer page */
void DISPLAY_Four_Line_Page(I2C_HandleTypeDef* i2cHandler, char* line_one , char* line_two , char* line_three , char* line_four);

/* Used for testing these functions and to show some basic example on display */
void DISPLAY_Show_Test_Page(I2C_HandleTypeDef* i2cHandler);


#ifdef __cplusplus
}
#endif


#endif /* INC_DISPLAY_H_ */
