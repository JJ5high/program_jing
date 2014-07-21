#include "stm32f4xx.h"

#include "timer_delay.h"

#include "temp.h"

#include "DS1820.h"

#include "OneWire.h"

#include "stdio.h"

#define MaxDevices 5
uint64_t Address[MaxDevices];

void LED_Set(int led)
{
	GPIO_ResetBits(GPIOD, 0x0f << 12);
	led &= 0x0f;
	if (led != 0) {
		GPIO_SetBits(GPIOD, ((0x0f & led) << 12));
	}
}

int main()
{
	float temp=0;

	TIM_Delay_Init();

	DS1820_Init();

	while (1) {
	    if(OW_GetResetResult()!=OW_OK){
	         printf("NO Device or Bus Error\n");
		    }
		DS1820_Search(Address,MaxDevices);
		DS1820_TemperatureConvert(Address[0]);
		Delay_ms(500);
		DS1820_TemperatureGet(Address[0]);
		Delay_ms(500);
		temp=DS1820_TemperatureResult(Address[0]);
		printf("temp1£º%.2f\n",temp);
	}
}

void assert_failed(uint8_t* file, uint32_t line)
{
	printf("Wrong parameters value: file %s on line %ld\r\n", file, line);
	LED_Set(14);
	TIM_Delay_Init();
	while (1) {
	}
}
