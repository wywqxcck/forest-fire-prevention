#include "APP.h"

uint8_t Data_Arr[DATA_MAX] = {0x02,15,16,0,0,0,0};
uint8_t Flame_num;
uint8_t MQ_num;
uint8_t PM_num;
uint8_t CO2_num;
extern uint8_t CO2_Data[9];
void APP_Init(void)
{
	Flame_Init();
	DHT11_GPIO_Init();
	OLED_Init();
	MQ_Init();
	usart3_init(9600);
}
void Read_MQ(void)
{
	if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_1) == Bit_RESET) // ¼ì²âµ½»ðÑæ£¨µÍµçÆ½£©
     {
        MQ_num = 1;
     }
    else
     {
        MQ_num = 0;
     }
}
void Read_Flame(void)
{
	if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_12) == Bit_RESET) // ¼ì²âµ½»ðÑæ£¨µÍµçÆ½£©
     {
        Flame_num = 1;
     }
    else
     {
        Flame_num = 0;
     }
}
void Read_PM(void)
{
	PM_num = Get_GP2Y_Average(5);
}
void Read_CO2(void)
{
	CO2_num = CO2_Data[6]*256+CO2_Data[7];
}
void Read_Data(void)
{
	delay_ms(500); 
	 Read_MQ();
	Read_Flame();
	Read_PM(); 
	 Read_CO2();
	DHT11_Read_TempAndHumidity(&DHT11Data);
	Data_Arr[T] = DHT11Data.temp_int;
	Data_Arr[H] = DHT11Data.humi_int;
	Data_Arr[FLAME] = Flame_num;
	Data_Arr[PM] = PM_num;
	Data_Arr[CO2] = CO2_num;
	Data_Arr[PPM] = MQ_num;
//	printf("%d\r\n",Data_Arr[0]);
//	printf("%d\r\n",Data_Arr[1]);
//	printf("%d\r\n",Data_Arr[2]);
//	printf("%d\r\n",Data_Arr[3]);
//	printf("%d\r\n",Data_Arr[4]);
//	printf("%d\r\n",Data_Arr[5]);
//	printf("%d\r\n",Data_Arr[6]);
	Radio.Send( (uint8_t*) Data_Arr, DATA_MAX );
	printf("11111");
	Radio.Rx( RX_TIMEOUT_VALUE );	
	printf("22222");
}
               
void OLED_Show(void)
{
	char arr[100];
	sprintf(arr,"T=%d H=%d",Data_Arr[T],Data_Arr[H]);
	OLED_ShowString(0,0,(u8*)arr,16);
	sprintf(arr,"flame=%d MQ=%d",Data_Arr[FLAME],Data_Arr[PPM]);
	OLED_ShowString(0,2,(u8*)arr,16);
	sprintf(arr,"PM=%d",Data_Arr[PM]);
	OLED_ShowString(0,4,(u8*)arr,16);
	sprintf(arr,"CO2=%d",Data_Arr[CO2]);
	OLED_ShowString(0,6,(u8*)arr,16);
//	
}	
