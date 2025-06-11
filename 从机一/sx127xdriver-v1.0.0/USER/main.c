#include "stm32f10x.h"
#include "HAL_uart.h"
#include "usmart.h"
#include "string.h"
#include "radio.h"
#include "DHT11.h"
#include "delay.h"
#include "APP.h"
#include "adc.h"
#include "sx1276.h"
#include "Led.h"

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



const uint8_t PingMsg[] = "PING";
const uint8_t PongMsg[] = "PONG";

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
int main(void)
{
	bool isMaster = false;		//һ������Ϊ����һ������Ϊ�ӻ�
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
  APP_Init();
  Adc_Init();
  Radio.Rx( RX_TIMEOUT_VALUE );
  while( 1 )
  {
		OLED_Show();
	    Read_Data();
  }
}

void OnTxDone( void )
{
	Radio.Sleep( );
	printf("���ͳɹ�\r\n");
}

void OnRxDone( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr )
{
	Radio.Sleep( );
	memset(Buffer,0,BUFFER_SIZE);
	BufferSize = size;
	memcpy( Buffer, payload, BufferSize );
	RssiValue = rssi;
	SnrValue = snr;
	
	
	
	
	
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
}
