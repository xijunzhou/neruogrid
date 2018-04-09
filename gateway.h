#ifndef __GATEWAY_H__
#define __GATEWAY_H__


/*
     酒钢表具名称编码  

表具名称编码	表具名称
   11          热水表
   12          冷水表
   13          燃气表
   14          流量计
   21          压力表
   22          压力变送器
   31          温度计
   32          温度变送器
   41          电能表
   51          电流表
   61          电压表
   71          热量表

*/



enum meter_type_t
{
	UnboiledWater       = 0x10,
	DomesticHotWater    = 0x11,
	DrinkingWater       = 0x12,
	ReclaimedWater      = 0x13,
	CalorimeterHolt     = 0x20,
	CalorimeterCold     = 0x21,	
	GasMeter            = 0x30,
	ElectricEnergyMeter = 0x40,
	ElectricEnergyMeter3 = 0x41,
};

typedef enum 
{
	RxDown = 0,
	TxDowm,
	UpLine,
	OffLine,
}gprs_state_s;



#define SECOND 1
#define MINUTE (60 * SECOND)
#define HOUR (60 * MINUTE)
#define DAY (24 * HOUR)

#define MAX_DEV_NUM 2000

// --------------------------------采集策略宏定义
#define DATA_RULE_BASE 0.01     //数据采集变化基准        
#define RULE_MIN_CYCLE MINUTE   //规则更新最小周期
#define RULE_MAX_CYCLE HOUR     //规则更新最大周期

//采集策略回写数据库间隔
#define RULE_SAVE_CYCLE  (10*MINUTE)
#define RULE_DEFAUL_RATE 600   //设备默认采集周期:10分钟

//心跳周期
#define HEARTBEAT_CYCLE (3*MINUTE)

//采集频率更新周期
#define RULE_UPDATE_CYCLE (3*MINUTE)

#define RULE_DETECT_NUM  5       //计算采集周期时，历史记录样板数
#define MAX_NO_RESPONSE_COUNT 10 //设备无响应重复采集次数  


//IOT设备档案
typedef struct device_s
{
	int type;	        /*设备类型：水表、电表、气表、热表 等等*/
	char id[20];      /*设备id，电表为表计通讯地址*/
	char name[20];    /*设备描述*/
	char net_id[10];   /*无线采集时对应的网络id*/
	long collect_time;/*最新一次采集时间*/
	int collect_cycle;/*采集周期*/
	int no_response;/*无响应次数*/
}device_t;

typedef struct gateway_device_s
{
	char id[20];               /*网关设备id*/
	char name[64];             /*网关描述*/
	int dev_num;               /*网关连接终端设备个数*/
	int now;                   /*当前采集的终端设备顺序编号*/
	int next;                  /*下一个采集的终端设备顺序编号*/
	device_t dev[MAX_DEV_NUM]; /*终端设备*/	
}gateway_t;

#endif