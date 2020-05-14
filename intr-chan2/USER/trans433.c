#include "trans433.h"
#include "uart.h"
#include "radio.h"
#include "stm8s_flash.h"



///**��֤У��**/
//bool CheckCrc(){
//  u8 i;
//  u8 len=G_RX433_Data[11]+12;
//  u8 check=0;
//  for(i=0;i<len;i++){
//    check+=G_RX433_Data[i];
//  }
//  if(check==G_RX433_Data[len]){
//    return TRUE;
//  }else
//    return FALSE;
//}
///**У�����**/
//void CalculateCrc(u8 *p,u8 len){
//  u8 i;
//  u8 crc=0;
////  Str433_tx.data[Str433_tx.data_len]=0;
//  for(i=0;i<len;i++){
////    Str433_tx.data[Str433_tx.data_len]+= *p;
////    p++;
//    crc+=*p++;
//  }
//  Str433_tx.data[Str433_tx.data_len]=crc;
//}
///**�ж��Ƿ���Է���**/

//void Judge_433Send(){
//  if(Str433_flag.busy==1){
//    G_Rf433Send_RandCnt=rand() % (100 -1 + 1) + 1;
//  }else{
//   Str433_flag.tx_counter.sendread_flag=TRUE;
//  }
//}

u8 G_Rf433Send_RandCnt;  //�������
/**���ʹ���**/
void Process_433Send(void){
  static u16 G_SendCount;
  if(Str433_flag.tx_counter.sendread_flag==TRUE){
    Str433_flag.data_sta=inwork;
    Str433_flag.tx_counter.sendread_flag=FALSE;
    RF_StartTx((u8*)(&Str433_tx),Str433_tx.data_len+13,500);
    if(Str433_flag.ack==1){
      G_Rf433Send_RandCnt=(rand() %5) + 2;
      G_SendCount++;
      if(G_SendCount>=SENDCOUNT_MAX){
        if(Str433_flag.wait_master==1){
         G_SendCount=0;      //����������Ļ������η��͵� �������ȡ��
        }else{
          Str433_flag.ack=0;
          G_SendCount=0;
          Cmt2300_GoStby();
          Cmt2300_SetSynWord(G_Endian_LocalAddr); //������Ͷ��û���յ���Ҫ�Ļ�sync
          Cmt2300_GoSleep();
        }
      }
    }
  }
}
/* Manages the slave operation */
void OnSlave(void)
{
//    char str[32];
    
    switch(RF_Process())
    {
    case RF_IDLE:
    {
			//SysSetData.Rf433Send_flag=1;
      RF_StartRx(G_RX433_Data,64,0xFFFFFFFF);
        break;
    }
    
    case RF_RX_DONE:
    {

        Str433_flag.data_sta=workok;
        break;
    }
    
    case RF_TX_DONE:
    {
        
        Str433_flag.data_sta=nowork;
	RF_StartRx(G_RX433_Data,64,0xFFFFFFFF);
        break;
    }
    
    case RF_ERROR:
    {

       Str433_flag.tx_counter.sendread_flag=FALSE;
        break;
    }
    
    default:
        break;
    }
}
/**��ȡ����**/
enum E_processRead_flag Process_433Read(){
  if(Str433_flag.data_sta==workok){
    Str433_flag.data_sta=nowork;
    Str_433_tx_data * recive_data;
    recive_data=(Str_433_tx_data * )G_RX433_Data; //������ǿ���ýṹ��ָ�빹��
    
    if(G_RX433_Data[8]==0x00){ //����������ͶԱ�Ŀ�ĵ�ַ�Ƿ���ȷ
//     if(recive_data->target_addr==Str433_tx.source_addr){
//        if(CheckCrc()==TRUE){
          switch(recive_data->cmd_type){
            case 0x25:
               Touch=recive_data->data[1]; //Touch=G_RX433_Data[11];//ȡ�������Ŀ���״̬
               Str433_tx.target_addr=recive_data->source_addr;
               Str433_tx.device_type=SWITCH;
               Str433_tx.cmd_type=(0x11+0x80);
               Str433_tx.data_len=2;
               Str433_tx.Id=recive_data->Id;
               Str433_tx.data[Str433_tx.data_len]=0;//У��λ
               Str433_tx.data[0]=Touch;
               Str433_tx.data[1]=SW_Chan;
//               G_SendCount=0;
//               CalculateCrc((u8 *)(&Str433_tx),Str433_tx.data_len+12);  //����У��
               delay_ms(100);
               Cmt2300_GoStby();
               Cmt2300_SetResendTime(80);
               Cmt2300_GoSleep();
               Str433_flag.tx_counter.sendread_flag=TRUE;//��������һ��
               Str433_flag.ack=0;  //���ͺ���Ҫ�����ظ�
//               Judge_433Send(); //�ж��Ƿ���Է���
               
               break;
          case 0x23:
             Str433_flag.ack=0;
             Cmt2300_GoStby();
             Cmt2300_SetSynWord(G_Endian_LocalAddr); //�յ��ظ���Ҳ��Ҫ��sync
             Cmt2300_GoSleep();
//             UART1_SendString("�յ��ظ�\r\n");
            break;
          case 0x22:
             Str433_flag.ack=0;
             break;
           case 0x27: //ɾ������
            Str433_flag.ack=0;
            Str433_flag.master_sta=0;
            
            Str433_tx.target_addr=recive_data->source_addr;
            Str433_tx.device_type=SWITCH;
            Str433_tx.cmd_type=(0x27+0x80);
            Str433_tx.Id=recive_data->Id;
            Str433_tx.data_len=1;
            Str433_tx.data[0]=Touch;
            Str433_tx.data[Str433_tx.data_len]=0;//У��λ
            Str433_flag.tx_counter.sendread_flag=TRUE;//��������һ��
            Str433_flag.ack=1;  
              
            FLASH_Unlock(FLASH_MEMTYPE_DATA);  //ֻҪ�յ�ɾ������
            FLASH_EraseByte(MasterAddr_Mark1); //���������ַ�ı�־λ
          default:
            break;
          } 
//        }else{
//          return crcError;
//        }
//      } return master;
    }else if(G_RX433_Data[8]==0x11){ //�����ң�����ͶԱ�ң����Դ��ַ�뱾���Դ��ַ��ͬ��
      if(recive_data->source_addr==G_RmeoteAddr){
//        Touch^=recive_data->data[0];
        if((recive_data->data[0]&0xF0)==0xF0){
          Touch&=recive_data->data[0];
        }else{
          Touch|=recive_data->data[0];
        }
      }return remote;
    }else {
       return notMasterOrRemote;
    }
  }return getData;
}
