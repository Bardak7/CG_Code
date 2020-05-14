#ifndef __PROTOCOL_H
#define __PROTOCOL_H
#include "sys.h"
#include "stdint.h"
#include "rf433comm.h"
#include "FreeRTOS.h"
#include "rtc.h"

//版本号为第一版本
#define	VER											1
 //WIFI发送缓存
#define WIFI_SEND_BUF_MAX				256

/**定时任务相关**/
#define TIMINGTASK_DATALEN_MAX  200  	//定义数据从[20](包头后)到0x21 0x71最多只能有200个字节。
#define TIMINGTASK_LIST_LEN_MAX	10		//定义定时任务最多有10个条目
#define TIMINGTASK_CMD_MAX			3		//字义最多定时任务可以包含3个控制命令
#define TIMINGTASK_HEAD_LEN_MAX	50		//定义头部（命令数组前的所有数据）



/**开关分机列表最大相关**/
#define	SWITCH_LIST_MAX					10		//最多有10个开关列表
#define	SLAVE_LIST_MAX					10		//最多有10个分机列表

/**相应的保存地址相关**/
//此处为出产的UID
#define DEVIDL_FLASH_ADDR 			0x1FFFF7E8	
#define DEVIDH_FLASH_ADDR 			0x1FFFF7EC
#define DEVIDM_FLASH_ADDR 			0x1FFFF7F0
//此处为滚动烧录的4个字节
#define ROLLING_CODE_ADDR				0x0800FFF0

//设备是否被锁定Flash保存地址
#define LOCK_FLAG_FLASH_ADDR   	0x0801B004
//分机保存地址和间隔  interval
#define SLAVE_LIST_FLASH_ADDR  	0x0801B100
//4个字节的间隔
#define	SLAVE_LIST_FLASH_INVL		4
//开关保存地址和间隔  interval
#define SWITCH_LIST_FLASH_ADDR  0x0801B200
#define SWITCH_LIST_FLASH_INVL	6
/*
				定时任务Flash存储方式如下
	*		低		总长度		(short)	从名称到 !q 的长度(包含!q)
	*		|			头部长度	(short)	从名称到日期的长度(包含日期)
	*		|			状态		(short)    //这里以一个short来保存方便flash
	*		|			名称		(len+content)	
	*		|			频次		(Byte)
	*					时间		(len+content)
	*					日期		(len+content)
	*					命令数组(len+ content)
	*							--	ID				(long)
	*							--	命令码			(len+conten)
	*							--	执行延时		(short)
	*		高		状态		(Byte)
	*/
#define TIME_LIST_FLASH_ADDR   	0x0801C000
//分机 开关 定时任务列表都分配 4KFlash空间

/*定时任务设为闹钟相关*/
//最近需要执行的任务的时间戳
#define	RECENT_TASK_UTC					ul_gTimeTaskUnixTimArr[TIMINGTASK_LIST_LEN_MAX]
//RTC的闹钟周期 300秒5分钟一闹 得网络时间
#define ALARM_CYCLE							300

/*ADC获取内部温度相关周期时间*/
#define	TEMP_CYCLE			3000/TIM1_RELOAD_MS
/*按键时间相关*/
//按键5秒时间
#define	PRESS_LONG			5000/TIM1_RELOAD_MS
//
#define LONG_PRESS_BIT	0x01
#define SHORT_PRESS_BIT	0x02

/*学习相关*/
//10S除去 定时的秒数就是计数值
#define STUDY_TIME_MAX					10000/TIM1_RELOAD_MS
#define STUDY_SUCCUESS					0x01UL
#define STUDY_FAILED						0x02UL
#define STUDY_SEND433						0x03
#define STUDY_SENDIR						0x04

#define STUDY_MODE_IR		1U
#define STUDY_MODE_RF		2U

//***心跳时间 6000 ms **/
#define HEART_TIM				6000/TIM1_RELOAD_MS
//*学习时脉冲最大容差，超过容差即断定不是同一个数据 50*7 =350us*/
#define PULSE_TOL				7			
#define CUSTOM_LIB			0x5AA5

//typedef struct IrStudy//红外学习体数据结构-28
//{
//	u16 HeadHCnt;//头码高电平时延，n*13.16us
//	u16 HeadLCnt;//头码低电平时延，n*13.16us
//	u16 Data1HCnt;  
//	u16 Data1LCnt;
//	u16 Data0HCnt;
//	u16 Data0LCnt;
//	u8 Type;  //ir-1,433-2,315-3
//	u8 Len;
//	u8 Data[50];
//}IrStudy_DATA;

/*wifi反馈结构体*/
typedef struct{
	u16 dataLock;
	u16 reportLen;
	u8  reportBuf[WIFI_SEND_BUF_MAX];
}ST_Report;

/*分机消息*/
typedef struct{
	u32 slaveID;
	u8	slaveState;
}ST_SlaveInfo;
/*开关消息*/
typedef struct{
	u32 switchID;
	u8	channle;
	u8	state;
}ST_SwitchInfo;
/*主机相关消息结构体*/
typedef struct{
  uint64_t			deviceID;
	u16						version;
	u16						lockInfo;	//是否锁定主机	
	u8 						temp;			//温度
	u8						switchCount;
	u8						slaveCount;
	u8						timeTaskCount;
	ST_SwitchInfo	stSwitch[SWITCH_LIST_MAX];
	ST_SlaveInfo	stSlave	[SLAVE_LIST_MAX];
}ST_HostInfo;
/***********************************************************************************************
************************************************************************************************/
																								/*以下的为读取的格式化*/
/**
  *格式化包头 共20 个字节
  */
typedef struct{
	u32 head;  				//4位包头 		固定为0x24676340
	u16 version;  		//2位版本 		目前为1
	u16 cmd;					//2位命令 		命令域详见本文件下方Define CMD
	u16 sn;						//2位序列号	
	u16 len;					//2位长度
//	uint64_t reserve;	//8位保留
	u32 reserveL;       //由于结构体对齐原因如果是64位上面的len便会多出几位来对齐这个造成数据错乱
	u32 reserveH; 
	u8 next;		//剩下的是数据块和结束符的0x7121  !q 这些指向了数据块的第一个字节
}ST_WifiDataPackHead;


/*
 *数据体里是一个long long 型64位地址
 * 广播命令， 添加分机，设备解锁，查询所有主分机状态，获取升级进度，添加开关
*/
typedef struct{
	u32	hostID2;
	u32 hostID1;
	u8	next;
}ST_Long;
/*
 * Long int int
 * 获取定时列表  ，更改定时状态， 获取单个定时任务删除定时任务
 * int+int 				或 int+char		或 int
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
 *	数据里是两个long long型 64位的 主 从机地址
 * 删除分机， 停止学习， 删除开关都是
 *               后加的两个 是学习命令用一位，控制开关用两个一位
 * long + char + char
*/
typedef struct{
//  u32	hostID1;
//	u32 hostID2;
	u32	slaveIDL;
	u32	slaveIDH;
	u8	studyModeOrChannel; //学习命令的学习类型或者是开关的路数
	u8	state;							//想要开关达成的状态
}ST_LongCharChar; //此处格式化主分机地址。

/*
 *数据体里是两个long long型 64位的 主 从机地址加上一个字符串类型 字符串有两位长度和内容组成
 * 控制设备
 * long+String(short+content)
*/
typedef struct{
//  u32	hostID1;
//	u32 hostID2;
	u32	slaveIDL;
	u32	slaveIDH;
	u16 stringLen;
	u8	content; //这里指向的是Stirng的第一个元素可取指针用作它用
}ST_LongStirng;
/*
 *两个两个long long型 64位的 主 从机地址加上一个2字节的版本号加一个String
 * long + short +String(short+content)
*/
typedef struct{
//	u32	hostID1;
//	u32 hostID2;
	u32	slaveIDL;
	u32	slaveIDH;
	u16	newVersion;
	u16 stringLen;
	u8	content; //这里指向的是Stirng的第一个元素可取指针用作它用
}ST_LongShortString;
/*
 *一个long long型 64位的 主 从机地址加上一个数组类对象 数组类对象里是一个 64位地址
 * 批量查询主分机状态 批量查询开关状态
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
																									/*以上的为读取的格式化*/
/***********************************************************************************************
************************************************************************************************/
													/*以下的为发送的格式化*/

/**
  *	Long +epilog
	* 与服务器通信 主机登陆
  */
typedef struct{
	u32 idL;
	u32 idH;
	u16 epilog;
}ST_LongS;
/**
  *Int+Long +epilog
	* 广播命令， 添加分机  的回复
  */
typedef struct{
	u32 result;    	//成功返回1，失败返回0
//	u8	next;				//指向下一个元素
	u32 idL;
	u32 idH;
	u16 epilog;
}ST_IntLong;
/**
  *Int+Long +epilog
	* 广播命令， 添加分机  的回复
  */
typedef struct{
	u32 result;    	//成功返回1，失败返回0
//	u8	next;				//指向下一个元素
	u32 idL;
	u32 idH;
	u8	channle; 	//返回开关路数
	u8 	state;		//返回开关状态
	u16 epilog;
}ST_IntLongCC;
/**
  *Int+Int +epilog
	* 添加定时任务  的回复
  */
typedef struct{
	u32 result;    	//成功返回1，失败返回0

	u32 taskID;
	u16 epilog;
}ST_IntIntS;
/**
  *Int +epilog
	* 删除定时任务，	修改定时任务，	修改定时任务状态，	删除分机，	删除开关，	固件升级 的回复
  */
typedef struct{
	u32 result;    	//成功返回1，失败返回0
	u16 epilog;
}ST_Int;
/**
  *Int+char
	*控制开关，	控制设备，	设备锁定和解锁	的回复
  */
typedef struct{
	u32 result;    	//成功返回1，失败返回0
	u8	state;			//返回控制后的状态或解锁上锁后的状态
	u8 epilogl;
	u8 epilogh;
}ST_IntChar;
/**
  *Int+short
	*获取升级进度	的回复 
  */
typedef struct{
	u32 result;    				//成功返回1，失败返回0
	u16	PROBarArrLen;			//进度条回复0－100(PROB=progress bar) 或者array len
	u16 epilog;
}ST_IntShort;
/**
  *Int+String
	*学习命令	的回复 
  */
typedef struct{
	u32 result;    				//成功返回1，失败返回0
	u16	strLen;						//字符串长度
	u8	content[50];					//字符串从这里开始复制
}ST_IntString;
/**
  *Array(Long Char Short Char	Char)
	* 查询所有主分机状态，指查询评分机状态
  */
typedef struct{   
	u32 idL;
	u32 idH;							//8字节地址由两个4字节组成 设备ID
//	uint64_t id;
	u8	deviceTpye;				//主机类型
	u8 	vsion2;						//版本号
	u8	vsion1;
	u8	temp;							//温度
	u8	state;						//状态
	u8	next;							//array最后的位置
}ST_Array_LCSCC;				//Long Char Short Char	Char
/**
  *Array(Long Char	Char)
	* 查询开关状态
  */
typedef struct{
	u32 idL;
	u32 idH;							//8字节地址由两个4字节组成 设备ID
	u8	Channle;					//开关路数
	u8	state;						//开关状态
	u16 epilog;						//结束符
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
/*此函数位于freertos.c用于入队 不可以中断中调用*/
u8 Wifi_QueueIn (ST_Report * st_report);
/**Ddfine function macro*/
//主机登陆
#define LOGIN_IT(HigherPriority)											DeviceLogin(1,HigherPriority)
#define LOGIN_NORM()																	DeviceLogin(0,NULL)	
//主机心跳
#define HEARBEAT()																  WifiHeartbeatReport(0,NULL)
//错误返回 成功返回和  广播返回
#define ERROR_REPORT()													WifiIntReport(RESULT_FALSE)
#define SUCCESS_REPORT()												WifiIntReport(RESULT_TRUE)
#define BROADCAST_FUN(addrH,addrL) 						WifiIntLongReport(addrH,addrL) 
//添加分机和回复
#define ADD_SLAVE_FUN()																	RFGenericSend(SYNC_WORD_ADD,RF_ADD_SLAVE,0U,NULL) //targetAddr cmd len data0 data1 参数
#define ADD_SLAVE_REPORT(addrL) 								WifiIntLongReport(0U,addrL) 
//删除分机和回复
#define DELETE_SLAVE_FUN(targetAddr)										RFGenericSend(targetAddr,RF_DEL_SLAVE,0U,NULL)
#define DELETE_SLAVE_REPORT(result)						WifiIntReport(result)

//学习命令和回复
#define STUDY_FUN(studyType)														GenericStudy(studyType)
#define STUDY_REPORT()													WifiIntStrReport()
//#define STOP_FUN()																			StopStudy();
#define STOP_FUN(result)												WifiIntReport(result)
// 控制和控制返回
#define CONTROL_FUN(next,len)														SendControlCmd(next,len)
#define CONTROL_REPORT(state)									WifiIntCharReport(state)
//控制分机和回复
#define CONTROL_SLAVE_FUN(targetAddr,len,content)				RFGenericSend(targetAddr,RF_CONTROL_DEVICE,len,content)

#define LOCK_FUN()																			SwitchHostLockState()
#define UPDATA_FUN()
#define QUERY_FUN(next)															WifiQueryAllDeviceReport()
#define	BATCH_QUERY_FUN()												WifiQueryAllDeviceReport()
//WifiQueryDeviceReport(next)
//添加定时任务和回复
#define ADD_LIST_MODE																0x7F
#define ADD_TIME_LIST_FUN(next)													ModifyTimeTaskList(next,ADD_LIST_MODE)
#define ADD_TIME_LIST_REPORT(id)								WifiIntIntReport(id+1)
//删除定时任务和回复																							//mode-1是因为客户端不能识别ID为0所以之前报告给客户端的时候都是ID加了1的	
#define DELETE_TIMED_TASK_FUN(next,mode)								ModifyTimeTaskList(next,((mode-1)|0x80))
#define DELETE_TIMED_TASK_REPORT()									WifiIntReport(RESULT_TRUE)
//更改定时任务
#define MODIFY_TIMED_TASK_FUN(next,mode)								ModifyTimeTaskList(next,mode-1)

#define GET_TIMED_TASK_LIST_FUN(startPos,quantity)			GetTimeTaskList(startPos,quantity)
#define GET_ONE_TIMED_TASK_FUN(taskID)									Get_TimeTaskInfo(taskID-1)												
//更改定时任务状态
#define MODIFY_TIMED_TASK_STATE_FUN(id,state)  				ModifyTimeTaskSate(id-1,state)
//添加开关和回复
#define ADD_SWITCH_FUN()																RFGenericSend(SYNC_WORD_ADD,RF_ADD_SWITCH,0U,NULL)
#define ADD_SWITCH_REPORT(addr,channel,state) 	WifiIntLongCCReport(addr,channel,state)
//控制开关和回复
#define CONTROL_SWITCH_FUN(targetAddr,content)					RFGenericSend(targetAddr,RF_CONTROL_DEVICE,2U,content)
#define CONTROL_SWITCH_REPORT(state)						WifiIntCharReport(state)
//删除开关和回复
#define DELETE_SWITCH_FUN(targetAddr)										RFGenericSend(targetAddr,RF_DEL_SLAVE,0U,NULL)
#define DELETE_SWITCH_REPORT(result)						WifiIntReport(result)
#define QUERY_SWTICH_STATE_FUN(switchID)								QuerySwitch(switchID)

#define GET_UPDATE_PROGRESS_FUN()



#define SET_DEFAULT_SYNC()															SetDefaultSYNC()


//extern ST_WifiDataPackHead		*const	stp_gWifiDataHeadToSend; //包头的指针永远只指向发送数组的第一个位置 const其指针其指针所指不可变
//extern ST_IntIntS			*		volatile		stp_gIntIntSend;//			=	(ST_IntIntS		*)&stp_gWifiDataHeadToSend->next;//在定义时强制转换必须是一个const类型
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
