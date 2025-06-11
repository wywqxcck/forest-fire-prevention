#include "DHT11.h"

void DHT11_GPIO_Init(void);
static void DHT11_Mode_IPU(void);
static void DHT11_Mode_Out_PP(void);
static void DHT11_Reset(void);
static uint8_t DHT11_Readbit(void);
static uint8_t DHT11_ReadByte(void);
uint8_t DHT11_Read_TempAndHumidity(DHT11_Data_TypeDef *DHT11_Data);



// ʹ�ô˱�������DHT11����
DHT11_Data_TypeDef DHT11Data;

#define	DHT11_Dout_GPIO_CLK     	RCC_APB2Periph_GPIOB
#define	DHT11_Dout_GPIO_PORT     	GPIOB
#define	DHT11_Dout_GPIO_PIN      	GPIO_Pin_13

#define	DHT11_Dout_0	          	GPIO_ResetBits(DHT11_Dout_GPIO_PORT, DHT11_Dout_GPIO_PIN) 
#define	DHT11_Dout_1	         		GPIO_SetBits(DHT11_Dout_GPIO_PORT, DHT11_Dout_GPIO_PIN) 
#define	DHT11_Dout_IN()	     			GPIO_ReadInputDataBit(DHT11_Dout_GPIO_PORT, DHT11_Dout_GPIO_PIN)

/**
  * @brief  DHT11��ʼ��
  * @param  None
  * @retval None
  * @note   None
*/
void DHT11_GPIO_Init(void)
{		
    GPIO_InitTypeDef GPIO_InitStructure; 

    RCC_APB2PeriphClockCmd(DHT11_Dout_GPIO_CLK | RCC_APB2Periph_AFIO, ENABLE);	
														   
    GPIO_InitStructure.GPIO_Pin = DHT11_Dout_GPIO_PIN;	
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;   
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz; 
    GPIO_Init(DHT11_Dout_GPIO_PORT, &GPIO_InitStructure );

	DHT11_Dout_1;
}
/**
	* @brief  �޸�����Ϊ��������
  * @param  None
  * @retval None
  * @note   None
*/
static void DHT11_Mode_IPU(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
	
    GPIO_InitStructure.GPIO_Pin = DHT11_Dout_GPIO_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; 
    GPIO_Init(DHT11_Dout_GPIO_PORT, &GPIO_InitStructure);
}
/**
	* @brief  �޸�����Ϊ�������
  * @param  None
  * @retval None
  * @note   None
*/
static void DHT11_Mode_Out_PP(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
														   
    GPIO_InitStructure.GPIO_Pin = DHT11_Dout_GPIO_PIN;	
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;   
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(DHT11_Dout_GPIO_PORT, &GPIO_InitStructure);	 	 
}
/**
	* @brief  DHT11��λ
  * @param  None
  * @retval None
  * @note   None
*/
static void DHT11_Reset(void)
{
	DHT11_Mode_Out_PP();
	DHT11_Dout_0;
	delay_ms(20);
	DHT11_Dout_1;
	delay_us(30);
}
/**
  * @brief  ���DHT11״̬
  * @param  None
  * @retval 0: ����,1: δ��⵽�豸
	* @note   None
*/
uint8_t DHT11_Check(void)
{
  uint8_t timeout = 0;
	DHT11_Reset();
	DHT11_Mode_IPU();
  while (DHT11_Dout_IN() && timeout < 100)
  {
      timeout++;
      delay_us(1);
  }
  if(timeout >= 100)
  {
      return 1;
  }
  timeout = 0;
  while (!DHT11_Dout_IN() && timeout < 100)
  {
      timeout++;
      delay_us(1);
  };
  if(timeout >= 100)
  {
      return 1;
  }
  return 0;
}
/**
  * @brief  DHT11��ȡ1bit
  * @param  None
  * @retval ���ض�ȡ��bit
  * @note   None
*/
static uint8_t DHT11_Readbit(void)
{
	uint8_t timeout = 0;
	while (DHT11_Dout_IN() && timeout < 100)
  {
    timeout++;
    delay_us(1);
  }
	timeout = 0;
	while (!DHT11_Dout_IN() && timeout < 100)
  {
    timeout++;
    delay_us(1);
  }
	delay_us(40);
	if(DHT11_Dout_IN())
		return 1;
  return 0;
}
/**
  * @brief  DHT11��ȡ1Byte
  * @param  None
  * @retval None
  * @note   ���ض�ȡ��Byte
*/
static uint8_t DHT11_ReadByte(void)
{
  uint8_t dat = 0;
	uint8_t i;
  for (i = 1; i <= 8; i++)
  {
    dat <<= 1;
		dat |= DHT11_Readbit();
  }
  return dat;
}

/**
  * @brief  DHT11��ȡ40bit ȫ������
  * @param  None
  * @retval None
  * @note   8bit ʪ������ + 8bit ʪ��С�� + 8bit �¶����� + 8bit �¶�С�� + 8bit У���
*/
uint8_t DHT11_Read_TempAndHumidity(DHT11_Data_TypeDef *DHT11_Data)
{  
	//DHT11_Reset();
  if(DHT11_Check() == 0)
	{
		DHT11_Data->humi_int = DHT11_ReadByte();
		DHT11_Data->humi_deci = DHT11_ReadByte();
		DHT11_Data->temp_int = DHT11_ReadByte();
		DHT11_Data->temp_deci = DHT11_ReadByte();
		DHT11_Data->check_sum = DHT11_ReadByte();
#ifdef DEBUG_printf
	//printf("tem:%d  hum:%d\n", DHT11_Data->temp_int, DHT11_Data->humi_int);
#endif
		if(DHT11_Data->check_sum == DHT11_Data->humi_int + DHT11_Data->humi_deci + DHT11_Data->temp_int + DHT11_Data->temp_deci)
		{
			return 0;
		}
		
  }else{
    return 1;
  }
	return 1;
}




