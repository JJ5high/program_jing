/**
 * Hardware initialization.
 */
#include "OneWire.h"
/* Backup of BRR register for different communication speeds*/
static uint16_t iUSART9600;
static uint16_t iUSART115200;
void OW_Init(void) {
    GPIO_InitTypeDef GPIO_InitStruct;
    USART_InitTypeDef USART_InitStructure;

    /* Enable clock for periphetials */
    OW_GPIO_TX_CLOCK();

#ifndef OW_USE_SINGLE_PIN
    OW_GPIO_RX_CLOCK();
#endif

    OW_USART_CLOCK();

    /* Alternate function config on TX pin */
    GPIO_PinAFConfig(OW_TX_PIN_PORT, OW_TX_PIN_SOURCE, OW_USART_AF);

    /* TX pin configuration */
    GPIO_InitStruct.GPIO_Pin = OW_TX_PIN_PIN;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_OD;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(OW_TX_PIN_PORT, &GPIO_InitStruct);

	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = OW_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = OW_PREPRIO;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = OW_SUBPRIO;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	USART_ITConfig(USART3,USART_IT_RXNE,ENABLE);



#ifndef OW_USE_SINGLE_PIN 
    /* Alternate function config on RX pin */
    GPIO_PinAFConfig(OW_RX_PIN_PORT, OW_RX_PIN_SOURCE, OW_USART_AF);
    GPIO_InitStruct.GPIO_Pin = OW_RX_PIN_PIN;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
    GPIO_Init(OW_RX_PIN_PORT, &GPIO_InitStruct);
#endif

    /* USART configuration */
    USART_StructInit(&USART_InitStructure);
    
    USART_InitStructure.USART_BaudRate = 115200;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
    USART_Init(OW_USART, &USART_InitStructure);

    /* BRR register backup for 115200 Baud */
    iUSART115200 = OW_USART->BRR;

    /* BRR register backup for 9600 Baud */
    USART_InitStructure.USART_BaudRate = 9600;
    USART_Init(OW_USART, &USART_InitStructure);
    iUSART9600 = OW_USART->BRR;

#ifdef OW_USE_SINGLE_PIN 
    /* Half duplex enable, for single pin communication */
    USART_HalfDuplexCmd(OW_USART, ENABLE);
#endif

    /* USART enable */
    USART_Cmd(OW_USART, ENABLE);
}

static void (*p_callback)(void);
static volatile int operation;
static volatile int rebuff;//read buffer
static volatile   uint8_t iPresence;
volatile int busy=0;
volatile int datlen;
volatile int cmdbuff;
/**
 * Read one byte.
 * @return Received byte.
 */
volatile int read_done_flag = 0;
OW_State OW_ByteRead_As(void (*callback)(void)) {
	if(busy == 1){
		printf("busy");
		return OW_BUSY;
	}
	else{
	    rebuff = 0;
	    p_callback=callback;
		datlen = 0;
		busy = 1;
		operation = OW_OP_READ;
	    USART_SendData(OW_USART, OW_1);
	}
	return OW_OK;
}
uint8_t OW_ByteRead(void){
	read_done_flag = 0;
	volatile int t=0xffffff;
	OW_ByteRead_As(callback_byteread);
	while(read_done_flag == 0 && t>0){
		 t--;
	 }
	 if(t==0){
		 Error_ow();
	 }
	return OW_GetByteReadResult();
}
void callback_byteread(void){
	read_done_flag=1;
}

uint8_t OW_BitRead(void) {
    /* Make sure that all communication is done and receive buffer is cleared */
    while (USART_GetFlagStatus(OW_USART, USART_FLAG_TC) == RESET);
    while (USART_GetFlagStatus(OW_USART, USART_FLAG_RXNE) == SET)
        USART_ReceiveData(OW_USART);  

    /* Send byte */
    USART_SendData(OW_USART, OW_1);

    /* Wait for response */
    while (USART_GetFlagStatus(OW_USART, USART_FLAG_TC) == RESET);

    /* Receive data */
    if (USART_ReceiveData(OW_USART) != OW_1) return 0;

    return 1;
}

void OW_BitWrite(const uint8_t bBit) {
    uint8_t bData = OW_0;

    if (bBit) bData = OW_1;

    /* Make sure that all communication is done */
    while (USART_GetFlagStatus(OW_USART, USART_FLAG_RXNE) == SET)
        USART_ReceiveData(OW_USART);
    while (USART_GetFlagStatus(OW_USART, USART_FLAG_TC) == RESET);

    /* Send byte */
    USART_SendData(OW_USART, bData);
}

uint8_t OW_GetByteReadResult(void)
{
	return rebuff;
}

/**
 * Write one byte.
 * @param bByte Byte to be transmited.
 */
OW_State OW_ByteWrite_As(const uint8_t bByte,void (*callback)(void)) {

    if(busy){
    	printf("busy");
    	return OW_BUSY;
    }else{
    	cmdbuff=bByte;
    	p_callback=callback;
    	busy=1;
    	datlen=0;
    	operation=OW_OP_WRITE;
    	USART_SendData(OW_USART, (cmdbuff & 0x01)?OW_1:OW_0);
    }
	return OW_OK;
}

volatile int write_done_flag = 0;

void OW_ByteWrite(const uint8_t bByte)
{
	volatile int t=0xffffff;
	write_done_flag = 0;
	OW_ByteWrite_As(bByte, callback_bytewrite);
	while(write_done_flag == 0 && t>0){
		 t--;
	 }
	 if(t==0){
		 Error_ow();
	 }
}

void callback_bytewrite(void)
{
	write_done_flag = 1;
}

/**
 * Set RX/TX pin into strong pull-up state.
 */
void OW_StrongPullUp(void) {
#ifdef OW_USE_PARASITE_POWER

#ifdef OW_USE_SINGLE_PIN 
//	USART_Cmd(OW_USART, DISABLE);
    USART_HalfDuplexCmd(OW_USART, DISABLE);
#endif

    GPIO_SetBits(OW_TX_PIN_PORT, OW_TX_PIN_PIN);
    OW_TX_PIN_PORT->OTYPER &= ~(OW_TX_PIN_PIN);

#endif
}

/**
 * Set RX/TX pin into weak pull-up state.
 */
void OW_WeakPullUp(void) {
#ifdef OW_USE_PARASITE_POWER

    GPIO_SetBits(OW_TX_PIN_PORT, OW_TX_PIN_PIN);
    OW_TX_PIN_PORT->OTYPER |= OW_TX_PIN_PIN;

#ifdef OW_USE_SINGLE_PIN 
//    USART_Cmd(OW_USART, ENABLE);
    USART_HalfDuplexCmd(OW_USART, ENABLE);
#endif

#endif
}



/**
 * Communication reset and device presence detection.
 * @return OW_OK if device found or OW_NO_DEV if not.
 */


volatile int reset_state;
volatile int reset_done_flag=0;
void (*PReset_callback)(void);
OW_State OW_Reset_As(void (*callback)(void)) {
	if(busy){
		printf("busy");
		return OW_BUSY;
	}else{
	busy=1;
	operation = OW_OP_RESET;
	PReset_callback=callback;
	reset_state = 0;
    /* Set USART baudrate to 9600 Baud */
    OW_USART->BRR = iUSART9600;

    /* Make sure that all communication is done and receive buffer is cleared */
    USART_ClearFlag(OW_USART, USART_FLAG_TC);
    while (USART_GetFlagStatus(OW_USART, USART_FLAG_RXNE) == SET)
            USART_ReceiveData(OW_USART);
    USART_SendData(OW_USART, OW_R);
	}
//	while(busy);
	return OW_OK;
}

OW_State OW_Reset(void){
	 reset_done_flag=0;
     OW_Reset_As(callback_Reset);
     volatile int t=0xffffff;
	 while(reset_done_flag==0 && t>0){
		 t--;
	 }
	 if(t==0){
		 Error_ow();
	 }
	 return OW_GetResetResult();
}
void Error_ow(void){
	if(operation == OW_OP_RESET){
		printf("while_reset is wrong");
	}else if(operation == OW_OP_READ){
		printf("while_read is wrong");
	}else if(operation == OW_OP_WRITE){
		printf("while_write is wrong");
	}
}

void callback_Reset(void){
   reset_done_flag=1;
}

OW_State OW_GetResetResult(void)
{
	if ((iPresence != OW_R) && ((iPresence != 0x00))) 
		return OW_OK;
	else
		return OW_NO_DEV;
}


void USART_OW_IRQHandeler(void)
{
	uint8_t bData = OW_0;
	uint8_t recvData;
	if(USART_GetITStatus(OW_USART, USART_IT_RXNE) == SET){

		recvData = USART_ReceiveData(OW_USART);
        if(operation == OW_OP_FREE){
        	busy=0;
        }
        else if(operation == OW_OP_WRITE){
			if(datlen<7){
				datlen++;
				if(cmdbuff&(1<<datlen)){
					bData = OW_1;
				}else{
					bData = OW_0;
				}
				USART_SendData(OW_USART,bData);
			}else{
				busy = 0;
				operation = OW_OP_FREE;
				p_callback();
			}
		}else if (operation == OW_OP_READ){
			if (recvData == OW_1){
				rebuff|=(1<<datlen);
			}
			datlen++;
			if(datlen < 8){
				USART_SendData(OW_USART, OW_1);
			}else{
				busy = 0;
				operation = OW_OP_FREE;
				p_callback();
			}
		}else if(operation == OW_OP_RESET){

				if(reset_state == 0){
				busy=0;
				reset_state = 1;
				iPresence = recvData;
				OW_USART->BRR = iUSART115200;
				operation = OW_OP_FREE;
				PReset_callback();

//			}else if(reset_state == 1){

			}else{

				busy=0;
				//error
			}
		}else{
			busy=0;

			//error
		}
	}
}
