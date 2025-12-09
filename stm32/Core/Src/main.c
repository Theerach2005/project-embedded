/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "Joystick.h"
#include "Buzzer.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef enum GameStateEnum{
    GAME_IDLE = 0x20,
    GAME_RUNNING = 0x21,
    GAME_OVER = 0x22
} GameState_t;

typedef enum GameButtonEnum{
	BUTTON_1 = 0x80,
	BUTTON_2 = 0x90
} Button_t;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */


#define NEW_GAME_BUTTON_Pin GPIO_PIN_0
#define JOY_CALIBRATE_BUTTON_Pin GPIO_PIN_13
#define GAME_BUTTON_1_Pin GPIO_PIN_4
#define GAME_BUTTON_2_Pin GPIO_PIN_5

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;
DMA_HandleTypeDef hdma_adc1;

TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;

UART_HandleTypeDef huart2;
UART_HandleTypeDef huart6;

/* USER CODE BEGIN PV */

int melody[] = {
147, 220, 294, 370, 110, 165, 220, 278, 123, 185, 247, 294, 92, 139, 185, 220, 98, 147, 196, 247, 73, 110, 147, 185, 98, 147, 196, 247, 110, 165, 220, 278, 740, 659, 147, 220, 294, 370, 110, 165, 220, 278, 587, 554, 123, 185, 247, 294, 92, 139, 185, 220, 494, 440, 98, 147, 196, 247, 73, 110, 147, 185, 494, 554, 98, 147, 196, 247, 110, 165, 220, 278, 440, 587, 740, 440, 554, 659, 147, 220, 294, 370, 110, 165, 220, 278, 370, 494, 587, 370, 440, 554, 123, 185, 247, 294, 92, 139, 185, 220, 294, 392, 494, 294, 370, 440, 98, 147, 196, 247, 73, 110, 147, 185, 294, 392, 494, 330, 440, 554, 98, 147, 196, 247, 110, 165, 220, 278, 587, 740, 880, 784, 147, 220, 294, 370, 110, 165, 220, 278, 740, 587, 740, 659, 123, 185, 247, 294, 92, 139, 185, 220, 587, 494, 587, 740, 98, 147, 196, 247, 73, 110, 147, 185, 784, 988, 880, 784, 98, 147, 196, 247, 110, 165, 220, 278, 587, 740, 740, 659, 880, 784, 147, 220, 294, 370, 110, 165, 220, 278, 587, 740, 587, 554, 740, 659, 123, 185, 247, 294, 92, 139, 185, 220, 494, 587, 494, 440, 587, 740, 98, 147, 196, 247, 73, 110, 147, 185, 494, 784, 988, 554, 880, 784, 98, 147, 196, 247, 110, 165, 220, 278, 587, 554, 587, 440, 440, 554, 147, 220, 294, 370, 110, 165, 220, 278, 587, 740, 880, 988, 123, 185, 247, 294, 92, 139, 185, 220, 784, 740, 659, 784, 740, 659, 587, 554, 98, 147, 196, 247, 73, 110, 147, 185, 494, 440, 587, 587, 554, 98, 147, 196, 247, 110, 165, 220, 278, 587, 554, 587, 440, 440, 554, 147, 220, 294, 370, 110, 165, 220, 278, 587, 740, 880, 988, 123, 185, 247, 294, 92, 139, 185, 220, 784, 740, 659, 784, 740, 659, 587, 554, 98, 147, 196, 247, 73, 110, 147, 185, 494, 440, 587, 587, 554, 98, 147, 196, 247, 110, 165, 220, 278, 587, 554, 587, 440, 554, 440, 659, 740, 147, 220, 294, 370, 110, 165, 220, 278, 587, 587, 554, 494, 554, 740, 880, 988, 123, 185, 247, 294, 92, 139, 185, 220, 784, 740, 659, 784, 740, 659, 587, 554, 98, 147, 196, 247, 73, 110, 147, 185, 494, 440, 392, 370, 330, 392, 370, 330, 98, 147, 196, 247, 110, 165, 220, 278, 587, 659, 740, 784, 880, 659, 880, 784, 147, 220, 294, 370, 110, 165, 220, 278, 740, 988, 880, 784, 880, 784, 740, 659, 123, 185, 247, 294, 92, 139, 185, 220, 587, 494, 494, 554, 587, 740, 659, 587, 740, 98, 147, 196, 247, 73, 110, 147, 185, 784, 587, 554, 494, 554, 587, 554, 98, 147, 196, 247, 110, 165, 220, 278, 880, 740, 784, 880, 740, 784, 880, 440, 494, 554, 587, 659, 740, 784, 147, 220, 294, 370, 110, 165, 220, 278, 740, 587, 659, 740, 370, 392, 440, 494, 440, 392, 440, 370, 392, 440, 123, 185, 247, 294, 92, 139, 185, 220, 392, 494, 440, 392, 370, 330, 370, 330, 294, 330, 370, 392, 440, 494, 98, 147, 196, 247, 73, 110, 147, 185, 392, 494, 440, 494, 554, 587, 440, 494, 554, 587, 659, 740, 784, 880, 98, 147, 196, 247, 110, 165, 220, 278, 880, 740, 784, 880, 740, 784, 880, 440, 494, 554, 587, 659, 740, 784, 147, 220, 294, 370, 110, 165, 220, 278, 740, 587, 659, 740, 370, 392, 440, 494, 440, 392, 440, 370, 392, 440, 123, 185, 247, 294, 92, 139, 185, 220, 392, 494, 440, 392, 370, 330, 370, 330, 294, 330, 370, 392, 440, 494, 98, 147, 196, 247, 73, 110, 147, 185, 392, 494, 440, 494, 554, 587, 440, 494, 554, 587, 659, 740, 784, 880, 98, 147, 196, 247, 110, 165, 220, 278, 740, 587, 659, 740, 659, 587, 659, 554, 587, 659, 740, 659, 587, 554, 147, 220, 294, 370, 110, 165, 220, 278, 587, 494, 554, 587, 587, 659, 740, 784, 740, 659, 740, 587, 554, 587, 123, 185, 247, 294, 92, 139, 185, 220, 494, 587, 554, 494, 440, 392, 440, 392, 370, 392, 440, 494, 554, 587, 98, 147, 196, 247, 73, 110, 147, 185, 494, 587, 554, 587, 554, 494, 554, 440, 494, 554, 587, 659, 740, 784, 98, 147, 196, 247, 110, 165, 220, 278, 740, 587, 659, 740, 659, 587, 659, 554, 587, 659, 740, 659, 587, 554, 147, 220, 294, 370, 110, 165, 220, 278, 587, 494, 554, 587, 587, 659, 740, 784, 740, 659, 740, 587, 554, 587, 123, 185, 247, 294, 92, 139, 185, 220, 494, 587, 554, 494, 440, 392, 440, 392, 370, 392, 440, 494, 554, 587, 98, 147, 196, 247, 73, 110, 147, 185, 494, 587, 554, 587, 554, 494, 554, 440, 494, 554, 587, 659, 740, 784, 98, 147, 196, 247, 110, 165, 220, 278, 880, 587, 740, 880, 880, 988, 880, 784, 147, 220, 294, 370, 110, 165, 220, 278, 740, 740, 740, 784, 740, 659, 123, 185, 247, 294, 92, 139, 185, 220, 587, 523, 494, 523, 587, 440, 440, 98, 147, 196, 247, 73, 110, 147, 185, 587, 523, 494, 523, 587, 554, 98, 147, 196, 247, 110, 165, 220, 278, 880, 880, 880, 988, 880, 784, 147, 220, 294, 370, 110, 165, 220, 278, 740, 740, 740, 784, 740, 659, 123, 185, 247, 294, 92, 139, 185, 220, 587, 523, 494, 523, 587, 440, 440, 98, 147, 196, 247, 73, 110, 147, 185, 587, 523, 494, 523, 587, 554, 98, 147, 196, 247, 110, 165, 220, 278, 740, 370, 392, 370, 330, 659, 740, 659, 147, 220, 294, 370, 110, 165, 220, 278, 587, 370, 587, 494, 440, 440, 392, 440, 123, 185, 247, 294, 92, 139, 185, 220, 494, 494, 554, 494, 440, 440, 392, 440, 98, 147, 196, 247, 73, 110, 147, 185, 494, 494, 440, 494, 554, 554, 494, 554, 98, 147, 196, 247, 110, 165, 220, 278, 587, 587, 659, 587, 554, 554, 587, 554, 147, 220, 294, 370, 110, 165, 220, 278, 494, 494, 440, 494, 554, 554, 740, 659, 123, 185, 247, 294, 92, 139, 185, 220, 587, 587, 659, 784, 740, 370, 440, 740, 98, 147, 196, 247, 73, 110, 147, 185, 740, 784, 740, 784, 659, 440, 659, 554, 98, 147, 196, 247, 110, 165, 220, 278, 587, 740, 587, 659, 740, 587, 554, 659, 554, 587, 659, 554, 147, 220, 294, 370, 110, 165, 220, 278, 494, 587, 494, 554, 587, 494, 440, 554, 880, 784, 740, 659, 123, 185, 247, 294, 92, 139, 185, 220, 494, 587, 784, 740, 659, 784, 440, 740, 587, 659, 740, 880, 98, 147, 196, 247, 73, 110, 147, 185, 784, 988, 988, 880, 784, 988, 554, 659, 880, 784, 740, 659, 98, 147, 196, 247, 110, 165, 220, 278, 587, 740, 587, 554, 587, 440, 440, 659, 440, 494, 554, 440, 147, 220, 294, 370, 110, 165, 220, 278, 494, 587, 587, 659, 587, 740, 587, 554, 740, 740, 659, 587, 554, 123, 185, 247, 294, 92, 139, 185, 220, 392, 494, 494, 440, 494, 554, 440, 587, 740, 659, 587, 740, 98, 147, 196, 247, 73, 110, 147, 185, 494, 784, 587, 554, 494, 554, 440, 587, 659, 554, 98, 147, 196, 247, 110, 165, 220, 278, 440, 587, 740, 740, 740, 784, 740, 659, 147, 220, 294, 370, 110, 165, 220, 278, 587, 587, 587, 659, 587, 554, 123, 185, 247, 294, 92, 139, 185, 220, 392, 494, 587, 440, 98, 147, 196, 247, 73, 110, 147, 185, 392, 494, 440, 554, 98, 147, 196, 247, 110, 165, 220, 278, 587, 740, 880, 740, 880, 740, 880, 784, 988, 740, 880, 659, 784, 147, 220, 294, 370, 110, 165, 220, 278, 587, 740, 587, 740, 587, 740, 659, 784, 587, 740, 554, 659, 123, 185, 247, 294, 92, 139, 185, 220, 494, 587, 523, 494, 523, 587, 440, 370, 98, 147, 196, 247, 73, 110, 147, 185, 587, 523, 494, 523, 440, 554, 554, 98, 147, 196, 247, 110, 165, 220, 278, 370, 440, 587, 440, 440, 494, 440, 392, 147, 220, 294, 370, 110, 165, 220, 278, 370, 370, 370, 392, 370, 330, 123, 185, 247, 294, 92, 139, 185, 220, 587, 523, 494, 523, 587, 440, 440, 98, 147, 196, 247, 73, 110, 147, 185, 587, 523, 494, 523, 587, 554, 98, 147, 196, 247, 110, 165, 220, 278, 440, 440, 440, 494, 440, 392, 147, 220, 294, 370, 110, 165, 220, 278, 554, 587, 494, 587, 440, 523, 123, 185, 247, 294, 92, 139, 185, 220, 440, 494, 392, 440, 587, 440, 98, 147, 196, 247, 73, 110, 147, 185, 392, 494, 330, 440, 98, 147, 196, 247, 110, 165, 220, 278, 440, 440, 587, 740, 659, 147, 220, 294, 370, 110, 165, 220, 278, 370, 554, 587, 587, 988, 1175, 659, 880, 1047, 123, 185, 247, 294, 92, 139, 185, 220, 587, 880, 988, 784, 740, 880, 1175, 587, 740, 880, 98, 147, 196, 247, 73, 110, 147, 185, 587, 784, 988, 659, 554, 659, 880, 784, 98, 147, 196, 247, 110, 165, 220, 278, 440, 740, 880, 440, 392, 147, 220, 294, 370, 110, 165, 220, 278, 370, 740, 740, 659, 123, 185, 247, 294, 92, 139, 185, 220, 587, 784, 740, 880, 294, 587, 98, 147, 196, 247, 73, 110, 147, 185, 494, 587, 659, 330, 659, 440, 554, 98, 147, 196, 247, 110, 165, 220, 278, 440, 554, 294, 370, 440, 587, 278, 73, 110, 185, 0
};

int noteDurations[] = {
250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 1000, 1000, 250, 250, 250, 250, 250, 250, 250, 250, 1000, 1000, 250, 250, 250, 250, 250, 250, 250, 250, 1000, 1000, 250, 250, 250, 250, 250, 250, 250, 250, 1000, 1000, 250, 250, 250, 250, 250, 250, 250, 250, 1000, 1000, 1000, 1000, 1000, 1000, 250, 250, 250, 250, 250, 250, 250, 250, 1000, 1000, 1000, 1000, 1000, 1000, 250, 250, 250, 250, 250, 250, 250, 250, 1000, 1000, 1000, 1000, 1000, 1000, 250, 250, 250, 250, 250, 250, 250, 250, 1000, 1000, 1000, 1000, 1000, 1000, 250, 250, 250, 250, 250, 250, 250, 250, 500, 500, 500, 500, 250, 250, 250, 250, 250, 250, 250, 250, 500, 500, 750, 250, 250, 250, 250, 250, 250, 250, 250, 250, 500, 500, 500, 500, 250, 250, 250, 250, 250, 250, 250, 250, 500, 500, 750, 250, 250, 250, 250, 250, 250, 250, 250, 250, 500, 500, 500, 500, 500, 500, 250, 250, 250, 250, 250, 250, 250, 250, 500, 500, 500, 750, 750, 250, 250, 250, 250, 250, 250, 250, 250, 250, 500, 500, 500, 500, 500, 500, 250, 250, 250, 250, 250, 250, 250, 250, 500, 500, 500, 750, 750, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 500, 500, 250, 250, 250, 250, 250, 250, 250, 250, 500, 500, 750, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 500, 750, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 500, 500, 250, 250, 250, 250, 250, 250, 250, 250, 500, 500, 750, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 500, 750, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 125, 125, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 125, 125, 250, 250, 750, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 125, 125, 250, 125, 125, 125, 125, 125, 125, 125, 125, 125, 125, 250, 250, 250, 250, 250, 250, 250, 250, 250, 125, 125, 250, 125, 125, 125, 125, 125, 125, 125, 125, 125, 125, 250, 250, 250, 250, 250, 250, 250, 250, 250, 125, 125, 250, 125, 125, 125, 125, 125, 125, 125, 125, 125, 125, 250, 250, 250, 250, 250, 250, 250, 250, 250, 125, 125, 250, 125, 125, 125, 125, 125, 125, 125, 125, 125, 125, 250, 250, 250, 250, 250, 250, 250, 250, 250, 125, 125, 250, 125, 125, 125, 125, 125, 125, 125, 125, 125, 125, 250, 250, 250, 250, 250, 250, 250, 250, 250, 125, 125, 250, 125, 125, 125, 125, 125, 125, 125, 125, 125, 125, 250, 250, 250, 250, 250, 250, 250, 250, 250, 125, 125, 250, 125, 125, 125, 125, 125, 125, 125, 125, 125, 125, 250, 250, 250, 250, 250, 250, 250, 250, 250, 125, 125, 250, 125, 125, 125, 125, 125, 125, 125, 125, 125, 125, 250, 250, 250, 250, 250, 250, 250, 250, 250, 125, 125, 250, 125, 125, 125, 125, 125, 125, 125, 125, 125, 125, 250, 250, 250, 250, 250, 250, 250, 250, 250, 125, 125, 250, 125, 125, 125, 125, 125, 125, 125, 125, 125, 125, 250, 250, 250, 250, 250, 250, 250, 250, 250, 125, 125, 250, 125, 125, 125, 125, 125, 125, 125, 125, 125, 125, 250, 250, 250, 250, 250, 250, 250, 250, 250, 125, 125, 250, 125, 125, 125, 125, 125, 125, 125, 125, 125, 125, 250, 250, 250, 250, 250, 250, 250, 250, 250, 125, 125, 250, 125, 125, 125, 125, 125, 125, 125, 125, 125, 125, 250, 250, 250, 250, 250, 250, 250, 250, 250, 125, 125, 250, 125, 125, 125, 125, 125, 125, 125, 125, 125, 125, 250, 250, 250, 250, 250, 250, 250, 250, 250, 125, 125, 250, 125, 125, 125, 125, 125, 125, 125, 125, 125, 125, 250, 250, 250, 250, 250, 250, 250, 250, 250, 125, 125, 250, 125, 125, 125, 125, 125, 125, 125, 125, 125, 125, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 750, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 500, 375, 125, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 500, 500, 250, 250, 250, 250, 250, 250, 250, 250, 750, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 750, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 500, 375, 125, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 750, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 125, 125, 250, 250, 250, 250, 125, 125, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 125, 125, 250, 250, 250, 250, 125, 125, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 125, 125, 250, 250, 250, 250, 125, 125, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 125, 125, 250, 250, 250, 250, 125, 125, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 125, 125, 250, 250, 250, 250, 125, 125, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 125, 125, 250, 250, 250, 250, 250, 125, 125, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 125, 125, 250, 250, 250, 250, 125, 125, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 125, 125, 250, 250, 500, 500, 500, 500, 250, 250, 250, 250, 250, 250, 250, 250, 750, 750, 750, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 750, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 1000, 1000, 500, 500, 250, 250, 250, 250, 250, 250, 250, 250, 1000, 1000, 750, 250, 250, 250, 250, 250, 250, 250, 250, 250, 750, 750, 750, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 750, 750, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 500, 375, 125, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 750, 750, 250, 250, 250, 250, 250, 250, 250, 250, 250, 750, 750, 750, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 750, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 500, 375, 125, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 500, 500, 250, 250, 250, 250, 250, 250, 250, 250, 750, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 500, 500, 1000, 1000, 500, 500, 250, 250, 250, 250, 250, 250, 250, 250, 500, 500, 500, 500, 500, 500, 250, 250, 250, 250, 250, 250, 250, 250, 1000, 1000, 1000, 1000, 250, 250, 250, 250, 250, 250, 250, 250, 500, 1000, 1000, 1000, 500, 250, 250, 250, 250, 250, 250, 250, 250, 500, 500, 500, 1000, 1000, 1000, 500, 500, 500, 250, 250, 250, 250, 250, 250, 250, 250, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 250, 250, 250, 250, 250, 250, 250, 250, 750, 750, 750, 250, 500, 500, 500, 500, 250, 250, 250, 250, 250, 250, 250, 250, 1000, 1000, 1000, 750, 250, 250, 250, 250, 250, 250, 250, 250, 250, 1000, 1000, 750, 250, 250, 250, 250, 250, 250, 250, 250, 250, 1000, 1000, 500, 500, 500, 500, 250, 250, 250, 250, 250, 250, 250, 250, 750, 750, 250, 750, 750, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 0
};

uint32_t mSize = sizeof(melody)/sizeof(melody[0]);
uint32_t i=0;

uint32_t lastPress = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_ADC1_Init(void);
static void MX_TIM2_Init(void);
static void MX_USART6_UART_Init(void);
static void MX_TIM3_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void print(char text[]) {
	char buf[100];
	sprintf(buf, text);
	HAL_UART_Transmit(&huart2, buf, strlen(buf), HAL_MAX_DELAY);
}

void uart_send_byte(uint8_t b) {
    HAL_UART_Transmit(&huart6, &b, 1, HAL_MAX_DELAY);
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_USART2_UART_Init();
  MX_ADC1_Init();
  MX_TIM2_Init();
  MX_USART6_UART_Init();
  MX_TIM3_Init();
  /* USER CODE BEGIN 2 */
  Joystick_Init();
  Joystick_CalibrateBlocking();
  initBuzzer();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  print("start\n");
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 100;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.ScanConvMode = ENABLE;
  hadc1.Init.ContinuousConvMode = ENABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 2;
  hadc1.Init.DMAContinuousRequests = ENABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_0;
  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_480CYCLES;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_1;
  sConfig.Rank = 2;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 8399;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 199;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */

}

/**
  * @brief TIM3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 50;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 9999;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 100;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */
  HAL_TIM_MspPostInit(&htim3);

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief USART6 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART6_UART_Init(void)
{

  /* USER CODE BEGIN USART6_Init 0 */

  /* USER CODE END USART6_Init 0 */

  /* USER CODE BEGIN USART6_Init 1 */

  /* USER CODE END USART6_Init 1 */
  huart6.Instance = USART6;
  huart6.Init.BaudRate = 115200;
  huart6.Init.WordLength = UART_WORDLENGTH_8B;
  huart6.Init.StopBits = UART_STOPBITS_1;
  huart6.Init.Parity = UART_PARITY_NONE;
  huart6.Init.Mode = UART_MODE_TX_RX;
  huart6.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart6.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart6) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART6_Init 2 */

  /* USER CODE END USART6_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA2_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA2_Stream0_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Stream0_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream0_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10, GPIO_PIN_RESET);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : PC0 */
  GPIO_InitStruct.Pin = GPIO_PIN_0;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : LD2_Pin */
  GPIO_InitStruct.Pin = LD2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LD2_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : PB10 */
  GPIO_InitStruct.Pin = GPIO_PIN_10;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : PB4 PB5 */
  GPIO_InitStruct.Pin = GPIO_PIN_4|GPIO_PIN_5;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI0_IRQn, 2, 0);
  HAL_NVIC_EnableIRQ(EXTI0_IRQn);

  HAL_NVIC_SetPriority(EXTI4_IRQn, 1, 0);
  HAL_NVIC_EnableIRQ(EXTI4_IRQn);

  HAL_NVIC_SetPriority(EXTI9_5_IRQn, 2, 0);
  HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);

  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 2, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if (GPIO_Pin == GAME_BUTTON_1_Pin || GPIO_Pin == GAME_BUTTON_2_Pin) {
        uint32_t now = HAL_GetTick();

        // simple debounce: ignore presses within 200 ms
        if (now - lastPress < 200) return;
        lastPress = now;
        HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
    	float x,y;
    	Joystick_Get(&x, &y);
//		uart_send_byte((GPIO_Pin==GAME_BUTTON_1_Pin ? 0x80 : 0x90) | Joystick_GetDirection(x, y));
		Direction_t d = Joystick_read_direction(x, y);
		Button_t b = (GPIO_Pin == GAME_BUTTON_1_Pin) ? 0x80 : 0x90;
		char buff[100];
		sprintf(buff, "x:%f, y:%f, dir:%02x, encoded:%02x\n\r", x, y, (uint8_t)d, (uint8_t)(b | d));
		playNote();
		print(buff);
		uart_send_byte(b|d);
    }
}


/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
