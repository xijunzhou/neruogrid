#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/wait.h>
#include <net/if.h>        //for struct ifreq


#include <sys/ioctl.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>
#include <limits.h>
#include <time.h>
#include <pthread.h> 
#include <string.h>

//#include <sqlite3.h>  
#include "sqlite3.h"  
#include "gateway.h"
#include "comm.h"
#include "cj188.h"
#include "dlt645.h"
#include "protocol.h"
#include "host_ip_by_name.h"

#define COLLECT_ENERGY_FORWARDA_ONLY 0

#define JOHN_TEST 0
#define JOHN_DEBUG 1

#ifndef LAN_BUG
#define LAN_BUG 1
#endif
#undef LAN_BUG

#define SERVER_LAN_IP "192.168.1.120"


//Epeis 服务器
#define EPEIS_IP "118.24.14.213"

// 域名
#define SERVER_DNS_HOST "18y873720n.51mypc.cn"
#define SERVER_DNS_IP "103.44.145.245"
#define SERVER_PORT 24834

// MQTT
#ifdef LAN_BUG
#define MQTT_HOST  SERVER_LAN_IP
#else
#define MQTT_HOST  EPEIS_IP
#endif
const char* MQTT_PORT = "1883";

// TCP/IP
#ifdef LAN_BUG
#define SERVER_IP SERVER_LAN_IP
#else
#define SERVER_IP EPEIS_IP
#endif

#define HOST_NAME SERVER_DNS_HOST
#define HOST_PORT SERVER_PORT



#define SYS_DTU_STUP 0
#define BUFF_LEN 2048

#define DATA_LEN           0xFF           
#define MAX_EPOLL_EVENT    6

#define METER_BUFFER_SIZE  256
#define GPRS_BUFFER_SIZE   1024

#define LORA_UART
#define LORA_USB

#undef LORA_UART

#ifdef LORA_UART
#undef LORA_USB
#define LORA_DEV "/dev/ttyO1"
#define LORA_DEV_BRADRATE B115200
#else
#define LORA_USB
//#define LORA_DEV "/dev/ttyUSB0"
#define LORA_DEV "/dev/ttyUSB-lora"
#define LORA_DEV_BRADRATE B9600
#endif



#define GPRS_DEV "/dev/ttyO2"
#define GPRS_DEV_BRADRATE B115200
	  
int fd_gprs;  //文件描述符  gprs设备
int fd_meter; //文件描述符  meter设备

int epid_gprs;  //epoll标识符   gprs设备 
int epid_meter; //epoll标识符   meter设备

struct epoll_event events_gprs[MAX_EPOLL_EVENT];//事件集合  gprs设备
struct epoll_event events_meter[MAX_EPOLL_EVENT];//事件集合  meter设备
 


gprs_state_s gprs_State = OffLine;



char Meter_ReadBuff[METER_BUFFER_SIZE] = {'\0'};
char GPRS_ReadBuff[GPRS_BUFFER_SIZE] = {'\0'};

char water_cold_meter_buff[GPRS_BUFFER_SIZE] = {'\0'};
char gas_meter_buff[GPRS_BUFFER_SIZE] = {'\0'};
char heat_meter_buff[GPRS_BUFFER_SIZE] = {'\0'};
char elect_meter_buff[GPRS_BUFFER_SIZE] = {'\0'};
char elect3_meter_buff[GPRS_BUFFER_SIZE] = {'\0'};


char HeartBeatGateway[50] = {'\0'};
const char MessageRegistering[] = "Registering DSC...";
const char GPRS_TEST[] = "THIS IS DTU\r\n";


heat_meter_t heat;
gas_meter_t gas;
water_cold_meter_t water_cold;
cj188_t Cj188;
elect_meter_t1 Elect;
elect_meter_t3 Elect3;
protocol_t Protocol;



gateway_t gateway;
device_t *device;

int rule_update_cycle = 60;

//电表测试
const char METER_TEST[16] = {0x68,0x06,0x00,0x04,0x00,0x00,0x00,0x68,0x11,0x04,0x33,0x33,0x34,0x33,0xBC,0x16};

 //1234.5678           
 //2345.6789           
 //2017-11-23 12:15:10 
//水表、气表测试
const char METER_WGAS_TEST[] = 
{ 
	0x68,
	0x10,
	0x06,0x00,0x04,0x00,0x00,0x00,0x00,
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
	
	
//热量表测试	
const char METER_HEAT_TEST[] = 
{
	0x68,
	0x20,
	0x06,0x00,0x04,0x00,0x00,0x00,0x00,
	0x81,
	0x2e,
	0x1f,0x90,
	0x00,
	0x11,0x22,0x33,0x44,0x0b,
	0x11,0x22,0x33,0x44,0x0b,
	0x11,0x22,0x33,0x44,0x17,
	0x11,0x22,0x33,0x44,0x2c,
	0x11,0x22,0x33,0x44,0x2c,
	0x11,0x22,0x33,
	0x11,0x22,0x33,
	0x11,0x22,0x33,
	10,15,12,23,11,17,20,
	0x21,0x84,
	0x0a,
	0x16
};

//电表最近一次负荷记录
//主站命令：0x68,0x06,0x00,0x04,0x00,0x00,0x00,0x68,0x11,0x04,0x33,0x33,0x34,0x33,0xBC,0x16
const char METER_ELECT_TEST[] = 
{
	0x68,0x06,0x00,0x04,0x00,0x00,0x00,0x68,0x11,0x04,0x33,0x33,0x34,0x33,0xBC,0x16	
};

const char meterID[6][7] = 
{
	{0x68,0x00,0x00,0x00,0x04,0x00,0x06},
	{0x68,0x00,0x00,0x00,0x04,0x00,0x06},
	{0x68,0x00,0x00,0x00,0x04,0x00,0x06},
	{0x68,0x00,0x00,0x00,0x04,0x00,0x06},
	{0x68,0x00,0x00,0x00,0x04,0x00,0x06},
	{0x68,0x00,0x00,0x00,0x04,0x00,0x06}
};



int SetParity(int fd_gprs,speed_t bardrate,int databits, int parity,int stopbits)
{
  struct termios Opt;

  if(tcgetattr(fd_gprs, &Opt) != 0)
    {
        perror("tcgetattr fd_gprs");
        return -1;
    }

  Opt.c_cflag = (CLOCAL | CREAD | bardrate);        //一般必设置的标志
	Opt.c_oflag = 0;
	Opt.c_lflag = 0;
	Opt.c_iflag = 0;
  switch(databits)        //设置数据位数
    {
	    case 7:
	        Opt.c_cflag &= ~CSIZE;
	        Opt.c_cflag |= CS7;
	    	break;
	    case 8:
	        Opt.c_cflag &= ~CSIZE;
	        Opt.c_cflag |= CS8;
	    	break;
	    default:
	        fprintf(stderr, "Unsupported data size.\n");
	    	return -1;
    }

  switch(parity)            //设置校验位
    {
	    case 'n':
	    case 'N':
	        Opt.c_cflag &= ~PARENB;        //清除校验位
	        Opt.c_iflag &= ~INPCK;        //enable parity checking
	      break;
	    case 'o':
	    case 'O':
	        Opt.c_cflag |= PARENB;        //enable parity
	        Opt.c_cflag |= PARODD;        //奇校验
	        Opt.c_iflag |= INPCK;           //disable parity checking
	      break;
	    case 'e':     
	    case 'E':         
			    Opt.c_cflag |= PARENB;        //enable parity         
			    Opt.c_cflag &= ~PARODD;        //偶校验         
			    Opt.c_iflag |= INPCK;            //disable pairty checking         
		    break;    
	    case 's':     
	    case 'S':         
			    Opt.c_cflag &= ~PARENB;        //清除校验位         
			    Opt.c_cflag &= ~CSTOPB;        //??????????????         
			    Opt.c_iflag |= INPCK;          //disable pairty checking       
		    break;     
	    default:         
		      fprintf(stderr, "Unsupported parity.\n");         
		    return -1;         
  	}     
  switch(stopbits)        //设置停止位     
		{     
		  case 1:         
			  	Opt.c_cflag &= ~CSTOPB;         
			  break;     
		  case 2:       
		    	Opt.c_cflag |= CSTOPB;         
		  	break;     
		  default:       
		    	fprintf(stderr, "Unsupported stopbits.\n");         
		  	return -1;     
		}    	 

	Opt.c_cc[VTIME] = 100;        //设置超时为15sec     
	Opt.c_cc[VMIN] = 0;        //Update the Opt and do it now     
	if(tcsetattr(fd_gprs, TCSANOW, &Opt) != 0)     
	 {         
		 perror("tcsetattr fd_gprs");        
		 return -1;     
	 }    
  return 0; 
}


int gprs_EpollInit(void)  
{  
    struct epoll_event event;
		int ret;
		epid_gprs = epoll_create(MAX_EPOLL_EVENT);          //放在初始化  
	  if(epid_gprs < 0)
	  	{  		
	    	perror("epoll_create");
	    	return -1;
	    }
		event.data.fd = fd_gprs; /* return the fd to us later		*/
		event.events = EPOLLET | EPOLLIN;
		ret = epoll_ctl (epid_gprs, EPOLL_CTL_ADD, fd_gprs, &event);
		if (ret)
		perror ("epoll_ctl");
    return 0;
} 


int meter_EpollInit(void)  
{  
    struct epoll_event event;
		int ret;
		epid_meter = epoll_create(MAX_EPOLL_EVENT);          //放在初始化  
	  if(epid_meter < 0)
	  	{  		
	    	perror("epoll_create");
	    	return -1;
	    }
		event.data.fd = fd_meter; /* return the fd to us later		*/
		event.events = EPOLLET | EPOLLIN;

		ret = epoll_ctl (epid_meter, EPOLL_CTL_ADD, fd_meter, &event);
		if (ret)
		perror ("epoll_ctl");
    return 0;
}  

/****
  * Function: int gprs_hw_init(void)
  *@Describe:串口初始化 波特率 115200
             阻塞模式，数据格式：8-N-1 

  *@Param_1 : 
  *@Param_2 : 
  *@Param_3 : 
  *@Return  : 返回 串口设备描述符
  */

static int gprs_hw_init(void)
{
	struct termios opt;
	fd_gprs = open(GPRS_DEV,O_RDWR | O_NOCTTY |O_NONBLOCK);
	if(fd_gprs < 0)
	{
		printf("\r\ncan't open %s",GPRS_DEV);
			return fd_gprs;	
	}
	printf("\r\n open %s successfully!",GPRS_DEV);

	if(SetParity(fd_gprs,GPRS_DEV_BRADRATE,8,'N',1) != 0)
		return -1;
	printf("\r\nset %s %d: 8 N 1!",GPRS_DEV,GPRS_DEV_BRADRATE);	
	return 0;
}


static int meter_hw_init(void)
{

	struct termios opt;
	fd_meter = open(LORA_DEV,O_RDWR | O_NOCTTY |O_NONBLOCK);
	if(fd_meter < 0)
		{
		printf("\r\ncan't open %s",LORA_DEV);
		return fd_meter;	
	}
	printf("\r\n$open %s successfully!",LORA_DEV);		
	if(SetParity(fd_meter,LORA_DEV_BRADRATE,8,'N',1) != 0)
		return -1;

	printf("set %s %d: 8 N 1!",LORA_DEV,LORA_DEV_BRADRATE);	
		
	return 0;
}

int meter_hw_open(void)
{
	int ret = -1;
	ret = meter_hw_init();
	tcflush(fd_meter,TCIOFLUSH);  //清空串口输入输出缓存 
	if(ret == 0)
	{
		ret = meter_EpollInit();		
	}
	return ret;	
}

int meter_hw_close(void)
{
	close(epid_meter);
	close(fd_meter);
	printf("\n$close dev:%s",LORA_DEV);
}


int get_mac(char * mac, int len_limit)    //返回值是实际写入char * mac的字符个数（不包括'\0'）
{
    struct ifreq ifreq;
    int sock;

    if ((sock = socket (AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror ("socket");
        return -1;
    }
    strcpy (ifreq.ifr_name, "eth0");    //Currently,  get eth0
    if (ioctl (sock, SIOCGIFHWADDR, &ifreq) < 0)
    {
    	strcpy (ifreq.ifr_name, "eth1");    //Currently,  get eth1
	    if (ioctl (sock, SIOCGIFHWADDR, &ifreq) < 0)
	    {
	        strcpy (ifreq.ifr_name, "eth2");    //Currently,  get eth2
			    if (ioctl (sock, SIOCGIFHWADDR, &ifreq) < 0)
			    {
			        perror ("ioctl");
			        return -1;
			    }

	    }
    }    
    return snprintf (mac, len_limit, "%x%x%x%x%x%x", (unsigned char) ifreq.ifr_hwaddr.sa_data[0], (unsigned char) ifreq.ifr_hwaddr.sa_data[1], (unsigned char) ifreq.ifr_hwaddr.sa_data[2], (unsigned char) ifreq.ifr_hwaddr.sa_data[3], (unsigned char) ifreq.ifr_hwaddr.sa_data[4], (unsigned char) ifreq.ifr_hwaddr.sa_data[5]);
}




int io_init(void)
{
	
	if(0 != meter_hw_init())         //meter 设备初始化成功
		{
			close(fd_meter);		
    	perror("meter_dev_init");
    	return -1;
				
		}
	meter_EpollInit();		
  tcflush(fd_meter,TCIOFLUSH);  //清空串口输入输出缓存 
  
	if(0 != gprs_hw_init())          //gprs 设备初始化成功
		{
			close(fd_gprs);		
    	perror("gprs_dev_init");
    	return -1;
		}	
	gprs_EpollInit();		
 	tcflush(fd_gprs,TCIOFLUSH);   //清空串口输入输出缓存
 	return 0;	
}

void clean_meter_data_buff(void)
{
	tcflush(fd_meter,TCIOFLUSH);  //清空串口输入输出缓存 
}


int callBak_get_archeves(void* data, int ncols, char** values, char** headers)  
{  
 		static int num = 0; 		
 		if(ncols != 3)
 			{
 				printf("\narcheves col erro!!!\n");
	 			return -1;
	 		}
	 	if(num >= MAX_DEV_NUM)
    	{
    		num = MAX_DEV_NUM;
    	}	 		
 		gateway.dev[num].type = atoi(values[0]);
 		strcpy(gateway.dev[num].id,values[1]);
 		strcpy(gateway.dev[num].net_id,values[2]);

    
#if JOHN_DEBUG

		printf("\n%d |",gateway.dev[num].type);
		printf("%s |",gateway.dev[num].id);
		printf("%s",gateway.dev[num].net_id);
		printf("\n");
#endif    
    
    num++; 
    return 0;  
}



int loader_archive(void)
{
	int count,i,j = 0;
	char **dbResult; 
  int nRow, nColumn;
  int index=0;
	
  sqlite3 *db; 
	char *zErrMsg = NULL;
	int rc,ret; 
	
  char *sql = NULL;
  sql = (char *)malloc(sizeof(char) * 2000);
  if(sql == NULL)
  	return -1;

  rc = sqlite3_open("/var/config.db",&db);   //打开数据库  
  if(rc != SQLITE_OK)  
  {  
        printf("zErrMsg = %s\n",zErrMsg);  
        sqlite3_free(zErrMsg);  
        free(sql);
        return -1;  
  }
  
  ret = sqlite3_get_table( db, "select type,dev_id,net_id from archive", &dbResult, &nRow, &nColumn, &zErrMsg);
	if(ret != SQLITE_OK)
		{
			printf("zErrMsg = %s\n",zErrMsg);
			sqlite3_free(zErrMsg);  
			free(sql);
			return -1;
		}	

 	
    printf("\nTable archive Num:%d!\n", nRow);
    

    if(nRow == 0)
    	return 0;
    else if(nRow > MAX_DEV_NUM)
		{
			printf("\nTo many device! only %d device is arrowed!!!",MAX_DEV_NUM);
			nRow = MAX_DEV_NUM;			
		}
//得到总记录条数		
	  gateway.dev_num = nRow;   
	  
	     	
    // 前两个字段为字段名 field0, field1, row[0][0], row[0][1], row[1][0], row[1][1] ... ... ....
    // 是一维数组,不是二维数组,反正记着第0,第1列的值为字段名,然后才是字段值;
    
    printf( "%s | %s | %s\n", dbResult[0], dbResult[1], dbResult[2]);
    printf("--------------------------------\n");
    index = nColumn; //字段值从index开始,即前面nColumn个位字段名，
    for( i = 0; i < nRow ; i++ )
    {
      	gateway.dev[i].type = atoi(dbResult[index++]);
      	strcpy(gateway.dev[i].id,dbResult[index++]);
      	strcpy(gateway.dev[i].net_id,dbResult[index++]);
      	printf( "\n%x | %s | %s",gateway.dev[i].type,gateway.dev[i].id,gateway.dev[i].net_id);
      printf("\n");
    }
    printf("--------------------------------\n");
	
  free(sql); 
  sql = NULL;

	sqlite3_free(zErrMsg);  
  sqlite3_close(db);      //关闭数据库  	
  return 0;
}


int rule_init(void)
{
  int i,j,count = 0;
	char **dbResult; 
  int nRow, nColumn;
  int index=0;
	time_t tm;
  sqlite3 *db; 
	char *zErrMsg = NULL;
	int rc; 	
  char *sql = NULL;
  
  
  tm = time(NULL);
	for(i = 0 ; i < gateway.dev_num ; i ++)
   {
  	 gateway.dev[i].collect_time = tm;
     gateway.dev[i].collect_cycle = rand()%(60-10+1)+10;              //采集周期
     gateway.dev[i].no_response = 0;
   }
   
  sql = (char *)malloc(sizeof(char) * 2000);
  if(sql == NULL)
  	{
  		printf("\nCann't malloc memory to sql!!!");
			return -1;
    }
    
  rc = sqlite3_open("/var/config.db",&db);   //打开数据库  
  if(rc != SQLITE_OK)  
  {  
   printf("zErrMsg = %s\n",zErrMsg);
   sqlite3_free(zErrMsg);  
	 free(sql);  
   return -1;  
  }
    
  memset(sql,'\0',sizeof(sql));  
  rc = sqlite3_get_table( db, "select type,dev_id,net_id,collect_time,collect_cycle,no_response from rule order by collect_time desc", &dbResult, &nRow, &nColumn, &zErrMsg);
  if (rc == SQLITE_OK)
  {
    printf("Table rule Num:%d!\n", nRow);    
    if(nRow == 0)       //数据库中没有采集策略，初始化采集策略!
    	{
    		srand(tm);
    		for(i = 0 ; i < gateway.dev_num ; i ++)
    		{
          memset(sql,'\0',sizeof(sql));  
				  sprintf(sql,"INSERT INTO rule VALUES(%d,'%s','%s',%ld,%d,%d);",
				                                    gateway.dev[i].type,
				                                    gateway.dev[i].id,
				                                    gateway.dev[i].net_id,
				                                    gateway.dev[i].collect_time,
				                                    gateway.dev[i].collect_cycle,
				                                    gateway.dev[i].no_response);//拼接采集策略更新语句  
				  printf("\n %s",sql);                                 
				  rc = sqlite3_exec(db,sql,NULL,NULL,&zErrMsg);   //执行sqlite命令语句  
				  if (rc != SQLITE_OK)
				  {
				    printf("zErrMsg = %s\n",zErrMsg);
				  }
    		}
    		free(sql); 
    		sql = NULL;
				sqlite3_free(zErrMsg);  
			  sqlite3_close(db);      //关闭数据库
    		return 0;
    	}
    	
  }
  else
		{
			printf("zErrMsg = %s\n",zErrMsg);
		} 	
  free(sql); 
  sql = NULL;
	sqlite3_free(zErrMsg);  
  sqlite3_close(db);      //关闭数据库  	
  return rc;	 
}

int loader_rule(void)
{
  int i,j,count = 0;
	char **dbResult; 
  int nRow, nColumn;
  int index=0;
	time_t tm;
  sqlite3 *db; 
	char *zErrMsg = NULL;
	int rc,ret; 
	
  char *sql = NULL;
  sql = (char *)malloc(sizeof(char) * 2000);
  if(sql == NULL)
  	{
  		printf("\nCann't malloc memory to sql!!!");
			return -1;
    }

  rc = sqlite3_open("/var/config.db",&db);   //打开数据库  
  if(rc != SQLITE_OK)  
  {  
   printf("zErrMsg = %s\n",zErrMsg);
   sqlite3_free(zErrMsg);  
	 free(sql);  
   return -1;  
  }
  
  memset(sql,'\0',sizeof(sql));  
  ret = sqlite3_get_table( db, "select type,dev_id,net_id,collect_time,collect_cycle,no_response from rule order by collect_time desc", &dbResult, &nRow, &nColumn, &zErrMsg);
  if (ret == SQLITE_OK)
  {
  	
    printf("Table rule Num:%d!\n", nRow);
    
    if(nRow == 0)       //数据库中没有采集策略，初始化采集策略!
    	{
    		tm = time(NULL);
    		srand(tm);
    		for(i = 0 ; i < gateway.dev_num ; i ++)
    		{
    			gateway.dev[i].collect_time = tm;
          //gateway.dev[i].collect_cycle = RULE_DEFAUL_RATE;
          gateway.dev[i].collect_cycle = rand()%(60-10+1)+10;              //采集周期
          gateway.dev[i].no_response = 0;
          
          memset(sql,'\0',sizeof(sql));  
				  sprintf(sql,"INSERT INTO rule VALUES(%d,'%s','%s',%ld,%d,%d);",
				                                    gateway.dev[i].type,
				                                    gateway.dev[i].id,
				                                    gateway.dev[i].net_id,
				                                    gateway.dev[i].collect_time,
				                                    gateway.dev[i].collect_cycle,
				                                    gateway.dev[i].no_response);//拼接采集策略更新语句  
				  printf("\n %s",sql);                                 
				  ret = sqlite3_exec(db,sql,NULL,NULL,&zErrMsg);   //执行sqlite命令语句  
				  if (ret != SQLITE_OK)
				  {
				    printf("zErrMsg = %s\n",zErrMsg);
				  }
    		}
    		free(sql); 
    		sql = NULL;
				sqlite3_free(zErrMsg);  
			  sqlite3_close(db);      //关闭数据库
    		return 0;
    	}

    // 前两个字段为字段名 field0, field1, row[0][0], row[0][1], row[1][0], row[1][1] ... ... ....
    // 是一维数组,不是二维数组,反正记着第0,第1列的值为字段名,然后才是字段值;
    
    printf( "%s | %s | %s | %s | %s | %s\n", dbResult[0], dbResult[1], dbResult[2],dbResult[3],dbResult[4],dbResult[5]);
    printf("--------------------------------\n");
    index = nColumn; //字段值从index开始,即前面nColumn个位字段名，
    for( i = 0; i < nRow ; i++ )
    {
      	gateway.dev[i].type = atoi(dbResult[index++]);
      	strcpy(gateway.dev[i].id,dbResult[index++]);
      	strcpy(gateway.dev[i].net_id,dbResult[index++]);      	
      	gateway.dev[i].collect_time = atoi(dbResult[index++]);
      	gateway.dev[i].collect_cycle = atoi(dbResult[index++]);
      	gateway.dev[i].no_response = atoi(dbResult[index++]);
      	printf( "\n%x | %s | %s | %ld | %d | %d",gateway.dev[i].type,gateway.dev[i].id,gateway.dev[i].net_id,gateway.dev[i].collect_time,gateway.dev[i].collect_cycle,gateway.dev[i].no_response);
      printf("\n");
    }
    printf("--------------------------------\n");
    
  }
  else
		{
			printf("zErrMsg = %s\n",zErrMsg);
		} 	
  free(sql); 
  sql = NULL;

	sqlite3_free(zErrMsg);  
  sqlite3_close(db);      //关闭数据库  	
  return ret;	
}



int save_rule(void)
{
	int i,j,count = 0;
	char **dbResult; 
  int nRow, nColumn;
  int index=0;
	time_t tm;
  sqlite3 *db; 
	char *zErrMsg = NULL;
	int rc,ret; 
	
  char *sql = NULL;
  sql = (char *)malloc(sizeof(char) * 2000);
  if(sql == NULL)
  	{
  		printf("\nCann't malloc memory to sql!!!");
			return -1;
    }

  rc = sqlite3_open("/var/config.db",&db);   //打开数据库  
  if(rc != SQLITE_OK)  
  {  
   printf("zErrMsg = %s\n",zErrMsg);
   sqlite3_free(zErrMsg);  
	 free(sql);  
   return -1;  
  }
 printf("\n\n---------------------------------------save rule to config.db now!");
 for(i = 0; i < gateway.dev_num ; i ++)
 {
	 memset(sql,'\0',sizeof(sql));  
	 sprintf(sql,"UPDATE rule set collect_time = %ld, collect_cycle = %d, no_response = %d where type = %d and dev_id = '%s';",
	                                    gateway.dev[i].collect_time,
	                                    gateway.dev[i].collect_cycle,
	                                    gateway.dev[i].no_response,
	                                    gateway.dev[i].type,
	                                    gateway.dev[i].id);//拼接采集策略更新语句  
	 printf("\n%s",sql);                                 
	 ret = sqlite3_exec(db,sql,NULL,NULL,&zErrMsg);   //执行sqlite命令语句  
	 if (ret != SQLITE_OK)
	  {
	    printf("zErrMsg = %s\n",zErrMsg);
	  }
	}
	
  free(sql); 
  sql = NULL;

	sqlite3_free(zErrMsg);  
  sqlite3_close(db);      //关闭数据库  	
  return ret;	

}

int db_init(void)
{
	sqlite3 *db; 
	char *zErrMsg = NULL;
	int rc,ret; 
	
  char *sql = NULL;
  sql = (char *)malloc(sizeof(char) * 2000);
  if(sql == NULL)
  	return -1;
  	
/*======================档案表，存放在archiver.db文件中==============*/  	
  rc = sqlite3_open("/var/config.db",&db);   //打开数据库  
  if(rc != SQLITE_OK)  
  {  
        printf("zErrMsg = %s\n",zErrMsg);  
        sqlite3_free(zErrMsg);  
			  free(sql);
        return -1;  
  }

/**************************************************************
* 设备类型   设备ID    无线通讯地址    
*  type      dev_id     net_id 
***************************************************************/
  memset(sql,'\0',sizeof(sql));  
  sprintf(sql,"CREATE TABLE IF NOT EXISTS archive(type integer,dev_id text,net_id text);");//创建表  
	ret = sqlite3_exec(db,sql,NULL,NULL,&zErrMsg);   //执行sqlite命令语句  
	if(ret != SQLITE_OK)
		{
			printf("zErrMsg = %s\n",zErrMsg);
			sqlite3_free(zErrMsg);  
			  free(sql);
			return -1;
		} 
  sqlite3_close(db);      //关闭数据库

/*======================采集策略表，存放在rule.db文件中================*/  
  rc = sqlite3_open("/var/config.db",&db);   //打开数据库  
  if(rc != SQLITE_OK)  
  {  
        printf("zErrMsg = %s\n",zErrMsg);
        sqlite3_free(zErrMsg);  
			  free(sql);
        return -1;  
  }
/**************************************************************
*  设备类型 设备ID    无线通讯地址   最新一次采集时间  采集周期   
*   type    dev_id     net_id       collect_time     collect_cycle
***************************************************************/
  memset(sql,'\0',sizeof(sql));  
  sprintf(sql,"CREATE TABLE IF NOT EXISTS rule(type integer,dev_id text,net_id text,collect_time integer,collect_cycle integer,no_response integer);");//创建表  
	ret = sqlite3_exec(db,sql,NULL,NULL,&zErrMsg);   //执行sqlite命令语句  
	if(ret != SQLITE_OK)
		{
			printf("zErrMsg = %s\n",zErrMsg);
		} 	
  free(sql); 
  sql = NULL;

	sqlite3_free(zErrMsg);  
  sqlite3_close(db);      //关闭数据库  	
  return 0;
}


int gateway_init(void)
{
	
  int ret;
  
  memset((void *)&gateway,'\0',sizeof(gateway_t));
  
  ret = db_init();
 	if( ret< 0)
 		return ret;	
 		
	ret = loader_archive();
	if( ret< 0)
 		return ret;	
 		
	rule_init();
		
	gateway.now = 0;
	gateway.next = 0;

	
	memset((void *)&Elect,'\0',sizeof(elect_meter_t1));
  memset((void *)&Elect3,'\0',sizeof(elect_meter_t3));
  memset((void *)&gas,'\0',sizeof(gas_meter_t));
  memset((void *)&heat,'\0',sizeof(heat_meter_t));
  
  Elect.Timestamp = Elect.DateRealTime;
  Elect3.Timestamp = Elect3.DateRealTime;
	Protocol.type = 0;
	Protocol.elect = &Elect;
	Protocol.elect3 = &Elect3;
	Protocol.gas = &gas ;
	Protocol.heat = &heat;
	Protocol.water_cold = &water_cold;
	
	Cj188.type = 0;
	Cj188.gas = &gas ;
	Cj188.water_cold = &water_cold;
	Cj188.heat = &heat;
	
	strcpy(gateway.name,"Gateway");
  get_mac(gateway.id, sizeof(gateway.id));
  sprintf(HeartBeatGateway,"HeartBeat/%s/%s",gateway.name,gateway.id);
  printf("\nThis is: %s_%s\n",gateway.name,gateway.id);	
	

	return 0;
}


// "xxx":"yyy",
char *json_john(char *str,char *topic,char* value)
{
	char *pstr = str; 

  strcat(str,"\\\"");
  strcat(str,topic);
  strcat(str,"\\\":");

  strcat(str,"\\\"");
  strcat(str,value);
  strcat(str,"\\\",");
  
  return pstr;
}
 // "xxx":"yyy"
char *json_john1(char *str,char *topic,char* value)
{
	char *pstr = str; 

  strcat(str,"\\\"");
  strcat(str,topic);
  strcat(str,"\\\":");

  strcat(str,"\\\"");
  strcat(str,value);
  strcat(str,"\\\"");
  
  return pstr;
}


void update_elect_meter_data(void)
{
	int len,data = 0;
	char *pstr = NULL;
	time_t now;
	struct tm *p;	
	char value[100] = {'\0'}; 
  memset(elect_meter_buff,'\0',sizeof(elect_meter_buff));
  strcat(elect_meter_buff,"{");
//type  
  len = sprintf(value,"%02x",Elect.type);
  value[len] = '\0';
  json_john(elect_meter_buff,"type",value);
  
//id 
  len = sprintf(value,"%s",Elect.id);
  value[len] = '\0';
  json_john(elect_meter_buff,"id",value);


//Timestamp  
  len = sprintf(value,"%s",Elect.DateRealTime);
  value[len] = '\0';
  json_john(elect_meter_buff,"Timestamp",value);

//EnergyForward  
  len = sprintf(value,"%.2f kWh",(double)Elect.EnergyForward/100);
  value[len] = '\0';
  json_john(elect_meter_buff,"EnergyForward",value);  

//EnergyBackward  
  len = sprintf(value,"%.2f kvarh",(double)Elect.EnergyBackward/100);
  value[len] = '\0';
  json_john(elect_meter_buff,"EnergyBackward",value);
  
//Voltage  
  len = sprintf(value,"%.1f v",(double)Elect.Voltage/10);
  value[len] = '\0';
  json_john(elect_meter_buff,"Voltage",value);
  
//Current  
  len = sprintf(value,"%.3f A",(double)Elect.Current/1000);
  value[len] = '\0';
  json_john(elect_meter_buff,"Current",value);
  
//Frequency  
  len = sprintf(value,"%.2f hz",(double)Elect.Frequency/100);
  value[len] = '\0';
  json_john(elect_meter_buff,"Frequency",value);
  
//Factor  
  len = sprintf(value,"%.3f",(double)Elect.Factor/1000);
  value[len] = '\0';
  json_john(elect_meter_buff,"Factor",value);
  
//ActPower  
  len = sprintf(value,"%.4f kw",(double)Elect.ActPower/10000);
  value[len] = '\0';
  json_john(elect_meter_buff,"ActPower",value);
  
//ReactPower  
  len = sprintf(value,"%.4f kvar",(double)Elect.ReactPower/10000);
  value[len] = '\0';
  json_john1(elect_meter_buff,"ReactPower",value);   
  
  strcat(elect_meter_buff,"}");
	
}

void update_elect3_meter_data(void)
{
	int len,data = 0;
	char *pstr = NULL;
	char value[100] = {'\0'}; 
	
	time_t now;
	struct tm *p;
	char ptime[50];
	
  memset(elect3_meter_buff,'\0',sizeof(elect3_meter_buff));
  strcat(elect3_meter_buff,"{");
//type  
  len = sprintf(value,"%02x",Elect3.type);
  value[len] = '\0';
  json_john(elect3_meter_buff,"type",value);
  
//id 
  len = sprintf(value,"%s",Elect3.id);
  value[len] = '\0';
  json_john(elect3_meter_buff,"id",value);


#if 1
          now = time(NULL);
					p = localtime(&now);		
					len = sprintf(Elect3.DateRealTime,"%04d%02d%02d%02d%02d%02d",1900 + p->tm_year,p->tm_mon+1,p->tm_mday,p->tm_hour,p->tm_min,p->tm_sec);
					Elect3.DateRealTime[len] = '\0';
#endif

//Timestamp  
  len = sprintf(value,"%s",Elect3.DateRealTime);
  value[len] = '\0';
  json_john(elect3_meter_buff,"Timestamp",value);

//EnergyForward  
  len = sprintf(value,"%.2f kWh",(double)Elect3.EnergyForward/100);
  value[len] = '\0';
  json_john(elect3_meter_buff,"EnergyForward",value);  

//EnergyBackward  
  len = sprintf(value,"%.2f kvarh",(double)Elect3.EnergyBackward/100);
  value[len] = '\0';
  json_john(elect3_meter_buff,"EnergyBackward",value);
  
//VoltageA  
  len = sprintf(value,"%.1f v",(double)Elect3.VoltageA/10);
  value[len] = '\0';
  json_john(elect3_meter_buff,"VoltageA",value);
//VoltageB  
  len = sprintf(value,"%.1f v",(double)Elect3.VoltageB/10);
  value[len] = '\0';
  json_john(elect3_meter_buff,"VoltageB",value);  
//VoltageC  
  len = sprintf(value,"%.1f v",(double)Elect3.VoltageC/10);
  value[len] = '\0';
  json_john(elect3_meter_buff,"VoltageC",value);
   
//CurrentA 
  len = sprintf(value,"%.3f A",(double)Elect3.CurrentA/1000);
  value[len] = '\0';
  json_john(elect3_meter_buff,"CurrentA",value);
//CurrentB  
  len = sprintf(value,"%.3f A",(double)Elect3.CurrentB/1000);
  value[len] = '\0';
  json_john(elect3_meter_buff,"CurrentB",value);
//CurrentC 
  len = sprintf(value,"%.3f A",(double)Elect3.CurrentC/1000);
  value[len] = '\0';
  json_john(elect3_meter_buff,"CurrentC",value);      
  
//Frequency  
  len = sprintf(value,"%.2f hz",(double)Elect3.Frequency/100);
  value[len] = '\0';
  json_john(elect3_meter_buff,"Frequency",value);
  
//Factor  
  len = sprintf(value,"%.3f",(double)Elect3.Factor/1000);
  value[len] = '\0';
  json_john(elect3_meter_buff,"Factor",value);
//FactorA  
  len = sprintf(value,"%.3f",(double)Elect3.Factor/1000);
  value[len] = '\0';
  json_john(elect3_meter_buff,"FactorA",value);
//FactorB  
  len = sprintf(value,"%.3f",(double)Elect3.Factor/1000);
  value[len] = '\0';
  json_john(elect3_meter_buff,"FactorB",value);
//FactorC  
  len = sprintf(value,"%.3f",(double)Elect3.Factor/1000);
  value[len] = '\0';
  json_john(elect3_meter_buff,"FactorC",value);
        
//ActPower  
  len = sprintf(value,"%.4f kw",(double)Elect3.ActPower/10000);
  value[len] = '\0';
  json_john(elect3_meter_buff,"ActPower",value);
//ActPowerA  
  len = sprintf(value,"%.4f kw",(double)Elect3.ActPower/10000);
  value[len] = '\0';
  json_john(elect3_meter_buff,"ActPowerA",value);
//ActPowerB  
  len = sprintf(value,"%.4f kw",(double)Elect3.ActPower/10000);
  value[len] = '\0';
  json_john(elect3_meter_buff,"ActPowerB",value);
//ActPowerC  
  len = sprintf(value,"%.4f kw",(double)Elect3.ActPower/10000);
  value[len] = '\0';
  json_john(elect3_meter_buff,"ActPowerC",value);

          
//ReactPower  
  len = sprintf(value,"%.4f kvar",(double)Elect3.ReactPower/10000);
  value[len] = '\0';
  json_john(elect3_meter_buff,"ReactPower",value);   
//ReactPowerA  
  len = sprintf(value,"%.4f kvar",(double)Elect3.ReactPower/10000);
  value[len] = '\0';
  json_john(elect3_meter_buff,"ReactPowerA",value); 
//ReactPowerB  
  len = sprintf(value,"%.4f kvar",(double)Elect3.ReactPower/10000);
  value[len] = '\0';
  json_john(elect3_meter_buff,"ReactPowerB",value); 
//ReactPowerC  
  len = sprintf(value,"%.4f kvar",(double)Elect3.ReactPower/10000);
  value[len] = '\0';
  json_john(elect3_meter_buff,"ReactPowerC",value); 

//EnergyReactCombination1  
  len = sprintf(value,"%.2f kvar",(double)Elect3.EnergyReactCombination1/100);
  value[len] = '\0';
  json_john(elect3_meter_buff,"EnergyReactCombination1",value); 
//EnergyReactCombination2  
  len = sprintf(value,"%.2f kvar",(double)Elect3.EnergyReactCombination2/100);
  value[len] = '\0';
  json_john(elect3_meter_buff,"EnergyReactCombination2",value); 

//QuadrantReactCombination1  
  len = sprintf(value,"%.2f kvar",(double)Elect3.QuadrantReactCombination1/100);
  value[len] = '\0';
  json_john(elect3_meter_buff,"QuadrantReactCombination1",value); 
//QuadrantReactCombination2  
  len = sprintf(value,"%.2f kvar",(double)Elect3.QuadrantReactCombination2/100);
  value[len] = '\0';
  json_john(elect3_meter_buff,"QuadrantReactCombination2",value); 
//QuadrantReactCombination3  
  len = sprintf(value,"%.2f kvar",(double)Elect3.QuadrantReactCombination3/100);
  value[len] = '\0';
  json_john(elect3_meter_buff,"QuadrantReactCombination3",value); 
//QuadrantReactCombination4  
  len = sprintf(value,"%.2f kvar",(double)Elect3.QuadrantReactCombination4/100);
  value[len] = '\0';
  json_john(elect3_meter_buff,"QuadrantReactCombination4",value); 

//ActiveForwardDemand  
  len = sprintf(value,"%.4f kvar",(double)Elect3.ActiveForwardDemand/10000);
  value[len] = '\0';
  json_john(elect3_meter_buff,"ActiveForwardDemand",value); 
//ReactForwardDemand  
  len = sprintf(value,"%.4f kvar",(double)Elect3.ReactForwardDemand/10000);
  value[len] = '\0';
  json_john1(elect3_meter_buff,"ReactForwardDemand",value); 
            
  strcat(elect3_meter_buff,"}");
  


	
}

void update_heat_meter_data(void)
{
	int len,data = 0;
	char *pstr = NULL;

	char value[100] = {'\0'};
 
  memset(heat_meter_buff,'\0',sizeof(heat_meter_buff));
  strcat(heat_meter_buff,"{");

//type  
  len = sprintf(value,"%02x",heat.type);
  value[len] = '\0';
  json_john(heat_meter_buff,"type",value);
  
//id 
  len = sprintf(value,"%s",heat.id);
  value[len] = '\0';
  json_john(heat_meter_buff,"id",value);

//Timestamp  
  len = sprintf(value,"%s",heat.data.DateRealTime);
  value[len] = '\0';
  json_john(heat_meter_buff,"Timestamp",value);

//heatAccountDay  
  len = sprintf(value,"%.2f %s",(double)heat.data.heatAccountDay/100,heat.unit.U_heatAccountDay);
  value[len] = '\0';
  json_john(heat_meter_buff,"heatAccountDay",value);

//heatCurrent  
  len = sprintf(value,"%.2f %s",(double)heat.data.heatCurrent/100,heat.unit.U_heatCurrent);
  value[len] = '\0';
  json_john(heat_meter_buff,"heatCurrent",value);

//Powerheat  
  len = sprintf(value,"%.2f %s",(double)heat.data.Powerheat/100,heat.unit.U_Powerheat);
  value[len] = '\0';
  json_john(heat_meter_buff,"Powerheat",value);  

//Flow  
  len = sprintf(value,"%.4f %s",(double)heat.data.Flow/10000,heat.unit.U_Flow);
  value[len] = '\0';
  json_john(heat_meter_buff,"Flow",value);

//AccumulatedFlow  
  len = sprintf(value,"%.2f %s",(double)heat.data.AccumulatedFlow/100,heat.unit.U_AccumulatedFlow);
  value[len] = '\0';
  json_john(heat_meter_buff,"AccumulatedFlow",value);

//TSupplyWater  
  len = sprintf(value,"%.2f %s",(double)heat.data.TSupplyWater/100,heat.unit.U_TSupplyWater);
  value[len] = '\0';
  json_john(heat_meter_buff,"TSupplyWater",value);     

//TReturnWater  
  len = sprintf(value,"%.2f %s",(double)heat.data.TReturnWater/100,heat.unit.U_TReturnWater);
  value[len] = '\0';
  json_john(heat_meter_buff,"TReturnWater",value); 

//WorkHours  
  len = sprintf(value,"%ld %s",heat.data.WorkHours,heat.unit.U_WorkHours);
  value[len] = '\0';
  json_john(heat_meter_buff,"WorkHours",value); 

//StateValve  
  if(heat.data.StateValve == 0)
  	json_john(heat_meter_buff,"StateValve","Openning");  
  else if(heat.data.StateValve == 1)
  	json_john(heat_meter_buff,"StateValve","Closed");
	else
		json_john(heat_meter_buff,"StateValve","abnormality");

//StateBattery  
  if(heat.data.StateBattery == 0)
  	json_john1(heat_meter_buff,"StateBattery","Normal");  
  else
  	json_john1(heat_meter_buff,"StateBattery","abnormality");  
   
  strcat(heat_meter_buff,"}");
  


	
}


void update_gas_meter_data(void)
{
	int len,data = 0;
	char *pstr = NULL;

	char value[100] = {'\0'};
 
  memset(gas_meter_buff,'\0',sizeof(gas_meter_buff));
  strcat(gas_meter_buff,"{");

//type  
  len = sprintf(value,"%02x",gas.type);
  value[len] = '\0';
  json_john(gas_meter_buff,"type",value);
  
//id 
  len = sprintf(value,"%s",gas.id);
  value[len] = '\0';
  json_john(gas_meter_buff,"id",value);

//Timestamp  
  len = sprintf(value,"%s",gas.data.DateRealTime);
  value[len] = '\0';
  json_john(gas_meter_buff,"Timestamp",value);

//AccumulatedFlowCurrent  
  len = sprintf(value,"%.2f %s",(double)gas.data.AccumulatedFlowCurrent/100,gas.unit.U_AccumulatedFlowCurrent);
  value[len] = '\0';
  json_john(gas_meter_buff,"AccumulatedFlowCurrent",value); 

//AccumulatedFlowAccountDay  
  len = sprintf(value,"%.2f %s",(double)gas.data.AccumulatedFlowAccountDay/100,gas.unit.U_AccumulatedFlowAccountDay);
  value[len] = '\0';
  json_john(gas_meter_buff,"AccumulatedFlowAccountDay",value);
  
//StateValve  
  if(gas.data.StateValve == 0)
  	json_john(gas_meter_buff,"StateValve","Openning");  
  else if(gas.data.StateValve == 1)
  	json_john(gas_meter_buff,"StateValve","Closed");
	else
		json_john(gas_meter_buff,"StateValve","abnormality");
		
//StateBattery  
  if(gas.data.StateBattery == 0)
  	json_john1(gas_meter_buff,"StateBattery","Normal");  
  else
  	json_john1(gas_meter_buff,"StateBattery","low-voltage");  

  
  strcat(gas_meter_buff,"}");
  


	
}

void update_water_cold_meter_data(void)
{
	int len,data = 0;
	char *pstr = NULL;

	char value[100] = {'\0'};
 
  memset(water_cold_meter_buff,'\0',sizeof(water_cold_meter_buff));
  strcat(water_cold_meter_buff,"{");

//type  
  len = sprintf(value,"%02x",water_cold.type);
  value[len] = '\0';
  json_john(water_cold_meter_buff,"type",value);
  
//id 
  len = sprintf(value,"%s",water_cold.id);
  value[len] = '\0';
  json_john(water_cold_meter_buff,"id",value);

//Timestamp  
  len = sprintf(value,"%s",water_cold.data.DateRealTime);
  value[len] = '\0';
  json_john(water_cold_meter_buff,"Timestamp",value);

//AccumulatedFlowCurrent  
  len = sprintf(value,"%.2f %s",(double)water_cold.data.AccumulatedFlowCurrent/100,water_cold.unit.U_AccumulatedFlowCurrent);
  value[len] = '\0';
  json_john(water_cold_meter_buff,"AccumulatedFlowCurrent",value); 

//AccumulatedFlowAccountDay  
  len = sprintf(value,"%.2f %s",(double)water_cold.data.AccumulatedFlowAccountDay/100,water_cold.unit.U_AccumulatedFlowAccountDay);
  value[len] = '\0';
  json_john(water_cold_meter_buff,"AccumulatedFlowAccountDay",value);
  
//StateValve  
  if(water_cold.data.StateValve == 0)
  	json_john(water_cold_meter_buff,"StateValve","Openning");  
  else if(water_cold.data.StateValve == 1)
  	json_john(water_cold_meter_buff,"StateValve","Closed");
	else
		json_john(water_cold_meter_buff,"StateValve","abnormality");
		
//StateBattery  
  if(heat.data.StateBattery == 0)
  	json_john1(water_cold_meter_buff,"StateBattery","Normal");  
  else
  	json_john1(water_cold_meter_buff,"StateBattery","low-voltage");  

  
  strcat(water_cold_meter_buff,"}");
  


	
}

int db_save(int type)
{
	int ret,len,rc;
	sqlite3 *db;
	char table_name[100] = {'\0'};
	char *zErrMsg = NULL;
	char *sql = NULL;
  sql = (char *)malloc(sizeof(char) * 2000);
  if(sql == NULL)
  	return -1;
  	
  rc = sqlite3_open("/var/data.db",&db);   //打开数据库  
  if(rc != SQLITE_OK)  
  {  
        printf("zErrMsg = %s\n",zErrMsg);  
        sqlite3_free(zErrMsg);
        free(sql);
        return -1;  
  }
  
  memset(sql, '\0', sizeof(char) * 2000);
	if(type == 0x10)	
		{
			//创建表名
			sprintf(table_name,"water_meter_%02x_%s",type,water_cold.id);
			sprintf(sql,"create table if not exists %s (Timestamp integer,dev_id text,value text);",table_name);//创建表  
			
	    ret = sqlite3_exec(db,sql,NULL,NULL,&zErrMsg);   //执行sqlite命令语句  
			if(ret != SQLITE_OK)
				{
					printf("zErrMsg = %s\n",zErrMsg);
					sqlite3_free(zErrMsg);
					free(sql); 
					return -1;
				}
			
			//存入一条记录
			update_water_cold_meter_data();

			len = sprintf(sql,"insert into %s values(%ld,'%s','%s')",
																table_name,
																//water_cold.data.DateRealTime,
																time(NULL),
																water_cold.id,
																water_cold_meter_buff);           
		}
	else if(type == 0x30)	
		{			
				//创建表名
			sprintf(table_name,"gas_meter_%02x_%s",type,gas.id);
			sprintf(sql,"create table if not exists %s (Timestamp integer,dev_id text,value text);",table_name);//创建表  
	    ret = sqlite3_exec(db,sql,NULL,NULL,&zErrMsg);   //执行sqlite命令语句  
			if(ret != SQLITE_OK)
				{
					printf("zErrMsg = %s\n",zErrMsg);
					sqlite3_free(zErrMsg);
					free(sql); 
					return -1;
				}
			//存入一条记录
			update_gas_meter_data();

			len = sprintf(sql,"insert into %s values(%ld,'%s','%s')",
			                      table_name,
			                     //gas.data.DateRealTime,
			                     time(NULL),
			                      gas.id,
			                      gas_meter_buff);
           
		}		
	else if(type >= 0x20 && type <= 0x21)
		{
			//创建表名
			sprintf(table_name,"heat_meter_%02x_%s",type,heat.id);
			sprintf(sql,"create table if not exists %s (Timestamp integer,dev_id text,value text);",table_name);//创建表  
	    ret = sqlite3_exec(db,sql,NULL,NULL,&zErrMsg);   //执行sqlite命令语句  
			if(ret != SQLITE_OK)
				{
					printf("zErrMsg = %s\n",zErrMsg);
					sqlite3_free(zErrMsg);
					free(sql); 
					return -1;
				}
			//存入一条记录
			update_heat_meter_data();

			len = sprintf(sql,"insert into %s values(%ld,'%s','%s')",
			                      table_name,
			                      //heat.data.DateRealTime,
			                      time(NULL),
			                      heat.id,
			                      heat_meter_buff);

		}
	else if(type == 0x40)
		{
			//创建表名
			sprintf(table_name,"elect_meter_%02x_%s",type,Elect.id);
			sprintf(sql,"create table if not exists %s (Timestamp integer,dev_id text,value text);",table_name);//创建表  
	    ret = sqlite3_exec(db,sql,NULL,NULL,&zErrMsg);   //执行sqlite命令语句  
			if(ret != SQLITE_OK)
				{
					printf("zErrMsg = %s\n",zErrMsg);
					sqlite3_free(zErrMsg);
					free(sql); 
					return -1;
				}
			//存入一条记录
			update_elect_meter_data();

			len = sprintf(sql,"insert into %s values(%ld,'%s','%s')",
			                      table_name,
			                      //Elect.DateRealTime,
			                      time(NULL),
			                      Elect.id,
			                      elect_meter_buff); 
                       
		}	
	else if(type == 0x41)
		{
			//创建表名
			sprintf(table_name,"elect_meter_%02x_%s",type,Elect3.id);
			sprintf(sql,"create table if not exists %s (Timestamp integer,dev_id text,value text);",table_name);//创建表  
	    ret = sqlite3_exec(db,sql,NULL,NULL,&zErrMsg);   //执行sqlite命令语句  
			if(ret != SQLITE_OK)
				{
					printf("zErrMsg = %s\n",zErrMsg);
					sqlite3_free(zErrMsg);
					free(sql);
					return -1;
				}
			//存入一条记录
			update_elect3_meter_data();

			len = sprintf(sql,"insert into %s values(%ld,'%s','%s')",
			                      table_name,
			                      //Elect3.DateRealTime,
			                      time(NULL),
			                      Elect3.id,
			                      elect3_meter_buff);
		}			
	else
		{
			printf("\r\n Type %d No definition!\r\n",type);
			return -1;
		}	
  sql[len] = '\0';

  ret = sqlite3_exec(db,sql,NULL,NULL,&zErrMsg);
	if(ret != SQLITE_OK)
		printf("%s\n",zErrMsg);
	else	
	  printf("\nSave to Database:%s\n",sql);
	free(sql); 
  sql = NULL;  
	sqlite3_free(zErrMsg);  
  sqlite3_close(db);      //关闭数据库
	return 0;
}

int db_save1(int type)
{
	int ret,len,rc;
	sqlite3 *db;
	char table_name[100] = {'\0'};
	char *zErrMsg = NULL;
	char *sql = NULL;
  sql = (char *)malloc(sizeof(char) * 2000);
  if(sql == NULL)
  	return -1;
  	
  rc = sqlite3_open("/var/data.db",&db);   //打开数据库  
  if(rc != SQLITE_OK)  
  {  
        printf("zErrMsg = %s\n",zErrMsg);  
        sqlite3_free(zErrMsg);
        free(sql);
        return -1;  
  }
  
  memset(sql, '\0', sizeof(char) * 2000);
	if(type == 0x10)	
		{
			//创建表名
			sprintf(table_name,"tab_%02x_%s",type,water_cold.id);
			sprintf(sql,"create table if not exists %s (Timestamp integer,dev_id text,master_value integer,value text);",table_name);//创建表  
			
	    ret = sqlite3_exec(db,sql,NULL,NULL,&zErrMsg);   //执行sqlite命令语句  
			if(ret != SQLITE_OK)
				{
					printf("zErrMsg = %s\n",zErrMsg);
					sqlite3_free(zErrMsg);
					free(sql); 
					return -1;
				}
			
			//存入一条记录
			update_water_cold_meter_data();

			len = sprintf(sql,"insert into %s values(%ld,'%s',%ld,'%s')",
																table_name,
																//water_cold.data.DateRealTime,
																time(NULL),
																water_cold.id,
																water_cold.data.AccumulatedFlowCurrent,
																water_cold_meter_buff);           
		}
	else if(type == 0x30)	
		{			
				//创建表名
			sprintf(table_name,"tab_%02x_%s",type,gas.id);
			sprintf(sql,"create table if not exists %s (Timestamp integer,dev_id text,master_value integer,value text);",table_name);//创建表  
	    ret = sqlite3_exec(db,sql,NULL,NULL,&zErrMsg);   //执行sqlite命令语句  
			if(ret != SQLITE_OK)
				{
					printf("zErrMsg = %s\n",zErrMsg);
					sqlite3_free(zErrMsg);
					free(sql); 
					return -1;
				}
			//存入一条记录
			update_gas_meter_data();

			len = sprintf(sql,"insert into %s values(%ld,'%s',%ld,'%s')",
			                      table_name,
			                     //gas.data.DateRealTime,
			                     time(NULL),
			                      gas.id,
			                      gas.data.AccumulatedFlowCurrent,
			                      gas_meter_buff);
           
		}		
	else if(type >= 0x20 && type <= 0x21)
		{
			//创建表名
			sprintf(table_name,"tab_%02x_%s",type,heat.id);
			sprintf(sql,"create table if not exists %s (Timestamp integer,dev_id text,master_value integer,value text);",table_name);//创建表  
	    ret = sqlite3_exec(db,sql,NULL,NULL,&zErrMsg);   //执行sqlite命令语句  
			if(ret != SQLITE_OK)
				{
					printf("zErrMsg = %s\n",zErrMsg);
					sqlite3_free(zErrMsg);
					free(sql); 
					return -1;
				}
			//存入一条记录
			update_heat_meter_data();

			len = sprintf(sql,"insert into %s values(%ld,'%s',%ld,'%s')",
			                      table_name,
			                      //heat.data.DateRealTime,
			                      time(NULL),
			                      heat.id,
			                      heat.data.AccumulatedFlow,
			                      heat_meter_buff);

		}
	else if(type == 0x40)
		{
			//创建表名
			sprintf(table_name,"tab_%02x_%s",type,Elect.id);
			sprintf(sql,"create table if not exists %s (Timestamp integer,dev_id text,master_value integer,value text);",table_name);//创建表  
	    ret = sqlite3_exec(db,sql,NULL,NULL,&zErrMsg);   //执行sqlite命令语句  
			if(ret != SQLITE_OK)
				{
					printf("zErrMsg = %s\n",zErrMsg);
					sqlite3_free(zErrMsg);
					free(sql); 
					return -1;
				}
			//存入一条记录
			update_elect_meter_data();

			len = sprintf(sql,"insert into %s values(%ld,'%s',%ld,'%s')",
			                      table_name,
			                      //Elect.DateRealTime,
			                      time(NULL),
			                      Elect.id,
			                      Elect.EnergyForward,
			                      elect_meter_buff); 
                       
		}	
	else if(type == 0x41)
		{
			//创建表名
			sprintf(table_name,"tab_%02x_%s",type,Elect3.id);
			sprintf(sql,"create table if not exists %s (Timestamp integer,dev_id text,master_value integer,value text);",table_name);//创建表  
	    ret = sqlite3_exec(db,sql,NULL,NULL,&zErrMsg);   //执行sqlite命令语句  
			if(ret != SQLITE_OK)
				{
					printf("zErrMsg = %s\n",zErrMsg);
					sqlite3_free(zErrMsg);
					free(sql);
					return -1;
				}
			//存入一条记录
			update_elect3_meter_data();

			len = sprintf(sql,"insert into %s values(%ld,'%s',%ld,'%s')",
			                      table_name,
			                      //Elect3.DateRealTime,
			                      time(NULL),
			                      Elect3.id,
			                      Elect3.EnergyForward,
			                      elect3_meter_buff);
		}			
	else
		{
			printf("\r\n Type %d No definition!\r\n",type);
			return -1;
		}	
  sql[len] = '\0';

  ret = sqlite3_exec(db,sql,NULL,NULL,&zErrMsg);
	if(ret != SQLITE_OK)
		printf("%s\n",zErrMsg);
	else	
	  printf("\nSave to Database:%s\n",sql);
	free(sql); 
  sql = NULL;  
	sqlite3_free(zErrMsg);  
  sqlite3_close(db);      //关闭数据库
	return 0;
}



/*********************************************************
      Hongdian H7000 GPRS DTU Software Suite 3.4.2
      H/W: HWL92-7118-GM2V54[V56]081203
      S/W: ZTP131-342-C-B20081106P0400
      Copyright (C) 2003-2008 Hongdian Inc.
      All Rights Reserved
      Service Code:*99***1#
      PPP Username:
      Access Point Name:CMNET
      DSC IP Address:103.44.145.245
      DSC IP Port:35082
      Checking......
      Searching GPRS network...
      Registered, home network.
      Signal strength is <9>.
      Module initialized.
      Dialing...
      Dial successfully.
      Connecting with DSC...
      Connect OK.
      Registering DSC...

*********************************************************/

  
int meter_epoll_read(char * ReadBuff, int ReadLen)  
{       
    int ret = 0; 
    int read_len = ReadLen; 
    int i =0,witeNum= 0;  
    while (1)   
    {  
        witeNum = epoll_wait(epid_meter, events_meter, 1, 10000); //串口阻塞模式接收，阻塞等待时间为10秒：10000毫秒
        if (witeNum < 0)
        {
					perror ("epoll_wait");
					return 1;
				} 
  //      printf("witeNum0 = %d\n   ", witeNum);  
        if( witeNum == 0)  
           return 0;  
        for (i = 0; i < witeNum; i++)   
        {
            if ((events_meter[i].events & EPOLLERR)  
                    || (events_meter[i].events & EPOLLHUP)  
                    || (!(events_meter[i].events & EPOLLIN)))
            {  
                printf("no data!\n");  
                break;  
            }   
            else if (events_meter[i].events & EPOLLIN)   //有数据进入  接收数据 
            { 
#if 1          	
							while (read_len != 0 && (ret = read(events_meter[i].data.fd, ReadBuff, read_len)) !=	0) 
							{
								if (ret == -1) 
								{
										if (errno == EINTR)
											continue;
								//		perror ("read");
										break;
								}
								read_len -= ret;
								ReadBuff += ret;
							}
							tcdrain(events_meter[i].data.fd);  
              tcflush(events_meter[i].data.fd,TCIOFLUSH);  
  						return ReadLen - read_len; 
#else
							read_len = read(events_meter[i].data.fd, ReadBuff, ReadLen);  							
              tcdrain(events_meter[i].data.fd);  
              tcflush(events_meter[i].data.fd,TCIOFLUSH);       
  						return read_len;    
#endif
            }  
        }  
    }  
  
    return read_len ;  
}

int gprs_epoll_read(char * ReadBuff, int ReadLen)  
{       
    int ret = 0; 
    int read_len = ReadLen; 
    int i =0,witeNum= 0;  
    while (1)   
    {  
        witeNum = epoll_wait(epid_gprs, events_gprs, 1, 50); 
        if (witeNum < 0)
        {
					perror ("epoll_wait");
					return 1;
				} 
  //      printf("witeNum0 = %d\n   ", witeNum);  
        if( witeNum == 0)  
           return 0;  
        for (i = 0; i < witeNum; i++)   
        {  
  
            if ((events_gprs[i].events & EPOLLERR)  
                    || (events_gprs[i].events & EPOLLHUP)  
                    || (!(events_gprs[i].events & EPOLLIN)))   
            {  
                printf("no data!\n");  
                break;  
            }   
            else if (events_gprs[i].events & EPOLLIN)   //有数据进入  接收数据 
            { 
#if 1          	
							while (read_len != 0 && (ret = read(events_gprs[i].data.fd, ReadBuff, read_len)) !=	0) 
							{
								if (ret == -1) 
								{
										if (errno == EINTR)
											continue;
								//		perror ("read");
										break;
								}
								read_len -= ret;
								ReadBuff += ret;
							}
							tcdrain(events_gprs[i].data.fd);  
              tcflush(events_gprs[i].data.fd,TCIOFLUSH);  
  						return ReadLen - read_len; 
#else
							read_len = read(events_gprs[i].data.fd, ReadBuff, ReadLen);  							
              tcdrain(events_gprs[i].data.fd);  
              tcflush(events_gprs[i].data.fd,TCIOFLUSH);       
  						return read_len;    
#endif
            }  
        }  
    }  
  
    return read_len ;  
}

int get_meter(int num)
{
	if(num < 0 || num >= gateway.dev_num)
		return -1;
		

  device = &gateway.dev[num];
  
#if(JOHN_DEBUG)	
	printf("\nGet a New device:");
	printf("\n");
	printf("type = %d,dev_id = %s,net_id = %s,collect_time = %ld,collect_cycle = %d",device->type,device->id,device->net_id,device->collect_time,device->collect_cycle);
  printf("\n");
#endif

  gateway.now = num;
  gateway.next = num + 1;
  if(gateway.next >= gateway.dev_num)
		gateway.next = 0;
		
	return device->type;
}



char* cmd_system(const char* command)
{
    char* result = "";
    FILE *fpRead;
    fpRead = popen(command, "r");
    char buf[1024];
    memset(buf,'\0',sizeof(buf));
    while(fgets(buf,1024-1,fpRead)!=NULL)
    { 
     result = buf;
    }
    if(fpRead!=NULL)
        pclose(fpRead);
    return result;
}



const char* MQTT_START = "mosquitto -v";
const char* MQTT_SUB_CMD = "mosquitto_sub -h 192.168.1.100 -p 1883 -v -t Elect/0000040006/sensor";
const char* MQTT_PUB_CMD = "mosquitto_pub -h 192.168.1.100 -p 1883  -t +/+/sensor -m 12";

int mosquitto_service(void)
{
    FILE *fpRead;   
    char buf[1024]; 
    fpRead = popen("mosquitto -v", "w");
    if(fpRead == NULL)
    {
      printf("popen() error!(mosquitto service start fail)\n");
      return -1;
    }
    memset(buf,'\0',sizeof(buf));
    while(fgets(buf,1024-1,fpRead)!=NULL)
    {
    	printf("%s", buf);
    }
    pclose(fpRead);
    
   printf("mosquitto service exit!\n");
   return 0;  	
}





int sublish_topic(const char* topic)
{
    FILE *fpRead;
    
    char buf[1024];
    memset(buf,'\0',sizeof(buf));
    sprintf((char *)buf,"mosquitto_sub -h %s -p %s -v -t %s",MQTT_HOST,MQTT_PORT,topic);
    
    fpRead = popen(buf, "r");    
    if(fpRead == NULL)
    {
      printf("popen() error!(mosquitto_sub)\n");
      return -1;
    }
    memset(buf,'\0',sizeof(buf));
    while(fgets(buf,1024-1,fpRead)!=NULL);
    pclose(fpRead);
    
   printf("%s:%s\n",topic,buf);
   return 0;  	
}



void gprs_breath(void)
{
	int size;

				size = write(fd_gprs, "GATEWAY",7);//向串口发送指令  
          
        if(size != 7)  
        {  
            printf("\r\ngprs Send breath erro!");
        }  
        else
        	{
        		printf("\r\ngprs Send breath to DCS\r\n");  
        	}
    	
}

void gprs_write(void)
{
	int size;

				size = write(fd_gprs, GPRS_TEST,sizeof(GPRS_TEST));//向串口发送指令  
          
        if(size != sizeof(GPRS_TEST))  
        {  
            printf("\r\ngprs Send data erro!");
        }  
        else
        	{
        		printf("\r\ngprs Send %d bytes data to DCS:%s\r\n", size,GPRS_TEST);  
        	}
    	
}

void gprs_read(void)
{
	int size;
	time_t tm;

        bzero(GPRS_ReadBuff,sizeof(GPRS_ReadBuff)); 
        size = gprs_epoll_read(GPRS_ReadBuff,GPRS_BUFFER_SIZE);//读取字节放到缓存 
        if(size != 0) 
        {
		        GPRS_ReadBuff[size] = '\0'; 
		        if(0 == strncmp(GPRS_ReadBuff,MessageRegistering,sizeof(MessageRegistering)))
							gprs_State = OffLine;
						else
							gprs_State = UpLine;	
						printf("\r\nRecived %d bytes from DSC:%s\r\n",size,GPRS_ReadBuff);	 
						
						tm = time((time_t *)NULL);
						printf("\n Now:%s",asctime(localtime(&tm)));	
				}
	
}


int udp_msg_sender(char *buf,int byte)
{
		char server_ip[20] = {'\0'};
	  int i,flag,client_fd;
    struct sockaddr_in ser_addr;
    struct hostent *host;
    struct timeval timeout;
    char *ip = NULL;
    
    socklen_t len;
    struct sockaddr_in src;
    char in_buf[BUFF_LEN];

    client_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if(client_fd < 0)
    {
        printf("create socket fail!\n");
        return -1;
    }
    memset(&ser_addr, 0, sizeof(ser_addr));
    ser_addr.sin_family = AF_INET;    



#if 0
    //通过gethostbyname()得到服务端的主机信息
    if((host = gethostbyname(HOST_NAME)) == NULL)
    {
        printf("gethostbyname() error\n");
        return -1;
    } 
    ip = inet_ntoa(*(struct in_addr*)host->h_addr_list[0]);
    printf("%s IP:%s\n",HOST_NAME,ip); /*打印IP地址*/    		
    ser_addr.sin_addr.s_addr = inet_addr(ip);
#else 
    //通过IP地址访问
    ip = SERVER_IP;	
    ser_addr.sin_addr.s_addr = inet_addr(ip);
#endif       
    ser_addr.sin_port = htons(HOST_PORT);  //注意网络序转换 		
    len = sizeof(struct sockaddr_in);
    sendto(client_fd, buf, byte, 0, (struct sockaddr*)&ser_addr, len);
    printf("\nclient send %d byte to service(%s):%s\n",byte,ip,buf);                                     //打印自己发送的信息
    memset(in_buf, '\0', BUFF_LEN);    
 
    // 设置超时    
    timeout.tv_sec = 2;//秒
    timeout.tv_usec = 0;//微秒
    if (setsockopt(client_fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) == -1)
    {
        perror("setsockopt failed:");
    }
 		recvfrom(client_fd, in_buf, BUFF_LEN, 0, (struct sockaddr*)&src, &len);  //接收来自server的信息
 
    printf("\nHost Server return:%s\n",in_buf);    
    close(client_fd);
    return 0;    
}



void gprs_msg_sender(char *buf,int byte)
{
	int size;  
  size = write(fd_gprs, buf,byte);//向串口发送指令  
    
  if(size != byte)  
  	{  
      printf("\r\ngprs Send data erro!");
  	}  
  else
  	{
  		printf("\r\ngprs Send %d bytes data to DCS:%s\r\n", size,buf);  
  	}
  
}

int publish_topic(const char* topic,const char* value)
{
    FILE *fpRead;
    char buf[1024];
    memset(buf,'\0',sizeof(buf));
    sprintf(buf,"mosquitto_pub -h %s -p %s -t %s -m \"%s\"",MQTT_HOST,MQTT_PORT,topic,value);
  //  printf("%s\n",buf);
    fpRead = popen(buf, "w");
    if(fpRead == NULL)
    {
      printf("popen() error!(mosquitto_pub)\n");
      return -1;
    }
    memset(buf,'\0',sizeof(buf));
    while(fgets(buf,1024-1,fpRead)!=NULL);
    pclose(fpRead);
    
   printf("\nPublish %s:%s Successfull!\n",topic,value);
   return 0;  	
}



int meter_read(void)
{
 int size,i;
 int type;
// clean_meter_data_buff();
 sleep(1);
 memset(Meter_ReadBuff,'\0',sizeof(Meter_ReadBuff));  
 size = meter_epoll_read(Meter_ReadBuff,METER_BUFFER_SIZE);//读取字节放到缓存 
 
 if(size != 0) 
	 {
		 Meter_ReadBuff[size] = '\0'; 
		 printf("\r\nRecived %d bytes from Meter:",size);	
		 for(i=0;i<size;i++)
		 {
		 	  //Meter_ReadBuff[i] = (Meter_ReadBuff[i] & 0xff);
		 		printf("0x%02x ",Meter_ReadBuff[i]);
		 	}
		 return protocol_decode(Meter_ReadBuff,size);                                //解码		

	}
 return -1;
}

/*
* 阻塞方式采集表计数据
* 返回 0 表示接受到表计正常返回
* 返回 其他 表示没有接收到表计正常返回
*/
int call_device(device_t *dev)
{
	int size,len,ret;
	static int elect_data_type = 0;  //电能表采集数据类型：正向有功总电能、反向有功总电能、电压、电流……
	
	char buff[METER_BUFFER_SIZE] = {'\0'};
	
	int type = dev->type;
	
	switch(type)
	{
		case 0x10:
		case 0x11:
		case 0x12:	
		case 0x13:
		case 0x20:
		case 0x21:	
		case 0x30:		
			len = cj188_encode(buff,dev->id,type);   //CJ188 编码
			if(len == -1)
			{
				printf("\n cj188 encode fail!");
				return -1;
			}		
	    size = write(fd_meter, buff,len);       //编码 发送指令
	    if(size != len)
	    {
	    	printf("\r\nwrite deivce erro!");
	    	return -1;
	    }
	   
	    //sleep(8);                              //轮询机制，延时10秒钟等待电表反馈，保障线路空闲不冲突。epoll_wait中实现
			return meter_read();
			break;
		case 0x40:
			elect_data_type = ENERGY_FORWARDA;   /*正向有功总电能*/
      len = dlt645_encode(buff,dev->id,elect_data_type);
			if(len == -1)
			{
				printf("\n DLT645 encode fail!");
				return -1;
			}	
	    size = write(fd_meter, buff,len);       //编码 发送指令
	    if(size != len)
	    {
	    	printf("\nwrite deivce erro!");
	    	return -1;
			}
			
      //sleep(8);                              //轮询机制，延时10秒钟等待电表反馈，保障线路空闲不冲突。epoll_wait中实现
			
			                                  
#if COLLECT_ENERGY_FORWARDA_ONLY	                  //只采集正向有功总电能		
			return meter_read();
#endif		

			ret = meter_read();	
			if(ret != -1)
				{
				if(strcmp(device->id,Protocol.id) != 0)
					{
						printf("\ndevice requrie/response:%s/%s!",device->id,Protocol.id);
						return -1;
					}	
				}
			elect_data_type = ENERGY_BACKWARDA;    /*反向有功总电能*/		
			type++;
		case 0x41:
      len = dlt645_encode(buff,dev->id,elect_data_type);
			if(len == -1)
			{
				printf("\n dLT645 encode fail!");
				return -1;
			}	
	    size = write(fd_meter, buff,len);       //编码 发送指令
	    if(size != len)
	    {
	    	printf("\n write deivce erro!!");
	    	return -1;
			}
			
			//sleep(8);                              //轮询机制，延时10秒钟等待电表反馈，保障线路空闲不冲突。epoll_wait中实现
ret = meter_read();	
			if(ret != -1)
				{
				if(strcmp(device->id,Protocol.id) != 0)
					{
						printf("\ndevice requrie/response:%s/%s!",device->id,Protocol.id);
						return -1;
					}	
				}	
			elect_data_type = VOLTAGE;             /*电压*/
			type++;				
		case 0x42:
      len = dlt645_encode(buff,dev->id,elect_data_type);
			if(len == -1)
			{
				printf("\n dLT645 encode fail!");
				return -1;
			}	
	    size = write(fd_meter, buff,len);       //编码 发送指令
	    if(size != len)
	    {
	    	printf("\n write deivce erro!");
	    	return -1;
			}
			
			//sleep(8);                              //轮询机制，延时10秒钟等待电表反馈，保障线路空闲不冲突。epoll_wait中实现
ret = meter_read();	
			if(ret != -1)
				{
				if(strcmp(device->id,Protocol.id) != 0)
					{
						printf("\ndevice requrie/response:%s/%s!",device->id,Protocol.id);
						return -1;
					}	
				}	
			elect_data_type = CURRENT;           /*电流电流*/
			type++;			
		case 0x43:
      len = dlt645_encode(buff,dev->id,elect_data_type);
			if(len == -1)
			{
				printf("\n dLT645 encode fail!");
				return -1;
			}	
	    size = write(fd_meter, buff,len);       //编码 发送指令
	    if(size != len)
	    {
	    	printf("\n write deivce erro!");
	    	return -1;
			}
			
			//sleep(8);                              //轮询机制，延时10秒钟等待电表反馈，保障线路空闲不冲突。epoll_wait中实现
ret = meter_read();	
			if(ret != -1)
				{
				if(strcmp(device->id,Protocol.id) != 0)
					{
						printf("\ndevice requrie/response:%s/%s!",device->id,Protocol.id);
						return -1;
					}	
				}	
			elect_data_type = POWER_ACT;           /*瞬时总有功功率*/
			type++;			
		case 0x44:
      len = dlt645_encode(buff,dev->id,elect_data_type);
			if(len == -1)
			{
				printf("\n dLT645 encode fail!");
				return -1;
			}	
	    size = write(fd_meter, buff,len);       //编码 发送指令
	    if(size != len)
	    {
	    	printf("\n write deivce erro!");
	    	return -1;
			}
			
			//sleep(8);                              //轮询机制，延时10秒钟等待电表反馈，保障线路空闲不冲突。epoll_wait中实现
ret = meter_read();	
			if(ret != -1)
				{
				if(strcmp(device->id,Protocol.id) != 0)
					{
						printf("\ndevice requrie/response:%s/%s!",device->id,Protocol.id);
						return -1;
					}	
				}	
			elect_data_type = POWER_REACT;        /*瞬时总无功功率*/
			type++;			
		case 0x45:
      len = dlt645_encode(buff,dev->id,elect_data_type);
			if(len == -1)
			{
				printf("\n dLT645 encode fail!");
				return -1;
			}	
	    size = write(fd_meter, buff,len);       //编码 发送指令
	    if(size != len)
	    {
	    	printf("\n write deivce erro!");
	    	return -1;
			}
			
			//sleep(8);                              //轮询机制，延时10秒钟等待电表反馈，保障线路空闲不冲突。epoll_wait中实现
			meter_read(); 
			if(strcmp(device->id,Protocol.id) != 0)
				{
					printf("\ndevice requrie/response:%s/%s!",device->id,Protocol.id);
					return -1;
				}	
			elect_data_type = FACTOR;               /*总功率因素*/
			type++;			
		case 0x46:
      len = dlt645_encode(buff,dev->id,elect_data_type);
			if(len == -1)
			{
				printf("\n dLT645 encode fail!");
				return -1;
			}	
	    size = write(fd_meter, buff,len);       //编码 发送指令
	    if(size != len)
	    {
	    	printf("\n write deivce erro!");
	    	return -1;
			}
			
			//sleep(8);                              //轮询机制，延时10秒钟等待电表反馈，保障线路空闲不冲突。epoll_wait中实现
ret = meter_read();	
			if(ret != -1)
				{
				if(strcmp(device->id,Protocol.id) != 0)
					{
						printf("\ndevice requrie/response:%s/%s!",device->id,Protocol.id);
						return -1;
					}	
				}	
			elect_data_type =FREQUENCY;           /*电网频率*/					
		case 0x47:
      len = dlt645_encode(buff,dev->id,elect_data_type);
			if(len == -1)
			{
				printf("\n dLT645 encode fail!");
				return -1;
			}	
	    size = write(fd_meter, buff,len);       //编码 发送指令
	    if(size != len)
	    {
	    	printf("\n write deivce erro!");
	    	return -1;
			}
			
			//sleep(8);                              //轮询机制，延时10秒钟等待电表反馈，保障线路空闲不冲突。epoll_wait中实现
ret = meter_read();	
			if(ret != -1)
				{
				if(strcmp(device->id,Protocol.id) != 0)
					{
						printf("\ndevice requrie/response:%s/%s!",device->id,Protocol.id);
						return -1;
					}	
				}
			return ret;	
			break;											
		default:
			printf("\n Cann't suppor device(meter) type!");
			break;	
	}
	
	return -1;
}

/************************************************************************
*  轮训方式：
*    1、从内存策略表，查找采集时间(collect_time)小于当前系统时间的设备
*    2、只要满足条件就退出查找过程
*    3、将改设备采集时间更新：当前时间(now_time) + 采集周期(collect_cycle)
*/
device_t *select_device_from_rule(int now)
{
	int i;
	device_t *dev = NULL;
	time_t now_time = time(NULL);	
  if(now >= gateway.dev_num)
  	{
  		printf("\n Overflow! gateway devices %d",gateway.dev_num - 1);
  		gateway.now = 0;
		  gateway.next = 1;
			return NULL;
		}

	for(i = now ; i < gateway.dev_num ; i ++)
	{
		if(now_time >= gateway.dev[i].collect_time && gateway.dev[i].no_response < MAX_NO_RESPONSE_COUNT) //选择查找采集时间(collect_time)小于当前系统时间的设备，跳过连续10次异常的设备，在异常处理程序中处理
		//if(now_time >= gateway.dev[i].collect_time) //选择查找采集时间(collect_time)小于当前系统时间的设备，
			{		  	
		    dev = &gateway.dev[i];
        gateway.now = i;
		    gateway.next = i + 1;
			  if(gateway.next >= gateway.dev_num)
			  	{
					  gateway.next = 0;
				  }		 		    
		    printf("\n\n\n---------------");
		    printf("Choosed(%d/%d) %sDevice %s",gateway.now,gateway.dev_num,asctime(localtime(&now_time)),dev->id);
	      return dev;
		  }	
	}
	gateway.next = 0;
	//printf("\n No Device need Collect!");
	
	return NULL;

}

void handle_device(void)
{
	int type;
	time_t now;
  device_t *dev = select_device_from_rule(gateway.next);
	if(dev == NULL)
		return;
  else
  	device = dev;
  	
  if(-1 == meter_hw_open())
  	return ;
  type = call_device(device);
  meter_hw_close();
  
  now = time(NULL);
	if(type < 0)                                     //设备无响应
		{
			gateway.dev[gateway.now].no_response ++;                                       //设备无响应次数+1
			gateway.dev[gateway.now].collect_time = now + gateway.dev[gateway.now].collect_cycle; 
			printf("\nDevice %s no response,%d,next Colloct after %ds \n",device->id,device->no_response,gateway.dev[gateway.now].collect_cycle);
			return;
		}
//	printf("\nSnd:%x %s",device->type,device->id);
//	printf("\nGet:%x %s",Protocol.type,Protocol.id);
	
	//返回的类型和设备ID 与 发送的类型和设备ID 一致
	if(Protocol.type == device->type && (strcmp(Protocol.id,device->id) == 0))  
		{
			gateway.dev[gateway.now].no_response = 0;                                               //设备无响应次数清零			
			gateway.dev[gateway.now].collect_time = now + gateway.dev[gateway.now].collect_cycle;          //将改设备采集时间更新：当前时间(now_time) + 采集周期(collect_cycle)			
	    printf("\nDevice %s Responsed, next Colloct after %ds \n",device->id,gateway.dev[gateway.now].collect_cycle);	
		}
	//返回的类型和设备ID 与 发送的类型和设备ID 不一致	
	else
		{
			gateway.dev[gateway.now].no_response ++;                                                //设备无响应次数+1
			gateway.dev[gateway.now].collect_time = now + now % 100;
			printf("\nDevice %s Response\n",Protocol.id);
		}
	
  if (0 == db_save1(type))
		{
			if(type == 0x10)
				publish_topic("water_meter",water_cold_meter_buff);
			
			else if(type == 0x30)
				publish_topic("gas_meter",gas_meter_buff);
			
			else if(type >= 0x20 && type <= 0x21)
				publish_topic("heat_meter",heat_meter_buff);
		
			else if(type == 0x40)
				publish_topic("elect_meter",elect_meter_buff);
		
			else if(type == 0x41)
				publish_topic("elect3_meter",elect3_meter_buff);
		}
}

int heart_beat(void)
{
	int len = 0;
	
	char Rtime[20] = {'\0'};
	time_t now = time(NULL);
	//Timestamp  
  len = sprintf(Rtime,"%ld",(long)now);
  Rtime[len] = '\0';  
  printf("\n\n\n-----HeartBeat to cloud!%s",ctime(&now));
  return publish_topic(HeartBeatGateway,Rtime);
}

int handle_breath(void)
{
	static long breath_timer = 0;
	time_t now = time(NULL);
	if(now - breath_timer > HEARTBEAT_CYCLE)
		{
		  breath_timer = now;
			heart_beat();
		}
  return 0;
}

int handle_database(void)
{
	static long save_timer = 0;
	time_t now = time(NULL);
	if(now - save_timer > RULE_SAVE_CYCLE)
		{
		  save_timer = now;
			return save_rule();
		}
	return 0;	
}


int update_rule(void)
{
	int i,j,rc;
	long data0,data1,data2,data_x0,data_x1;
	char **dbResult; 
  int nRow, nColumn;
  sqlite3 *db;

	int read_row_num = RULE_DETECT_NUM;
	int min_update_cycle;
	double arry_yy[read_row_num];
	double arry_xx[read_row_num];
	double sum_yy,sum_xx;
	
	long Timestamp0,master_value0,Timestamp1,master_value1;

	
	double x_in = 0.0;
	double y_out = 0.0;
	
	char table_name[100] = {'\0'};
	char *zErrMsg = NULL;
	char *sql = NULL;
  sql = (char *)malloc(sizeof(char) * 2000);
  if(sql == NULL)
  	return -1;
  	
  rc = sqlite3_open("/var/data.db",&db);   //打开数据库  
  if(rc != SQLITE_OK)  
  {  
        printf("zErrMsg = %s\n",zErrMsg);  
        sqlite3_free(zErrMsg);
        free(sql);
        return -1;  
  }
  
  printf("\n\n\n-----------------------------------------update rule start,now cycle = %d\n",rule_update_cycle);
  
  
  for(i=0;i<gateway.dev_num;i++)
  {
  	 memset(sql, '\0', sizeof(char) * 2000);
  	 
  	 sprintf(sql,"select Timestamp,master_value from tab_%02x_%s order by Timestamp desc limit 0,%d",
	                                    gateway.dev[i].type,
	                                    gateway.dev[i].id,
	                                    read_row_num);//拼接采集策略更新语句 
	   printf("\n\n%s,", sql);                                 
     rc = sqlite3_get_table( db, sql, &dbResult, &nRow, &nColumn, &zErrMsg);
     x_in = 0.0;
     y_out = 0.0;
     sum_xx = 0.0;
     sum_yy = 0.0; 
     if (rc == SQLITE_OK)
     	{
     		printf("nRow:%d!", nRow);
    		
     		if(nRow > 2 && nRow <= read_row_num)       //记录数必须2条以上，才计算新的采集策略!
     			{
     				Timestamp0 = atol(dbResult[nColumn++]);
     				master_value0 = atol(dbResult[nColumn++]);
     				
     				for (j = 0 ; j < nRow - 1 ; j ++)
     				{     					
     					Timestamp1 = atol(dbResult[nColumn++]);
     					master_value1 = atol(dbResult[nColumn++]);
     					
     					arry_yy[j] = Timestamp0 - Timestamp1;
     					arry_xx[j] = master_value0 - master_value1;
     					
     					Timestamp0 = Timestamp1;
     					master_value0 = master_value1;
     				}
						printf("\n**************get trend function: y = bx + a ");     			  
     			  printf("\narry_yy = ");
     			  for (j = 0 ; j < nRow - 1 ; j ++)
     			  {
     			  	sum_yy += arry_yy[j];
     			  	printf("%f,",arry_yy[j]);
     			  }
     			  
     			  printf("\narry_xx = ");
     			  for (j = 0 ; j < nRow - 1 ; j ++)
     			  {
     			  	sum_xx += arry_xx[j];
     			  	printf("%f,",arry_xx[j]);	
     			  }
						//printf("\nsum_yy:%f sum_xx:%f",sum_yy,sum_xx);
						if(sum_yy == 0.0)  //所有时间都一样，数据异常，不处理
							continue;
		        x_in = DATA_RULE_BASE * 100;  //计算增长DATA_RULE_BASE值需要的时间		        
		        y_out = trend(arry_xx,arry_yy,nRow-1,x_in);    //一元线性回归算法
            printf("\n**************trend result :x_in = %f,y_out = %f ",x_in,y_out);
		        if(y_out <= 0.0)                               //在电表应用中，数据只会增加，不会减少，所以一次方程只能是斜率正向增长
		        	y_out = gateway.dev[i].collect_cycle * nRow / 2;
	     			if(y_out < RULE_MIN_CYCLE)
            	y_out = RULE_MIN_CYCLE;
            if(y_out > RULE_MAX_CYCLE)
            	y_out = RULE_MAX_CYCLE;
     			}
     		else
     			{
     				continue; 
     			}	   			
     	}
     	else  
		  {  
		     printf("zErrMsg = %s\n",zErrMsg);
		     continue;
		  }
		  
		 printf("\n%s collect_cycle from %d to ",gateway.dev[i].id,gateway.dev[i].collect_cycle); 
		 gateway.dev[i].collect_cycle = y_out;
		 printf("%d!",gateway.dev[i].collect_cycle);
	     				
	}
	
	 min_update_cycle = RULE_MAX_CYCLE;
	 for(i=0;i<gateway.dev_num;i++)
	 {
	 	if(gateway.dev[i].no_response != 0)
	 		{
			 	gateway.dev[i].collect_cycle = gateway.dev[i].collect_cycle * gateway.dev[i].no_response;
			 	if(gateway.dev[i].collect_cycle > RULE_MAX_CYCLE)
			 		gateway.dev[i].collect_cycle = RULE_MAX_CYCLE;
			}
	 	if(gateway.dev[i].collect_cycle < min_update_cycle)
		 	min_update_cycle = gateway.dev[i].collect_cycle;
	 }
	 
	rule_update_cycle = min_update_cycle;
	
  printf("\n-----------------------------------------update rule end,now cycle = %d\n",rule_update_cycle);
  free(sql); 
  sql = NULL;

	sqlite3_free(zErrMsg);  
  sqlite3_close(db);      //关闭数据库  	
  return rc;	
}

int handle_rule(void)
{
	int rc = 0;;
  static long rule_timer = 0;
  static long rule_save = 0;
	time_t now = time(NULL);
	if(now - rule_timer > rule_update_cycle)
	//if(now - rule_timer > RULE_UPDATE_CYCLE)	
		{
		  rule_timer = now;
			rc = update_rule();
			if(rc != -1)
				if(now - rule_save > HOUR)
				   return save_rule();
		}
  return rc;	
}

int main(void)
{
	int ret;
//  io_init(); //在使用中打开、关闭!!!!!
   
  ret = gateway_init(); 
  if(ret)
  	return -1;
  	
	while(1)
	{	
		handle_device();
		handle_rule();
		handle_breath();
		//sleep(3);
	}

  return 0;  
	
}  
