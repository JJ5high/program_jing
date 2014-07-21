/*
 * temp.c
 *
 *  Created on: 2012-10-20
 *      Author: Administrator
 */

#include "ownet.h"
#include "findtype.h"
#include "stdio.h"

#define TEMP_MAX_SENSOR_COUNT 10

int LastTemperature[TEMP_MAX_SENSOR_COUNT] = {0};
static uchar TempSensorSN[TEMP_MAX_SENSOR_COUNT][8];
static int NumDevices = 0;

#define PORTNUM 0

//初始化，返回值为温度传感器个数
int Temp_Init()
{
	int i;
	for(i = 0; i<TEMP_MAX_SENSOR_COUNT; i++)
		LastTemperature[i] = 0;
	NumDevices = FindDevices(PORTNUM, TempSensorSN, 0x28, TEMP_MAX_SENSOR_COUNT);
	return NumDevices;
}

int Temp_DoRead(int iSensor)
{
	uchar send_block[30], lastcrc8;
	int send_cnt, tsht = 0, i, loop = 0;
//	LED_Set(6);
	if(iSensor >= NumDevices)
		return 0;
//	LED_Set(7);
	owSerialNum(PORTNUM, TempSensorSN[iSensor], FALSE);

	for (loop = 0; loop < 2; loop++) {
		// access the device
		if (owAccess(PORTNUM)) {
			// send the convert command and if nesessary start power delivery
			if (!owWriteByte(PORTNUM, 0x44))
					return 0;

			// access the device
			if (owAccess(PORTNUM)) {
				// create a block to send that reads the temperature
				// read scratchpad command
				send_cnt = 0;
				send_block[send_cnt++] = 0xBE;
				// now add the read bytes for data bytes and crc8
				for (i = 0; i < 9; i++)
					send_block[send_cnt++] = 0xFF;

				// now send the block
				if (owBlock(PORTNUM, FALSE, send_block, send_cnt)) {
					// initialize the CRC8
					setcrc8(PORTNUM, 0);
					// perform the CRC8 on the last 8 bytes of packet
					for (i = send_cnt - 9; i < send_cnt; i++)
						lastcrc8 = docrc8(PORTNUM, send_block[i]);

					// verify CRC8 is correct
					if (lastcrc8 == 0x00) {
						// calculate the high-res temperature
						tsht = send_block[2] << 8;
						tsht = tsht | send_block[1];
						if (tsht & 0x00001000)
							tsht = tsht | 0xffff0000;
						if(!(tsht == 1360 && LastTemperature[iSensor] == 0)){
							LastTemperature[iSensor] = tsht;
						}else{
							tsht = 0;
						}
						// success
						break;
					}
				}
			}
		}
	}
	return tsht;
}

void RomReadCode(uchar RomCode[])
{
	uchar i;
	for(i=0;i<8;i++)
	{
		RomCode[i]=owReadByte(0);
//		printf("text%d\n",i);
	}
}

uchar CrcByte(uchar abyte)
{
	uchar i,crc=0;
	uchar GEN=0x8c;
	for(i=0;i<8;i++)
	{
		if((crc^abyte)&0x01)
		{
			crc>>=1;
			crc^=GEN;
		}
		else crc>>=1;
		abyte>>=1;
	}
	return crc;
}

uchar CrcCheck(uchar *p,uchar len)
{
	uchar crc=0;
	while(len--)
		crc=CrcByte(crc^*p++);
	return crc;
}

void TempSimpleRead()
{
	uchar i,RomCode[8];
	if(!owAcquire(0, NULL))
		return ;
//	LED_Set(1);
//	if(!owTouchReset(0))
//		return;
	LED_Set(2);
	owTouchByte(0,0x33);
	RomReadCode(RomCode);
	for(i=0;i<8;i++)
	{
		if(i==0)
			printf("ID:\n");
		printf("%d",RomCode[i]);
	}
//	if(CrcCheck(RomCode,8))
//		return;
	owTouchByte(0, 0xCC);
	owTouchByte(0, 0x44);
	msDelay(1000);
	owTouchReset(0);
	owTouchByte(0, 0xCC);
	LED_Set(3);
	owTouchByte(0, 0xBE);
	LED_Set(4);
	int tmp = owTouchByte(0, 0xff);
	tmp = tmp | (owTouchByte(0, 0xff) << 8);
	printf("\n%d\n", tmp);
//	printf("\n%.2f\n", tmp*0.0625);
	LED_Set(5);
}

inline int Temp_GetLastValue(int iSensor)
{
	return LastTemperature[iSensor];
}

