/*
 * tetris.h
 *
 *  Created on: 12. 9. 2026.
 */

#ifndef INC_TETRIS_H_
#define INC_TETRIS_H_

#include "tim.h"

extern volatile uint8_t score;

void Tetris_Init(void);
void Tetris_Main_Menu(void);

void init_lfsr_seed(TIM_HandleTypeDef *htim);
uint32_t get_random_32(void);
uint32_t get_random_range(uint32_t max);
void shuffle_bag(void);
uint8_t get_next_piece(void);

void Tetris_HandleInput(void);
void Tetris_Update(void);
void Tetris_Draw(void);

void Tetris_MoveLeft(void);
void Tetris_MoveRight(void);
void Tetris_MoveDown(void);
void Tetris_Rotate(void);

void Tetris_CheckLines(void);
void Tetris_DebugPrintBoard(void);


#endif /* INC_TETRIS_H_ */
