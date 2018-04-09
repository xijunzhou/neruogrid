#ifndef __PROTOCOL_H__
#define __PROTOCOL_H__

#include "dlt645.h"
#include "cj188.h"


/*
type = 10H~19H ,30H~49H     =>     L = 9 + 19
type = 20H~29H              =>     L = 9 + 43
type = 40H                  =>     L = 9 + 32
type = 41H                  =>     L = 9 + 32
水:   T: [0x10 0x11 0x12 0x13 0x20 0x21]
气:   T: 0x30
电:   T: 0x40(单相) 0x41(三相)

UnboiledWater       T = 0x10
DomesticHotWater    T = 0x11
DrinkingWater       T = 0x12
ReclaimedWater      T = 0x13

CalorimeterHolt     T = 0x20
CalorimeterCold     T = 0x21

GasMeter            T = 0x30

ElectricEnergyMeter T = 0x40
*/

/*
USER_FRAME

Sync1(0xa5)  type   Dlen  DATA  Sync2(0x5a)		

    |        |       |      |       |
		 
	  0        1       2     3-N     N+1

type = 0x10 - 0x19 , 0x30:
DATA defined with "wgas_meter_t" in cj188.h  

type = 0x20 - 0x29:
DATA defined with "heat_meter_t" in cj188.h  

type = 0x40:
DATA defined with "single_phase_meter_t" in dlt645.h  

type = 0x41:
DATA defined with "three_phase_meter" in dlt645.h  

*/


/*
DLT645

	0    1    2    3    4    5    6    7    8    9   10-N  N+1  N+2
		                                       
	|    |    |    |    |    |    |    |    |    |    |     |   |
	                                         
0x68  A0   A1    A2   A3  A4   A5  0x68   C    L   DATA  CS  0x16

*/


/*
CJ188

	0    1   2   3   4   5   6   7    8   9   10  11~N  N+1  N+2
		                                       
	|    |   |   |   |   |   |   |    |   |    |   |    |    |
	                                         
0x68   T  A0   A1  A2  A3  A4  A5  A6   C    L  DATA  CS  0x16

*/



#define USER_FRAME_LEN  0     /*帧起始位*/
#define USER_FRAME_TYPE 1     /*类型*/
#define USER_FRAME_ADDR 2     /*地址域*/
#define USER_FRAME_DATA 9     /*数据起始位*/

typedef enum 
{
	DLT645 = 1,
	CJ188,
	USER_FRAME,
}protocl_s;

typedef struct protocol_s
{
	int type;
	char *id;
	void *Pmeter;
	elect_meter_t1 *elect;
	elect_meter_t3 *elect3;
	gas_meter_t  *gas;
	water_cold_meter_t *water_cold;
	heat_meter_t *heat;
}protocol_t;

extern elect_meter_t1 Elect;       
extern elect_meter_t3 Elect3;        
extern gas_meter_t gas;  
extern water_cold_meter_t water_cold;
extern heat_meter_t heat; 
 
extern cj188_t Cj188; 
extern protocol_t Protocol;

int protocol_decode(unsigned char *buff,int size);
#endif