#ifndef __LED_H
#define __LED_H	 
#include "stm32f10x.h"


#define LED1_ON 	GPIO_ResetBits(GPIOC,GPIO_Pin_13)

#define LED1_OFF 	GPIO_SetBits(GPIOC,GPIO_Pin_13)

#define BEEP_ON 	GPIO_ResetBits(GPIOA,GPIO_Pin_1)

#define BEEP_OFF 	GPIO_SetBits(GPIOA,GPIO_Pin_1)


void LED_Init(void);//≥ı ºªØ


		 				    
#endif
