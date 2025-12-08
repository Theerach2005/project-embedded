#include "Buzzer.h"

extern print(char text[]);

void initBuzzer(void) {
	HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
	htim3.Instance->CCR1 = VOL;
	noTone();
}

int freqToPSC(int freq) {
	if (freq==0) return 0;
	return (TIM_FREQ/(10000*freq));
}

void noTone(void) {
	htim3.Instance->PSC = 0;
}

void playTone(int *tone, int *duration, int *pause, int size) {
	for (int i=0;i<size;i++) {
		int pres = freqToPSC(tone[i]);
		int dur = duration[i];
		int pauseBetweenTones = 0;
		if (pause!=NULL) pauseBetweenTones = pause[i] - duration[i];
		htim3.Instance->PSC = pres;
		HAL_Delay(dur);
		noTone();
		HAL_Delay(pauseBetweenTones);
	}
}

void playNote(void) {
	htim3.Instance->PSC = freqToPSC(melody[i]);
	i = (i+1)%mSize;
	uint32_t now = HAL_GetTick();
	while (HAL_GetTick() - now < 50);
//	HAL_Delay(50);
	noTone();
}


