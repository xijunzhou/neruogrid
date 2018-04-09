#include <stdlib.h>
#include <stdio.h> /* Standard input/output definitions */
#include <string.h> /* String function definitions */
#include <time.h>

#include "gateway.h"
#include "comm.h"
#include "cj188.h"

/*  示例
const char METER_TEST_ACK[] = 
{0x68,0x10,0x06,0x00,0x04,0x00,0x00,0x00,0x00,
	0x81,
	0x16,
	0x1f,0x90,
	0x00,
	0x12,0x34,0x56,0x78,0x2c,
	0x23,0x45,0x67,0x89,0x2c,
	10,15,12,23,11,17,20,
	0x21,0x84,
	0x9d,
	0x16};
 */


int CJ188_UNIT_CODE[] = 
        {0x02,0x05,0x08,0x0A,0x01,0x0B,0x0E,0x11,0x13,0x14,0x17,0x1A,0x29,0x2C,0x32,0x35};
char *CJ188_UNIT[] =
				{"wh","kWh","mWh","mWhx100","J","kJ","Mj","GJ","GJx100","W","kW","mW","L","m3","L/h","m3/h"};

char *cj188_unit(int code)
{
	int i;
	char *unit = NULL;
	for(i = 0; i < sizeof(CJ188_UNIT_CODE); i++)
		if(code == CJ188_UNIT_CODE[i])
			unit = CJ188_UNIT[i];
  return unit;			
}			
				
				
int cj188_decode(char *buff,int nbyte,cj188_t *Cj188)
{
	int i,type,len;
	long res;
	char temp[10] = {0,1,2,3,4,5,6,7,8,9};
	unsigned char cs = 0;
	char *data = buff;
	
	for(i = 0; i < nbyte - 2; i++)
				cs += data[i];
//	printf("cs : %x,data[cs] = %x",cs,data[i]);
	
	Cj188->type = -1;
	if(cs == data[i])
	{
			//		printf("\r\ncj188 cs ok!\n");

					if(data[CJ188_CTRL] == CJ188_CTRL0_READ_DATA)						
						{
							printf("\nrecive other host cmd\n");
							return -1;	
						}				
					if(data[CJ188_CTRL] != CJ188_CTRL1_READ_DATA)
						{
							printf("应答帧异常CTRL1:0x%02x\n",data[CJ188_CTRL]);
							return Cj188->type;	
						}
					
					if(data[CJ188_DATA] != CJ188_DI0_READ_DATA || data[CJ188_DATA + 1] != CJ188_DI1_READ_DATA)						
						{
							printf("不支持的数据项,数据标识DI0,DI1:0x%02x%02x\n",data[CJ188_DATA],data[CJ188_DATA + 1]);
							return Cj188->type;	
						}

					Cj188->type = data[1];
				
					
					if(Cj188->type >= 0x10 && Cj188->type <= 0x19)   //水表
					{	
				
						Cj188->water_cold->type = Cj188->type;                          //类型
						memcpy(Cj188->water_cold->data.states,&data[WG_ZTST],2);               //状态
										
						//ID   
						len = sprintf(Cj188->water_cold->id,"%02x%02x%02x%02x%02x%02x%02x",data[CJ188_ADDR+6],data[CJ188_ADDR+5],data[CJ188_ADDR+4],data[CJ188_ADDR+3],data[CJ188_ADDR+2],data[CJ188_ADDR+1],data[CJ188_ADDR]);
						Cj188->water_cold->id[len] = '\0';
						Cj188->id = Cj188->water_cold->id;
						
						//实时时间
						len = sprintf(Cj188->water_cold->data.DateRealTime,"%02d%02d%02d%02d%02d%02d%02d",data[WG_SSSJ+6],data[WG_SSSJ+5],data[WG_SSSJ+4],data[WG_SSSJ+3],data[WG_SSSJ+2],data[WG_SSSJ+1],data[WG_SSSJ]);
						Cj188->water_cold->data.DateRealTime[len] = '\0';	
						
						memcpy(temp,&data[WG_AccumulatedFlowCurrent],5);                          //当前累积流量
					//	ReverseByArray(temp,5);
						Cj188->water_cold->data.AccumulatedFlowCurrent = bcds_to_int32(temp,4);
						strcpy(Cj188->water_cold->unit.U_AccumulatedFlowCurrent,cj188_unit(temp[4]));
						
				  	memcpy(temp,&data[WG_JSRAccumulatedFlow],5);                         //结算日累积流量
				 // 	ReverseByArray(temp,5);
						Cj188->water_cold->data.AccumulatedFlowAccountDay = bcds_to_int32(temp,4);
						strcpy(Cj188->water_cold->unit.U_AccumulatedFlowAccountDay,cj188_unit(temp[4]));
						
		  	
						if(Cj188->water_cold->data.states[0] & BATTERY_MASK)                           //电池欠压
								Cj188->water_cold->data.StateValve = BATTER_LOW;
						else                                                             //电池正常
							Cj188->water_cold->data.StateValve = BATTERY_OK;
							
						if((Cj188->water_cold->data.states[0] & FAMEN_MASK) == FAMEN_CLOSE_MASK)       //阀门关
								Cj188->water_cold->data.StateBattery = FAMEN_CLOSE;
						else if((Cj188->water_cold->data.states[0] & FAMEN_MASK) == FAMEN_OPEN_MASK)   //阀门开
								Cj188->water_cold->data.StateBattery = FAMEN_OPEN;
						else                                                             //阀门异常
								Cj188->water_cold->data.StateBattery = FAMEN_ABNORMAL;		

			  	
					}					
					else if(Cj188->type >= 0x20 && Cj188->type <= 0x29)           //热量表
						{
							
						Cj188->heat->type = Cj188->type;                                  //类型       
						memcpy(Cj188->heat->data.states,&data[HEAT_ZTST],2);                     //状态       
						
						//ID   
						len = sprintf(Cj188->heat->id,"%02x%02x%02x%02x%02x%02x%02x",data[CJ188_ADDR+6],data[CJ188_ADDR+5],data[CJ188_ADDR+4],data[CJ188_ADDR+3],data[CJ188_ADDR+2],data[CJ188_ADDR+1],data[CJ188_ADDR]);
						Cj188->heat->id[len] = '\0';
						Cj188->id = Cj188->heat->id;
						
						//实时时间
						len = sprintf(Cj188->heat->data.DateRealTime,"%02d%02d%02d%02d%02d%02d%02d",data[HEAT_SSSJ+6],data[HEAT_SSSJ+5],data[HEAT_SSSJ+4],data[HEAT_SSSJ+3],data[HEAT_SSSJ+2],data[HEAT_SSSJ+1],data[HEAT_SSSJ]);
						Cj188->heat->data.DateRealTime[len] = '\0';							
						
							memcpy(temp,&data[HEAT_heatAccountDay],5);                                /*结算日热量*/   
							Cj188->heat->data.heatAccountDay = bcds_to_int32(temp,4);                      
							strcpy(Cj188->heat->unit.U_heatAccountDay,cj188_unit(temp[4]));							
	                                                                            
							memcpy(temp,&data[HEAT_heatCurrent],5);                                /*当前热量*/     
							Cj188->heat->data.heatCurrent = bcds_to_int32(temp,4);                         
							strcpy(Cj188->heat->unit.U_heatCurrent,cj188_unit(temp[4]));               
                                                                               
							memcpy(temp,&data[HEAT_Powerheat],5);							                  /*热功率*/                   
							Cj188->heat->data.Powerheat = bcds_to_int32(temp,4);                              
							strcpy(Cj188->heat->unit.U_Powerheat,cj188_unit(temp[4]));                    
                                                                                   
							memcpy(temp,&data[HEAT_Flow],5);							                  /*流量*/                     
							Cj188->heat->data.Flow = bcds_to_int32(temp,4);                                             
							strcpy(Cj188->heat->unit.U_Flow,cj188_unit(temp[4]));                 
                                                                               
							memcpy(temp,&data[HEAT_AccumulatedFlow],5);							                   /*累计流量*/           
							Cj188->heat->data.AccumulatedFlow = bcds_to_int32(temp,4);                           
							strcpy(Cj188->heat->unit.U_AccumulatedFlow,cj188_unit(temp[4]));                 
							                                                                   
							memcpy(temp,&data[HEAT_TSupplyWater],3);							                   /*供水温度*/             
							Cj188->heat->data.TSupplyWater = bcds_to_int32(temp,3);                              
							strcpy(Cj188->heat->unit.U_TSupplyWater,"C");                                   
                                                                                 
							memcpy(temp,&data[HEAT_TReturnWater],3);							                    /*回水温度*/       
							Cj188->heat->data.TReturnWater = bcds_to_int32(temp,3);                                           
							strcpy(Cj188->heat->unit.U_TReturnWater,"C");                               
                                                                                
							memcpy(temp,&data[HEAT_WorkHours],3);							                   /*累计工作时间*/    
							Cj188->heat->data.WorkHours = bcds_to_int32(temp,3);                             
							strcpy(Cj188->heat->unit.U_WorkHours,"h");    
							
							if(Cj188->heat->data.states[0] & BATTERY_MASK)                           //电池欠压
									Cj188->heat->data.StateValve = BATTER_LOW;
							else                                                             //电池正常
								Cj188->heat->data.StateValve = BATTERY_OK;
								
							if((Cj188->heat->data.states[0] & FAMEN_MASK) == FAMEN_CLOSE_MASK)       //阀门关
									Cj188->heat->data.StateBattery = FAMEN_CLOSE;
							else if((Cj188->heat->data.states[0] & FAMEN_MASK) == FAMEN_OPEN_MASK)   //阀门开
									Cj188->heat->data.StateBattery = FAMEN_OPEN;
							else                                                             //阀门异常
									Cj188->heat->data.StateBattery = FAMEN_ABNORMAL;											  	
						}
					else if(Cj188->type == 0x30)                                 //气表
					{	
				
						Cj188->gas->type = Cj188->type;                          //类型
						memcpy(Cj188->gas->data.states,&data[WG_ZTST],2);        //状态
										
						//ID   
						len = sprintf(Cj188->gas->id,"%02x%02x%02x%02x%02x%02x%02x",data[CJ188_ADDR+6],data[CJ188_ADDR+5],data[CJ188_ADDR+4],data[CJ188_ADDR+3],data[CJ188_ADDR+2],data[CJ188_ADDR+1],data[CJ188_ADDR]);
						Cj188->gas->id[len] = '\0';
						Cj188->id = Cj188->gas->id;
						
						//实时时间
						len = sprintf(Cj188->gas->data.DateRealTime,"%02d%02d%02d%02d%02d%02d%02d",data[WG_SSSJ+6],data[WG_SSSJ+5],data[WG_SSSJ+4],data[WG_SSSJ+3],data[WG_SSSJ+2],data[WG_SSSJ+1],data[WG_SSSJ]);
						Cj188->gas->data.DateRealTime[len] = '\0';	
						
						memcpy(temp,&data[WG_AccumulatedFlowCurrent],5);                          //当前累积流量
					//	ReverseByArray(temp,5);
						Cj188->gas->data.AccumulatedFlowCurrent = bcds_to_int32(temp,4);
						strcpy(Cj188->gas->unit.U_AccumulatedFlowCurrent,cj188_unit(temp[4]));
						
				  	memcpy(temp,&data[WG_JSRAccumulatedFlow],5);                         //结算日累积流量
				 // 	ReverseByArray(temp,5);
						Cj188->gas->data.AccumulatedFlowAccountDay = bcds_to_int32(temp,4);
						strcpy(Cj188->gas->unit.U_AccumulatedFlowAccountDay,cj188_unit(temp[4]));
						
		  	
						if(Cj188->gas->data.states[0] & BATTERY_MASK)                           //电池欠压
								Cj188->gas->data.StateValve = BATTER_LOW;
						else                                                             //电池正常
							Cj188->gas->data.StateValve = BATTERY_OK;
							
						if((Cj188->gas->data.states[0] & FAMEN_MASK) == FAMEN_CLOSE_MASK)       //阀门关
								Cj188->gas->data.StateBattery = FAMEN_CLOSE;
						else if((Cj188->gas->data.states[0] & FAMEN_MASK) == FAMEN_OPEN_MASK)   //阀门开
								Cj188->gas->data.StateBattery = FAMEN_OPEN;
						else                                                             //阀门异常
								Cj188->gas->data.StateBattery = FAMEN_ABNORMAL;		

			  	
					}	
	
	}
	return Cj188->type; 
}

int cj188_encode(char cj188[],char *id,int type)
{
	static unsigned char SER = 0;
	int i,len;
	unsigned char cs;
	cs = 0;
	len = 0;
	cj188[CJ188_START]= CJ188_FRAME_START;                         /*帧起始位   */  
	cj188[CJ188_TYPE] = type;
	
	StringToBcd(id,&cj188[CJ188_ADDR],1);                           /*地址域   */
	 
	cj188[CJ188_CTRL] = CJ188_CTRL0_READ_DATA;                     /*控制码位   */   
	cj188[CJ188_DLEN] = 3;                                         /*数据长度位 */ 
	len = CJ188_DATA;
	
	cj188[len++] = CJ188_DI0_READ_DATA;                           /*数据位     */   
	cj188[len++] = CJ188_DI1_READ_DATA;
	cj188[len++] = SER;
	
  for(i=0;i<len;i++)                              
		{
			cs += cj188[i];
		}
	cj188[len++]= cs;                                                /*校验位     */ 
	cj188[len++]= CJ188_FRAME_END;	                                /*帧结束位   */
	return len;	
}