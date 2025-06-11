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

#define USE_BAND_433	//选择一个频率
#define USE_MODEM_LORA	//选择lora模式
//#define USE_MODEM_FSK	//选择FSK模式

#define FSK_SINGLE_CARRIER	0	//单载波射频测试，注意：单载波模式必须在FSK模式下
#define LORA_CONTINUE	0	//lora连续发射模式

#if defined( USE_BAND_433 )

#define RF_FREQUENCY                                434000000 // Hz

#elif defined( USE_BAND_780 )

#define RF_FREQUENCY                                780000000 // Hz

#elif defined( USE_BAND_868 )

#define RF_FREQUENCY                                868000000 // Hz

#elif defined( USE_BAND_915 )

#define RF_FREQUENCY                                915000000 // Hz

#else
    #error "请在编译器选项中选择一个频段。"
#endif

#define TX_OUTPUT_POWER                             20        // dBm（发射功率，1278芯片最大设置20dBm）

#if defined( USE_MODEM_LORA )

#define LORA_BANDWIDTH                              0         // [0: 125 kHz,（带宽，带宽设置越宽，传输速率越快，但是传输距离越近）
                                                              //  1: 250 kHz,
                                                              //  2: 500 kHz,
                                                              //  3: Reserved]
#define LORA_SPREADING_FACTOR                       7         // [SF7..SF12] （扩频因子，扩频因子设置越大，传输距离越远，但是传输速率越慢）
#define LORA_CODINGRATE                             1         // [1: 4/5,    （编码率，编码率用于检验传输有无出错，默认使用4/5）
                                                              //  2: 4/6,
                                                              //  3: 4/7,
                                                              //  4: 4/8]
#define LORA_PREAMBLE_LENGTH                        8         // 前导码长度，最常用的有8bit和16bit，这里使用8bit
#define LORA_SYMBOL_TIMEOUT                         5         // 帧超时，帧数为5
#define LORA_FIX_LENGTH_PAYLOAD_ON                  false     // 数据负载混合长度，默认不开启
#define LORA_IQ_INVERSION_ON                        false     // 默认不开启

#elif defined( USE_MODEM_FSK )

#define FSK_FDEV                                    25e3      // Hz
#define FSK_DATARATE                                50e3      // bps
#define FSK_BANDWIDTH                               50e3      // Hz
#define FSK_AFC_BANDWIDTH                           83.333e3  // Hz
#define FSK_PREAMBLE_LENGTH                         5         // Same for Tx and Rx
#define FSK_FIX_LENGTH_PAYLOAD_ON                   false

#else
    #error "请在编译器选项中选择一个频段。"
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
	Res =USART_ReceiveData(USART1);	//读取接收到的数据
		
	if((USART_RX_STA&0x8000)==0){//接收未完成
		if(USART_RX_STA&0x4000){//接收到了0x0d
			if(Res!=0x0a)
				USART_RX_STA=0;//接收错误,重新开始
			else
				USART_RX_STA|=0x8000;	//接收完成了 
			
		}else{ //还没收到0X0D
			if(Res==0x0d)
				USART_RX_STA|=0x4000;
			else{
				USART_RX_BUF[USART_RX_STA&0X3FFF]=Res ;
				USART_RX_STA++;
				if(USART_RX_STA>(USART_REC_LEN-1))
					USART_RX_STA=0;//接收数据错误,重新开始接收	  
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
	bool isMaster = false;		//一个设置为主机一个设置为从机
	LED_Init();
	delay_init();	    	 //延时函数初始化
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);	//NVIC(中断优先级管理)分组配置,注意:这个分组整个程序只能有一次,配置后不要修改,否则会出现很多问题
	HALUart1Init(115200,uart1callBackTest);
	myPrintf(LEVEL_DEBUG,"init ok\r\n");
	if(isMaster){
		myPrintf(LEVEL_DEBUG,"this is master\r\n");
	}else{
		myPrintf(LEVEL_DEBUG,"this is slave\r\n"); 
	}
	usmart_dev.init(SystemCoreClock/1000000);	//初始化USMART	
	
	// LoRa 初始化
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
	SX1276Write( REG_FEILSB, ( SX1276Read( REG_FEILSB ) | (1<<3) ) );	//设置TxContinuousMode 为1
	Radio.Send( Buffer, BufferSize );	//发送任意数据可以进入持续发射模式(带调制信息)，此时无法退出，只有复位才能切换模式
	while(1){
	}
#endif
#if FSK_SINGLE_CARRIER
	//单载波模式，这个必须在FSK模式下
	myPrintf(LEVEL_DEBUG,"enter fsk mode\r\n");
	SX1276Write( REG_FDEVMSB, 0x00 );
	SX1276Write( REG_FDEVLSB, 0x00 );	//设置 fdev 为0
	Radio.Send( Buffer, BufferSize );	//发送任意数据可以进入单载波发射模式，此时无法退出，只有复位才能切换模式
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
	printf("发送成功\r\n");
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
	
	printf("发送超时!\r\n");
}

void OnRxTimeout( void )
{
	Radio.Sleep( );
	printf("等待接收数据......\r\n");
	Radio.Rx( RX_TIMEOUT_VALUE );
}

void OnRxError( void )
{
	Radio.Sleep( );
	myPrintf(LEVEL_DEBUG,"发送错误\r\n");
}
