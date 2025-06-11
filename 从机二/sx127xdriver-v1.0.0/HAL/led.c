#include "stm32f10x.h"                  // Device header
#include "led.h"
void LED_Init(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC | RCC_APB2Periph_AFIO, ENABLE);	 //ʹ��GPIOBʱ�Ӻ͸��ù��ܣ�Ҫ�ȴ򿪸��ò����޸ĸ��ù��ܣ�
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable,ENABLE);						 //Ҫ�ȿ�ʱ�ӣ�����ӳ�䣻����ʾ�ر�JTAG��ʹ��SWD��

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	GPIO_InitStructure.GPIO_Pin    =  GPIO_Pin_13;
	GPIO_InitStructure.GPIO_Mode   =  GPIO_Mode_Out_PP; 		 //�������
	GPIO_InitStructure.GPIO_Speed  =  GPIO_Speed_50MHz;		 //IO���ٶ�Ϊ50MHz
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin    =  GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Mode   =  GPIO_Mode_Out_PP; 		 //�������
	GPIO_InitStructure.GPIO_Speed  =  GPIO_Speed_50MHz;		 //IO���ٶ�Ϊ50MHz
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	LED1_OFF;	LED2_OFF;

}
