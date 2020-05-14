/**
  ******************************************************************************
  * File Name          : TIM.c
  * Description        : This file provides code for the configuration
  *                      of the TIM instances.
  ******************************************************************************
  * This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether 
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * Copyright (c) 2018 STMicroelectronics International N.V. 
  * All rights reserved.
  *
  * Redistribution and use in source and binary forms, with or without 
  * modification, are permitted, provided that the following conditions are met:
  *
  * 1. Redistribution of source code must retain the above copyright notice, 
  *    this list of conditions and the following disclaimer.
  * 2. Redistributions in binary form must reproduce the above copyright notice,
  *    this list of conditions and the following disclaimer in the documentation
  *    and/or other materials provided with the distribution.
  * 3. Neither the name of STMicroelectronics nor the names of other 
  *    contributors to this software may be used to endorse or promote products 
  *    derived from this software without specific written permission.
  * 4. This software, including modifications and/or derivative works of this 
  *    software, must execute solely and exclusively on microcontroller or
  *    microprocessor devices manufactured by or for STMicroelectronics.
  * 5. Redistribution and use of this software other than as permitted under 
  *    this license is void and will automatically terminate your rights under 
  *    this license. 
  *
  * THIS SOFTWARE IS PROVIDED BY STMICROELECTRONICS AND CONTRIBUTORS "AS IS" 
  * AND ANY EXPRESS, IMPLIED OR STATUTORY WARRANTIES, INCLUDING, BUT NOT 
  * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A 
  * PARTICULAR PURPOSE AND NON-INFRINGEMENT OF THIRD PARTY INTELLECTUAL PROPERTY
  * RIGHTS ARE DISCLAIMED TO THE FULLEST EXTENT PERMITTED BY LAW. IN NO EVENT 
  * SHALL STMICROELECTRONICS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, 
  * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
  * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
  * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "tim.h"
//#include "FreeRTOSConfig.h"
#include "gpio.h"
#include "FreeRTOS.h"
#include "task.h"
#include "protocol.h"
#include "usart.h"
#include "ir.h"
//extern volatile u8 	 uc_vgTim2CountMode; 

volatile 				u16  us_vgPulseCnt; //记录电平时长计数器50us

extern volatile u8 uc_vgTim1CountRetryRF;

extern TaskHandle_t	SendCMTTaskHandle;
extern TaskHandle_t	StudyTaskHandler;
extern TaskHandle_t	KeyTaskHandle;
extern vu16 us_vgTim1CountHeart;
extern vu8 	uc_gUartTim4OutCunt;
extern vu8	uc_vgStudyTim1Out;
extern vu16 us_vgTim1CountTemp;
//按键按下标志和时间
extern					u8	uc_gPressFlag;
extern 					vu8	uc_vgTim1KeyCount;
//处理WIFI任务句柄
extern TaskHandle_t  ProcessWifiTaskHandle;
extern u8 uc_gUartSendLock;

extern volatile EM_ConState em_gWifiState;
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

TIM_HandleTypeDef htim1;
TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;
TIM_HandleTypeDef htim4;

volatile unsigned long long FreeRTOSRunTimeTicks;


/* TIM1 init function */
void MX_TIM1_Init(u32 Prescaler,u32 Period)
{
  TIM_ClockConfigTypeDef sClockSourceConfig;
  TIM_MasterConfigTypeDef sMasterConfig;

  htim1.Instance = TIM1;
  htim1.Init.Prescaler = Prescaler;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = Period;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim1) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }
	HAL_TIM_Base_Start_IT(&htim1); 
//		__HAL_TIM_ENABLE(&htim1);
}
/* TIM2 init function */
void MX_TIM2_Init(u32 Prescaler,u32 Period)
{
  TIM_ClockConfigTypeDef sClockSourceConfig;
  TIM_MasterConfigTypeDef sMasterConfig;

  htim2.Instance = TIM2;
  htim2.Init.Prescaler = Prescaler;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = Period;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;

  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }
  HAL_TIM_Base_Start_IT(&htim2); 
	__HAL_TIM_DISABLE(&htim2);
}
/* TIM3 init function */
void MX_TIM3_Init(u32 Prescaler,u32 Period)
{
  TIM_ClockConfigTypeDef sClockSourceConfig;
  TIM_MasterConfigTypeDef sMasterConfig;

  htim3.Instance = TIM3;
  htim3.Init.Prescaler = Prescaler;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = Period;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }
  HAL_TIM_Base_Start_IT(&htim3); 
	__HAL_TIM_DISABLE(&htim3);
	
}
/* TIM4 init function */
void MX_TIM4_Init(u32 Prescaler,u32 Period)
{
  TIM_ClockConfigTypeDef sClockSourceConfig;
  TIM_MasterConfigTypeDef sMasterConfig;

  htim4.Instance = TIM4;
  htim4.Init.Prescaler = Prescaler;
  htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim4.Init.Period = Period;
  htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim4) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim4, &sClockSourceConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim4, &sMasterConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }
  HAL_TIM_Base_Start_IT(&htim4); 
	__HAL_TIM_DISABLE(&htim4);
	//__HAL_TIM_ENABLE(&htim4);
	
}
/**
  *FreeRTOS与运行时间和任务状态收集有关的配置选项的一个时基
  */
//void ConfigureTimerForRunTimeStats(void)
//{
//	FreeRTOSRunTimeTicks = 0;
//	MX_TIM3_Init(50-1,64-1); //周期50us，频率20K
//}
/**
  * Initialize Time Clock
  */
void HAL_TIM_Base_MspInit(TIM_HandleTypeDef* tim_baseHandle)
{

  if(tim_baseHandle->Instance==TIM1)
  {
  /* USER CODE BEGIN TIM1_MspInit 0 */

  /* USER CODE END TIM1_MspInit 0 */
    /* TIM1 clock enable */
    __HAL_RCC_TIM1_CLK_ENABLE();
  /* USER CODE BEGIN TIM1_MspInit 1 */

  /* USER CODE END TIM1_MspInit 1 */
  }
  else if(tim_baseHandle->Instance==TIM2)
  {
  /* USER CODE BEGIN TIM2_MspInit 0 */

  /* USER CODE END TIM2_MspInit 0 */
    /* TIM2 clock enable */
    __HAL_RCC_TIM2_CLK_ENABLE();
//		__HAL_RCC_TIM2_CLK_DISABLE();
  /* USER CODE BEGIN TIM2_MspInit 1 */

  /* USER CODE END TIM2_MspInit 1 */
  }
  else if(tim_baseHandle->Instance==TIM3)
  {
  /* USER CODE BEGIN TIM3_MspInit 0 */

  /* USER CODE END TIM3_MspInit 0 */
    /* TIM3 clock enable */
    __HAL_RCC_TIM3_CLK_ENABLE();
//		__HAL_RCC_TIM3_CLK_DISABLE(); //平常TIM2和TIM3是关闭的
  /* USER CODE BEGIN TIM3_MspInit 1 */

  /* USER CODE END TIM3_MspInit 1 */
  }
	else if(tim_baseHandle->Instance == TIM4)
	{
		__HAL_RCC_TIM4_CLK_ENABLE();
	}
}

void HAL_TIM_Base_MspDeInit(TIM_HandleTypeDef* tim_baseHandle)
{

  if(tim_baseHandle->Instance==TIM1)
  {
  /* USER CODE BEGIN TIM1_MspDeInit 0 */

  /* USER CODE END TIM1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_TIM1_CLK_DISABLE();
  /* USER CODE BEGIN TIM1_MspDeInit 1 */

  /* USER CODE END TIM1_MspDeInit 1 */
  }
  else if(tim_baseHandle->Instance==TIM2)
  {
  /* USER CODE BEGIN TIM2_MspDeInit 0 */

  /* USER CODE END TIM2_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_TIM2_CLK_DISABLE();

    /* TIM2 interrupt Deinit */
//    HAL_NVIC_DisableIRQ(TIM2_IRQn);
  /* USER CODE BEGIN TIM2_MspDeInit 1 */

  /* USER CODE END TIM2_MspDeInit 1 */
  }
  else if(tim_baseHandle->Instance==TIM3)
  {
  /* USER CODE BEGIN TIM3_MspDeInit 0 */

  /* USER CODE END TIM3_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_TIM3_CLK_DISABLE();
  /* USER CODE BEGIN TIM3_MspDeInit 1 */

  /* USER CODE END TIM3_MspDeInit 1 */
  }
	else if(tim_baseHandle->Instance == TIM4)
	{
		__HAL_RCC_TIM4_CLK_DISABLE();
	}
} 

/**定时器通用回调函数*/
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
/**TIM2中断处理**/	
	/*定时器1回调 50us 主要用做记录*/
  if(htim == (&htim2)){ //50us
	   //printf("TIM2 interrupt!\r\n");
		if(uc_vgTim2CountMode == INCREASE){
			us_vgPulseCnt++;
		}else if(uc_vgTim2CountMode == DECREASE){
			if(us_vgPulseCnt > 0)
				us_vgPulseCnt--;
		}
//		HAL_GPIO_TogglePin(GPIOB,GPIO_PIN_7);
	}
/**TIM3中断处理**/	
	else if(htim == (&htim3)){
		if(uc_vgSendIRFlag == 1){
			HAL_GPIO_TogglePin(GPIOB,GPIO_PIN_9); //翻转IR的输出相当于载波
		}
	}
/**TIM4串口超时定时中断**/	
	else if(htim == (&htim4)){
		//HAL_GPIO_TogglePin(GPIOB,GPIO_PIN_7);
				//串口接收超时
		if(uc_gUartTim4OutCunt > 0){
			uc_gUartTim4OutCunt--;
			if(uc_gUartTim4OutCunt == 0){
				/*通过任务通知模拟二值信号量占用更少的RAM*/
				vTaskNotifyGiveFromISR(ProcessWifiTaskHandle,&xHigherPriorityTaskWoken);
			 __HAL_TIM_DISABLE(&htim4);	
			 USART_RX_STA = 0;
			 uc_gUartSendLock = 0;
			}
//解除发送锁
			
		}
		portYIELD_FROM_ISR(xHigherPriorityTaskWoken);//判断是否有高优先级的任务需要切换
	}
/**TIM1中断处理  TIM1_RELOAD_MS **/
	else if(htim == (&htim1)){
//	  HAL_GPIO_TogglePin(GPIOB,GPIO_PIN_7);
		//无线重发
		if(uc_vgTim1CountRetryRF > 0){
			
			if(uc_vgTim1CountRetryRF == 1){
				vTaskNotifyGiveFromISR(SendCMTTaskHandle,&xHigherPriorityTaskWoken);
			}
			uc_vgTim1CountRetryRF--;
		}
		//按键按下时间
		if(uc_gPressFlag == 1){
			uc_vgTim1KeyCount++;
			if(uc_vgTim1KeyCount >= PRESS_LONG){
				MCU_LED_EN;
			}
		}

		if(uc_vgStudyTim1Out > 0){
			uc_vgStudyTim1Out--;
			if(uc_vgStudyTim1Out == 1){ //学习超时返回失败
				StopStudy();
//				xTaskNotifyFromISR(StudyTaskHandler,STUDY_FAILED,eSetBits,&xHigherPriorityTaskWoken);
			}
		}
		//心跳上报计时在心跳任务中上报
		if(us_vgTim1CountHeart > 0){
			us_vgTim1CountHeart--;
			if(us_vgTim1CountHeart == 1){
//				ESP_ERS_EN;
//				MCU_LED_DIS;
//				em_gWifiState = connect_router;
			}
		}
		if(us_vgTim1CountTemp > 0){
			us_vgTim1CountTemp--;
		}
		portYIELD_FROM_ISR(xHigherPriorityTaskWoken);//判断是否有高优先级的任务需要切换
	}
}
/* USER CODE BEGIN 1 */
 
/* USER CODE END 1 */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
