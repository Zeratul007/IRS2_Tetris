/*
 * 7seg.h
 *
 *  Created on: Sep 9, 2026
 *      Author: djord
 */

#ifndef INC_7SEG_H_
#define INC_7SEG_H_

#include "main.h"

void SevenSeg_Init(void);

void SevenSeg_DisplayDigit(uint8_t digit, uint8_t value);

void SevenSeg_DisplayNumber(uint8_t number);

void SevenSeg_Clear(void);

#endif /* INC_7SEG_H_ */
