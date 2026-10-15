/*
 * 7seg.c
 *
 *  Created on: Sep 9, 2026
 *      Author: djord
 */

#include "7seg.h"
#include <string.h>


/* =========================================================
 * Segment pins
 *
 * Common-anode display:
 * LOW  = segment ON
 * HIGH = segment OFF
 * ========================================================= */

#define SEG_A_PORT     GPIOA
#define SEG_A_PIN      GPIO_PIN_10

#define SEG_B_PORT     GPIOA
#define SEG_B_PIN      GPIO_PIN_9

#define SEG_C_PORT     GPIOA
#define SEG_C_PIN      GPIO_PIN_8

#define SEG_D_PORT     GPIOB
#define SEG_D_PIN      GPIO_PIN_10

#define SEG_E_PORT     GPIOB
#define SEG_E_PIN      GPIO_PIN_5

#define SEG_F_PORT     GPIOB
#define SEG_F_PIN      GPIO_PIN_4

#define SEG_G_PORT     GPIOB
#define SEG_G_PIN      GPIO_PIN_3


/* Digit select pins */
#define SEL1_PORT      GPIOB
#define SEL1_PIN       GPIO_PIN_6

#define SEL2_PORT      GPIOC
#define SEL2_PIN       GPIO_PIN_7


/* =========================================================
 * Segment patterns
 *
 * Bit order:
 * bit 0 = A
 * bit 1 = B
 * bit 2 = C
 * bit 3 = D
 * bit 4 = E
 * bit 5 = F
 * bit 6 = G
 *
 * 1 = segment should be ON
 * 0 = segment should be OFF
 * ========================================================= */

static const uint8_t digit_patterns[10] =
{
    0b00111111,  // 0
    0b00000110,  // 1
    0b01011011,  // 2
    0b01001111,  // 3
    0b01100110,  // 4
    0b01101101,  // 5
    0b01111101,  // 6
    0b00000111,  // 7
    0b01111111,  // 8
    0b01101111   // 9
};


/* =========================================================
 * Turn all segments OFF
 * ========================================================= */

static void Segments_Off(void)
{
    HAL_GPIO_WritePin(SEG_A_PORT, SEG_A_PIN, GPIO_PIN_SET);
    HAL_GPIO_WritePin(SEG_B_PORT, SEG_B_PIN, GPIO_PIN_SET);
    HAL_GPIO_WritePin(SEG_C_PORT, SEG_C_PIN, GPIO_PIN_SET);
    HAL_GPIO_WritePin(SEG_D_PORT, SEG_D_PIN, GPIO_PIN_SET);
    HAL_GPIO_WritePin(SEG_E_PORT, SEG_E_PIN, GPIO_PIN_SET);
    HAL_GPIO_WritePin(SEG_F_PORT, SEG_F_PIN, GPIO_PIN_SET);
    HAL_GPIO_WritePin(SEG_G_PORT, SEG_G_PIN, GPIO_PIN_SET);
}


/* =========================================================
 * Set segments according to pattern
 * ========================================================= */

static void SetSegments(uint8_t pattern)
{

	HAL_GPIO_WritePin(SEG_A_PORT, SEG_A_PIN,
                      (pattern & (1 << 0)) ? GPIO_PIN_RESET : GPIO_PIN_SET);

    HAL_GPIO_WritePin(SEG_B_PORT, SEG_B_PIN,
                      (pattern & (1 << 1)) ? GPIO_PIN_RESET : GPIO_PIN_SET);

    HAL_GPIO_WritePin(SEG_C_PORT, SEG_C_PIN,
                      (pattern & (1 << 2)) ? GPIO_PIN_RESET : GPIO_PIN_SET);

    HAL_GPIO_WritePin(SEG_D_PORT, SEG_D_PIN,
                      (pattern & (1 << 3)) ? GPIO_PIN_RESET : GPIO_PIN_SET);

    HAL_GPIO_WritePin(SEG_E_PORT, SEG_E_PIN,
                      (pattern & (1 << 4)) ? GPIO_PIN_RESET : GPIO_PIN_SET);

    HAL_GPIO_WritePin(SEG_F_PORT, SEG_F_PIN,
                      (pattern & (1 << 5)) ? GPIO_PIN_RESET : GPIO_PIN_SET);

    HAL_GPIO_WritePin(SEG_G_PORT, SEG_G_PIN,
                      (pattern & (1 << 6)) ? GPIO_PIN_RESET : GPIO_PIN_SET);
}


/* =========================================================
 * Initialization
 * ========================================================= */

void SevenSeg_Init(void)
{
    Segments_Off();

    /* Both digits OFF initially */
    HAL_GPIO_WritePin(SEL1_PORT, SEL1_PIN, GPIO_PIN_SET);
    HAL_GPIO_WritePin(SEL2_PORT, SEL2_PIN, GPIO_PIN_SET);
}


/* =========================================================
 * Display one digit
 *
 * digit = 1 or 2
 * value = 0...9
 * ========================================================= */

void SevenSeg_DisplayDigit(uint8_t digit, uint8_t value)
{
    if (digit < 0 || digit > 1 || value > 9)
        return;

    /* Turn both digits OFF first */
    HAL_GPIO_WritePin(SEL1_PORT, SEL1_PIN, GPIO_PIN_SET);
    HAL_GPIO_WritePin(SEL2_PORT, SEL2_PIN, GPIO_PIN_SET);

    /* Set the segment pattern */
    SetSegments(digit_patterns[value]);

    /* Turn selected digit ON */
    if (digit == 0)
    {
        HAL_GPIO_WritePin(SEL1_PORT, SEL1_PIN, GPIO_PIN_RESET);
    }
    else
    {
        HAL_GPIO_WritePin(SEL2_PORT, SEL2_PIN, GPIO_PIN_RESET);
    }
}


/* =========================================================
 * Display a two-digit number
 *
 * Example:
 * SevenSeg_DisplayNumber(42);
 *
 * Note: this function multiplexes the digits.
 * It should be called repeatedly from the main loop.
 * ========================================================= */

void SevenSeg_DisplayNumber(uint8_t number)
{
    static uint8_t current_digit = 0;

    if (number > 99)
        number = 99;

    uint8_t tens = number / 10;
    uint8_t ones = number % 10;

    if (current_digit == 0)
    {
        SevenSeg_DisplayDigit(0, tens);
        current_digit = 1;
    }
    else
    {
        SevenSeg_DisplayDigit(1, ones);
        current_digit = 0;
    }
}

/* =========================================================
 * Clear display
 * ========================================================= */

void SevenSeg_Clear(void)
{
    /* Turn both digits OFF */
    HAL_GPIO_WritePin(SEL1_PORT, SEL1_PIN, GPIO_PIN_SET);
    HAL_GPIO_WritePin(SEL2_PORT, SEL2_PIN, GPIO_PIN_SET);

    /* Turn all segments OFF */
    Segments_Off();
}
