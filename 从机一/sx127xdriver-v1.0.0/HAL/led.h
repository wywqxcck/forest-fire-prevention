#ifndef __LED_H
#define __LED_H	 
#include "stm32f10x.h"


#define LED2_ON 	GPIO_ResetBits(GPIOC,GPIO_Pin_13)

#define LED2_OFF 	GPIO_SetBits(GPIOC,GPIO_Pin_13)

#define LED1_ON 	GPIO_ResetBits(GPIOB,GPIO_Pin_15)

#define LED1_OFF 	GPIO_SetBits(GPIOB,GPIO_Pin_15)

void LED_Init(void);//��ʼ��

		 				    
#endif
