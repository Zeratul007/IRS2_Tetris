/*
 * oled.h
 *
 *  Created on: 29 Dec 2022
 *      Author: ACA
 */


/*
		ssd1306.h file is External display library file.
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


#ifndef INC_SSD1306_H_
#define INC_SSD1306_H_ 100

/* C++ detection */
#ifdef __cplusplus
extern "C" {
#endif

/**
 * This SSD1306 LCD uses I2C for communication
 *
 * Library features functions for drawing lines, rectangles and circles.
 *
 * It also allows you to draw texts and characters using appropriate functions provided in library.
 *
 * Default pinout
 *
SSD1306    |STM32F10x    |DESCRIPTION
VCC        |3.3V         |
GND        |GND          |
SCL        |PB6          |Serial clock line
SDA        |PB7          |Serial data line
 */


#include "main.h"
#include "font.h"

/* Device address for I2C communication protocol */
#define SSD1306_DEVICE_ADDR		0x78	/* For specially my OLED device this is it's address: 0x78 */

/* SSD1306 settings */
/* SSD1306 OLED width in pixels */
#define SSD1306_WIDTH			128
/* SSD1306 OLED height in pixels */
#define SSD1306_HEIGHT			64

#ifndef SSD1306_I2C_TIMEOUT
#define SSD1306_I2C_TIMEOUT		20000	// 0x00000100 also worked fine for SSD1306_Init (function defined in ssd1306.c)
#endif

/**
 * @brief  SSD1306 color enumeration
 */
typedef enum {
	SSD1306_COLOR_BLACK = 0x00,	/*!< Black color, no pixel */
	SSD1306_COLOR_WHITE = 0x01	/*!< Pixel is set. Color depends on LCD */
} SSD1306_color_t;


/* SSD1306 structure */
typedef struct {
	uint8_t device_address;
	uint16_t currentX;
	uint16_t currentY;
	uint8_t inverted;
	uint8_t initialized;

} SSD1306_t;


typedef enum {
	SSD1306_ERROR	= 0x00,
	SSD1306_OK		= 0x01
} SSD1306_status_t;

/* Send command */
#define SEND_COMMAND_TO_SSD1306(i2cHandler, devAddr, command)	HAL_I2C_Mem_Write((i2cHandler),\
																					(devAddr),\
																					0x00,\
																					1,\
																					(command),\
																					1,\
																					SSD1306_I2C_TIMEOUT)

/* Absolute value */
#define ABS(x)   ((x) > 0 ? (x) : -(x))



/**
 * @brief  Finds SSD1306 device specific address.
 * @note   This function is called on the start just to find SSD1306 address which is dedicated for I2C communication.
 * @param  i2cHandler: handler to the I2C structure, which is used for I2C communication.
 * @retval None
 */
void SSD1306_Find_Device_Address(I2C_HandleTypeDef* i2cHandler);



/**
 * @brief  Initializes SSD1306 LCD
 * @param  i2cHandler: handler to the I2C structure, which is used for I2C communication.
 * @retval Initialization status:
 *           - 0: LCD was not detected on I2C port
 *           - > 0: LCD initialized OK and ready to use
 */
uint8_t SSD1306_Init(I2C_HandleTypeDef* i2cHandler);

/**
 * @brief  Updates buffer from internal RAM to LCD
 * @note   This function must be called each time you do some changes to LCD, to update buffer from RAM to LCD
 * @param  None
 * @retval None
 */
void SSD1306_Update_Screen(I2C_HandleTypeDef* i2cHandler);

/**
 * @brief  Toggles pixels invertion inside internal RAM
 * @note   @ref SSD1306_UpdateScreen() must be called after that in order to see updated LCD screen
 * @param  None
 * @retval None
 */
void SSD1306_Toggle_Invert(void);

/**
 * @brief  Fills entire LCD with desired color
 * @note   @ref SSD1306_UpdateScreen() must be called after that in order to see updated LCD screen
 * @param  Color: Color to be used for screen fill. This parameter can be a value of @ref SSD1306_COLOR_t enumeration
 * @retval None
 */
void SSD1306_Fill(SSD1306_color_t Color);

/**
 * @brief  Draws pixel at desired location
 * @note   @ref SSD1306_UpdateScreen() must called after that in order to see updated LCD screen
 * @param  x: X location. This parameter can be a value between 0 and SSD1306_WIDTH - 1
 * @param  y: Y location. This parameter can be a value between 0 and SSD1306_HEIGHT - 1
 * @param  color: Color to be used for screen fill. This parameter can be a value of @ref SSD1306_COLOR_t enumeration
 * @retval None
 */
void SSD1306_Draw_Pixel(uint16_t x, uint16_t y, SSD1306_color_t color);

/**
 * @brief  Sets cursor pointer to desired location for strings
 * @param  x: X location. This parameter can be a value between 0 and SSD1306_WIDTH - 1
 * @param  y: Y location. This parameter can be a value between 0 and SSD1306_HEIGHT - 1
 * @retval None
 */
void SSD1306_Go_To_XY(uint16_t x, uint16_t y);

/**
 * @brief  Puts character to internal RAM
 * @note   @ref SSD1306_UpdateScreen() must be called after that in order to see updated LCD screen
 * @param  ch: Character to be written
 * @param  *Font: Pointer to @ref FontDef_t structure with used font
 * @param  color: Color used for drawing. This parameter can be a value of @ref SSD1306_COLOR_t enumeration
 * @retval Character written
 */
char SSD1306_Put_Char(char ch, font_t* font, SSD1306_color_t color);

/**
 * @brief  Puts string to internal RAM
 * @note   @ref SSD1306_UpdateScreen() must be called after that in order to see updated LCD screen
 * @param  *str: String to be written
 * @param  *Font: Pointer to @ref FontDef_t structure with used font
 * @param  color: Color used for drawing. This parameter can be a value of @ref SSD1306_COLOR_t enumeration
 * @retval Zero on success or character value when function failed
 */
char SSD1306_Put_String(char* str, font_t* font, SSD1306_color_t color);

/**
 * @brief  Draws line on LCD
 * @note   @ref SSD1306_UpdateScreen() must be called after that in order to see updated LCD screen
 * @param  x0: Line X start point. Valid input is 0 to SSD1306_WIDTH - 1
 * @param  y0: Line Y start point. Valid input is 0 to SSD1306_HEIGHT - 1
 * @param  x1: Line X end point. Valid input is 0 to SSD1306_WIDTH - 1
 * @param  y1: Line Y end point. Valid input is 0 to SSD1306_HEIGHT - 1
 * @param  c: Color to be used. This parameter can be a value of @ref SSD1306_COLOR_t enumeration
 * @retval None
 */
void SSD1306_Draw_Line(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, SSD1306_color_t c);

/**
 * @brief  Draws rectangle on LCD
 * @note   @ref SSD1306_UpdateScreen() must be called after that in order to see updated LCD screen
 * @param  x: Top left X start point. Valid input is 0 to SSD1306_WIDTH - 1
 * @param  y: Top left Y start point. Valid input is 0 to SSD1306_HEIGHT - 1
 * @param  w: Rectangle width in units of pixels
 * @param  h: Rectangle height in units of pixels
 * @param  c: Color to be used. This parameter can be a value of @ref SSD1306_COLOR_t enumeration
 * @retval None
 */
void SSD1306_Draw_Rectangle(uint16_t x, uint16_t y, uint16_t w, uint16_t h, SSD1306_color_t c);

/**
 * @brief  Draws filled rectangle on LCD
 * @note   @ref SSD1306_UpdateScreen() must be called after that in order to see updated LCD screen
 * @param  x: Top left X start point. Valid input is 0 to SSD1306_WIDTH - 1
 * @param  y: Top left Y start point. Valid input is 0 to SSD1306_HEIGHT - 1
 * @param  w: Rectangle width in units of pixels
 * @param  h: Rectangle height in units of pixels
 * @param  c: Color to be used. This parameter can be a value of @ref SSD1306_COLOR_t enumeration
 * @retval None
 */
void SSD1306_Draw_Filled_Rectangle(uint16_t x, uint16_t y, uint16_t w, uint16_t h, SSD1306_color_t c);

/**
 * @brief  Draws triangle on LCD
 * @note   @ref SSD1306_UpdateScreen() must be called after that in order to see updated LCD screen
 * @param  x1: First coordinate X location. Valid input is 0 to SSD1306_WIDTH - 1
 * @param  y1: First coordinate Y location. Valid input is 0 to SSD1306_HEIGHT - 1
 * @param  x2: Second coordinate X location. Valid input is 0 to SSD1306_WIDTH - 1
 * @param  y2: Second coordinate Y location. Valid input is 0 to SSD1306_HEIGHT - 1
 * @param  x3: Third coordinate X location. Valid input is 0 to SSD1306_WIDTH - 1
 * @param  y3: Third coordinate Y location. Valid input is 0 to SSD1306_HEIGHT - 1
 * @param  c: Color to be used. This parameter can be a value of @ref SSD1306_COLOR_t enumeration
 * @retval None
 */
void SSD1306_Draw_Triangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t x3, uint16_t y3, SSD1306_color_t color);

/**
 * @brief  Draws triangle on LCD
 * @note   @ref SSD1306_UpdateScreen() must be called after that in order to see updated LCD screen
 * @param  x1: First coordinate X location. Valid input is 0 to SSD1306_WIDTH - 1
 * @param  y1: First coordinate Y location. Valid input is 0 to SSD1306_HEIGHT - 1
 * @param  x2: Second coordinate X location. Valid input is 0 to SSD1306_WIDTH - 1
 * @param  y2: Second coordinate Y location. Valid input is 0 to SSD1306_HEIGHT - 1
 * @param  x3: Third coordinate X location. Valid input is 0 to SSD1306_WIDTH - 1
 * @param  y3: Third coordinate Y location. Valid input is 0 to SSD1306_HEIGHT - 1
 * @param  c: Color to be used. This parameter can be a value of @ref SSD1306_COLOR_t enumeration
 * @retval None
 */
void SSD1306_Draw_Filled_Triangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t x3, uint16_t y3, SSD1306_color_t color);

/**
 * @brief  Draws circle to STM buffer
 * @note   @ref SSD1306_UpdateScreen() must be called after that in order to see updated LCD screen
 * @param  x: X location for center of circle. Valid input is 0 to SSD1306_WIDTH - 1
 * @param  y: Y location for center of circle. Valid input is 0 to SSD1306_HEIGHT - 1
 * @param  r: Circle radius in units of pixels
 * @param  c: Color to be used. This parameter can be a value of @ref SSD1306_COLOR_t enumeration
 * @retval None
 */
void SSD1306_Draw_Circle(int16_t x0, int16_t y0, int16_t r, SSD1306_color_t c);

/**
 * @brief  Draws filled circle to STM buffer
 * @note   @ref SSD1306_UpdateScreen() must be called after that in order to see updated LCD screen
 * @param  x: X location for center of circle. Valid input is 0 to SSD1306_WIDTH - 1
 * @param  y: Y location for center of circle. Valid input is 0 to SSD1306_HEIGHT - 1
 * @param  r: Circle radius in units of pixels
 * @param  c: Color to be used. This parameter can be a value of @ref SSD1306_COLOR_t enumeration
 * @retval None
 */
void SSD1306_Draw_Filled_Circle(int16_t x0, int16_t y0, int16_t r, SSD1306_color_t c);

/**
 * @brief  Initializes SSD1306 LCD
 * @param  None
 * @retval Initialization status:
 *           - 0: LCD was not detected on I2C port
 *           - > 0: LCD initialized OK and ready to use
 */
void SSD1306_I2C_Init(void);

/**
 * @brief  Writes single byte to slave
 * @param  *I2Cx: I2C used
 * @param  address: 7 bit slave address, left aligned, bits 7:1 are used, LSB bit is not used
 * @param  reg: register to write to
 * @param  data: data to be written
 * @retval None
 */
void SSD1306_I2C_Write(uint8_t address, uint8_t reg, uint8_t data);

/**
 * @brief  Writes multi bytes to slave
 * @param  *I2Cx: I2C used
 * @param  address: 7 bit slave address, left aligned, bits 7:1 are used, LSB bit is not used
 * @param  reg: register to write to
 * @param  *data: pointer to data array to write it to slave
 * @param  count: how many bytes will be written
 * @retval None
 */
void SSD1306_I2C_Write_Multi(I2C_HandleTypeDef* i2cHandler, uint8_t device_address, uint8_t* data, uint16_t count);

/**
 * @brief
 * @param  None
 * @retval None
 */
void SSD1306_ON(I2C_HandleTypeDef* i2cHandler, uint16_t device_address);

/**
 * @brief
 * @param  None
 * @retval None
 */
void SSD1306_OFF(I2C_HandleTypeDef* i2cHandler, uint16_t device_address);

/**
 * @brief
 * @param
 * @retval
 */

void SSD1306_Debug_PrintBuffer(void);

uint8_t SSD1306_Send_Command(I2C_HandleTypeDef* i2cHandler, uint16_t device_address, uint8_t command);

/**
 * @brief
 * @param
 * @retval
 */
uint8_t SSD1306_Send_Data(I2C_HandleTypeDef* i2cHandler, uint16_t device_address, uint8_t* data, uint16_t count);

/**
 * @brief
 * @param
 * @retval
 */
uint8_t SSD1306_Send_Row_Data(I2C_HandleTypeDef* i2cHandler, uint16_t device_address, uint8_t* data);


#ifdef __cplusplus
}
#endif

#endif /* INC_SSD1306_H_ */
