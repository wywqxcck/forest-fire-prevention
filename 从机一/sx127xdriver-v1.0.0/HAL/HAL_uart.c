#include "HAL_uart.h"

uint8_t DebugLevel=LEVEL_ALL;	//��ӡ��������С��DebugLevel����Ϣ,DebugLevel���Զ�̬����

static HAL_UartcallBack_t __uart1CallBackFunc=NULL;
//����1�жϷ������
//ע��,��ȡUSARTx->SR�ܱ���Ī������Ĵ���   	
u8 USART_RX_BUF[USART_REC_LEN];     //���ջ���,���USART_REC_LEN���ֽ�.
//����״̬
//bit15��	������ɱ�־
//bit14��	���յ�0x0d
//bit13~0��	���յ�����Ч�ֽ���Ŀ
u16 USART_RX_STA=0;       //����״̬���	  

/*��δ���������֧��printf��ӡ�����ڵĴ���*/
#pragma import(__use_no_semihosting)             
//��׼����Ҫ֧�ֵĺ���
struct __FILE 
{
	int handle;
};

FILE __stdout;       
//����_sys_exit()�Ա��⹤���ڰ�����״̬
void _sys_exit(int x) 
{
	x = x; 
}
//�ض���fputc����
//�����Ҫ����MCU������ϣ��printf���ĸ����������ȷ�� __WAIT_TODO__
int fputc(int ch, FILE *f)
{
	//ע�⣺USART_FLAG_TXE�Ǽ�鷢�ͻ������Ƿ�Ϊ�գ����Ҫ�ڷ���ǰ��飬������������߷���Ч�ʣ����������ߵ�ʱ����ܵ������һ���ַ���ʧ
	//USART_FLAG_TC�Ǽ�鷢����ɱ�־������ڷ��ͺ��飬����������˯�߶�ʧ�ַ����⣬����Ч�ʵͣ����͹����з��ͻ������Ѿ�Ϊ���ˣ����Խ�����һ�������ˣ�������ΪҪ�ȴ�������ɣ�����Ч�ʵͣ�
	//��Ҫ����һ���ã�һ����Ч�����
	
	//ѭ���ȴ�ֱ�����ͻ�����Ϊ��(TX Empty)��ʱ���Է������ݵ�������
  while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET)
  {}
	USART_SendData(USART1, (uint8_t) ch);

  /* ѭ���ȴ�ֱ�����ͽ���*/
  //while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET)
  //{}

	return ch;
}

//uart1(PA_9 TX,PA_10 RX)��ʼ������
uint8_t HALUart1Init(uint32_t uartBand,HAL_UartcallBack_t uartCallBackFunc){
	GPIO_InitTypeDef  GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	
	__uart1CallBackFunc=uartCallBackFunc;
	
	USART_DeInit(USART1);  //��λ����
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1|RCC_APB2Periph_GPIOA, ENABLE);//��������ʱ��,ע��APB1��APB2ʱ��ʹ�ܺ�����һ��
	
	//PA_9 ��ʼ��Ϊ����ģʽ
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	 //IO���ٶ�Ϊ50MHz�����ﲻ�ô��Σ�ֱ��д��������ٶ�50MHZ
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_AF_PP;//Ƭ�����裬����ģʽ(��Ҫ���PSEL���ò���ȷ���Ǹ����ĸ�����)
	GPIO_Init(GPIOA, &GPIO_InitStructure);	//��ʼ��GPIO
	//PA_10��ʼ��Ϊ��������ģʽ
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_IPU;//��������
	GPIO_Init(GPIOA, &GPIO_InitStructure);	//��ʼ��GPIO

	if(NULL!=uartCallBackFunc){//û�лص�������ʱ�����������жϣ���ʱ�Ĵ����ܷ�������
		//�ж����ȼ�������ǰ�ж����ȼ�����Ϊ2(NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2))�Ѿ����壬��Ҫ�޸�
		NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;	//ָ�������ĸ��ж�
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3 ;//��ռ���ȼ�Ϊ3
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		//�����ȼ�3
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQͨ��ʹ��
		NVIC_Init(&NVIC_InitStructure);	//���� NVIC
	}

	//USART����
	USART_InitStructure.USART_BaudRate = uartBand;	//���ô��ڲ�����
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;	//�ֳ�Ϊ8
	USART_InitStructure.USART_StopBits = USART_StopBits_1;	//1��ֹͣλ
	USART_InitStructure.USART_Parity = USART_Parity_No;	//����żУ��
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;	//������
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//�շ�ģʽ
	USART_Init(USART1, &USART_InitStructure); //����USART����
	if(NULL!=uartCallBackFunc){
		USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);	//���������ж�(�������޷���������,û�н��ջص������Ͳ������ˣ���Ϊû�лص���û�д�����յĵ����ݣ�������Ҳû������)
	}
	USART_Cmd(USART1, ENABLE);                    //ʹ��USART
	
	return 0;
}

//USART1�жϺ���(����)
//�����Ҫ����MCU���� __WAIT_TODO__
void USART1_IRQHandler(void){
	if(USART_GetFlagStatus(USART1, USART_FLAG_RXNE) == SET)  
	{
		if(NULL!=__uart1CallBackFunc){
			(*__uart1CallBackFunc)(USART_ReceiveData(USART1));	//ִ�лص�����
		}
	}
}

/*//USART2�жϺ���(����)
//�����Ҫ����MCU���� __WAIT_TODO__
void USART2_IRQHandler(void){
	uint8_t ch;
	if(USART_GetFlagStatus(USART2, USART_FLAG_RXNE) == SET)  
	{
		ch=USART_ReceiveData(USART2);	//��ȡ���յ����ַ�
	}
}

//USART3�жϺ���(����)
//�����Ҫ����MCU���� __WAIT_TODO__
void USART3_IRQHandler(void){
	uint8_t ch;
	if(USART_GetFlagStatus(USART3, USART_FLAG_RXNE) == SET)  
	{
		ch=USART_ReceiveData(USART3);	//��ȡ���յ����ַ�
	}
}*/
