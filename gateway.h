#ifndef __GATEWAY_H__
#define __GATEWAY_H__


/*
     �Ƹֱ�����Ʊ���  

������Ʊ���	�������
   11          ��ˮ��
   12          ��ˮ��
   13          ȼ����
   14          ������
   21          ѹ����
   22          ѹ��������
   31          �¶ȼ�
   32          �¶ȱ�����
   41          ���ܱ�
   51          ������
   61          ��ѹ��
   71          ������

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

// --------------------------------�ɼ����Ժ궨��
#define DATA_RULE_BASE 0.01     //���ݲɼ��仯��׼        
#define RULE_MIN_CYCLE MINUTE   //���������С����
#define RULE_MAX_CYCLE HOUR     //��������������

//�ɼ����Ի�д���ݿ���
#define RULE_SAVE_CYCLE  (10*MINUTE)
#define RULE_DEFAUL_RATE 600   //�豸Ĭ�ϲɼ�����:10����

//��������
#define HEARTBEAT_CYCLE (3*MINUTE)

//�ɼ�Ƶ�ʸ�������
#define RULE_UPDATE_CYCLE (3*MINUTE)

#define RULE_DETECT_NUM  5       //����ɼ�����ʱ����ʷ��¼������
#define MAX_NO_RESPONSE_COUNT 10 //�豸����Ӧ�ظ��ɼ�����  


//IOT�豸����
typedef struct device_s
{
	int type;	        /*�豸���ͣ�ˮ����������ȱ� �ȵ�*/
	char id[20];      /*�豸id�����Ϊ���ͨѶ��ַ*/
	char name[20];    /*�豸����*/
	char net_id[10];   /*���߲ɼ�ʱ��Ӧ������id*/
	long collect_time;/*����һ�βɼ�ʱ��*/
	int collect_cycle;/*�ɼ�����*/
	int no_response;/*����Ӧ����*/
}device_t;

typedef struct gateway_device_s
{
	char id[20];               /*�����豸id*/
	char name[64];             /*��������*/
	int dev_num;               /*���������ն��豸����*/
	int now;                   /*��ǰ�ɼ����ն��豸˳����*/
	int next;                  /*��һ���ɼ����ն��豸˳����*/
	device_t dev[MAX_DEV_NUM]; /*�ն��豸*/	
}gateway_t;

#endif