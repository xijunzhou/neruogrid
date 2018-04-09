#ifndef _DLT645_H_
#define _DLT645_H_

/*       DLT645 ��������         */
#define ENERGY_FORWARDA  0    /*�����й��ܵ���*/
#define ENERGY_BACKWARDA 1    /*�����й��ܵ���*/
#define VOLTAGE 2             /*��ѹ*/
#define CURRENT 3            /*��������*/
#define POWER_ACT 4           /*˲ʱ���й�����*/
#define POWER_REACT 5        /*˲ʱ���޹�����*/
#define FACTOR 6               /*�ܹ�������*/
#define FREQUENCY 7           /*����Ƶ��*/
#define LOADRECORD 8          /*���ɼ�¼*/

#define ELECT_DATA_FRAME_TYPE_MAX 7          /*���ɼ�¼*/

/**      DLT645 ���ݽṹ��                   **/
typedef struct          /*�����������1:��ѹ������(A B C ����)*/ 
{                                                            
	 char Voltage[2];                          /*��ѹ 2 bytes ��λ��0.1V */                 
	 char Current[2];                          /*���� 2 bytes ��λ��0.001A */                 
}meter_data_t1;

typedef struct          /*�����������2:�ǵ�ѹ������(A B C ����)*/
{
	 char ActPower[3];                    /*�й����� 3 bytes ��λ��0.0001kW */              
	 char ReactPower[3];                  /*�޹����� 3 bytes ��λ��0.0001kvar */            
	 char Factor[2];                      /*�������� 2 bytes ��λ��0.001 */         
	 char EnergyForward[4];               /*�����й����� 4 bytes ��λ��0.01kWh */              
	 char EnergyBackward[4];              /*�����й����� 4 bytes ��λ��0.01kWh */  
	 char EnergyReactCombination1[4];     /*����޹�1 4 bytes ��λ��0.01kvarh */          
	 char EnergyReactCombination2[4];     /*����޹�2 4 bytes ��λ��0.01kvarh */     
	 char Quadrant1ReactCombination[3];   /*��1�����޹����� 3 bytes ��λ��0.01kvarh */          
	 char Quadrant2ReactCombination[3];   /*��2�����޹����� 3 bytes ��λ��0.01kvarh */          
	 char Quadrant3ReactCombination[3];   /*��2�����޹����� 3 bytes ��λ��0.01kvarh */          
	 char Quadrant4ReactCombination[3];   /*��4�����޹����� 3 bytes ��λ��0.01kvarh */
}meter_data_t2;



typedef struct          /*�����*/
{
	char id[20];                          /*��� */ 
	meter_data_t1 data1;                 /*A��B��C ��������1 */ 
	char Frequency[2];                   /*����Ƶ��     2 bytes ��λ��0.01Hz */  
	meter_data_t2 data2;                 /*�ܡ�A��B��C ��������2 */ 
	char ActiveForwardDemand[3];         /*��ǰ�й����� 3 bytes ��λ��0.0001kW */ 
	char ReactForwardDemand[3];          /*��ǰ�޹����� 3 bytes ��λ��0.0001kvar */	
}single_phase_meter;                     
                                                                            
typedef struct          /*�����*/
{                                        
	char id[20];                          /*��� */
	meter_data_t1 data1[3];              /*A��B��C ��������1 */ 
	char Frequency[2];                   /*����Ƶ��     2 bytes ��λ��0.01Hz */       
	meter_data_t2 data2[4];              /*�ܡ�A��B��C ��������2 */ 
	char ActiveForwardDemand[3];         /*��ǰ�й����� 3 bytes ��λ��0.0001kW */     
	char ReactForwardDemand[3];          /*��ǰ�޹����� 3 bytes ��λ��0.0001kvar */	  
}three_phase_meter;

typedef struct elect_s1
{
	int type;                    
	char id[20];  
	char DateRealTime[30];                          /*���ɼ�¼ʱ��YYYYMMDDHHMMSS*/                      
	char *Timestamp;
	long rtc;
	                
	long Voltage;                                  /*��ѹ*/  
	long Current;                                 /*����*/   
	long Frequency;                                /*����Ƶ��*/  
	long ActPower;                                /*˲ʱ���й�����*/
	long ReactPower;                              /*˲ʱ���޹�����*/      
	long Factor;                                  /*�ܹ�������*/      
	long EnergyForward;                            /*�����й��ܵ���*/              
	long EnergyBackward;                           /*�����й��ܵ���*/ 
	            
	long EnergyReactCombination1;                  /*����޹�(1\2)�ܵ���*/    
	long EnergyReactCombination2;  
	long QuadrantReactCombination1;                /*��(1\2\3\4)�����޹��ܵ���*/  
	long QuadrantReactCombination2;
	long QuadrantReactCombination3; 
	long QuadrantReactCombination4; 
	long ActiveForwardDemand;                      /*��ǰ�й�����*/    
	long ReactForwardDemand;                       /*��ǰ�޹�����*/
}elect_meter_t1;


typedef struct elect_s3
{
	int type;                   
	char id[20]; 
	char DateRealTime[30];                          /*���ɼ�¼ʱ��YYYYMMDDHHMMSS*/                       
	char *Timestamp;
	long rtc;
	             
	long VoltageA;                                 /*��ѹ/(A\B\C)�����ѹ*/  
	long VoltageB;       
	long VoltageC;      
	long CurrentA;                                 /*����/(A\B\C)�������*/       
	long CurrentB;      
	long CurrentC;       
	long Frequency;                                /*����Ƶ��*/
	       
	long ActPower;                                 /*�й�����(�ܡ�A\B\C ����)*/      
	long ActPowerA;        
	long ActPowerB;         
	long ActPowerC;          
	long ReactPower;                               /*�޹�����(�ܡ�A\B\C ����)*/        
	long ReactPowerA;
	long ReactPowerB;
	long ReactPowerC;
	long Factor;                                   /*��������(�ܡ�A\B\C ����)*/   
	long FactorA;     
	long FactorB;    
	long FactorC;                                   
	long EnergyForward;                            /*�����й��ܵ���*/ 
	long EnergyBackward;                           /*�����й��ܵ���*/
	        
	long EnergyReactCombination1;                  /*����޹�(1\2)�ܵ���*/          
	long EnergyReactCombination2;   
	                                                
	long QuadrantReactCombination1;                /*��(1\2\3\4)�����޹��ܵ���*/    
	long QuadrantReactCombination2;                                                 
	long QuadrantReactCombination3;                                                 
	long QuadrantReactCombination4;  
	                                               
	long ActiveForwardDemand;                      /*��ǰ�й�����*/                 
	long ReactForwardDemand;                       /*��ǰ�޹�����*/                 
}elect_meter_t3;


extern device_t *device;

int dlt645_decode(unsigned char *buff,int len,elect_meter_t1 *meter);
int dlt645_decode1(unsigned char *buff,int len,elect_meter_t3 *meter);/*֧������*/
int dlt645_encode(unsigned char dlt645[],char *id,int type);


#endif