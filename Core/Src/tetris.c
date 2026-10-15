/*
 * tetris.c
 *
 *  Created on: 12. 9. 2026.
 *
 */

#include "tetris.h"
#include "7seg.h"
#include "main.h"
#include "ssd1306.h"
#include "display.h"
#include "i2c.h"
#include "gpio.h"
#include <stdio.h>
#include <string.h>

/* =========================================================
 * TETRIS BOARD
 * ========================================================= */

//Board dimensions
#define BOARD_WIDTH     15 // must be less than (128 - BOARD_X - BOARD_WALL)/BLOCK_SIZE
#define BOARD_HEIGHT    30 // must be less than (64 - BOARD_Y - BOARD_WALL)/BLOCK_SIZE

#define BLOCK_SIZE      2

//Board top left coordinates

#define BOARD_X         2 // must be greater than BOARD_WALL
#define BOARD_Y         2

//Board wall and floor thickness

#define BOARD_WALL		2


/* =========================================================
 * TIMING
 * ========================================================= */

#define FALL_DELAY      250


/* =========================================================
 * BOARD
 *
 * 0 = empty
 * 1 = occupied
 * ========================================================= */

static uint8_t board[BOARD_HEIGHT][BOARD_WIDTH];


/* =========================================================
 * CURRENT PIECE
 * ========================================================= */

typedef struct
{
    int8_t x;
    int8_t y;

    uint8_t shape[4][4];

} Tetromino;


static Tetromino currentPiece;

/* =========================================================
 * NEW PEACE VARIABLES
 * ========================================================= */

#define BAG_SIZE 7

uint32_t lfsr_state = 0;
uint8_t tetris_bag[BAG_SIZE] = {0, 1, 2, 3, 4, 5, 6};
int8_t bag_index = -1; // Forces an immediate shuffle on the first pull


/* =========================================================
 * GAME VARIABLES
 * ========================================================= */


static uint32_t lastFallTime;

static uint8_t gameOver;

volatile uint8_t score = 0;

/* =========================================================
 * TETROMINO DEFINITIONS
 * ========================================================= */

static const uint8_t tetrominoes[7][4][4] =
{
    /* I */
    {
        {0,0,0,0},
        {1,1,1,1},
        {0,0,0,0},
        {0,0,0,0}
    },

    /* O */
    {
        {0,1,1,0},
        {0,1,1,0},
        {0,0,0,0},
        {0,0,0,0}
    },

    /* T */
    {
        {0,1,0,0},
        {1,1,1,0},
        {0,0,0,0},
        {0,0,0,0}
    },

    /* L */
    {
        {1,0,0,0},
        {1,1,1,0},
        {0,0,0,0},
        {0,0,0,0}
    },

    /* J */
    {
        {0,0,1,0},
        {1,1,1,0},
        {0,0,0,0},
        {0,0,0,0}
    },

    /* S */
    {
        {0,1,1,0},
        {1,1,0,0},
        {0,0,0,0},
        {0,0,0,0}
    },

    /* Z */
    {
        {1,1,0,0},
        {0,1,1,0},
        {0,0,0,0},
        {0,0,0,0}
    }
};

/* =========================================================
 * COLLISION CHECK
 * ========================================================= */

static uint8_t Tetris_Collision(Tetromino *piece)
{
    for (int y = 0; y < 4; y++)
    {
        for (int x = 0; x < 4; x++)
        {
            if (piece->shape[y][x])
            {
                int boardX = piece->x + x;
                int boardY = piece->y + y;

                /* Left / right walls */
                if (boardX < 0 || boardX >= BOARD_WIDTH)
                    return 1;

                /* Bottom */
                if (boardY >= BOARD_HEIGHT)
                    return 1;

                /* Existing block */
                if (boardY >= 0 && board[boardY][boardX])
                    return 1;
            }
        }
    }

    return 0;
}

/* =========================================================
 * NEW PEACE
 * ========================================================= */

void init_lfsr_seed(TIM_HandleTypeDef *htim) {
    // Read the current 32-bit hardware timer counter register directly
    lfsr_state = __HAL_TIM_GET_COUNTER(htim);

    // Safety check: LFSR state must NEVER be 0, or it will lock up
    if (lfsr_state == 0) {
        lfsr_state = 0xACE1u;
    }
}
uint32_t get_random_32(void) {
    // 32-bit maximal-length LFSR taps: bits 32, 22, 2, 1
    const uint32_t TAP_MASK = 0x80200003u;

    for (int i = 0; i < 32; i++) {
        uint32_t lsb = lfsr_state & 1;
        lfsr_state >>= 1;
        if (lsb) {
            lfsr_state ^= TAP_MASK;
        }
    }
    return lfsr_state;
}
uint32_t get_random_range(uint32_t max) {
    return get_random_32() % max;
}

void shuffle_bag(void) {
    // Reset standard pieces in the array
    tetris_bag[0] = 0; tetris_bag[1] = 1; tetris_bag[2] = 2;
    tetris_bag[3] = 3; tetris_bag[4] = 4; tetris_bag[5] = 5;
    tetris_bag[6] = 6;

    // Fisher-Yates shuffle
    for (int i = BAG_SIZE - 1; i > 0; i--) {
        uint32_t j = get_random_range(i + 1);
        uint8_t temp = tetris_bag[i];
        tetris_bag[i] = tetris_bag[j];
        tetris_bag[j] = temp;
    }
    bag_index = 0;
}

uint8_t get_next_piece(void) {
    if (bag_index < 0 || bag_index >= BAG_SIZE) {
        shuffle_bag();
    }
    return tetris_bag[bag_index++];
}

static void Tetris_NewPiece(void)
{
	uint8_t pieceIndex = get_next_piece();
	//bag_index++;

    memcpy(currentPiece.shape,
           tetrominoes[pieceIndex],
           sizeof(currentPiece.shape));


    currentPiece.x = BOARD_WIDTH/2;
    currentPiece.y = 0;

    if (Tetris_Collision(&currentPiece))
    {
        gameOver = 1;
    }
}

/* =========================================================
 * LOCK CURRENT PIECE INTO BOARD
 * ========================================================= */

static void Tetris_LockPiece(void)
{
    for (int y = 0; y < 4; y++)
    {
        for (int x = 0; x < 4; x++)
        {
            if (currentPiece.shape[y][x])
            {
                int boardX = currentPiece.x + x;
                int boardY = currentPiece.y + y;

                if (boardY >= 0 &&
                    boardY < BOARD_HEIGHT &&
                    boardX >= 0 &&
                    boardX < BOARD_WIDTH)
                {
                    board[boardY][boardX] = 1;
                }
            }
        }
    }
}



/* =========================================================
 * INITIALIZATION
 * ========================================================= */

void Tetris_Init(void)
{
    memset(board, 0, sizeof(board));

    gameOver = 0;

    lastFallTime = HAL_GetTick();

    Tetris_NewPiece();
}

void Tetris_Main_Menu(void)
{
	while(1)
	{
		if (button1State == 1)
				{
					init_lfsr_seed(&htim2);
					button1State = 0;
					break;
				}
	}

}

/* =========================================================
 * MOVE LEFT
 * ========================================================= */

void Tetris_MoveLeft(void)
{
    Tetromino test = currentPiece;

    test.x--;

    if (!Tetris_Collision(&test))
    {
        currentPiece.x--;
    }
}


/* =========================================================
 * MOVE RIGHT
 * ========================================================= */

void Tetris_MoveRight(void)
{
    Tetromino test = currentPiece;

    test.x++;

    if (!Tetris_Collision(&test))
    {
        currentPiece.x++;
    }
}


/* =========================================================
 * MOVE DOWN
 * ========================================================= */

void Tetris_MoveDown(void)
{
    Tetromino test = currentPiece;

    test.y++;

    if (!Tetris_Collision(&test))
    {
        currentPiece.y++;
    }
    else
    {
        /* Piece cannot move further */

        Tetris_LockPiece();

        Tetris_CheckLines();

        Tetris_NewPiece();
    }
}


/* =========================================================
 * ROTATE
 * ========================================================= */

void Tetris_Rotate(void)
{
    Tetromino rotated = currentPiece;

    /* Rotate 4x4 matrix clockwise */

    for (int y = 0; y < 4; y++)
    {
        for (int x = 0; x < 4; x++)
        {
            rotated.shape[x][3 - y] =
                    currentPiece.shape[y][x];
        }
    }

    if (!Tetris_Collision(&rotated))
    {
        currentPiece = rotated;
    }
}


/* =========================================================
 * CHECK AND REMOVE COMPLETED LINES
 * ========================================================= */

void Tetris_CheckLines(void)
{
    for (int y = BOARD_HEIGHT - 1; y >= 0; y--)
    {
        uint8_t full = 1;

        for (int x = 0; x < BOARD_WIDTH; x++)
        {
            if (board[y][x] == 0)
            {
                full = 0;
                break;
            }
        }

        if (full)
        {
            /* Move everything above down */

            for (int row = y; row > 0; row--)
            {
                for (int x = 0; x < BOARD_WIDTH; x++)
                {
                    board[row][x] = board[row - 1][x];
                }
            }

            /* Clear top row */

            for (int x = 0; x < BOARD_WIDTH; x++)
            {
                board[0][x] = 0;
            }

            /* Check this same row again */
            y++;

            /* Add to the score */
            score++;
        }
    }
}


/* =========================================================
 * HANDLE INPUT
 * ========================================================= */

void Tetris_HandleInput(void)
{
	if (button1State == 1)
	    {
	        Tetris_MoveLeft();
	        button1State = 0;
	    }

	if (button2State == 1)
	    {
	        Tetris_MoveRight();
	        button2State = 0;
	    }
	if (button3State == 1)
		{
			Tetris_Rotate();
			button3State = 0;
		}
	if (button4State == 1)
		{
			Tetris_MoveDown();
			button4State = 0;
		}
}


/* =========================================================
 * UPDATE GAME
 * ========================================================= */

void Tetris_Update(void)
{
    if (gameOver)
        return;


    uint32_t now = HAL_GetTick();


    if ((now - lastFallTime) >= FALL_DELAY)
    {
        lastFallTime = now;
        Tetris_MoveDown();
    }
}


/* =========================================================
 * DRAW GAME
 * ========================================================= */

void Tetris_Draw(void)
{
    SSD1306_Fill(SSD1306_COLOR_BLACK);

    /* ---------------------------------------------
     * Draw board
     * --------------------------------------------- */

    //Draw walls and floor
    SSD1306_Draw_Filled_Rectangle(
                        BOARD_X - BOARD_WALL,
                        0,
                        BOARD_WALL,
                        BOARD_HEIGHT*BLOCK_SIZE + BOARD_WALL,
                        SSD1306_COLOR_WHITE
                    );
    SSD1306_Draw_Filled_Rectangle(
						BOARD_X + BOARD_WIDTH*BLOCK_SIZE,
						0,
						BOARD_WALL,
						BOARD_HEIGHT*BLOCK_SIZE + BOARD_WALL,
						SSD1306_COLOR_WHITE
					);
    SSD1306_Draw_Filled_Rectangle(
						BOARD_X - BOARD_WALL,
						BOARD_HEIGHT*BLOCK_SIZE + BOARD_WALL,
						BOARD_WIDTH*BLOCK_SIZE + 2*BOARD_WALL,
						BOARD_WALL,
						SSD1306_COLOR_WHITE
					);

    //Draw the rest
    for (int y = 0; y < BOARD_HEIGHT; y++)
    {
        for (int x = 0; x < BOARD_WIDTH; x++)
        {
            if (board[y][x])
            {
            	SSD1306_Draw_Filled_Rectangle(
                    BOARD_X + x * BLOCK_SIZE,
                    BOARD_Y + y * BLOCK_SIZE,
                    BLOCK_SIZE,
                    BLOCK_SIZE,
                    SSD1306_COLOR_WHITE
                );
            }
        }
    }


    /* ---------------------------------------------
     * Draw current falling piece
     * --------------------------------------------- */

    if (!gameOver)
    {
        for (int y = 0; y < 4; y++)
        {
            for (int x = 0; x < 4; x++)
            {
                if (currentPiece.shape[y][x])
                {
					SSD1306_Draw_Filled_Rectangle(
                        BOARD_X +
                        (currentPiece.x + x) * BLOCK_SIZE,

                        BOARD_Y +
                        (currentPiece.y + y) * BLOCK_SIZE,

                        BLOCK_SIZE,
                        BLOCK_SIZE,

                        SSD1306_COLOR_WHITE
                    );
                }
            }
        }
    }
    /*else
    {
    	//debug screen
    	if (ind){
    		Tetris_DebugPrintBoard();
    		ind = 0;
    	}
    }*/


    /* ---------------------------------------------
     * Send framebuffer to OLED
     * --------------------------------------------- */


    SSD1306_Update_Screen(&hi2c2);
    //DISPLAY_Init(&hi2c2);

}
void Tetris_DebugPrintBoard(void)
{
    for (uint8_t y = 0; y < BOARD_HEIGHT; y++)
    {
        for (uint8_t x = 0; x < BOARD_WIDTH; x++)
        {
            if (board[y][x])
                printf("1");
            else
                printf("0");
        }

        //printf("\r\n");
    }
    fflush(stdout);
    //printf("\r\n");
}
