#include "stm32f10x.h"                  // Device header
#include "Delay.h"

void Key_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
}
uint8_t Key_GetNum(void)
{
	uint8_t KeyNum = 0;
	if(GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_12) == 0)
	{
		delay_ms(20);
		while (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_12) == 0);
		delay_ms(20);
		KeyNum = 1;
		return KeyNum;
	}
	else if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_13) == 0)
	{
		delay_ms(20);
		while (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_13) == 0);
		delay_ms(20);
		KeyNum = 2;
		return KeyNum;
	}
	else if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_14) == 0)
	{
		delay_ms(20);
		while (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_14) == 0);
		delay_ms(20);
		KeyNum = 3;
		return KeyNum;
	}
	return 0;
}
