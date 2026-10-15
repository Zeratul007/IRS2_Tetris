/*
 * oled.c
 *
 *  Created on: 29 Dec 2022
 *      Author: ACA
 */

#include "ssd1306.h"
#include <stdio.h>


/* Private variable */
static volatile SSD1306_t mySSD1306;

/* SSD1306 data buffer */
static uint8_t SSD1306_Buffer[SSD1306_WIDTH * SSD1306_HEIGHT / 8];


/*
 *	Definitions for functions declared in ssd1306.h
 *
 * */



void SSD1306_Find_Device_Address(I2C_HandleTypeDef* i2cHandler){

	mySSD1306.device_address = 0;

	while(mySSD1306.device_address < 128){
		if (HAL_I2C_IsDeviceReady(i2cHandler, mySSD1306.device_address, 1, 0x00000100) == HAL_OK){
			break;
		}
		mySSD1306.device_address++;
	}
}



uint8_t SSD1306_Init(I2C_HandleTypeDef* i2cHandler){

//	hi2c1 = *i2c;
//
//	/* Init I2C */
//	ssd1306_I2C_Init();
	/* This is done by i2c.c and i2c.h automatic generated files by STM32CubeMX
	 * and there is no necessary for us to do that here, and that's reason why above
	 * lines are commented. */

	/* Set default values */
	mySSD1306.currentX = 0;
	mySSD1306.currentY = 0;
	mySSD1306.initialized = 0;
	mySSD1306.inverted = 0;

	/* A little delay */
	uint32_t start_delay_iterator = 2500;
	while(start_delay_iterator > 0){
		start_delay_iterator--;
	}

	/*
	 *					****	Init LCD	****
	 *	For more info consult: https://cdn-shop.adafruit.com/datasheets/SSD1306.pdf
	 *	pages 28 to 32 contain tables with commands, and bellow is additional description of commands
	 *
	 *															// DEFAULT VALUES (after reset)
	 * */														// -	, single command
	if(SSD1306_Send_Command(i2cHandler, mySSD1306.device_address, 0xAE) != HAL_OK){	// Display OFF (sleep mode)
		return SSD1306_ERROR;
	}															// -	, double Byte command : first Byte
	if(SSD1306_Send_Command(i2cHandler, mySSD1306.device_address, 0x20) != HAL_OK){ // Set Memory Addressing Mode (next sent command will adjust which one!)
		return SSD1306_ERROR;
	}															// 0x10	, double Byte command : second Byte
	if(SSD1306_Send_Command(i2cHandler, mySSD1306.device_address, 0x10) != HAL_OK){ // 00 = Horizontal Addressing Mode, 01 = Vertical Addressing Mode
		return SSD1306_ERROR;														// 10,Page Addressing Mode (RESET), 11,Invalid
	}															// -	, single command
	if(SSD1306_Send_Command(i2cHandler, mySSD1306.device_address, 0xB0) != HAL_OK){ // Set Page Start Address for Page Addressing Mode,0-7
		return SSD1306_ERROR;														// 0xB0 = PAGE0 , ... , 0xB7 = PAGE7
	}															// 0xC0	, single command
	if(SSD1306_Send_Command(i2cHandler, mySSD1306.device_address, 0xC8) != HAL_OK){ // Set COM Output Scan Direction: 0xC8 = remapped mode, 0xC0 = normal mode
		return SSD1306_ERROR;														// For example, if this command is sent during normal display then the
																					// graphic display will be vertically flipped immediately
	}															// -	, single command
	if(SSD1306_Send_Command(i2cHandler, mySSD1306.device_address, 0x00) != HAL_OK){ // Sets lower nibble of column address (this command is only for page addressing mode)
		return SSD1306_ERROR;														// in this case 0x00 will give 0000 nibble
	}															// -	, single command
	if(SSD1306_Send_Command(i2cHandler, mySSD1306.device_address, 0x10) != HAL_OK){ // Set higher nibble of column address
		return SSD1306_ERROR;														// in this case 0x00 will give 0000 nibble
																					// which will result to COLUMN_0
	}															// 0x40	, single command
	if(SSD1306_Send_Command(i2cHandler, mySSD1306.device_address, 0x40) != HAL_OK){ // Set display start line - Set display RAM display start line register from
		return SSD1306_ERROR;														// 0-63 using lower 6 bits of this command which are in this case = 00 0000b
	}															// -	, double Byte command : first Byte
	if(SSD1306_Send_Command(i2cHandler, mySSD1306.device_address, 0x81) != HAL_OK){ // Set contrast control with next command
		return SSD1306_ERROR;
	}															// 0x7F	, double Byte command : second Byte
	if(SSD1306_Send_Command(i2cHandler, mySSD1306.device_address, 0x7F) != HAL_OK){ // Contrast value (RESET = 0x7F)
		return SSD1306_ERROR;														// as much this value is higher, contrast will be higher
	}															// 0xA0	, single command
	if(SSD1306_Send_Command(i2cHandler, mySSD1306.device_address, 0xA1) != HAL_OK){ // Set segment Re-map
		return SSD1306_ERROR;														// 0xA1 = column address 127 will be mapped to SEG0
	}															// 0xA6	, single command
	if(SSD1306_Send_Command(i2cHandler, mySSD1306.device_address, 0xA6) != HAL_OK){ // Set normal/inverse display
		return SSD1306_ERROR;														// 0xA6 = Normal, 0xA7 = Inverse
	}															// -	, double Byte command : first Byte
	if(SSD1306_Send_Command(i2cHandler, mySSD1306.device_address, 0xA8) != HAL_OK){ // Set multiplex ratio (from 16 to 64, 0 to 15 are invalid)
		return SSD1306_ERROR;
	}															// 0x3F	, double Byte command : second Byte
	if(SSD1306_Send_Command(i2cHandler, mySSD1306.device_address, 0x3F) != HAL_OK){ // 0x3F = 63 decimally => MUX ratio =  64
		return SSD1306_ERROR;
	}															// 0xA4 , single command
	if(SSD1306_Send_Command(i2cHandler, mySSD1306.device_address, 0xA4) != HAL_OK){ //0xA4 = Output follows RAM content, 0xA5 = Output ignores RAM content (ENTIRE DISPLAY ON)
		return SSD1306_ERROR;
	}															// It was just a try, and it works!!!!!
//	if(SSD1306_Send_Command(i2cHandler, mySSD1306.device_address, 0xA5) != HAL_OK){ //0xA5 = Output ignores RAM content (ENTIRE DISPLAY ON)
//			return SSD1306_ERROR;
//	}
																// -	, double Byte command : first Byte
	if(SSD1306_Send_Command(i2cHandler, mySSD1306.device_address, 0xD3) != HAL_OK){ // Set display offset: set vertical shift by COM from 0d~63d
		return SSD1306_ERROR;
	}															// 0x00	, double Byte command : second Byte
	if(SSD1306_Send_Command(i2cHandler, mySSD1306.device_address, 0x00) != HAL_OK){ // 0x00 = Not offset
		return SSD1306_ERROR;
	}															// -	, double Byte command : first Byte
	if(SSD1306_Send_Command(i2cHandler, mySSD1306.device_address, 0xD5) != HAL_OK){ // Set display clock divide ratio/oscillator frequency
		return SSD1306_ERROR;
	}															// 0x80	, double Byte command : second Byte
	if(SSD1306_Send_Command(i2cHandler, mySSD1306.device_address, 0xF0) != HAL_OK){ // divide ratio = 1, oscillator frequency = MAX
		return SSD1306_ERROR;
	}															// -	, double Byte command : first Byte
	if(SSD1306_Send_Command(i2cHandler, mySSD1306.device_address, 0xD9) != HAL_OK){ // Set pre-charge period
		return SSD1306_ERROR;
	}															// 0x22	, double Byte command : second Byte
	if(SSD1306_Send_Command(i2cHandler, mySSD1306.device_address, 0x22) != HAL_OK){ // phase1 and phase2 adjustment = by default
		return SSD1306_ERROR;
	}															// -	, double Byte command : first Byte
	if(SSD1306_Send_Command(i2cHandler, mySSD1306.device_address, 0xDA) != HAL_OK){ // Set COM pins hardware configuration
		return SSD1306_ERROR;
	}															// 0x12	, double Byte command : second Byte
	if(SSD1306_Send_Command(i2cHandler, mySSD1306.device_address, 0x12) != HAL_OK){	// => alternative COM pin configuration, disable COM left/right re-map
		return SSD1306_ERROR;
	}															// -	, double Byte command : first Byte
	if(SSD1306_Send_Command(i2cHandler, mySSD1306.device_address, 0xDB) != HAL_OK){ // Set Vcomh deselect level
		return SSD1306_ERROR;
	}															// 0x20	, double Byte command : second Byte
	if(SSD1306_Send_Command(i2cHandler, mySSD1306.device_address, 0x20) != HAL_OK){ // =>Vcomh deselect level = 0.77xVcc
		return SSD1306_ERROR;
	}															//
	if(SSD1306_Send_Command(i2cHandler, mySSD1306.device_address, 0x8D) != HAL_OK){ //--set DC-DC enable
		return SSD1306_ERROR;
	}															//
	if(SSD1306_Send_Command(i2cHandler, mySSD1306.device_address, 0x14) != HAL_OK){ //
		return SSD1306_ERROR;
	}															// -	, single command
	if(SSD1306_Send_Command(i2cHandler, mySSD1306.device_address, 0xAF) != HAL_OK){ // Turn on SSD1306 panel
		return SSD1306_ERROR;
	}


	/* Clear screen */
	//SSD1306_Fill(SSD1306_COLOR_BLACK);
	SSD1306_Fill(SSD1306_COLOR_WHITE);
	/* Update screen */
	SSD1306_Update_Screen(i2cHandler);


	/* After initialization if all commands have passed we can assume that SSD1306 OLED is initalized. */
	mySSD1306.initialized = 0x01;

	return mySSD1306.initialized;	// returning "0x01" as symbol that intialization have passed successfully
}



void SSD1306_Update_Screen(I2C_HandleTypeDef* i2cHandler) {
	uint8_t m;

	for (m = 0; m < 8; m++) {
		/* This Code is following datasheet on page 34 */
		if(SSD1306_Send_Command(i2cHandler, mySSD1306.device_address, 0xB0 + m) != HAL_OK){
			return;
		}
		if(SSD1306_Send_Command(i2cHandler, mySSD1306.device_address, 0x00) != HAL_OK){
			return;
		}
		if(SSD1306_Send_Command(i2cHandler, mySSD1306.device_address, 0x10) != HAL_OK){
			return;
		}

		/* Write multi data */
		SSD1306_I2C_Write_Multi(i2cHandler, mySSD1306.device_address, &SSD1306_Buffer[SSD1306_WIDTH * m], SSD1306_WIDTH);
	}
}



void SSD1306_Toggle_Invert(void) {
	uint16_t i;

	/* Toggle invert */
	mySSD1306.inverted = !mySSD1306.inverted;

	/* Do memory toggle */
	for (i = 0; i < sizeof(SSD1306_Buffer); i++) {
		SSD1306_Buffer[i] = ~SSD1306_Buffer[i]; //maybe xor
	}
}



void SSD1306_Fill(SSD1306_color_t color) {
	/* Set memory */
	if (color == SSD1306_COLOR_BLACK){
		for(int i = 0; i < sizeof(SSD1306_Buffer); i++){
			SSD1306_Buffer[i] = 0x00;
		}
	}
	else{
		for(int i = 0; i < sizeof(SSD1306_Buffer); i++){
			SSD1306_Buffer[i] = 0xFF;
		}
	}
}



void SSD1306_Draw_Pixel(uint16_t x, uint16_t y, SSD1306_color_t color) {
	if (x >= SSD1306_WIDTH || y >= SSD1306_HEIGHT) {
		/* Error */
		return;
	}

	/* Check if pixels are inverted */
	if (mySSD1306.inverted) {
		color = (SSD1306_color_t)!color;
	}

	/* Set color */
	if (color == SSD1306_COLOR_WHITE) {
		SSD1306_Buffer[x + (y / 8) * SSD1306_WIDTH] |= 1 << (y % 8);
	}
	else {
		SSD1306_Buffer[x + (y / 8) * SSD1306_WIDTH] &= ~(1 << (y % 8));
	}
}



void SSD1306_Go_To_XY(uint16_t x, uint16_t y) {
	/* Set write pointers */
	mySSD1306.currentX = x;
	mySSD1306.currentY = y;
}



char SSD1306_Put_Char(char ch, font_t* font, SSD1306_color_t color) {
	uint32_t i, b, j;

	/* Check available space in LCD */
	if (
		SSD1306_WIDTH <= (mySSD1306.currentX + font->font_width) ||
		SSD1306_HEIGHT <= (mySSD1306.currentY + font->font_height)
	) {
		/* Error */
		return 0;
	}

	/* Go through font */
	for (i = 0; i < font->font_height; i++) {
		b = font->data[(ch - 32) * font->font_height + i];
		for (j = 0; j < font->font_width; j++) {
			if ((b << j) & 0x8000) {
				SSD1306_Draw_Pixel(mySSD1306.currentX + j, (mySSD1306.currentY + i), (SSD1306_color_t) color);
			} else {
				SSD1306_Draw_Pixel(mySSD1306.currentX + j, (mySSD1306.currentY + i), (SSD1306_color_t)!color);
			}
		}
	}

	/* Increase pointer */
	mySSD1306.currentX += font->font_width;

	/* Return character written */
	return ch;
}



char SSD1306_Put_String(char* str, font_t* font, SSD1306_color_t color){
	/* Write characters */
	while (*str) {
		/* Write character by character */
		if (SSD1306_Put_Char(*str, font, color) != *str) {
			/* Return error */
			return *str;
		}

		/* Increase string pointer */
		str++;
	}

	/* Everything OK, zero should be returned */
	return *str;
}



void SSD1306_Draw_Line(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, SSD1306_color_t c){
	int16_t dx, dy, sx, sy, err, e2, i, tmp;

	/* Check for overflow */
	if (x0 >= SSD1306_WIDTH) {
		x0 = SSD1306_WIDTH - 1;
	}
	if (x1 >= SSD1306_WIDTH) {
		x1 = SSD1306_WIDTH - 1;
	}
	if (y0 >= SSD1306_HEIGHT) {
		y0 = SSD1306_HEIGHT - 1;
	}
	if (y1 >= SSD1306_HEIGHT) {
		y1 = SSD1306_HEIGHT - 1;
	}

	dx = (x0 < x1) ? (x1 - x0) : (x0 - x1);
	dy = (y0 < y1) ? (y1 - y0) : (y0 - y1);
	sx = (x0 < x1) ? 1 : -1;
	sy = (y0 < y1) ? 1 : -1;
	err = ((dx > dy) ? dx : -dy) / 2;

	if (dx == 0) {
		if (y1 < y0) {
			tmp = y1;
			y1 = y0;
			y0 = tmp;
		}

		if (x1 < x0) {
			tmp = x1;
			x1 = x0;
			x0 = tmp;
		}

		/* Vertical line */
		for (i = y0; i <= y1; i++) {
			SSD1306_Draw_Pixel(x0, i, c);
		}

		/* Return from function */
		return;
	}

	if (dy == 0) {
		if (y1 < y0) {
			tmp = y1;
			y1 = y0;
			y0 = tmp;
		}

		if (x1 < x0) {
			tmp = x1;
			x1 = x0;
			x0 = tmp;
		}

		/* Horizontal line */
		for (i = x0; i <= x1; i++) {
			SSD1306_Draw_Pixel(i, y0, c);
		}

		/* Return from function */
		return;
	}

	while (1) {
		SSD1306_Draw_Pixel(x0, y0, c);
		if (x0 == x1 && y0 == y1) {
			break;
		}
		e2 = err;
		if (e2 > -dx) {
			err -= dy;
			x0 += sx;
		}
		if (e2 < dy) {
			err += dx;
			y0 += sy;
		}
	}
}



void SSD1306_Draw_Rectangle(uint16_t x, uint16_t y, uint16_t w, uint16_t h, SSD1306_color_t c){
	/* Check input parameters */
	if (x >= SSD1306_WIDTH || y >= SSD1306_HEIGHT) {
		/* Return error */
		return;
	}

	/* Check width and height */
	if ((x + w) >= SSD1306_WIDTH) {
		w = SSD1306_WIDTH - x;
	}
	if ((y + h) >= SSD1306_HEIGHT) {
		h = SSD1306_HEIGHT - y;
	}

	/* Draw 4 lines */
	SSD1306_Draw_Line(x, y, x + w, y, c);         /* Top line */
	SSD1306_Draw_Line(x, y + h, x + w, y + h, c); /* Bottom line */
	SSD1306_Draw_Line(x, y, x, y + h, c);         /* Left line */
	SSD1306_Draw_Line(x + w, y, x + w, y + h, c); /* Right line */
}



void SSD1306_Draw_Filled_Rectangle(uint16_t x, uint16_t y, uint16_t w, uint16_t h, SSD1306_color_t c){
	uint8_t i;

	/* Check input parameters */
	if (x >= SSD1306_WIDTH || y >= SSD1306_HEIGHT) {
		/* Return error */
		return;
	}

	/* Check width and height */
	if ((x + w) >= SSD1306_WIDTH) {
		w = SSD1306_WIDTH - x;
	}
	if ((y + h) >= SSD1306_HEIGHT) {
		h = SSD1306_HEIGHT - y;
	}

	/* Draw lines */
	for (i = 0; i <= h; i++) {
		/* Draw lines */
		SSD1306_Draw_Line(x, y + i, x + w, y + i, c);
	}
}



void SSD1306_Draw_Triangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t x3, uint16_t y3, SSD1306_color_t color){
	/* Draw lines */
	SSD1306_Draw_Line(x1, y1, x2, y2, color);
	SSD1306_Draw_Line(x2, y2, x3, y3, color);
	SSD1306_Draw_Line(x3, y3, x1, y1, color);
}



void SSD1306_Draw_Filled_Triangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t x3, uint16_t y3, SSD1306_color_t color){
	int16_t deltax = 0, deltay = 0, x = 0, y = 0, xinc1 = 0, xinc2 = 0,
	yinc1 = 0, yinc2 = 0, den = 0, num = 0, numadd = 0, numpixels = 0,
	curpixel = 0;

	deltax = ABS(x2 - x1);
	deltay = ABS(y2 - y1);
	x = x1;
	y = y1;

	if (x2 >= x1) {
		xinc1 = 1;
		xinc2 = 1;
	} else {
		xinc1 = -1;
		xinc2 = -1;
	}

	if (y2 >= y1) {
		yinc1 = 1;
		yinc2 = 1;
	} else {
		yinc1 = -1;
		yinc2 = -1;
	}

	if (deltax >= deltay){
		xinc1 = 0;
		yinc2 = 0;
		den = deltax;
		num = deltax / 2;
		numadd = deltay;
		numpixels = deltax;
	} else {
		xinc2 = 0;
		yinc1 = 0;
		den = deltay;
		num = deltay / 2;
		numadd = deltax;
		numpixels = deltay;
	}

	for (curpixel = 0; curpixel <= numpixels; curpixel++) {
		SSD1306_Draw_Line(x, y, x3, y3, color);

		num += numadd;
		if (num >= den) {
			num -= den;
			x += xinc1;
			y += yinc1;
		}
		x += xinc2;
		y += yinc2;
	}
}



void SSD1306_Draw_Circle(int16_t x0, int16_t y0, int16_t r, SSD1306_color_t c){
	int16_t f = 1 - r;
	int16_t ddF_x = 1;
	int16_t ddF_y = -2 * r;
	int16_t x = 0;
	int16_t y = r;

	SSD1306_Draw_Pixel(x0, y0 + r, c);
	SSD1306_Draw_Pixel(x0, y0 - r, c);
	SSD1306_Draw_Pixel(x0 + r, y0, c);
	SSD1306_Draw_Pixel(x0 - r, y0, c);

	while (x < y) {
		if (f >= 0) {
			y--;
			ddF_y += 2;
			f += ddF_y;
		}
		x++;
		ddF_x += 2;
		f += ddF_x;

		SSD1306_Draw_Pixel(x0 + x, y0 + y, c);
		SSD1306_Draw_Pixel(x0 - x, y0 + y, c);
		SSD1306_Draw_Pixel(x0 + x, y0 - y, c);
		SSD1306_Draw_Pixel(x0 - x, y0 - y, c);

		SSD1306_Draw_Pixel(x0 + y, y0 + x, c);
		SSD1306_Draw_Pixel(x0 - y, y0 + x, c);
		SSD1306_Draw_Pixel(x0 + y, y0 - x, c);
		SSD1306_Draw_Pixel(x0 - y, y0 - x, c);
	}
}



void SSD1306_Draw_Filled_Circle(int16_t x0, int16_t y0, int16_t r, SSD1306_color_t c){
	int16_t f = 1 - r;
	int16_t ddF_x = 1;
	int16_t ddF_y = -2 * r;
	int16_t x = 0;
	int16_t y = r;

	SSD1306_Draw_Pixel(x0, y0 + r, c);
	SSD1306_Draw_Pixel(x0, y0 - r, c);
	SSD1306_Draw_Pixel(x0 + r, y0, c);
	SSD1306_Draw_Pixel(x0 - r, y0, c);
	SSD1306_Draw_Pixel(x0, y0, c);

	while (x < y) {
		if (f >= 0) {
			y--;
			ddF_y += 2;
			f += ddF_y;
		}
		x++;
		ddF_x += 2;
		f += ddF_x;

		SSD1306_Draw_Line(x0 - x, y0 + y, x0 + x, y0 + y, c);
		SSD1306_Draw_Line(x0 + x, y0 - y, x0 - x, y0 - y, c);

		SSD1306_Draw_Line(x0 + y, y0 + x, x0 - y, y0 + x, c);
		SSD1306_Draw_Line(x0 + y, y0 - x, x0 - y, y0 - x, c);
	}
}



void SSD1306_I2C_Write_Multi(I2C_HandleTypeDef* i2cHandler, uint8_t device_address, uint8_t* data, uint16_t count){
	if (SSD1306_Send_Data(i2cHandler, device_address, data, count) != HAL_OK){
		return;
	}
}



void SSD1306_ON(I2C_HandleTypeDef* i2cHandler, uint16_t device_address) {
	SSD1306_Send_Command(i2cHandler, mySSD1306.device_address, 0x8D);
	SSD1306_Send_Command(i2cHandler, mySSD1306.device_address, 0x14);
	SSD1306_Send_Command(i2cHandler, mySSD1306.device_address, 0xAF);
}



void SSD1306_OFF(I2C_HandleTypeDef* i2cHandler, uint16_t device_address) {
	SSD1306_Send_Command(i2cHandler, mySSD1306.device_address, 0x8D);
	SSD1306_Send_Command(i2cHandler, mySSD1306.device_address, 0x10);
	SSD1306_Send_Command(i2cHandler, mySSD1306.device_address, 0xAE);
}

void SSD1306_Debug_PrintBuffer(void)
{
    for (uint8_t y = 0; y < 64; y++)
    {
        for (uint8_t x = 0; x < 128; x++)
        {
            uint16_t index = x + (y / 8) * 128;
            uint8_t bit = y % 8;

            if (SSD1306_Buffer[index] & (1 << bit))
                printf("1");
            else
                printf("0");
        }

        printf("\r\n");
    }
}

uint8_t SSD1306_Send_Command(I2C_HandleTypeDef* i2cHandler, uint16_t device_address, uint8_t command){
//	uint8_t* command_ptr;
//	*(command_ptr) = 0x00;
//	*(command_ptr + 1) = command;
	uint8_t command_buff[2];
	command_buff[0] = 0x00;
	command_buff[1] = command;
	uint8_t command_status = 0x03;	/* If nothing responds at HAL_I2C_Master_Transmit, but it will always I assume, this will be return value. */

	command_status = HAL_I2C_Master_Transmit(i2cHandler, device_address, command_buff, 0x0002, SSD1306_I2C_TIMEOUT);
	return command_status;
}



uint8_t SSD1306_Send_Data(I2C_HandleTypeDef* i2cHandler, uint16_t device_address, uint8_t* data, uint16_t count){

	uint8_t command_status = SSD1306_ERROR;

	if(count == SSD1306_WIDTH){
		command_status = SSD1306_Send_Row_Data(i2cHandler, device_address, data);
		return command_status;
	}
	else{
		return SSD1306_ERROR;
//		uint8_t* data_ptr;
//		if(count == 0x0001){
//			*(data_ptr) = 0x40;
//			*(data_ptr + 1) = *(data);
//		}
//		else{
//			*(data_ptr) = 0xC0;
//			for(int i = 0; i < count; i++){
//				*(data_ptr + i + 1) = data[i];
//			}
//		}
//
//		uint8_t command_status = 0x03;	/* If nothing responds at HAL_I2C_Master_Transmit, but it will always I assume, this will be return value. */
//
//		command_status = HAL_I2C_Master_Transmit(i2cHandler, device_address, data_buff, count + 1, SSD1306_I2C_TIMEOUT);
//		return command_status;
	}
}



uint8_t SSD1306_Send_Row_Data(I2C_HandleTypeDef* i2cHandler, uint16_t device_address, uint8_t* data){

	/* Continuous data mode */	// doesn't work, I don't know the reason
//	uint8_t data_buff[SSD1306_WIDTH+1];
//	data_buff[0] = 0xC0;
//	for(int i = 0; i < SSD1306_WIDTH; i++){
//		data_buff[i + 1] = *(data + i);
//	}
//
//	uint8_t command_status = 0x03;	/* If nothing responds at HAL_I2C_Master_Transmit, but it will always I assume, this will be return value. */
//
//	command_status = HAL_I2C_Master_Transmit(i2cHandler, device_address, data_buff, SSD1306_WIDTH + 1, SSD1306_I2C_TIMEOUT);
//	return command_status;

	/* Single data mode */
//	uint8_t data_buff[2];
//	data_buff[0] = 0x40;
//	for(int i = 0; i < SSD1306_WIDTH; i++){
//		data_buff[2] = *(data + i);
//		if(HAL_I2C_Master_Transmit(i2cHandler, device_address, data_buff, 0x0002, SSD1306_I2C_TIMEOUT) != HAL_OK){
//			return SSD1306_ERROR;
//		}
//	}
//	return SSD1306_OK;

	uint8_t data_buff[SSD1306_WIDTH+1];
	data_buff[0] = 0x40;
	for(int i = 0; i < SSD1306_WIDTH; i++){
		data_buff[i + 1] = *(data + i);
	}

	uint8_t command_status = 0x03;	/* If nothing responds at HAL_I2C_Master_Transmit, but it will always I assume, this will be return value. */

	command_status = HAL_I2C_Master_Transmit(i2cHandler, device_address, data_buff, SSD1306_WIDTH + 1, SSD1306_I2C_TIMEOUT);
	return command_status;
}
