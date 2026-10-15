/*
 * display.c
 *
 *  Created on: 31 Dec 2022
 *      Author: ACA
 */


#include "display.h"





void DISPLAY_Init(I2C_HandleTypeDef* i2cHandler){
	/* LCD IS CONNECTED TO i2c-2 change this if you are using other ports */
	if(SSD1306_Init(i2cHandler) != SSD1306_OK){
		// ERROR!
		return;
	}
	else{
		// ALL IS FINE!
		return;
	}
}



void DISPLAY_Clear(void){
	SSD1306_Fill(SSD1306_COLOR_BLACK);
}



void DISPLAY_Write_String(I2C_HandleTypeDef* i2cHandler, char* data , pixel_color_t color , uint8_t x , uint8_t y){
	SSD1306_Fill(SSD1306_COLOR_BLACK);
	SSD1306_Go_To_XY (x , y);
	SSD1306_Put_String(data , &font_7x10 , color == DISPLAY_PIXEL_BLUE ? SSD1306_COLOR_WHITE : SSD1306_COLOR_BLACK);
	SSD1306_Update_Screen(i2cHandler);
}



void DISPLAY_Write_No_Clear(I2C_HandleTypeDef* i2cHandler, char* data , pixel_color_t color , uint8_t x , uint8_t y){
	SSD1306_Go_To_XY (x , y);
	SSD1306_Put_String(data , &font_7x10 , color == DISPLAY_PIXEL_BLUE ? SSD1306_COLOR_WHITE : SSD1306_COLOR_BLACK);
	SSD1306_Update_Screen(i2cHandler);
}



void DISPLAY_Draw_Page(I2C_HandleTypeDef* i2cHandler, page_container_t* container){
	DISPLAY_Clear();
	DISPLAY_Write_No_Clear(i2cHandler, container->header , DISPLAY_PIXEL_BLUE , 5 , 5);
	DISPLAY_Write_No_Clear(i2cHandler, container->line_one , DISPLAY_PIXEL_BLUE , 5 , 20);
	DISPLAY_Write_No_Clear(i2cHandler, container->line_two , DISPLAY_PIXEL_BLUE , 5 , 35);
	DISPLAY_Write_No_Clear(i2cHandler, container->line_three , DISPLAY_PIXEL_BLUE , 5 , 50);
}



void DISPLAY_Three_Line_Page(I2C_HandleTypeDef* i2cHandler, char* line_one , char* line_two , char* line_three){
	page_container_t container;
	container.header = "Ebrahim Co"; //"GB CELL";
	container.line_one = line_one;
	container.line_two = line_two;
	container.line_three = line_three;
	container.page_index = 2;
	//updateState = DISPLAY_UPDATE_DENIED;
	DISPLAY_Draw_Page(i2cHandler, &container);
}



void DISPLAY_Four_Line_Page(I2C_HandleTypeDef* i2cHandler, char* line_one , char* line_two , char* line_three , char* line_four){
	page_container_t container;
	container.header = line_one;
	container.line_one = line_two;
	container.line_two = line_three;
	container.line_three = line_four;
	container.page_index = 3;
	//updateState = DISPLAY_UPDATE_DENIED;
	DISPLAY_Draw_Page(i2cHandler, &container);
}



__weak void DISPLAY_Show_Test_Page(I2C_HandleTypeDef* i2cHandler){
	page_container_t container;
	container.header = "TestHeader";
	container.line_one = "Line One";
	container.line_two = "Line Two";
	container.line_three = "Line Three";
	DISPLAY_Draw_Page(i2cHandler, &container);
}
