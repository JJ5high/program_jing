/*
 * stm32_ow.h
 *
 *  Created on: 2013Äê7ÔÂ23ÈÕ
 *      Author: user
 */

#ifndef STM32_OW_H_
#define STM32_OW_H_

#include "stm32f4xx.h"

#define OW0_USART						USART1
#define OW0_USART_INIT_CLK				RCC_APB2PeriphClockCmd
#define OW0_USART_CLK					RCC_APB2Periph_USART1
#define OW0_USART_RESET_BAUD_RATE		9600
#define OW0_USART_IO_BAUD_RATE			115200

#define OW0_GPIO						GPIOB
#define OW0_GPIO_INIT_CLK				RCC_AHB1PeriphClockCmd
#define OW0_GPIO_CLK					RCC_AHB1Periph_GPIOB
#define OW0_GPIO_Pin					GPIO_Pin_6
#define OW0_GPIO_PinSource				GPIO_PinSource6
#define OW0_GPIO_AF					GPIO_AF_USART1



#endif /* STM32_OW_H_ */
