/*
 * Joystick.h
 *
 *  Created on: Nov 29, 2025
 *      Author: napad
 */

#ifndef JOYSTICK_H_
#define JOYSTICK_H_

#include <stdint.h>
#include "main.h"
#include "math.h"

// Size of ADC buffer: [0] = X, [1] = Y
#define JOYSTICK_ADC_CHANNEL_COUNT   2
#define JOYSTICK_CAL_BUTTON_PORT GPIOC
#define JOYSTICK_CAL_BUTTON_PIN GPIO_PIN_13

// Deadzone around center in normalized units (-1..+1)
#define DEADZONE        0.10f   // center deadzone (radial)
#define AXIS_STRONG     0.85f   // how far you must push to count as "full"
#define AXIS_TOLERANCE  0.20f   // allowable noise on the perpendicular axis


typedef struct
{
    uint16_t x_min;
    uint16_t x_center;
    uint16_t x_max;

    uint16_t y_min;
    uint16_t y_center;
    uint16_t y_max;
} JoystickCal_t;

typedef enum DirectionEnum{
	NONE = 0x00,
	UP = 0x01,
	DOWN = 0x02,
	LEFT = 0x03,
	RIGHT = 0x04,
	UP_LEFT = 0x05,
	UP_RIGHT = 0x06,
	DOWN_LEFT = 0x07,
	DOWN_RIGHT = 0x08
} Direction_t;

extern volatile uint32_t g_joystickAdcBuf[JOYSTICK_ADC_CHANNEL_COUNT];
extern JoystickCal_t     g_joystickCal;

void Joystick_Init(void);
void Joystick_CalibrateBlocking(void);
void Joystick_Get(float *x_norm, float *y_norm);
Direction_t Joystick_read_direction(float x, float y);





#endif /* JOYSTICK_H_ */
