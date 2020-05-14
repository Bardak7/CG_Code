#ifndef __PROTOCOL_H
#define __PROTOCOL_H
#include "sys.h"
#include "stdint.h"
#include "rf433comm.h"
#include "FreeRTOS.h"
#include "rtc.h"

//�汾��Ϊ��һ�汾
#define	VER											1
 //WIFI���ͻ���
#define WIFI_SEND_BUF_MAX				256

/**��ʱ�������**/
#define TIMINGTASK_DATALEN_MAX  200  	//�������ݴ�[20](��ͷ��)��0x21 0x71���ֻ����200���ֽڡ�
#define TIMINGTASK_LIST_LEN_MAX	10		//���嶨ʱ���������10����Ŀ
#define TIMINGTASK_CMD_MAX			3		//������ඨʱ������԰���3����������
#define TIMINGTASK_HEAD_LEN_MAX	50		//����ͷ������������ǰ���������ݣ�



/**���طֻ��б�������**/
#define	SWITCH_LIST_MAX					10		//�����10�������б�
#define	SLAVE_LIST_MAX					10		//�����10���ֻ��б�

/**��Ӧ�ı����ַ���**/
//�˴�Ϊ������UID
#define DEVIDL_FLASH_ADDR 			0x1FFFF7E8	
#define DEVIDH_FLASH_ADDR 			0x1FFFF7EC
#define DEVIDM_FLASH_ADDR 			0x1FFFF7F0
//�˴�Ϊ������¼��4���ֽ�
#define ROLLING_CODE_ADDR				0x0800FFF0

//�豸�Ƿ�����Flash�����ַ
#define LOCK_FLAG_FLASH_ADDR   	0x0801B004
//�ֻ������ַ�ͼ��  interval
#define SLAVE_LIST_FLASH_ADDR  	0x0801B100
//4���ֽڵļ��
#define	SLAVE_LIST_FLASH_INVL		4
//���ر����ַ�ͼ��  interval
#define SWITCH_LIST_FLASH_ADDR  0x0801B200
#define SWITCH_LIST_FLASH_INVL	6
/*
				��ʱ����Flash�洢��ʽ����
	*		��		�ܳ���		(short)	�����Ƶ� !q �ĳ���(����!q)
	*		|			ͷ������	(short)	�����Ƶ����ڵĳ���(��������)
	*		|			״̬		(short)    //������һ��short�����淽��flash
	*		|			����		(len+content)	
	*		|			Ƶ��		(Byte)
	*					ʱ��		(len+content)
	*					����		(len+content)
	*					��������(len+ content)
	*							--	ID				(long)
	*							--	������			(len+conten)
	*							--	ִ����ʱ		(short)
	*		��		״̬		(Byte)
	*/
#define TIME_LIST_FLASH_ADDR   	0x0801C000
//�ֻ� ���� ��ʱ�����б����� 4KFlash�ռ�

/*��ʱ������Ϊ�������*/
//�����Ҫִ�е������ʱ���
#define	RECENT_TASK_UTC					ul_gTimeTaskUnixTimArr[TIMINGTASK_LIST_LEN_MAX]
//RTC���������� 300��5����һ�� ������ʱ��
#define ALARM_CYCLE							300

/*ADC��ȡ�ڲ��¶��������ʱ��*/
#define	TEMP_CYCLE			3000/TIM1_RELOAD_MS
/*����ʱ�����*/
//����5��ʱ��
#define	PRESS_LONG			5000/TIM1_RELOAD_MS
//
#define LONG_PRESS_BIT	0x01
#define SHORT_PRESS_BIT	0x02

/*ѧϰ���*/
//10S��ȥ ��ʱ���������Ǽ���ֵ
#define STUDY_TIME_MAX					10000/TIM1_RELOAD_MS
#define STUDY_SUCCUESS					0x01UL
#define STUDY_FAILED						0x02UL
#define STUDY_SEND433						0x03
#define STUDY_SENDIR						0x04

#define STUDY_MODE_IR		1U
#define STUDY_MODE_RF		2U

//***����ʱ�� 6000 ms **/
#define HEART_TIM				6000/TIM1_RELOAD_MS
//*ѧϰʱ��������ݲ�����ݲ�϶�����ͬһ������ 50*7 =350us*/
#define PULSE_TOL				7			
#define CUSTOM_LIB			0x5AA5

//typedef struct IrStudy//����ѧϰ�����ݽṹ-28
//{
//	u16 HeadHCnt;//ͷ��ߵ�ƽʱ�ӣ�n*13.16us
//	u16 HeadLCnt;//ͷ��͵�ƽʱ�ӣ�n*13.16us
//	u16 Data1HCnt;  
//	u16 Data1LCnt;
//	u16 Data0HCnt;
//	u16 Data0LCnt;
//	u8 Type;  //ir-1,433-2,315-3
//	u8 Len;
//	u8 Data[50];
//}IrStudy_DATA;

/*wifi�����ṹ��*/
typedef struct{
	u16 dataLock;
	u16 reportLen;
	u8  reportBuf[WIFI_SEND_BUF_MAX];
}ST_Report;

/*�ֻ���Ϣ*/
typedef struct{
	u32 slaveID;
	u8	slaveState;
}ST_SlaveInfo;
/*������Ϣ*/
typedef struct{
	u32 switchID;
	u8	channle;
	u8	state;
}ST_SwitchInfo;
/*���������Ϣ�ṹ��*/
typedef struct{
  uint64_t			deviceID;
	u16						version;
	u16						lockInfo;	//�Ƿ���������	
	u8 						temp;			//�¶�
	u8						switchCount;
	u8						slaveCount;
	u8						timeTaskCount;
	ST_SwitchInfo	stSwitch[SWITCH_LIST_MAX];
	ST_SlaveInfo	stSlave	[SLAVE_LIST_MAX];
}ST_HostInfo;
/***********************************************************************************************
************************************************************************************************/
																								/*���µ�Ϊ��ȡ�ĸ�ʽ��*/
/**
  *��ʽ����ͷ ��20 ���ֽ�
  */
typedef struct{
	u32 head;  				//4λ��ͷ 		�̶�Ϊ0x24676340
	u16 version;  		//2λ�汾 		ĿǰΪ1
	u16 cmd;					//2λ���� 		������������ļ��·�Define CMD
	u16 sn;						//2λ���к�	
	u16 len;					//2λ����
//	uint64_t reserve;	//8λ����
	u32 reserveL;       //���ڽṹ�����ԭ�������64λ�����len�������λ���������������ݴ���
	u32 reserveH; 
	u8 next;		//ʣ�µ������ݿ�ͽ�������0x7121  !q ��Щָ�������ݿ�ĵ�һ���ֽ�
}ST_WifiDataPackHead;


/*
 *����������һ��long long ��64λ��ַ
 * �㲥��� ��ӷֻ����豸��������ѯ�������ֻ�״̬����ȡ�������ȣ���ӿ���
*/
typedef struct{
	u32	hostID2;
	u32 hostID1;
	u8	next;
}ST_Long;
/*
 * Long int int
 * ��ȡ��ʱ�б�  �����Ķ�ʱ״̬�� ��ȡ������ʱ����ɾ����ʱ����
 * int+int 				�� int+char		�� int
*/
typedef struct{
//	u32	hostID1;
//	u32 hostID2;
	union{
		u32 taskID;
		u32	startPos;
	}intData1;
//	u32 intdata1;
	union{
		u32 amount;
		u8	state;
	}intData2;
//	u16 intdata2;
}ST_IntInt;
/*
 *	������������long long�� 64λ�� �� �ӻ���ַ
 * ɾ���ֻ��� ֹͣѧϰ�� ɾ�����ض���
 *               ��ӵ����� ��ѧϰ������һλ�����ƿ���������һλ
 * long + char + char
*/
typedef struct{
//  u32	hostID1;
//	u32 hostID2;
	u32	slaveIDL;
	u32	slaveIDH;
	u8	studyModeOrChannel; //ѧϰ�����ѧϰ���ͻ����ǿ��ص�·��
	u8	state;							//��Ҫ���ش�ɵ�״̬
}ST_LongCharChar; //�˴���ʽ�����ֻ���ַ��

/*
 *��������������long long�� 64λ�� �� �ӻ���ַ����һ���ַ������� �ַ�������λ���Ⱥ��������
 * �����豸
 * long+String(short+content)
*/
typedef struct{
//  u32	hostID1;
//	u32 hostID2;
	u32	slaveIDL;
	u32	slaveIDH;
	u16 stringLen;
	u8	content; //����ָ�����Stirng�ĵ�һ��Ԫ�ؿ�ȡָ����������
}ST_LongStirng;
/*
 *��������long long�� 64λ�� �� �ӻ���ַ����һ��2�ֽڵİ汾�ż�һ��String
 * long + short +String(short+content)
*/
typedef struct{
//	u32	hostID1;
//	u32 hostID2;
	u32	slaveIDL;
	u32	slaveIDH;
	u16	newVersion;
	u16 stringLen;
	u8	content; //����ָ�����Stirng�ĵ�һ��Ԫ�ؿ�ȡָ����������
}ST_LongShortString;
/*
 *һ��long long�� 64λ�� �� �ӻ���ַ����һ����������� �������������һ�� 64λ��ַ
 * ������ѯ���ֻ�״̬ ������ѯ����״̬
 * array (short+long)
*/
typedef struct{
//	u32	hostID1;
//	u32 hostID2;
	u16	arraylen; //
	u16	usIDLL;
	u16 usIDLH;
	u16	usIDHL;
	u16	usIDHH;
}ST_ShortLong;
																									/*���ϵ�Ϊ��ȡ�ĸ�ʽ��*/
/***********************************************************************************************
************************************************************************************************/
													/*���µ�Ϊ���͵ĸ�ʽ��*/

/**
  *	Long +epilog
	* �������ͨ�� ������½
  */
typedef struct{
	u32 idL;
	u32 idH;
	u16 epilog;
}ST_LongS;
/**
  *Int+Long +epilog
	* �㲥��� ��ӷֻ�  �Ļظ�
  */
typedef struct{
	u32 result;    	//�ɹ�����1��ʧ�ܷ���0
//	u8	next;				//ָ����һ��Ԫ��
	u32 idL;
	u32 idH;
	u16 epilog;
}ST_IntLong;
/**
  *Int+Long +epilog
	* �㲥��� ��ӷֻ�  �Ļظ�
  */
typedef struct{
	u32 result;    	//�ɹ�����1��ʧ�ܷ���0
//	u8	next;				//ָ����һ��Ԫ��
	u32 idL;
	u32 idH;
	u8	channle; 	//���ؿ���·��
	u8 	state;		//���ؿ���״̬
	u16 epilog;
}ST_IntLongCC;
/**
  *Int+Int +epilog
	* ��Ӷ�ʱ����  �Ļظ�
  */
typedef struct{
	u32 result;    	//�ɹ�����1��ʧ�ܷ���0

	u32 taskID;
	u16 epilog;
}ST_IntIntS;
/**
  *Int +epilog
	* ɾ����ʱ����	�޸Ķ�ʱ����	�޸Ķ�ʱ����״̬��	ɾ���ֻ���	ɾ�����أ�	�̼����� �Ļظ�
  */
typedef struct{
	u32 result;    	//�ɹ�����1��ʧ�ܷ���0
	u16 epilog;
}ST_Int;
/**
  *Int+char
	*���ƿ��أ�	�����豸��	�豸�����ͽ���	�Ļظ�
  */
typedef struct{
	u32 result;    	//�ɹ�����1��ʧ�ܷ���0
	u8	state;			//���ؿ��ƺ��״̬������������״̬
	u8 epilogl;
	u8 epilogh;
}ST_IntChar;
/**
  *Int+short
	*��ȡ��������	�Ļظ� 
  */
typedef struct{
	u32 result;    				//�ɹ�����1��ʧ�ܷ���0
	u16	PROBarArrLen;			//�������ظ�0��100(PROB=progress bar) ����array len
	u16 epilog;
}ST_IntShort;
/**
  *Int+String
	*ѧϰ����	�Ļظ� 
  */
typedef struct{
	u32 result;    				//�ɹ�����1��ʧ�ܷ���0
	u16	strLen;						//�ַ�������
	u8	content[50];					//�ַ��������￪ʼ����
}ST_IntString;
/**
  *Array(Long Char Short Char	Char)
	* ��ѯ�������ֻ�״̬��ָ��ѯ���ֻ�״̬
  */
typedef struct{   
	u32 idL;
	u32 idH;							//8�ֽڵ�ַ������4�ֽ���� �豸ID
//	uint64_t id;
	u8	deviceTpye;				//��������
	u8 	vsion2;						//�汾��
	u8	vsion1;
	u8	temp;							//�¶�
	u8	state;						//״̬
	u8	next;							//array����λ��
}ST_Array_LCSCC;				//Long Char Short Char	Char
/**
  *Array(Long Char	Char)
	* ��ѯ����״̬
  */
typedef struct{
	u32 idL;
	u32 idH;							//8�ֽڵ�ַ������4�ֽ���� �豸ID
	u8	Channle;					//����·��
	u8	state;						//����״̬
	u16 epilog;						//������
}ST_Array_LCC;				//Long  Char	Char

typedef enum {
	disconnect			=0U,
	connect_router	=1U,
	dns_founded			=2U,
	login_ok				=3U
}EM_ConState;
/***********************************************************************************************
************************************************************************************************/
/**
  *Enumeration of CMD 
  */
//typedef enum{
//	broadcaset =1,
//	addSlave,
//	deviceLock,
//	queryAllState,
//	getUpDataProgress,
//	addSwitch
//}EM_LongCMD;

#define PKT_HDR			0x24676340
/*Define CMD cloud and host communication*/
#define GET_IP  		0x0000
#define LOG_IN  		0x0001
#define HEARTBEAT		0x0002
#define UPDATES     0x0003
#define HANDSHAKE   0x0004

/*Define CMD host and client communication*/
#define LOGIN_OK								0x0001
#define HEARTBEAT								0x0002

#define BROADCAST								0x2001
#define ADD_SLAVE								0x2002
#define STUDY										0x2003
#define STOP_STUDY      				0x2004
#define CONTROL         				0x2005
#define LOCK            				0x2006
#define UPDATE_BY_CLIENT				0x2007
#define QUERY_DEVICE_STATE			0x2008
#define BATCH_QUERY							0x2009
#define ADD_TIMED_TASK					0x200A
#define DELETE_TIMED_TASK				0x200B
#define MODIFY_TIMED_TASK   		0x200C
#define GET_TIMED_TASK_LIST 		0x200D
#define GET_ONE_TIMED_TASK			0x200E
#define MODIFY_TIMED_TASK_STATE	0x200F
#define ADD_SWITCH							0x2010
#define QUERY_SWTICH_STATE			0x2011
#define CONTROL_SWITCH					0x2012
#define GET_UPDATE_PROGRESS			0x2013
#define DELETE_SLAVE						0x2014
#define DELETE_SWITCH						0x2015
/**Define Study type**/
#define STUDY_IR			0x01
#define STUDY_RF433		0x02
#define STUDY_RF315		0x03
/**Define epilog code  result code **/
#define EPILOG				0x7121
#define RESULT_TRUE 	1
#define RESULT_FALSE	0

void Protocol_Init(void);
void Extract_Time(ST_Time* time,const u8 *timePos);
u16 findLen(const u8 * source );
//void InitWifiSendHeader(ST_WifiDataPackHead  * const headerSend,const ST_WifiDataPackHead *const headerReceive);
void InitWifiSendHeader(const ST_WifiDataPackHead *const headerReceive);

u8 Get_Time(void);
u8 Check_DNS(void);

void WifiIntLongReport				(u32 idH,u32 idL);
void WifiIntReport						(u32 result);
void WifiIntIntReport					(u32 id);
void WifiIntCharReport				(u8 state);
void WifiIntLongCCReport			(u32 addr,u8 channel,u8 state);
void WifiIntStrReport					(void);
void WifiQueryAllDeviceReport	(void);
void WifiHeartbeatReport			(u8 mode,BaseType_t * xHigherPriorityTaskWoken);

BOOL QuerySwitch(u32 switchID);
void DeviceLogin					(u8 mode,BaseType_t * xHigherPriorityTaskWoken);
void SwitchHostLockState	(void);
u8 	 ModifyTimeTaskList		(const u8 * next,u8 mode);
void ModifyTimeTaskSate		(u32 id,u8 state);
void GetTimeTaskList			(u32 startPos,u32 quantity);
void Get_TimeTaskInfo			(u32 taskID);
BOOL ExecuteTask					(u8 taskID);
void DataInit							(void);
void EraseDevice(void);
/*�˺���λ��freertos.c������� �������ж��е���*/
u8 Wifi_QueueIn (ST_Report * st_report);
/**Ddfine function macro*/
//������½
#define LOGIN_IT(HigherPriority)											DeviceLogin(1,HigherPriority)
#define LOGIN_NORM()																	DeviceLogin(0,NULL)	
//��������
#define HEARBEAT()																  WifiHeartbeatReport(0,NULL)
//���󷵻� �ɹ����غ�  �㲥����
#define ERROR_REPORT()													WifiIntReport(RESULT_FALSE)
#define SUCCESS_REPORT()												WifiIntReport(RESULT_TRUE)
#define BROADCAST_FUN(addrH,addrL) 						WifiIntLongReport(addrH,addrL) 
//��ӷֻ��ͻظ�
#define ADD_SLAVE_FUN()																	RFGenericSend(SYNC_WORD_ADD,RF_ADD_SLAVE,0U,NULL) //targetAddr cmd len data0 data1 ����
#define ADD_SLAVE_REPORT(addrL) 								WifiIntLongReport(0U,addrL) 
//ɾ���ֻ��ͻظ�
#define DELETE_SLAVE_FUN(targetAddr)										RFGenericSend(targetAddr,RF_DEL_SLAVE,0U,NULL)
#define DELETE_SLAVE_REPORT(result)						WifiIntReport(result)

//ѧϰ����ͻظ�
#define STUDY_FUN(studyType)														GenericStudy(studyType)
#define STUDY_REPORT()													WifiIntStrReport()
//#define STOP_FUN()																			StopStudy();
#define STOP_FUN(result)												WifiIntReport(result)
// ���ƺͿ��Ʒ���
#define CONTROL_FUN(next,len)														SendControlCmd(next,len)
#define CONTROL_REPORT(state)									WifiIntCharReport(state)
//���Ʒֻ��ͻظ�
#define CONTROL_SLAVE_FUN(targetAddr,len,content)				RFGenericSend(targetAddr,RF_CONTROL_DEVICE,len,content)

#define LOCK_FUN()																			SwitchHostLockState()
#define UPDATA_FUN()
#define QUERY_FUN(next)															WifiQueryAllDeviceReport()
#define	BATCH_QUERY_FUN()												WifiQueryAllDeviceReport()
//WifiQueryDeviceReport(next)
//��Ӷ�ʱ����ͻظ�
#define ADD_LIST_MODE																0x7F
#define ADD_TIME_LIST_FUN(next)													ModifyTimeTaskList(next,ADD_LIST_MODE)
#define ADD_TIME_LIST_REPORT(id)								WifiIntIntReport(id+1)
//ɾ����ʱ����ͻظ�																							//mode-1����Ϊ�ͻ��˲���ʶ��IDΪ0����֮ǰ������ͻ��˵�ʱ����ID����1��	
#define DELETE_TIMED_TASK_FUN(next,mode)								ModifyTimeTaskList(next,((mode-1)|0x80))
#define DELETE_TIMED_TASK_REPORT()									WifiIntReport(RESULT_TRUE)
//���Ķ�ʱ����
#define MODIFY_TIMED_TASK_FUN(next,mode)								ModifyTimeTaskList(next,mode-1)

#define GET_TIMED_TASK_LIST_FUN(startPos,quantity)			GetTimeTaskList(startPos,quantity)
#define GET_ONE_TIMED_TASK_FUN(taskID)									Get_TimeTaskInfo(taskID-1)												
//���Ķ�ʱ����״̬
#define MODIFY_TIMED_TASK_STATE_FUN(id,state)  				ModifyTimeTaskSate(id-1,state)
//��ӿ��غͻظ�
#define ADD_SWITCH_FUN()																RFGenericSend(SYNC_WORD_ADD,RF_ADD_SWITCH,0U,NULL)
#define ADD_SWITCH_REPORT(addr,channel,state) 	WifiIntLongCCReport(addr,channel,state)
//���ƿ��غͻظ�
#define CONTROL_SWITCH_FUN(targetAddr,content)					RFGenericSend(targetAddr,RF_CONTROL_DEVICE,2U,content)
#define CONTROL_SWITCH_REPORT(state)						WifiIntCharReport(state)
//ɾ�����غͻظ�
#define DELETE_SWITCH_FUN(targetAddr)										RFGenericSend(targetAddr,RF_DEL_SLAVE,0U,NULL)
#define DELETE_SWITCH_REPORT(result)						WifiIntReport(result)
#define QUERY_SWTICH_STATE_FUN(switchID)								QuerySwitch(switchID)

#define GET_UPDATE_PROGRESS_FUN()



#define SET_DEFAULT_SYNC()															SetDefaultSYNC()


//extern ST_WifiDataPackHead		*const	stp_gWifiDataHeadToSend; //��ͷ��ָ����Զָֻ��������ĵ�һ��λ�� const��ָ����ָ����ָ���ɱ�
//extern ST_IntIntS			*		volatile		stp_gIntIntSend;//			=	(ST_IntIntS		*)&stp_gWifiDataHeadToSend->next;//�ڶ���ʱǿ��ת��������һ��const����
//extern ST_Int					*		volatile		stp_gIntSend;
//extern ST_IntChar			*		volatile		stp_gIntCharSend;
//extern ST_IntShort		*		volatile		stp_gIntShortSend;
//extern ST_IntLong			*		volatile		stp_gIntLongSend;
//extern ST_IntLongCC		*		volatile		stp_gIntLongCCSend;
//extern ST_IntString		*		volatile		stp_gIntString;
//extern ST_Array_LCSCC	*		volatile		stp_gArr_LCSCC; //
//extern ST_Array_LCSCC	*		volatile		stp_gArr_LCSCC1; //

extern ST_HostInfo st_gHostInfo;
extern u8 uc_gTimTaskList2Arr[TIMINGTASK_LIST_LEN_MAX][TIMINGTASK_HEAD_LEN_MAX];
extern u32 ul_gTimeTaskUnixTimArr[TIMINGTASK_LIST_LEN_MAX+1];
extern u8 uc_gTimCmdArr[TIMINGTASK_DATALEN_MAX];
#endif
