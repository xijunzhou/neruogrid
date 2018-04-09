#ifndef _DLT645_H_
#define _DLT645_H_

/*       DLT645 数据类型         */
#define ENERGY_FORWARDA  0    /*正向有功总电能*/
#define ENERGY_BACKWARDA 1    /*反向有功总电能*/
#define VOLTAGE 2             /*电压*/
#define CURRENT 3            /*电流电流*/
#define POWER_ACT 4           /*瞬时总有功功率*/
#define POWER_REACT 5        /*瞬时总无功功率*/
#define FACTOR 6               /*总功率因素*/
#define FREQUENCY 7           /*电网频率*/
#define LOADRECORD 8          /*负荷记录*/

#define ELECT_DATA_FRAME_TYPE_MAX 7          /*负荷记录*/

/**      DLT645 数据结构体                   **/
typedef struct          /*电表负荷数据项1:电压、电流(A B C 三相)*/ 
{                                                            
	 char Voltage[2];                          /*电压 2 bytes 单位：0.1V */                 
	 char Current[2];                          /*电流 2 bytes 单位：0.001A */                 
}meter_data_t1;

typedef struct          /*电表负荷数据项2:非电压、电流(A B C 三相)*/
{
	 char ActPower[3];                    /*有功功率 3 bytes 单位：0.0001kW */              
	 char ReactPower[3];                  /*无功功率 3 bytes 单位：0.0001kvar */            
	 char Factor[2];                      /*功率因素 2 bytes 单位：0.001 */         
	 char EnergyForward[4];               /*正向有功电能 4 bytes 单位：0.01kWh */              
	 char EnergyBackward[4];              /*反向有功电能 4 bytes 单位：0.01kWh */  
	 char EnergyReactCombination1[4];     /*组合无功1 4 bytes 单位：0.01kvarh */          
	 char EnergyReactCombination2[4];     /*组合无功2 4 bytes 单位：0.01kvarh */     
	 char Quadrant1ReactCombination[3];   /*第1象限无功电能 3 bytes 单位：0.01kvarh */          
	 char Quadrant2ReactCombination[3];   /*第2象限无功电能 3 bytes 单位：0.01kvarh */          
	 char Quadrant3ReactCombination[3];   /*第2象限无功电能 3 bytes 单位：0.01kvarh */          
	 char Quadrant4ReactCombination[3];   /*第4象限无功电能 3 bytes 单位：0.01kvarh */
}meter_data_t2;



typedef struct          /*单相表*/
{
	char id[20];                          /*表号 */ 
	meter_data_t1 data1;                 /*A、B、C 负荷数据1 */ 
	char Frequency[2];                   /*电网频率     2 bytes 单位：0.01Hz */  
	meter_data_t2 data2;                 /*总、A、B、C 负荷数据2 */ 
	char ActiveForwardDemand[3];         /*当前有功需量 3 bytes 单位：0.0001kW */ 
	char ReactForwardDemand[3];          /*当前无功需量 3 bytes 单位：0.0001kvar */	
}single_phase_meter;                     
                                                                            
typedef struct          /*三相表*/
{                                        
	char id[20];                          /*表号 */
	meter_data_t1 data1[3];              /*A、B、C 负荷数据1 */ 
	char Frequency[2];                   /*电网频率     2 bytes 单位：0.01Hz */       
	meter_data_t2 data2[4];              /*总、A、B、C 负荷数据2 */ 
	char ActiveForwardDemand[3];         /*当前有功需量 3 bytes 单位：0.0001kW */     
	char ReactForwardDemand[3];          /*当前无功需量 3 bytes 单位：0.0001kvar */	  
}three_phase_meter;

typedef struct elect_s1
{
	int type;                    
	char id[20];  
	char DateRealTime[30];                          /*负荷记录时间YYYYMMDDHHMMSS*/                      
	char *Timestamp;
	long rtc;
	                
	long Voltage;                                  /*电压*/  
	long Current;                                 /*电流*/   
	long Frequency;                                /*电网频率*/  
	long ActPower;                                /*瞬时总有功功率*/
	long ReactPower;                              /*瞬时总无功功率*/      
	long Factor;                                  /*总功率因素*/      
	long EnergyForward;                            /*正向有功总电能*/              
	long EnergyBackward;                           /*反向有功总电能*/ 
	            
	long EnergyReactCombination1;                  /*组合无功(1\2)总电能*/    
	long EnergyReactCombination2;  
	long QuadrantReactCombination1;                /*第(1\2\3\4)象限无功总电能*/  
	long QuadrantReactCombination2;
	long QuadrantReactCombination3; 
	long QuadrantReactCombination4; 
	long ActiveForwardDemand;                      /*当前有功需量*/    
	long ReactForwardDemand;                       /*当前无功需量*/
}elect_meter_t1;


typedef struct elect_s3
{
	int type;                   
	char id[20]; 
	char DateRealTime[30];                          /*负荷记录时间YYYYMMDDHHMMSS*/                       
	char *Timestamp;
	long rtc;
	             
	long VoltageA;                                 /*电压/(A\B\C)三相电压*/  
	long VoltageB;       
	long VoltageC;      
	long CurrentA;                                 /*电流/(A\B\C)三相电流*/       
	long CurrentB;      
	long CurrentC;       
	long Frequency;                                /*电网频率*/
	       
	long ActPower;                                 /*有功功率(总、A\B\C 三相)*/      
	long ActPowerA;        
	long ActPowerB;         
	long ActPowerC;          
	long ReactPower;                               /*无功功率(总、A\B\C 三相)*/        
	long ReactPowerA;
	long ReactPowerB;
	long ReactPowerC;
	long Factor;                                   /*功率因素(总、A\B\C 三相)*/   
	long FactorA;     
	long FactorB;    
	long FactorC;                                   
	long EnergyForward;                            /*正向有功总电能*/ 
	long EnergyBackward;                           /*反向有功总电能*/
	        
	long EnergyReactCombination1;                  /*组合无功(1\2)总电能*/          
	long EnergyReactCombination2;   
	                                                
	long QuadrantReactCombination1;                /*第(1\2\3\4)象限无功总电能*/    
	long QuadrantReactCombination2;                                                 
	long QuadrantReactCombination3;                                                 
	long QuadrantReactCombination4;  
	                                               
	long ActiveForwardDemand;                      /*当前有功需量*/                 
	long ReactForwardDemand;                       /*当前无功需量*/                 
}elect_meter_t3;


extern device_t *device;

int dlt645_decode(unsigned char *buff,int len,elect_meter_t1 *meter);
int dlt645_decode1(unsigned char *buff,int len,elect_meter_t3 *meter);/*支持三相*/
int dlt645_encode(unsigned char dlt645[],char *id,int type);


#endif