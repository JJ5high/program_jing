/**
 *******************************************************************************
 * @file    OneWire.h
 * @author  Vojtěch Vigner <vojtech.vigner@gmail.com>
 * @version V1.0.5
 * @date    12-February-2013
 * 
 * @brief   Provides 1-Wire bus support for STM32Fxxx devices.
 * 
 * @see     OneWire.c documentation 
 * 
 * @copyright The BSD 3-Clause License. 
 * 
 * @section License
 *          Copyright (c) 2013, Vojtěch Vigner <vojtech.vigner@gmail.com> 
 *           
 *          All rights reserved.
 * 
 *          Redistribution and use in source and binary forms, with or without 
 *          modification, are permitted provided that the following conditions
 *          are met:
 *              - Redistributions of source code must retain the above 
 *                copyright notice, this list of conditions and the following 
 *                disclaimer. 
 *              - Redistributions in binary form must reproduce the above 
 *                copyright notice, this list of conditions and the following
 *                disclaimer in the documentation and/or other materials
 *                provided with the distribution.
 *              - The name of the contributors can not be used to endorse or 
 *                promote products derived from this software without specific
 *                prior written permission.
 *
 *          THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *          "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *          LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *          FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE 
 *          COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *          INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *          BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *          LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER 
 *          CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT 
 *          LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN 
 *          ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
 *          POSSIBILITY OF SUCH DAMAGE.
 *******************************************************************************
 */

#ifndef ONEWIRE_H
#define ONEWIRE_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "stdint.h"

    /**************************************************************************/
    /* Hardware specific configuration */
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_usart.h"
#include "stm32f4xx_rcc.h"

    /* Enables parasite powered device support */
#define OW_USE_PARASITE_POWER   1

    /* Enables single pin communication */
#define OW_USE_SINGLE_PIN       1

/* Bus specific commands */
#define OW_ROM_READ         0x33
#define OW_ROM_MATCH        0x55
#define OW_ROM_SKIP         0xCC
#define OW_ROM_SEARCH       0xF0
#define OW_ALARM_SEARCH     0xEC

/* Private defines */
#define OW_R                0xF0
#define OW_0                0x00
#define OW_1                0xFF


#define OW_OP_READ  1
#define OW_OP_WRITE 2
#define OW_OP_RESET 3
#define OW_OP_FREE  4

#define OW_IRQn    USART3_IRQn
#define OW_PREPRIO 0
#define OW_SUBPRIO 0



    /* Has to be USART TX pin */
#define OW_TX_PIN_PORT          GPIOB
#define OW_TX_PIN_PIN           GPIO_Pin_10
#define OW_TX_PIN_SOURCE        GPIO_PinSource10

#define USART_OW_IRQHandeler    USART3_IRQHandler

#ifndef OW_USE_SINGLE_PIN
    /* Has to be USART RX pin */
#define OW_RX_PIN_PORT          GPIOD
#define OW_RX_PIN_PIN           GPIO_Pin_9
#define OW_RX_PIN_SOURCE        GPIO_PinSource9
#endif

#define OW_USART                USART3
#define OW_USART_AF             GPIO_AF_USART3

#define OW_USART_CLOCK()        RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE)

#define OW_GPIO_TX_CLOCK()      RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE)

#ifndef OW_USE_SINGLE_PIN
#define OW_GPIO_RX_CLOCK()      RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE)
#endif

    /**************************************************************************/

    /* Public defines */
#define OW_ADDRESS_ALL              0

    typedef enum _OW_State {
        OW_NO_DEV = 0,
        OW_OK = 1,
        OW_BUSY=2,
    } OW_State;


    /* Hardware initialization */
   void OW_Init(void);
   void Error_ow(void);
    /* Communication functions */
   OW_State OW_ByteRead_As(void (*callback)(void));
   uint8_t OW_ByteRead(void);
	uint8_t OW_BitRead(void);
	void callback_byteread(void);
	void OW_BitWrite(const uint8_t bBit);
	uint8_t OW_GetByteReadResult(void);
	OW_State OW_ByteWrite_As(const uint8_t bByte,void (*callback)(void));
	void OW_ByteWrite(const uint8_t bByte);
	void callback_bytewrite(void);
	void OW_StrongPullUp(void);
	void OW_WeakPullUp(void);
	OW_State OW_Reset_As(void (*callback)(void));
	OW_State OW_Reset(void);
	void callback_Reset(void);
	OW_State OW_GetResetResult(void);
//	void USART_OW_IRQHandeler(void);
	

 /* 1-Wire search */
    void OW_FamilySkipSetup(void);
    /* Utilities */
    uint8_t OW_CRCCalculate(uint8_t iCRC, uint8_t iValue);

   
    uint64_t OW_SearchFirst(uint8_t iFamilyCode);
    uint64_t OW_SearchNext(void);

	
	
    /* Parasite powered devices support */


    /* ROM operations */
    uint64_t OW_ROMRead(void);
    void OW_ROMMatch(uint64_t iAddress, void (*callback)(void));
    void CB_ROMMatch(void);
    OW_State OW_ROMMatch_GetResult(void);
	

#endif //ONEWIRE_H
