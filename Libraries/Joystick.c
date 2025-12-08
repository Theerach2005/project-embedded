#include "Joystick.h"

extern ADC_HandleTypeDef hadc1;
extern UART_HandleTypeDef huart2;

volatile uint32_t g_joystickAdcBuf[JOYSTICK_ADC_CHANNEL_COUNT];
JoystickCal_t g_joystickCal;

static void wait_button_press_release(void) {
	while (HAL_GPIO_ReadPin(JOYSTICK_CAL_BUTTON_PORT, JOYSTICK_CAL_BUTTON_PIN) == GPIO_PIN_SET) {
	}
	char t[] = "Wait press\n\r";
	HAL_UART_Transmit(&huart2, t, strlen(t), HAL_MAX_DELAY);
	HAL_Delay(50);
	while (HAL_GPIO_ReadPin(JOYSTICK_CAL_BUTTON_PORT, JOYSTICK_CAL_BUTTON_PIN)
			== GPIO_PIN_RESET) {
		char t[] = "Wait de-press\n\r";
		HAL_UART_Transmit(&huart2, t, strlen(t), HAL_MAX_DELAY);
	}
	HAL_Delay(50);
}

static uint16_t adc_read_avg(uint8_t idx, uint16_t samples) {
	uint32_t sum = 0;
	for (uint16_t i = 0; i < samples; i++) {
		sum += g_joystickAdcBuf[idx];
		HAL_Delay(2);
	}
	return (uint16_t) (sum / samples);
}

static float map_axis(uint16_t raw, uint16_t min, uint16_t center, uint16_t max) {
	float value;

	if (raw >= center) {
		if (max == center)
			return 0.0f;
		value = (float) (raw - center) / (float) (max - center);
	} else {
		if (center == min)
			return 0.0f;
		value = (float) (raw - center) / (float) (center - min);
	}

	if (value > 1.0f)
		value = 1.0f;
	if (value < -1.0f)
		value = -1.0f;

	if (value > -DEADZONE && value < DEADZONE) {
		value = 0.0f;
	}

	return value;
}

static void Joystick_SetDefaultCalibration(void) {
	g_joystickCal.x_center = 2048;
	g_joystickCal.x_min = 0;
	g_joystickCal.x_max = 4095;

	g_joystickCal.y_center = 2048;
	g_joystickCal.y_min = 0;
	g_joystickCal.y_max = 4095;
}

void Joystick_Init(void) {
	HAL_ADC_Start_DMA(&hadc1, (uint32_t*) g_joystickAdcBuf,
	JOYSTICK_ADC_CHANNEL_COUNT);
	Joystick_SetDefaultCalibration();
}

void Joystick_CalibrateBlocking(void) {
	// Step 1: center (button press)
//	wait_button_press_release();
	g_joystickCal.x_center = adc_read_avg(0, 100);
	g_joystickCal.y_center = adc_read_avg(1, 100);
	char s[100];
	snprintf(s, sizeof(s), "cen_x:%d cen_y:%d\n\r", g_joystickCal.x_center, g_joystickCal.y_center);
	HAL_UART_Transmit(&huart2, s, strlen(s), HAL_MAX_DELAY);

//	 Step 2: X range (move full left/right, press button)
    g_joystickCal.x_min = 4095;
    g_joystickCal.x_max = 0;
    char t1[] = "ReadX\r\n";
    HAL_UART_Transmit(&huart2, t1, strlen(t1), HAL_MAX_DELAY);
    while (1) {
        uint16_t x = g_joystickAdcBuf[0];
        if (x < g_joystickCal.x_min) g_joystickCal.x_min = x;
        if (x > g_joystickCal.x_max) g_joystickCal.x_max = x;
        char v[100];
        ReadJoy();
        snprintf(v, sizeof(v), "minx:%d maxx:%d\r\n",g_joystickCal.x_min,g_joystickCal.x_max);
        HAL_UART_Transmit(&huart2, v, strlen(v), HAL_MAX_DELAY);
        if (HAL_GPIO_ReadPin(JOYSTICK_CAL_BUTTON_PORT, JOYSTICK_CAL_BUTTON_PIN) == GPIO_PIN_RESET) {
            wait_button_press_release();
            break;
        }
        HAL_Delay(500);
    }

// Step 3: Y range (move full up/down, press button)
    g_joystickCal.y_min = 4095;
    g_joystickCal.y_max = 0;
    char t[] = "ReadY\n\r";
    HAL_UART_Transmit(&huart2, t, strlen(t), HAL_MAX_DELAY);
    while (1) {
        uint16_t y = g_joystickAdcBuf[1];
        if (y < g_joystickCal.y_min) g_joystickCal.y_min = y;
        if (y > g_joystickCal.y_max) g_joystickCal.y_max = y;

        char v[100];
        ReadJoy();
        snprintf(v, sizeof(v), "miny:%d maxy:%d\n\r", g_joystickCal.y_min,g_joystickCal.y_max);
        HAL_UART_Transmit(&huart2, v, strlen(v), HAL_MAX_DELAY);

        if (HAL_GPIO_ReadPin(JOYSTICK_CAL_BUTTON_PORT, JOYSTICK_CAL_BUTTON_PIN) == GPIO_PIN_RESET) {
            wait_button_press_release();
            break;
        }
        HAL_Delay(500);
    }

	if (g_joystickCal.x_min > g_joystickCal.x_center)
		g_joystickCal.x_min = g_joystickCal.x_center - 1;
	if (g_joystickCal.x_max < g_joystickCal.x_center)
		g_joystickCal.x_max = g_joystickCal.x_center + 1;

	if (g_joystickCal.y_min > g_joystickCal.y_center)
		g_joystickCal.y_min = g_joystickCal.y_center - 1;
	if (g_joystickCal.y_max < g_joystickCal.y_center)
		g_joystickCal.y_max = g_joystickCal.y_center + 1;
}

void Joystick_Get(float *x_norm, float *y_norm) {
	uint16_t raw_x = g_joystickAdcBuf[0];
	uint16_t raw_y = g_joystickAdcBuf[1];

	char buf[100];
	sprintf(buf,"raw_x:%d, raw_y:%d\n\r",raw_x,raw_y);
	print(buf);

//    if (x_norm)
	*x_norm = map_axis(raw_x, g_joystickCal.x_min, g_joystickCal.x_center,
			g_joystickCal.x_max);

//    if (y_norm)
	*y_norm = map_axis(raw_y, g_joystickCal.y_min, g_joystickCal.y_center,
			g_joystickCal.y_max);
}

void ReadJoy(void) {
	uint16_t raw_x = g_joystickAdcBuf[0];
	uint16_t raw_y = g_joystickAdcBuf[1];

	char buf[100];
	sprintf(buf,"raw_x:%d, raw_y:%d\n\r",raw_x,raw_y);
	print(buf);
}


Direction_t Joystick_read_direction(float x, float y)
{
    float mag = sqrtf(x*x + y*y);
    if (mag < DEADZONE)
        return NONE;

    float angle = atan2f(-y, x);               // radians
    float deg = angle * 180.0f / 3.14159265f;  // degrees

    if (deg < 0) deg += 360.0f; // wrap to [0,360)

    // 4 directions: each sector = 90° (center ±45°)
    if (deg >= 315.0f || deg < 45.0f)   return RIGHT;
    if (deg >= 45.0f  && deg < 135.0f)  return UP;
    if (deg >= 135.0f && deg < 225.0f)  return LEFT;
    if (deg >= 225.0f && deg < 315.0f)  return DOWN;

    return NONE;
}



