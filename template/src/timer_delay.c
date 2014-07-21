//A timer is used to generate accurate time delay
//The timer is prescaled at 1MHz and the max value of counter is 10000, so the period is 10ms

#include "stm32f4xx.h"

#define TIM_DELAY						TIM12
#define TIM_DELAY_CLK					RCC_APB1Periph_TIM12
#define TIM_DELAY_INIT_CLK				RCC_APB1PeriphClockCmd
#define TIM_DELAY_SPEED				(SystemCoreClock/2)
#define TIM_DELAY_COUNTER_MAX_VALUE	10000

void TIM_Delay_Init(void)
{
	static int configrued = 0;
	if (!configrued) {
		TIM_DELAY_INIT_CLK(TIM_DELAY_CLK, ENABLE);
		TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
		TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
		TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
		TIM_TimeBaseInitStructure.TIM_Prescaler = TIM_DELAY_SPEED / 1000000 - 1;
		TIM_TimeBaseInitStructure.TIM_Period = TIM_DELAY_COUNTER_MAX_VALUE - 1;
		TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0;
		TIM_TimeBaseInit(TIM_DELAY, &TIM_TimeBaseInitStructure);

		TIM_Cmd(TIM_DELAY, ENABLE);
	}
}

void Delay_ms(int len)
{

	while(len > 0){
		int start_time = TIM_GetCounter(TIM_DELAY);
		int end_time = start_time+1000;
		if(end_time >= TIM_DELAY_COUNTER_MAX_VALUE)
			end_time -= TIM_DELAY_COUNTER_MAX_VALUE;
		int tmp = 1000000;//To prevent dead loop
		while(1){
			int t = TIM_GetCounter(TIM_DELAY);
			if(t >= start_time+1000)
				break;
			if(t < start_time && end_time > start_time)
				break;
			if(t < start_time && t >= end_time)
				break;
			tmp --;
			if(tmp == 0)
				break;
		}
		len--;
	}
}
