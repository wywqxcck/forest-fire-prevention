#ifndef __HAL_UART_H__
#define __HAL_UART_H__

#include "stm32f10x.h"
#include "stdio.h"

#define USART_REC_LEN  			200  	//�����������ֽ��� 200
#define EN_USART1_RX 			1		//ʹ�ܣ�1��/��ֹ��0������1����

//��ӡ��������С��DebugLevel����Ϣ
#define LEVEL_OFF			0			//�رմ�ӡ
#define LEVEL_ERR			1			//������Ϣ
#define LEVEL_WARNING	2			//������Ϣ
#define LEVEL_INFO		3			//��ͨ��Ϣ
#define LEVEL_DEBUG		4		//������Ϣ
#define LEVEL_ALL			255		//�������д�ӡ
extern uint8_t DebugLevel;
#if HAVE_OS
	#define myPrintf(Level, ...) \
	do{                                   \
		vPortEnterCritical(); \
		if ( Level<DebugLevel ){      \
			if(LEVEL_DEBUG==Level){	\
				printf("[%s()-%d]",__func__,__LINE__); \
			}	\
			printf(__VA_ARGS__);               \
		}                               \
		vPortExitCritical();			\
	}while(0)
#else
	#define myPrintf(Level, ...) \
	do{                                   \
		if ( Level<DebugLevel ){      \
			if(LEVEL_DEBUG==Level){	\
				printf("[%s()-%d]",__func__,__LINE__); \
			}	\
			printf(__VA_ARGS__);               \
		}                               \
	}while(0)
#endif

typedef void (*HAL_UartcallBack_t)(uint8_t data);

extern u8  USART_RX_BUF[USART_REC_LEN]; //���ջ���,���USART_REC_LEN���ֽ�.ĩ�ֽ�Ϊ���з� 
extern u16 USART_RX_STA;         		//����״̬���	

uint8_t HALUart1Init(uint32_t uartBand,HAL_UartcallBack_t uartCallBackFunc);

#endif	//end of __HAL_UART_H__
