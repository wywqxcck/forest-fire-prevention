#ifndef __APP_H_
#define __APP_H_

#include "stm32f10x.h"
#include "stdint.h"
#include "adc.h"
#include "delay.h"
#include "DHT11.h"
#include "led.h"
#include "sx1276.h"
#include "Flame.h"
#include "oled.h"
#include "usart3.h"
#include "sys.h"
#include "MQ.h"
#define RX_TIMEOUT_VALUE                            1000  // ���ճ�ʱ��ֵ����λms
#define BUFFER_SIZE                                 64    // ���ݸ��س��ȣ�ʹ��64bit���ݳ���
#define DATA_MAX 7

enum Data{
	T = 1,
	H,
	FLAME,
	PM,
	CO2,
	PPM,
};
void APP_Init(void);
void Read_Flame(void);
void Read_MQ(void);
void Read_PM(void);
void Read_CO2(void);
void OLED_Show(void);
void Read_Data(void);
#endif

