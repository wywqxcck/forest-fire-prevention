#ifndef __KEY_H
#define __KEY_H

void Key_Init(void);
uint8_t Key_GetNum(void);
#define KEY1    GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_12)//��ȡ����1
#define KEY2    GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_13)//��ȡ����2
#define KEY3    GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_14)//��ȡ����3
#endif
