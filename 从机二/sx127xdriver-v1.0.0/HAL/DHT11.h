#ifndef __DHT11_AS_H
#define __DHT11_AS_H

#include "stm32f10x.h"
#include "delay.h"
#include <stdio.h>
#define DEBUG_printf
typedef struct
{
	uint8_t  humi_int;		//ʪ�ȵ���������
	uint8_t  humi_deci;	 	//ʪ�ȵ�С������
	uint8_t  temp_int;	 	//�¶ȵ���������
	uint8_t  temp_deci;	 	//�¶ȵ�С������
	uint8_t  check_sum;	 	//У���                 
} DHT11_Data_TypeDef;

extern DHT11_Data_TypeDef DHT11Data;

void DHT11_GPIO_Init(void);
uint8_t DHT11_Read_TempAndHumidity(DHT11_Data_TypeDef *DHT11_Data);
void DHT11(void);
uint8_t DHT11_Check(void);


#endif
