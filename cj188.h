#ifndef _CJ188_H_
#define _CJ188_H_

#define BATTERY_MASK  0x20 
#define FAMEN_MASK    0xc0 

#define BATTERY_OK_MASK     0x00 
#define BATTERY_LOW_MASK    0x20 

#define FAMEN_OPEN_MASK        0x00 
#define FAMEN_CLOSE_MASK       0x40 
#define FAMEN_ABNORMAL_MASK    0xc0 

#define FAMEN_OPEN    0   /*���Ŵ�*/
#define FAMEN_CLOSE    1   /*���Źر�*/
#define FAMEN_ABNORMAL 2   /*�����쳣*/


#define BATTERY_OK    0   /*�������*/
#define BATTER_LOW     1   /*���Ƿѹ*/

/*       CJ188 ֡�ṹ         */

#define CJ188_START  0    /*֡��ʼλ*/
#define CJ188_TYPE   1
#define CJ188_ADDR   2     /*��ַ��*/

#define CJ188_CTRL   9      /*������λ*/
#define CJ188_DLEN   10	       /*���ݳ���λ*/
#define CJ188_DATA   11         /*����λ*/

/*       CJ188 ֡����         */

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



/*       CJ188 ��������  ˮ����       */

#define WG_AccumulatedFlowCurrent  (CJ188_DATA + 3)    /*��ǰ�ۻ�����*/
#define WG_JSRAccumulatedFlow (CJ188_DATA + 8)    /*�������ۻ�����*/
#define WG_SSSJ    (CJ188_DATA + 13)   /*ʵʱʱ��*/
#define WG_ZTST    (CJ188_DATA + 20)   /*״̬*/


/*       CJ188 ��������    ��     */
#define HEAT_heatAccountDay  (CJ188_DATA + 3)    /*����������*/
#define HEAT_heatCurrent   (CJ188_DATA + 8)    /*��ǰ����*/
#define HEAT_Powerheat    (CJ188_DATA + 13)   /*�ȹ���*/
#define HEAT_Flow     (CJ188_DATA + 18)   /*����*/
#define HEAT_AccumulatedFlow   (CJ188_DATA + 23)   /*�ۼ�����*/
#define HEAT_TSupplyWater   (CJ188_DATA + 28)    /*��ˮ�¶�*/
#define HEAT_TReturnWater   (CJ188_DATA + 31)   /*��ˮ�¶�*/
#define HEAT_WorkHours (CJ188_DATA + 34)   /*�ۼƹ���ʱ��*/
#define HEAT_SSSJ   (CJ188_DATA + 37)   /*ʵʱʱ��*/
#define HEAT_ZTST   (CJ188_DATA + 44)   /*״̬*/

typedef struct heat_meter_u
{	 
	char U_heatAccountDay[8];        //������������λ         
	char U_heatCurrent[8];           //��ǰ������λ          
	char U_Powerheat[8];             //�ȹ��ʵ�λ         
	char U_Flow[8];                   //������λ        
	char U_AccumulatedFlow[8];        //�ۼ�������λ            
	char U_TSupplyWater[8];          //��ˮ�¶ȵ�λ              
	char U_TReturnWater[8];          //��ˮ�¶ȵ�λ       
	char U_WorkHours[8];              //�ۼƹ���ʱ�䵥λ
}heat_unit_t;	

typedef struct heat_meter_d
{
	long heatAccountDay;        //����������           
	long heatCurrent;           //��ǰ����          
	long Powerheat;             //�ȹ���         
	long Flow;                   //����        
	long AccumulatedFlow;        //�ۼ�����            
	long TSupplyWater;          //��ˮ�¶�              
	long TReturnWater;          //��ˮ�¶�       
	long WorkHours;              //�ۼƹ���ʱ��
	char DateRealTime[30];          //ʵʱʱ�� 
	char states[2];                //���״̬
	int StateValve;            //����״̬     
	int StateBattery;          //���״̬ 
}heat_data_t;	

//������
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
	char U_AccumulatedFlowCurrent[8];        //��ǰ�ۻ�������λ           
	char U_AccumulatedFlowAccountDay[8];     //�������ۻ�������λ
}water_gas_unit_t;	

typedef struct water_gas_meter_d
{
	long AccumulatedFlowCurrent;        //��ǰ�ۻ�����           
	long AccumulatedFlowAccountDay;     //�������ۻ�����          
	char DateRealTime[30];          //ʵʱʱ��              
	char states[2];                //���״̬
	int StateValve;            //����״̬     
	int StateBattery;          //���״̬ 
}water_gas_data_t;	

//ú����
typedef struct gas_meter_s
{
	int type;                    
	char id[20];                       
	char *Timestamp;
	long rtc;
	water_gas_data_t data;
	water_gas_unit_t unit;         
}gas_meter_t;

//��ˮ��
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