#include "stm32f10x.h"                  // Device header
#include "led.h"
void LED_Init(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC | RCC_APB2Periph_AFIO, ENABLE);	 //使能GPIOB时钟和复用功能（要先打开复用才能修改复用功能）
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable,ENABLE);						 //要先开时钟，再重映射；这句表示关闭JTAG，使能SWD。

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	GPIO_InitStructure.GPIO_Pin    =  GPIO_Pin_13;
	GPIO_InitStructure.GPIO_Mode   =  GPIO_Mode_Out_PP; 		 //推挽输出
	GPIO_InitStructure.GPIO_Speed  =  GPIO_Speed_50MHz;		 //IO口速度为50MHz
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin    =  GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Mode   =  GPIO_Mode_Out_PP; 		 //推挽输出
	GPIO_InitStructure.GPIO_Speed  =  GPIO_Speed_50MHz;		 //IO口速度为50MHz
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	LED1_OFF;	LED2_OFF;

}
