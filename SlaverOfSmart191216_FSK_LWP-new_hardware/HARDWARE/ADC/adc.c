#include "sys.h"
 
void T_AdcTemp_Init(void)  
{
	ADC_InitTypeDef ADC_InitStructure; 
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB|RCC_APB2Periph_ADC1, ENABLE);	  
	RCC_ADCCLKConfig(RCC_PCLK2_Div6);  
  ADC_DeInit(ADC1);  
 	
	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;	
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;	
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;	
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;	
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;	
	ADC_InitStructure.ADC_NbrOfChannel = 1; 
	ADC_Init(ADC1, &ADC_InitStructure);	
	
	ADC_TempSensorVrefintCmd(ENABLE);
	ADC_Cmd(ADC1, ENABLE);	
	ADC_ResetCalibration(ADC1);	
  while(ADC_GetResetCalibrationStatus(ADC1));	
	ADC_StartCalibration(ADC1);	 
	while(ADC_GetCalibrationStatus(ADC1));		
}

u16 T_Get_AdcTemp(u8 ch)   
{
	ADC_RegularChannelConfig(ADC1, ch, 1, ADC_SampleTime_239Cycles5);		  			    
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);		
	while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC));
	return ADC_GetConversionValue(ADC1);	
}

u16 T_Get_Average(u8 ch,u8 times)
{
	u32 temp_val=0;
	u8 t;
	
	for(t=0;t<times;t++)
	{
		temp_val+=(u32)T_Get_AdcTemp(ch);
		delay_ms(5);
	}
	return temp_val/times;
} 	

short Get_Temprate(void)	//获取内部温度传感器值，扩大100，单位摄氏度
{
	u32 adcx;
	short result;
 	double temperate;
	adcx=T_Get_Average(ADC_Channel_16,10);//20
	temperate=(double)adcx*(3.3/4096);		
	temperate=(1.43-temperate)/0.0043+25;	 
	result=temperate*=100;				
	return result;
}

//===========================================
void T_AdcVolt_Init(void)  
{
	GPIO_InitTypeDef GPIO_InitStructure;
	ADC_InitTypeDef ADC_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB|RCC_APB2Periph_ADC2, ENABLE );	  
	RCC_ADCCLKConfig(RCC_PCLK2_Div6);  //48M/6=8M
  ADC_DeInit(ADC2);  
 
	//PB1                        
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;//ADC_Channel_9
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;	
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;	
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;	
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;	
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;	
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;	
	ADC_InitStructure.ADC_NbrOfChannel = 1; 
	ADC_Init(ADC2, &ADC_InitStructure);	
//	ADC_TempSensorVrefintCmd(ENABLE);
 
	ADC_Cmd(ADC2, ENABLE);	
	ADC_ResetCalibration(ADC2);	
  while(ADC_GetResetCalibrationStatus(ADC2));	
	
	ADC_StartCalibration(ADC2);	 
	while(ADC_GetCalibrationStatus(ADC2));		
}

u16 T_Get_AdcVolt(u8 ch)   
{
	ADC_RegularChannelConfig(ADC2, ch, 1, ADC_SampleTime_239Cycles5);		  			    
 
	ADC_SoftwareStartConvCmd(ADC2, ENABLE);		
	while(!ADC_GetFlagStatus(ADC2, ADC_FLAG_EOC));
	return ADC_GetConversionValue(ADC2);	
}

u16 T_Get_Volt(void)
{
	u16 temp_val=0;
	u8 t;
	for(t=0;t<10;t++)
	{
		temp_val+=T_Get_AdcVolt(ADC_Channel_17);	  //
		delay_ms(5);
	}
	return temp_val/10;
}

u16 T_Get_Average1(u8 ch,u8 times)
{
	u32 temp_val=0;
	u8 t;
	
	for(t=0;t<times;t++)
	{
		temp_val+=(u32)T_Get_AdcVolt(ch);
		delay_ms(5);
	}
	return temp_val/times;
} 	

short Get_Volt(void)	
{
	u32 adcx,adcy;
	short result;
 	double volt;
	adcx=T_Get_Average1(ADC_Channel_8,20);
	adcy=T_Get_Average1(ADC_Channel_17,20); 
	volt=(float)2400*adcx/adcy;
	result=volt;				
	return result;
}

