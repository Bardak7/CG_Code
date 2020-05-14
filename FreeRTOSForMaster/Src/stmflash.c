#include "stmflash.h"
#include "delay.h"
#include "FreeRTOS.h"
//#include "usart.h"

void STMFLASH_Write_NoCheck(u32 WriteAddr,u16 *pBuffer,u16 NumToWrite);

/**
  * @brief  STMFLASH_ReadHalfWord  ��ȡָ����ַ�İ���(16λ����)
  * @param  faddr:����ַ 
  * @retval ��Ӧ����.
  */ 

u16 STMFLASH_ReadHalfWord(u32 faddr)
{
	return *(vu16*)faddr; 
}
#if STM32_FLASH_WREN	//���ʹ����д   
/**
  * @brief  STMFLASH_Erase_Word  α����һ�������� ������������д0 ��ֱ��д �����������ĵ�ַ��
  * @param 	WriteAddr:��ʼ��ַ
  * @retval None
  */ 
void STMFLASH_Erase_Word(u32 WriteAddr)
{
	u16 us_zeroBuf[2] = {0,0,};
	HAL_FLASH_Unlock();					//����
	STMFLASH_Write_NoCheck(WriteAddr,us_zeroBuf,2);
	HAL_FLASH_Lock();		//����
}

/**
  * @brief  STMFLASH_Write_NoCheck  ������д��
  * @param 	WriteAddr:��ʼ��ַ
	*					pBuffer:����ָ��
	*					NumToWrite:����(16λ)��
  * @retval None
  */
void STMFLASH_Write_NoCheck(u32 WriteAddr,u16 *pBuffer,u16 NumToWrite)   
{ 			 		 
	u16 i;
	portENTER_CRITICAL();
	for(i=0;i<NumToWrite;i++)
	{
		HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD,WriteAddr,pBuffer[i]);
	    WriteAddr+=2;//��ַ����2.
	} 
	portEXIT_CRITICAL();
} 

#if STM32_FLASH_SIZE<256
#define STM_SECTOR_SIZE 1024 //�ֽ�
#else 
#define STM_SECTOR_SIZE	2048
#endif		 
u16 STMFLASH_BUF[STM_SECTOR_SIZE/2];//�����2K�ֽ�
/**
  * @brief  STMFLASH_Write ��ָ����ַ��ʼд��ָ�����ȵ�����
  * @param 	WriteAddr:��ʼ��ַ(�˵�ַ����Ϊ2�ı���!!)
	*					pBuffer:����ָ��
	*					NumToWrite:����(16λ)��(����Ҫд���16λ���ݵĸ���.)			
  * @retval None
  */
void STMFLASH_Write(u32 WriteAddr,u16 *pBuffer,u16 NumToWrite)	
{
	u32 secpos;	   //������ַ
	u16 secoff;	   //������ƫ�Ƶ�ַ(16λ�ּ���)
	u16 secremain; //������ʣ���ַ(16λ�ּ���)	   
 	u16 i;    
	u32 offaddr;   //ȥ��0X08000000��ĵ�ַ
	
	if(WriteAddr<STM32_FLASH_BASE||(WriteAddr>=(STM32_FLASH_BASE+1024*STM32_FLASH_SIZE)))return;//�Ƿ���ַ
	
	HAL_FLASH_Unlock();					//����
	offaddr=WriteAddr-STM32_FLASH_BASE;		//ʵ��ƫ�Ƶ�ַ.
	secpos=offaddr/STM_SECTOR_SIZE;			//������ַ  0~127 for STM32F103RBT6
	secoff=(offaddr%STM_SECTOR_SIZE)/2;		//�������ڵ�ƫ��(2���ֽ�Ϊ������λ.)
	secremain=STM_SECTOR_SIZE/2-secoff;		//����ʣ��ռ��С   
	if(NumToWrite<=secremain)secremain=NumToWrite;//�����ڸ�������Χ
	while(1) 
	{	
		STMFLASH_Read(secpos*STM_SECTOR_SIZE+STM32_FLASH_BASE,STMFLASH_BUF,STM_SECTOR_SIZE/2);//������������������
		for(i=0;i<secremain;i++)	//У������
		{
			if(STMFLASH_BUF[secoff+i]!=0XFFFF)break;//����Ƿ���Ҫ����
		}
		if(i<secremain)				//��Ҫ����
		{
			portENTER_CRITICAL();
			FLASH_PageErase(secpos*STM_SECTOR_SIZE+STM32_FLASH_BASE);	//�����������
			FLASH_WaitForLastOperation(FLASH_WAITETIME);            	//�ȴ��ϴβ������
			CLEAR_BIT(FLASH->CR, FLASH_CR_PER);							//���CR�Ĵ�����PERλ���˲���Ӧ����FLASH_PageErase()����ɣ�
																		//����HAL�����沢û������Ӧ����HAL��bug��
			portEXIT_CRITICAL();
			
			for(i=0;i<secremain;i++)//����
			{
				STMFLASH_BUF[i+secoff]=pBuffer[i];	  
			}
			STMFLASH_Write_NoCheck(secpos*STM_SECTOR_SIZE+STM32_FLASH_BASE,STMFLASH_BUF,STM_SECTOR_SIZE/2);//д����������  
		}else 
		{
			FLASH_WaitForLastOperation(FLASH_WAITETIME);       	//�ȴ��ϴβ������
			STMFLASH_Write_NoCheck(WriteAddr,pBuffer,secremain);//д�Ѿ������˵�,ֱ��д������ʣ������. 
		}
		if(NumToWrite==secremain)break;//д�������
		else//д��δ����
		{
			secpos++;				//������ַ��1
			secoff=0;				//ƫ��λ��Ϊ0 	 
		   	pBuffer+=secremain;  	//ָ��ƫ��
			WriteAddr+=secremain*2;	//д��ַƫ��(16λ���ݵ�ַ,��Ҫ*2)	   
		   	NumToWrite-=secremain;	//�ֽ�(16λ)���ݼ�
			if(NumToWrite>(STM_SECTOR_SIZE/2))secremain=STM_SECTOR_SIZE/2;//��һ����������д����
			else secremain=NumToWrite;//��һ����������д����
		}	
    break;
	}	
	HAL_FLASH_Lock();		//����
}
#endif


/**
  * @brief  STMFLASH_Read ��ָ����ַ��ʼ����ָ�����ȵ�����
  * @param	ReadAddr:��ʼ��ַ
	*					pBuffer:����ָ��
	*					NumToWrite:����(16λ)��
  * @retval None
  */
void STMFLASH_Read(u32 ReadAddr,u16 *pBuffer,u16 NumToRead)   	
{
	u16 i;
	portENTER_CRITICAL();
	for(i=0;i<NumToRead;i++)
	{
		pBuffer[i]=STMFLASH_ReadHalfWord(ReadAddr);//��ȡ2���ֽ�.
		ReadAddr+=2;//ƫ��2���ֽ�.	
	}
	portEXIT_CRITICAL();
}

//////////////////////////////////////////������///////////////////////////////////////////
//WriteAddr:��ʼ��ַ
//WriteData:Ҫд�������
void Test_Write(u32 WriteAddr,u16 WriteData)   	
{
	STMFLASH_Write(WriteAddr,&WriteData,1);//д��һ���� 
}
















