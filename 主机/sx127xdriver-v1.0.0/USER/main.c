#include "stm32f10x.h"
#include "HAL_uart.h"
#include "usmart.h"
#include "string.h"
#include "radio.h"
#include "delay.h"
#include "sx1276.h"
#include "timer.h"
#include "usart3.h"
#include "key.h"
#include "oled.h"
#include "flash.h"
#include "Led.h"
#include "Key.h"
#include "gizwits_product.h"
#include "gizwits_protocol.h"
uint8_t A_Data[12];
uint8_t B_Data[12];
extern u8 wifi_flag;
uint8_t oledID = 0;
dataPoint_t currentDataPoint;
#define USE_BAND_433	//ѡ��һ��Ƶ��
#define USE_MODEM_LORA	//ѡ��loraģʽ
//#define USE_MODEM_FSK	//ѡ��FSKģʽ

#define FSK_SINGLE_CARRIER	0	//���ز���Ƶ���ԣ�ע�⣺���ز�ģʽ������FSKģʽ��
#define LORA_CONTINUE	0	//lora��������ģʽ

#if defined( USE_BAND_433 )

#define RF_FREQUENCY                                434000000 // Hz

#elif defined( USE_BAND_780 )

#define RF_FREQUENCY                                780000000 // Hz

#elif defined( USE_BAND_868 )

#define RF_FREQUENCY                                868000000 // Hz

#elif defined( USE_BAND_915 )

#define RF_FREQUENCY                                915000000 // Hz

#else
    #error "���ڱ�����ѡ����ѡ��һ��Ƶ�Ρ�"
#endif

#define TX_OUTPUT_POWER                             20        // dBm�����书�ʣ�1278оƬ�������20dBm��

#if defined( USE_MODEM_LORA )

#define LORA_BANDWIDTH                              0         // [0: 125 kHz,��������������Խ����������Խ�죬���Ǵ������Խ����
                                                              //  1: 250 kHz,
                                                              //  2: 500 kHz,
                                                              //  3: Reserved]
#define LORA_SPREADING_FACTOR                       7         // [SF7..SF12] ����Ƶ���ӣ���Ƶ��������Խ�󣬴������ԽԶ�����Ǵ�������Խ����
#define LORA_CODINGRATE                             1         // [1: 4/5,    �������ʣ����������ڼ��鴫�����޳���Ĭ��ʹ��4/5��
                                                              //  2: 4/6,
                                                              //  3: 4/7,
                                                              //  4: 4/8]
#define LORA_PREAMBLE_LENGTH                        8         // ǰ���볤�ȣ���õ���8bit��16bit������ʹ��8bit
#define LORA_SYMBOL_TIMEOUT                         5         // ֡��ʱ��֡��Ϊ5
#define LORA_FIX_LENGTH_PAYLOAD_ON                  false     // ���ݸ��ػ�ϳ��ȣ�Ĭ�ϲ�����
#define LORA_IQ_INVERSION_ON                        false     // Ĭ�ϲ�����

#elif defined( USE_MODEM_FSK )

#define FSK_FDEV                                    25e3      // Hz
#define FSK_DATARATE                                50e3      // bps
#define FSK_BANDWIDTH                               50e3      // Hz
#define FSK_AFC_BANDWIDTH                           83.333e3  // Hz
#define FSK_PREAMBLE_LENGTH                         5         // Same for Tx and Rx
#define FSK_FIX_LENGTH_PAYLOAD_ON                   false

#else
    #error "���ڱ�����ѡ����ѡ��һ��Ƶ�Ρ�"
#endif

typedef enum
{
    LOWPOWER,
    RX,
    RX_TIMEOUT,
    RX_ERROR,
    TX,
    TX_TIMEOUT,
}States_t;

#define RX_TIMEOUT_VALUE                            5000  // ���ճ�ʱ��ֵ����λms
#define BUFFER_SIZE                                 64    // ���ݸ��س��ȣ�ʹ��64bit���ݳ���


uint16_t BufferSize = BUFFER_SIZE;
uint8_t Buffer[BUFFER_SIZE];

States_t State = LOWPOWER;

int8_t RssiValue = 0;
int8_t SnrValue = 0;

/*!
 * Radio events function pointer
 */
static RadioEvents_t RadioEvents;

/*!
 * \brief Function to be executed on Radio Tx Done event
 */
void OnTxDone( void );
void OLED_ShowA(void);
void OLED_ShowB(void);
void OLED_ShowC(void);
void userHandle(void);
void key_deal(void);
void OLED_Show(void);
/*!
 * \brief Function to be executed on Radio Rx Done event
 */
void OnRxDone( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr );

/*!
 * \brief Function executed on Radio Tx Timeout event
 */
void OnTxTimeout( void );

/*!
 * \brief Function executed on Radio Rx Timeout event
 */
void OnRxTimeout( void );

/*!
 * \brief Function executed on Radio Rx Error event
 */
void OnRxError( void );

void uart1callBackTest(uint8_t data){
	u8 Res;
	Res =USART_ReceiveData(USART1);	//��ȡ���յ�������
		
	if((USART_RX_STA&0x8000)==0){//����δ���
		if(USART_RX_STA&0x4000){//���յ���0x0d
			if(Res!=0x0a)
				USART_RX_STA=0;//���մ���,���¿�ʼ
			else
				USART_RX_STA|=0x8000;	//��������� 
			
		}else{ //��û�յ�0X0D
			if(Res==0x0d)
				USART_RX_STA|=0x4000;
			else{
				USART_RX_BUF[USART_RX_STA&0X3FFF]=Res ;
				USART_RX_STA++;
				if(USART_RX_STA>(USART_REC_LEN-1))
					USART_RX_STA=0;//�������ݴ���,���¿�ʼ����	  
			}
		}
	}
}
uint8_t testFun(uint8_t num,char *str){

	printf("\r\ntestFun:%d-%s\r\n",num,str);
	printf("rssi:%d\r\n",SX1276ReadRssi(MODEM_LORA));
	return 2;
}
void Gizwits_Init(void)
{
    TIM1_Int_Init(9, 7199);
    usart3_init(9600); // wifi��ʼ�� �����ʱ���Ϊ9600
    memset((uint8_t *)&currentDataPoint, 0, sizeof(dataPoint_t));
    gizwitsInit();
}

int main(void)
{
	bool isMaster = true;		//һ������Ϊ����һ������Ϊ�ӻ�
	LED_Init();
	delay_init();	    	 //��ʱ������ʼ��
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);	//NVIC(�ж����ȼ�����)��������,ע��:���������������ֻ����һ��,���ú�Ҫ�޸�,�������ֺܶ�����
	HALUart1Init(115200,uart1callBackTest);
	myPrintf(LEVEL_DEBUG,"init ok\r\n");
	if(isMaster){
		myPrintf(LEVEL_DEBUG,"this is master\r\n");
	}else{
		myPrintf(LEVEL_DEBUG,"this is slave\r\n");
	}
	usmart_dev.init(SystemCoreClock/1000000);	//��ʼ��USMART	
	
	// LoRa ��ʼ��
  RadioEvents.TxDone = OnTxDone;
  RadioEvents.RxDone = OnRxDone;
  RadioEvents.TxTimeout = OnTxTimeout;
  RadioEvents.RxTimeout = OnRxTimeout;
  RadioEvents.RxError = OnRxError;

  Radio.Init( &RadioEvents );

  Radio.SetChannel( RF_FREQUENCY );

#if defined( USE_MODEM_LORA )

  Radio.SetTxConfig( MODEM_LORA, TX_OUTPUT_POWER, 0, LORA_BANDWIDTH,
                                   LORA_SPREADING_FACTOR, LORA_CODINGRATE,
                                   LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
                                   true, 0, 0, LORA_IQ_INVERSION_ON, 3000 );
    
  Radio.SetRxConfig( MODEM_LORA, LORA_BANDWIDTH, LORA_SPREADING_FACTOR,
                                   LORA_CODINGRATE, 0, LORA_PREAMBLE_LENGTH,
                                   LORA_SYMBOL_TIMEOUT, LORA_FIX_LENGTH_PAYLOAD_ON,
                                   0, true, 0, 0, LORA_IQ_INVERSION_ON, true );

#elif defined( USE_MODEM_FSK )

  Radio.SetTxConfig( MODEM_FSK, TX_OUTPUT_POWER, FSK_FDEV, 0,
                                  FSK_DATARATE, 0,
                                  FSK_PREAMBLE_LENGTH, FSK_FIX_LENGTH_PAYLOAD_ON,
                                  true, 0, 0, 0, 3000 );
    
  Radio.SetRxConfig( MODEM_FSK, FSK_BANDWIDTH, FSK_DATARATE,
                                  0, FSK_AFC_BANDWIDTH, FSK_PREAMBLE_LENGTH,
                                  0, FSK_FIX_LENGTH_PAYLOAD_ON, 0, true,
                                  0, 0,false, true );

#else
    #error "Please define a frequency band in the compiler options."
#endif

#if LORA_CONTINUE
	myPrintf(LEVEL_DEBUG,"enter lora continue mode\r\n");
	SX1276Write( REG_FEILSB, ( SX1276Read( REG_FEILSB ) | (1<<3) ) );	//����TxContinuousMode Ϊ1
	Radio.Send( Buffer, BufferSize );	//�����������ݿ��Խ����������ģʽ(��������Ϣ)����ʱ�޷��˳���ֻ�и�λ�����л�ģʽ
	while(1){
	}
#endif
#if FSK_SINGLE_CARRIER
	//���ز�ģʽ�����������FSKģʽ��
	myPrintf(LEVEL_DEBUG,"enter fsk mode\r\n");
	SX1276Write( REG_FDEVMSB, 0x00 );
	SX1276Write( REG_FDEVLSB, 0x00 );	//���� fdev Ϊ0
	Radio.Send( Buffer, BufferSize );	//�����������ݿ��Խ��뵥�ز�����ģʽ����ʱ�޷��˳���ֻ�и�λ�����л�ģʽ
	while(1){
	}
#endif
  Radio.Rx( RX_TIMEOUT_VALUE );
  OLED_Init();
  Key_Init(); 
  OLED_ShowC();
  Gizwits_Init();
  if(KEY1 == RESET)
	{
		delay_ms(1000);
		delay_ms(1000);
		LED1_ON;
		gizwitsSetMode(WIFI_AIRLINK_MODE);
	}
	__set_PRIMASK(0);
	
  while(1)
  {
	key_deal();
	 OLED_Show(); 
	userHandle();
	gizwitsHandle((dataPoint_t *)&currentDataPoint);
  }
  
}
void key_deal(void)
{
		uint8_t temp;
		temp = Key_GetNum();
	
		switch(temp)
		{
			case 0:break;
			case 1:break;
			case 2:oledID =1;OLED_Clear();break;
			case 3:oledID =2;OLED_Clear();break;
		}
}
void OLED_Show(void)
{
	if(oledID == 0)
	{
		OLED_ShowC();
	}
	else if(oledID == 1)
	{
		OLED_ShowA();
	}
	else if(oledID == 2)
	{
		OLED_ShowB();
	}
}
void OLED_ShowC(void)
{
	 OLED_ShowCHinese(16,0,2);
	 OLED_ShowCHinese(32,0,3);
	 OLED_ShowCHinese(48,0,4);
	 OLED_ShowCHinese(64,0,5);
	 OLED_ShowCHinese(80,0,6);
	 OLED_ShowCHinese(96,0,7);
	 OLED_ShowString(0,2,"A",16);
	 OLED_ShowCHinese(9,2,0);
	 OLED_ShowCHinese(25,2,1);
	 OLED_ShowString(0,4,"B",16);
	 OLED_ShowCHinese(9,4,0);
	 OLED_ShowCHinese(25,4,1);
}
void OLED_ShowA(void)
{
	char arr[100];
	OLED_ShowString(0,0,"A",16);
	OLED_ShowCHinese(9,0,0);
	OLED_ShowCHinese(25,0,1);
	sprintf(arr,"T=%d H=%d",A_Data[0],A_Data[1]);
    OLED_ShowString(0,2,(u8*)arr,16);	
    sprintf(arr,"F=%d MQ=%d",A_Data[2],A_Data[5]);
    OLED_ShowString(0,4,(u8*)arr,16);	
	sprintf(arr,"CO2=%d PM=%d",A_Data[4],A_Data[3]);
    OLED_ShowString(0,6,(u8*)arr,16); 
}

void OLED_ShowB(void)
{
	char arr[100];
	OLED_ShowString(0,0,"B",16);
	OLED_ShowCHinese(9,0,0);
	OLED_ShowCHinese(25,0,1);
	sprintf(arr,"T=%d H=%d",B_Data[0],B_Data[1]);
    OLED_ShowString(0,2,(u8*)arr,16);	
    sprintf(arr,"F=%d MQ=%d",B_Data[2],B_Data[5]);
    OLED_ShowString(0,4,(u8*)arr,16);	
	sprintf(arr,"CO2=%d PM=%d",B_Data[4],B_Data[3]);
    OLED_ShowString(0,6,(u8*)arr,16); 
}
void userHandle(void)
{
  if(wifi_flag == 1)
  {
    currentDataPoint.valueA_F = A_Data[2];//Add Sensor Data Collection
    currentDataPoint.valueA_MQ = A_Data[5];//Add Sensor Data Collection
    currentDataPoint.valueB_F = B_Data[2];//Add Sensor Data Collection
    currentDataPoint.valueB_MQ = B_Data[5];//Add Sensor Data Collection
    currentDataPoint.valueA_temp = A_Data[0];//Add Sensor Data Collection
    currentDataPoint.valueA_Humidity = A_Data[1];//Add Sensor Data Collection
    currentDataPoint.valueB_temp = B_Data[0];//Add Sensor Data Collection
    currentDataPoint.valueB_Humidity = B_Data[1];//Add Sensor Data Collection
    currentDataPoint.valueA_CO2 = A_Data[4];//Add Sensor Data Collection
    currentDataPoint.valueA_PM = A_Data[3];//Add Sensor Data Collection
    currentDataPoint.valueB_CO2 = B_Data[4];//Add Sensor Data Collection
    currentDataPoint.valueB_PM = B_Data[3];//Add Sensor Data Collection   
  }
}

void OnTxDone( void )
{
	Radio.Sleep( );
	printf("���ͳɹ�\r\n");
}

void OnRxDone( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr )
{
	uint8_t i = 0;
	Radio.Sleep( );
	memset(Buffer,0,BUFFER_SIZE);
	BufferSize = size;
	memcpy( Buffer, payload, BufferSize );
	RssiValue = rssi;
	SnrValue = snr;
	
	
	
	
	if(Buffer[0]==0x01)//�ն�һ����Bufferʶ��
	{

			for(i = 0;i < 7;i++){
				A_Data[i] = Buffer[i+1];
			}
			printf("�ն�һOK\r\n\r\n");
//			printf("%d\r\n",A_Data[0]);
//			printf("%d\r\n",A_Data[1]);
//			printf("%d\r\n",A_Data[2]);
//			printf("%d\r\n",A_Data[3]);
//			printf("%d\r\n",A_Data[4]);
//			printf("%d\r\n",A_Data[5]);
//			printf("%d\r\n",Buffer[1]);
//			printf("%d\r\n",Buffer[2]);
//			printf("%d\r\n",Buffer[3]);
//			printf("%d\r\n",Buffer[4]);
//			printf("%d\r\n",Buffer[5]);
//			printf("%d\r\n",Buffer[6]);
			

	}
	
	else if(Buffer[0]==0x02)//�ն˶�����Bufferʶ��
	{
			printf("�ն˶�OK\r\n\r\n");
			for(i = 0;i < 7;i++){
				B_Data[i] = Buffer[i+1];
			}
	}
	else
	{
			printf("���մ���");
	}
	
	
	
	Radio.Rx( RX_TIMEOUT_VALUE );
}

void OnTxTimeout( void )
{
	Radio.Sleep( );
	printf("���ͳ�ʱ!\r\n");
}

void OnRxTimeout( void )
{
	Radio.Sleep( );
	printf("�ȴ���������......\r\n");
	Radio.Rx( RX_TIMEOUT_VALUE );
}

void OnRxError( void )
{
	Radio.Sleep( );
	myPrintf(LEVEL_DEBUG,"���ʹ���\r\n");
	Radio.Rx( RX_TIMEOUT_VALUE );
}
