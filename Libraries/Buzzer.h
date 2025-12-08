#ifndef BUZZER_H_
#define BUZZER_H_

#include "main.h"

extern TIM_HandleTypeDef htim3;

#define TIM_FREQ 100000000
#define VOL 50

extern int melody[];
extern int noteDurations[];
extern uint32_t mSize;
extern uint32_t i;

extern void initBuzzer(void);

extern int freqToPSC(int freq);

extern void noTone(void);

extern void playTone(int *tone, int *duration, int *pause, int size);

extern void playNote(void);



#endif /* BUZZER_H_ */
