/********************************Copyright (c)**********************************\
**
**                   (c) Copyright 2019, temp_duan, China, DY.
**                           All Rights Reserved
**
**                           By(�������ɿƼ����޹�˾)
**                             http://www.kpiot.top
**
**----------------------------------�ļ���Ϣ------------------------------------
** �ļ�����: timer.c
** ������Ա: temp_duan
** ��������: 2019-07-09
** �ĵ�����:
**
**----------------------------------�汾��Ϣ------------------------------------
** �汾����: V0.1
** �汾˵��: ��ʼ�汾
**
**------------------------------------------------------------------------------
\********************************End of Head************************************/


#include "timer.h"
#include "gizwits_product.h"
/*******************************************************************************
** ��������: TIM1_Int_Init
** ��������:
** ����˵��: arr: [����/��]
**			 psc: [����/��]
** ����˵��: None
** ������Ա: temp_duan
** ��������: 2019-07-09
**------------------------------------------------------------------------------
** �޸���Ա:
** �޸�����:
** �޸�����:
**------------------------------------------------------------------------------
********************************************************************************/
void TIM1_Int_Init(u16 arr, u16 psc)
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);

    TIM_TimeBaseStructure.TIM_Period = arr;
    TIM_TimeBaseStructure.TIM_Prescaler = psc;
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure);
    TIM_ITConfig(TIM1, TIM_IT_Update, ENABLE );

    NVIC_InitStructure.NVIC_IRQChannel = TIM1_UP_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    TIM_Cmd(TIM1, ENABLE);
}


/*******************************************************************************
** ��������: TIM3_Int_Init
** ��������:
** ����˵��: arr: [����/��]
**			 psc: [����/��]
** ����˵��: None
** ������Ա: temp_duan
** ��������: 2019-07-09
**------------------------------------------------------------------------------
** �޸���Ա:
** �޸�����:
** �޸�����:
**------------------------------------------------------------------------------
********************************************************************************/


/*******************************************************************************
** ��������: TIM4_Int_Init
** ��������:
** ����˵��: arr: [����/��]
**			 psc: [����/��]
** ����˵��: None
** ������Ա: temp_duan
** ��������: 2019-07-16
**------------------------------------------------------------------------------
** �޸���Ա:
** �޸�����:
** �޸�����:
**------------------------------------------------------------------------------
********************************************************************************/
void TIM4_Int_Init(u16 arr, u16 psc)
{
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE); //ʱ��ʹ��


    //��ʱ��TIM4��ʼ��
    TIM_TimeBaseStructure.TIM_Period = arr; //��������һ�������¼�װ�����Զ���װ�ؼĴ������ڵ�ֵ
    TIM_TimeBaseStructure.TIM_Prescaler = psc; //����������ΪTIMxʱ��Ƶ�ʳ�����Ԥ��Ƶֵ
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; //����ʱ�ӷָ�:TDTS = Tck_tim
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM���ϼ���ģʽ
    TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure); //����ָ���Ĳ�����ʼ��TIMx��ʱ�������λ

    TIM_ITConfig(TIM4, TIM_IT_Update, ENABLE ); //ʹ��ָ����TIM4�ж�,��������ж�

    //�ж����ȼ�NVIC����
    NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;  //TIM4�ж�
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;  //��ռ���ȼ�0��
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;  //�����ȼ�3��
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //IRQͨ����ʹ��
    NVIC_Init(&NVIC_InitStructure);  //��ʼ��NVIC�Ĵ���

    TIM_Cmd(TIM4, ENABLE); //ʹ�ܶ�ʱ��4
}


/*******************************************************************************
** ��������: TIM1_UP_IRQHandler
** ��������:
** ����˵��: None
** ����˵��: None
** ������Ա: temp_duan
** ��������: 2019-07-09
**------------------------------------------------------------------------------
** �޸���Ա:
** �޸�����:
** �޸�����:
**------------------------------------------------------------------------------
********************************************************************************/
//void TIM1_UP_IRQHandler(void)
//{
//    if(TIM_GetITStatus(TIM1, TIM_IT_Update) != RESET)
//    {
//		static uint8_t Key_Count = 0;
//		Key_Count++;
//		if(Key_Count >= 5){
//			Key_Count = 0;
//			button_ticks();
//		}
//    }
//    TIM_ClearFlag(TIM1, TIM_IT_Update );
//}


/*******************************************************************************
** ��������: TIM4_IRQHandler
** ��������:
** ����˵��: None
** ����˵��: None
** ������Ա: temp_duan
** ��������: 2019-07-09
**------------------------------------------------------------------------------
** �޸���Ա:
** �޸�����:
** �޸�����:
**------------------------------------------------------------------------------
********************************************************************************/
void TIM1_UP_IRQHandler(void)   //TIM4�ж�
{
    if (TIM_GetITStatus(TIM1, TIM_IT_Update) != RESET)  //���TIM4�����жϷ������
    {
		gizTimerMs();
    }
    TIM_ClearITPendingBit(TIM1, TIM_IT_Update  );  //���TIM4�����жϱ�־
}
