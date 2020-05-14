此工程建立在FreeRTOS上

  freertos.c 文件中建立了
	--void KeyTask(void *pvParameters)		用来监控按键的任务
	--void ProcessWifiTask(void *pvParameters)	用做处理APP或服务器通过WIFI发来的数据 再调用相应的函数做控制
	--void ReceiveCMTTask(void *pvParameters)	处理RFM300H接收到的数据 接收到数据触发硬件中断在硬件中断里消息通知再在这个任务中提取
	--void SendCMTTask(void *pvParameters)		发送RFM300H无线数据(FSK数据包) OOK需要发的数据在ProcessWifiTask做了处理
	--void SendWifiTask(void *pvParameters)		WIFI发送任务所有需要向APP或服务器报告的数据均由它执行报告 数据由全局变量数组传递
	--void StudyTask(void *pvParameters)		APP发送学习命令时ProcessWifiTask解码后任务通知StudyTask开始学习
	--void TimedTask(void *pvParameters)		定时任务处理 基于RTC每5分钟会获取一下网络时间如果任务发生时间小于5分钟就会以任务时间为闹钟
	--void vApplicationIdleHook( void )		空闲任务勾子函数用于喂狗和获取温度

  protocol.c 文件中主要是解码通过WIFI接收到的数据  通常用结构体强制转换类型来得到某些想要的数据

  rf433comm.c 文件中主要是对RFM300H的一些应用层操作相应的一些底层操作在 "RFM300H"文件夹中实现


  ir.c  adc.c dma.c time.c rtc.c iwdg.c gpio.c 见其名可知其意在文件中都有相应的注释