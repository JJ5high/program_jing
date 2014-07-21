/*
 * _write.c
 *
 *  Created on: 2014Äê3ÔÂ17ÈÕ
 *      Author: caroline
 */


#include <stdio.h>
#include <unistd.h>

#include "stm32f4xx.h"

int _write(int fd, char* ptr, int len)
{
	if (fd == STDOUT_FILENO || fd == STDERR_FILENO)
	{
		int i = 0;
		while(i<len)
			ITM_SendChar(ptr[i++]);
	}
	return len;
}

