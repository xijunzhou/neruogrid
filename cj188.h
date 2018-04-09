#ifndef _CJ188_H_
#define _CJ188_H_

#define BATTERY_MASK  0x20 
#define FAMEN_MASK    0xc0 

#define BATTERY_OK_MASK     0x00 
#define BATTERY_LOW_MASK    0x20 

#define FAMEN_OPEN_MASK        0x00 
#define FAMEN_CLOSE_MASK       0x40 
#define FAMEN_ABNORMAL_MASK    0xc0 

#define FAMEN_OPEN    0   /*阀门打开*/
#define FAMEN_CLOSE    1   /*阀门关闭*/
#define FAMEN_ABNORMAL 2   /*阀门异常*/


#define BATTERY_OK    0   /*电池正常*/
#define BATTER_LOW     1   /*电池欠压*/

/*       CJ188 帧结构         */

#define CJ188_START  0    /*帧起始位*/
#define CJ188_TYPE   1
#define CJ188_ADDR   2     /*地址域*/

#define CJ188_CTRL   9      /*控制码位*/
#define CJ188_DLEN   10	       /*数据长度位*/
#define CJ188_DATA   11         /*数据位*/

/*       CJ188 帧命令         */

#define CJ188_FRAME_START           0x68
#define CJ188_FRAME_END             0x16

#define CJ188_FRAME_READ_DATA       0x01
#define CJ188_FRAME_READ_ACK        0x81

#define CJ188_FRAME_WRITE_DATA      0x04
#define CJ188_FRAME_WRITE_ACK       0x84

#define CJ188_FRAME_READ_KEY        0x09
#define CJ188_FRAME_READ_KEY_ACK    0x89

#define CJ188_FRAME_READ_ADDR       0x03
#define CJ188_FRAME_READ_ADDR_ACK   0x83

#define CJ188_FRAME_WRITE_ADDR      0x15
#define CJ188_FRAME_WRITE_ADDR_ACK  0x95

#define CJ188_FRAME_WRITE_MOTOR     0x16
#define CJ188_FRAME_WRITE_MOTER_ACK 0x96

#define CJ188_CTRL0_READ_DATA 	0x01
#define CJ188_CTRL0_READ_KEY 	  0x01
#define CJ188_CTRL0_READ_ADDR 	0x01

#define CJ188_CTRL1_READ_DATA 	0x81
#define CJ188_CTRL1_READ_KEY 	  0x89
#define CJ188_CTRL1_READ_ADDR 	0x83

#define CJ188_CTRL2_READ_DATA 	0xc1
#define CJ188_CTRL2_READ_KEY 	  0xc1
#define CJ188_CTRL2_READ_ADDR 	0xc1


#define CJ188_DI0_READ_DATA     0x1f
#define CJ188_DI1_READ_DATA     0x90



/*       CJ188 数据类型  水、气       */

#define WG_AccumulatedFlowCurrent  (CJ188_DATA + 3)    /*当前累积流量*/
#define WG_JSRAccumulatedFlow (CJ188_DATA + 8)    /*结算日累积流量*/
#define WG_SSSJ    (CJ188_DATA + 13)   /*实时时间*/
#define WG_ZTST    (CJ188_DATA + 20)   /*状态*/


/*       CJ188 数据类型    热     */
#define HEAT_heatAccountDay  (CJ188_DATA + 3)    /*结算日热量*/
#define HEAT_heatCurrent   (CJ188_DATA + 8)    /*当前热量*/
#define HEAT_Powerheat    (CJ188_DATA + 13)   /*热功率*/
#define HEAT_Flow     (CJ188_DATA + 18)   /*流量*/
#define HEAT_AccumulatedFlow   (CJ188_DATA + 23)   /*累计流量*/
#define HEAT_TSupplyWater   (CJ188_DATA + 28)    /*供水温度*/
#define HEAT_TReturnWater   (CJ188_DATA + 31)   /*回水温度*/
#define HEAT_WorkHours (CJ188_DATA + 34)   /*累计工作时间*/
#define HEAT_SSSJ   (CJ188_DATA + 37)   /*实时时间*/
#define HEAT_ZTST   (CJ188_DATA + 44)   /*状态*/

typedef struct heat_meter_u
{	 
	char U_heatAccountDay[8];        //结算日热量单位         
	char U_heatCurrent[8];           //当前热量单位          
	char U_Powerheat[8];             //热功率单位         
	char U_Flow[8];                   //流量单位        
	char U_AccumulatedFlow[8];        //累计流量单位            
	char U_TSupplyWater[8];          //供水温度单位              
	char U_TReturnWater[8];          //回水温度单位       
	char U_WorkHours[8];              //累计工作时间单位
}heat_unit_t;	

typedef struct heat_meter_d
{
	long heatAccountDay;        //结算日热量           
	long heatCurrent;           //当前热量          
	long Powerheat;             //热功率         
	long Flow;                   //流量        
	long AccumulatedFlow;        //累计流量            
	long TSupplyWater;          //供水温度              
	long TReturnWater;          //回水温度       
	long WorkHours;              //累计工作时间
	char DateRealTime[30];          //实时时间 
	char states[2];                //表计状态
	int StateValve;            //阀门状态     
	int StateBattery;          //电池状态 
}heat_data_t;	

//热量表
typedef struct heat_meter_s
{
	int type;                    
	char id[20];                       
	char *Timestamp;
	long rtc;
	heat_data_t data;
	heat_unit_t unit;         
}heat_meter_t;


 typedef struct water_gas_meter_u
{	 
	char U_AccumulatedFlowCurrent[8];        //当前累积流量单位           
	char U_AccumulatedFlowAccountDay[8];     //结算日累积流量单位
}water_gas_unit_t;	

typedef struct water_gas_meter_d
{
	long AccumulatedFlowCurrent;        //当前累积流量           
	long AccumulatedFlowAccountDay;     //结算日累积流量          
	char DateRealTime[30];          //实时时间              
	char states[2];                //表计状态
	int StateValve;            //阀门状态     
	int StateBattery;          //电池状态 
}water_gas_data_t;	

//煤气表、
typedef struct gas_meter_s
{
	int type;                    
	char id[20];                       
	char *Timestamp;
	long rtc;
	water_gas_data_t data;
	water_gas_unit_t unit;         
}gas_meter_t;

//冷水表
typedef struct water_cold_meter_s
{
	int type;                    
	char id[20];                       
	char *Timestamp;
	long rtc;
	water_gas_data_t data;
	water_gas_unit_t unit;         
}water_cold_meter_t;


typedef struct cj188_s
{
	int type;
	char *id;
	void *Pmeter;
	gas_meter_t *gas;
	water_cold_meter_t *water_cold;
	heat_meter_t *heat;
}cj188_t;


int cj188_decode(char *buff,int nbyte,cj188_t *Cj188);
int cj188_encode(char cj188[],char *id,int type);

#endif