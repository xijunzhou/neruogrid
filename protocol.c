#include <stdlib.h>
#include <stdio.h> /* Standard input/output definitions */
#include <string.h> /* String function definitions */ 
#include <time.h>
#include "gateway.h"
#include "comm.h"
#include "protocol.h"

int decode(unsigned char *buff_src,int len_src,int *head,int *len_des)
{
	int type,start,Dlen;
	int i,flag;
	unsigned char *data = buff_src;
	
//	printf("\r\ndetecetting 主动上报!");
	//             detecet 主动上报
	i = 0;
	flag = 0;
	while(i < len_src)
	{
		switch(flag)
		{
			case 0:
				if(data[i] == 0xa5)
				{
					flag = 1;
					start = i;					
				//	printf("flag = 1;  ");
				}				
				break;
			case 1:
				if(i  == start + 1)
				{
					if(data[i] >= 0x10 && data[i] <= 0x49)
					{
						flag = 2;
						type = data[i];
					//	printf("flag = 2;  ");
					}
					else
						{
							flag = 0;	
							i = start;	
						}	
				}						
				break;
			case 2:
				if(i == start + 2)
				{
					Dlen = data[i];						
				}	
				if(i == start + 3 + Dlen)
				{
							if(data[i] == 0x5a)
								{
						//			printf("flag = 4;  ");
									*len_des = Dlen;
									*head = start + 3;
									return USER_FRAME;
								}
							else
								{
									flag = 0;	
									i = start;
								}	
				}
				
			break;	
			default:
				break;		
		}
		i++;
	}
//	printf("\r\ndetecetting DLT645!");
	//             detecet DLT645
	i = 0;
	flag = 0;
	while(i < len_src)
	{
		switch(flag)
		{
			case 0:
				if(data[i] == 0x68)
				{
					flag = 1;
					start = i;					
				//	printf("flag = 1;  ");
				}				
				break;
			case 1:
				if((i == 7 + start))
				{
					if(data[i] == 0x68)
					{
						flag = 2;
				//		printf("flag = 2;  ");
					}
					else
						{
							flag = 0;	
							i = start;	
						}	
				}						
				break;
			case 2:
				if(i == 9 + start)
				{
					Dlen = data[i];
					flag = 3;
			//		printf("flag = 3;  ");
				}						
				break;
			case 3:
				if(i == 11 + Dlen + start)
				{
			//		printf("flag = 33;  ");
					if(data[i] == 0x16)
					{
						*len_des = i - start + 1;
						*head = start;

						return DLT645;
					}
					flag = 0;	
					i = start;	
				}	
			break;	
			default:
				break;		
		}
		i++;
	}
	
	//printf("\r\ndetecetting CJ188!");
	//             detecet CJ188
	i = 0;
	flag = 0;
	while(i < len_src)
	{
		switch(flag)
		{
			case 0:
				if(data[i] == 0x68)
				{
					flag = 1;
					start = i;
				//	printf("flag = 1，start = %d;  ",start);
				}				
				break;
			case 1:
				if((data[i] >= 0x10 && data[i] < 0x19) || data[i] == 0x20 || data[i] == 0x21 || data[i] == 0x30)
				{
					flag = 2;
				//	printf("flag = 2;  ");
				}
				else
				{
					flag = 0;
					i = start;
				}			
				break;
			case 2:
				if(i == 10 + start)
				{
					Dlen = data[i];
					flag = 3;
				//	printf("flag = 3,Dlen = %d;  ",Dlen);
				}						
				break;
			case 3:
				if(i == CJ188_DATA + 1 + Dlen + start)
				{
				//	printf("flag = 4; data[i] = %x ",data[i]);
					if(data[i] == 0x16)
						{
							*len_des = i - start + 1;
							*head = start;
							return CJ188;
						}
					flag = 0;	
					i = start;
				}	
				break;
			default:
				break;		
		}
		i++;
	}
	
	printf("\r\ndecode error!");
	return -1;		
}

/*
-------------USER_FRAME-------------------
   |                                        |
 Sync1(0xa5)  type   Dlen  DATA  Sync2(0x5a)		

    |        |       |      |       |
		 
	  0        1       2     3-N     N+1  
	  
----------------DATA----------------------

单相表数据格式（每个数据字段占 4 byts,地位在前高位在后）:
  type,id,
  Voltage,Current,Frequency,ActPower,ReactPower,Factor,
  EnergyForward,EnergyBackward,
  EnergyReactCombination1 ,EnergyReactCombination2,
  QuadrantReactCombination1,QuadrantReactCombination2,QuadrantReactCombination3,QuadrantReactCombination4,
  ActiveForwardDemand,ReactForwardDemand 
        
三相表数据格式:
	type,id,
	VoltageA ,VoltageB ,VoltageC ,
	CurrentA ,CurrentB ,CurrentC,
	Frequency ,
	ActPower ,ActPowerA ,ActPowerB ,ActPowerC   
	ReactPower ,ReactPowerA ,ReactPowerB ,ReactPowerC ,
	Factor ,FactorA ,FactorB ,FactorC ,    
	EnergyForward ,EnergyBackward,
	EnergyReactCombination1 ,EnergyReactCombination2,
	QuadrantReactCombination1 ,QuadrantReactCombination2 ,QuadrantReactCombination3 ,QuadrantReactCombination4,
	ActiveForwardDemand ,ReactForwardDemand
	  
*/
int user_frame_decode(unsigned char *buff,int nbyte,protocol_t *Protocol)
{
	
	int i,type;
	time_t now;
	struct tm *p;
	unsigned char *ptime = NULL;
	unsigned char *data = buff;
	type = data[0];		

	if(type == 0x40)                                               //单相电表
		{
			Protocol->type = type;
			Elect.type = Protocol->type;
			for(i=0;i<6;i++)
				  	Elect.id[5 - i] = data[i];
			Elect.id[i] = '\0';
      Protocol->id = Elect.id;
      
					time(&now);
					ptime = asctime(localtime(&now));
					memcpy(Elect.Timestamp,ptime,sizeof(ptime));
					
					p = localtime(&now);
			
			Elect.DateRealTime[0] = (1900 + p->tm_year)/100;
			Elect.DateRealTime[1] = (1900 + p->tm_year)%100;
			Elect.DateRealTime[2] = p->tm_mon + 1;
			Elect.DateRealTime[3] = p->tm_mday;
			Elect.DateRealTime[4] = p->tm_hour;
			Elect.DateRealTime[5] = p->tm_min;
			Elect.DateRealTime[6] = p->tm_sec;
			
	                      
      i = 7;
			Elect.Voltage = dlt645_data_to_int32(&data[i],4);
			i += 4;
			Elect.Current = dlt645_data_to_int32(&data[i],4);
			i += 4;
			Elect.Frequency = dlt645_data_to_int32(&data[i],4);
			
			i += 4;
			Elect.ActPower = dlt645_data_to_int32(&data[i],4);
			i += 4;
			Elect.ReactPower = dlt645_data_to_int32(&data[i],4);
			i += 4;
			Elect.Factor = dlt645_data_to_int32(&data[i],4);
			
			i += 4;	
			Elect.EnergyForward = dlt645_data_to_int32(&data[i],4);
			i += 4;
			Elect.EnergyBackward = dlt645_data_to_int32(&data[i],4);
			i += 4;		
		
			Elect.EnergyReactCombination1 = dlt645_data_to_int32(&data[i],4);
			i += 4;	
			Elect.EnergyReactCombination2 = dlt645_data_to_int32(&data[i],4);
			i += 4;		
		
			Elect.QuadrantReactCombination1 = dlt645_data_to_int32(&data[i],4);
			i += 4;		
			Elect.QuadrantReactCombination2 = dlt645_data_to_int32(&data[i],4);
			i += 4;	
			Elect.QuadrantReactCombination3 = dlt645_data_to_int32(&data[i],4);
			i += 4;	
		  Elect.QuadrantReactCombination4 = dlt645_data_to_int32(&data[i],4);
			i += 4;	
				
	    Elect.ActiveForwardDemand = dlt645_data_to_int32(&data[i],4);
			i += 4;	
		  Elect.ReactForwardDemand = dlt645_data_to_int32(&data[i],4);
			i += 4;														

		}
	else if(type == 0x41)                                               //三相电表
		{
			Protocol->type = type;
			Elect3.type = Protocol->type;
			for(i=0;i<6;i++)
				  	Elect3.id[5 - i] = data[i];
			Elect3.id[i] = '\0';
      Protocol->id = Elect3.id;
      
					time(&now);
					ptime = asctime(localtime(&now));
					memcpy(Elect3.Timestamp,ptime,sizeof(ptime));
					
					p = localtime(&now);
			
			Elect3.DateRealTime[0] = (1900 + p->tm_year)/100;
			Elect3.DateRealTime[1] = (1900 + p->tm_year)%100;
			Elect3.DateRealTime[2] = p->tm_mon + 1;
			Elect3.DateRealTime[3] = p->tm_mday;
			Elect3.DateRealTime[4] = p->tm_hour;
			Elect3.DateRealTime[5] = p->tm_min;
			Elect3.DateRealTime[6] = p->tm_sec;
	                      
      i = 7;
			Elect3.VoltageA = dlt645_data_to_int32(&data[i],4);
			i += 4;
			Elect3.VoltageB = dlt645_data_to_int32(&data[i],4);
			i += 4;
			Elect3.VoltageC = dlt645_data_to_int32(&data[i],4);
			
			i += 4;
			Elect3.CurrentA = dlt645_data_to_int32(&data[i],4);
			i += 4;
			Elect3.CurrentB = dlt645_data_to_int32(&data[i],4);
			i += 4;
			Elect3.CurrentC = dlt645_data_to_int32(&data[i],4);
			
			i += 4;
			Elect3.Frequency = dlt645_data_to_int32(&data[i],4);
			
			i += 4;
			Elect3.ActPower = dlt645_data_to_int32(&data[i],4);
			i += 4;
			Elect3.ActPowerA = dlt645_data_to_int32(&data[i],4);
			i += 4;
			Elect3.ActPowerB = dlt645_data_to_int32(&data[i],4);
			i += 4;
			Elect3.ActPowerC = dlt645_data_to_int32(&data[i],4);
			
			i += 4;
			Elect3.ReactPower = dlt645_data_to_int32(&data[i],4);
			i += 4;
			Elect3.ReactPowerA = dlt645_data_to_int32(&data[i],4);
			i += 4;
			Elect3.ReactPowerB = dlt645_data_to_int32(&data[i],4);
			i += 4;
			Elect3.ReactPowerC = dlt645_data_to_int32(&data[i],4);
			
			i += 4;
			Elect3.Factor = dlt645_data_to_int32(&data[i],4);
			i += 4;
			Elect3.FactorA = dlt645_data_to_int32(&data[i],4);
			i += 4;
			Elect3.FactorB = dlt645_data_to_int32(&data[i],4);
			i += 4;
			Elect3.FactorC = dlt645_data_to_int32(&data[i],4);
			
			i += 4;	
			Elect3.EnergyForward = dlt645_data_to_int32(&data[i],4);
			i += 4;
			Elect3.EnergyBackward = dlt645_data_to_int32(&data[i],4);
			i += 4;		
		
			Elect3.EnergyReactCombination1 = dlt645_data_to_int32(&data[i],4);
			i += 4;	
			Elect3.EnergyReactCombination2 = dlt645_data_to_int32(&data[i],4);
			i += 4;		
		
			Elect3.QuadrantReactCombination1 = dlt645_data_to_int32(&data[i],4);
			i += 4;		
			Elect3.QuadrantReactCombination2 = dlt645_data_to_int32(&data[i],4);
			i += 4;	
			Elect3.QuadrantReactCombination3 = dlt645_data_to_int32(&data[i],4);
			i += 4;	
		  Elect3.QuadrantReactCombination4 = dlt645_data_to_int32(&data[i],4);
			i += 4;	
				
	    Elect3.ActiveForwardDemand = dlt645_data_to_int32(&data[i],4);
			i += 4;	
		  Elect3.ReactForwardDemand = dlt645_data_to_int32(&data[i],4);
			i += 4;														

		}	
	if(type >= 0x10 && type <= 0x30)   //水表、气表 热量表
		{
			if(-1 != cj188_decode(buff,nbyte,&Cj188))
			{
				Protocol->type = Cj188.type;
				Protocol->id = Cj188.id;		
			}
		}		

	return type;
}



int protocol_decode(unsigned char *buff,int size)
{
	int type = -1;	
	int len = size;
	int head = 0;
	type = decode(buff,size,&head,&len);
	
//	printf("\r\n------------type:%d,head:%d,len:%d: ",type,head,len);
//	for(int i = head;i < len + head;i++)
//		printf("%02x ",buff[i]);		
	if(type == DLT645)
		{
			

			if(-1 != dlt645_decode(&buff[head],len,&Elect))
			{
				printf("\ndetected DLT645 Frame!0x%02x:%s",Elect.type,Elect.id);
  		  Protocol.type = Elect.type;
  		  Protocol.id = Elect.id;
				return Elect.type;
			}	


			if(-1 != dlt645_decode1(&buff[head],len,&Elect3))
				{
					printf("\ndetected DLT645 Frame!,0x%02x:%s",Elect3.type,Elect3.id);
				//	dlt645_buff_print1(Elect3.type);
				  Protocol.type = Elect3.type;
				  Protocol.id = Elect3.id;
				  return Elect3.type;
			}
			else
				return -1;	
		}
	else if(type == CJ188)
		{
			

			if(-1 != cj188_decode(&buff[head],len,&Cj188))
				{
					printf("\n======>detected CJ188 Frame!0x%02x:%s",Cj188.type,Cj188.id);
				//	cj188_buff_print(Cj188.type);
					Protocol.type = Cj188.type;
					Protocol.id = Cj188.id;
					return Cj188.type;
				}
			else
				return -1;		
		}

	else if(type == USER_FRAME)
		{
			if(-1 != user_frame_decode(&buff[head],len,&Protocol))
				{
					printf("\ndetected User Data Frame!");
					return Protocol.type;
				  //return user_buff_print(Protocol.type);
			}
			else
				return -1;	
		}	
	else
		{
			printf("\n======>Can't identify data frame format!");	
		}
		return -1;
}

