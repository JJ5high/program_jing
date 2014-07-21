/*
 * temp.h
 *
 *  Created on: 2012-10-20
 *      Author: Administrator
 */

#ifndef TEMP_H_
#define TEMP_H_

int Temp_Init();
int Temp_DoRead(int iSensor);
inline int Temp_GetLastValue(int iSensor);

#endif /* TEMP_H_ */
