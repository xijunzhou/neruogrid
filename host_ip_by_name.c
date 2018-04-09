#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <errno.h>

#include <string.h>

const char SRV_IP[] = "8.8.8.8";

#define SRV_PORT  53
#define BUF_SIZE 1024

typedef unsigned short U16;

typedef struct _DNS_HDR
{  
  U16 id;
  U16 tag;
  U16 numq;
  U16 numa;
  U16 numa1;
  U16 numa2;
}DNS_HDR;
typedef struct _DNS_QER
{
   U16 type;
   U16 classes;
}DNS_QER;

char* get_host_ip_by_socekt(char* name,char *ip)
{
    int      i,servfd,clifd,len = 0;
    struct   sockaddr_in servaddr, addr;
    int      socklen = sizeof(servaddr);
    char     buf[BUF_SIZE];
    char     *p;
    DNS_HDR  *dnshdr = (DNS_HDR *)buf;
    DNS_QER  *dnsqer = (DNS_QER *)(buf + sizeof(DNS_HDR));
   
    if ((clifd  =  socket(AF_INET, SOCK_DGRAM, 0 ))  <   0 )
    {
         printf( " create socket error!\n " );
         return NULL;
    }
   
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    inet_aton(SRV_IP, &servaddr.sin_addr);
    servaddr.sin_port = htons(SRV_PORT);
   
    /*if (connect(clifd, (struct sockaddr *)&servaddr, socklen) < 0)
    {
          printf( " can't connect to %s!\n ", name);
          return -1;
    }*/
    memset(buf, 0, BUF_SIZE);
    dnshdr->id = (U16)1;
    dnshdr->tag = htons(0x0100);
    dnshdr->numq = htons(1);
    dnshdr->numa = 0;
   
    strcpy(buf + sizeof(DNS_HDR) + 1, name);
    p = buf + sizeof(DNS_HDR) + 1; i = 0;
    while (p < (buf + sizeof(DNS_HDR) + 1 + strlen(name)))
    {
        if ( *p == '.')
        {
            *(p - i - 1) = i;
            i = 0;
        }
        else
        {
            i++;
        }
        p++;
    }
    *(p - i - 1) = i;
       
    dnsqer = (DNS_QER *)(buf + sizeof(DNS_HDR) + 2 + strlen(name));
    dnsqer->classes = htons(1);
    dnsqer->type = htons(1);
   
     
    len = sendto(clifd, buf, sizeof(DNS_HDR) + sizeof(DNS_QER) + strlen(name) + 2, 0, (struct sockaddr *)&servaddr, sizeof(servaddr));
    //len = send(clifd, buf, sizeof(DNS_HDR) + sizeof(DNS_QER) + strlen(name) + 2, 0);
    i = sizeof(struct sockaddr_in);
    len = recvfrom(clifd, buf, BUF_SIZE, 0, (struct sockaddr *)&servaddr, &i);
    //len = recv(clifd, buf, BUF_SIZE, 0);
    if (len < 0)
    {
          printf("recv error\n");
          return NULL;
    }
    if (dnshdr->numa == 0)
    {
          printf("ack error\n");
          return NULL;
    }
    p = buf + len -4;
//    printf("\n%s ==> %u.%u.%u.%u\n", name, (unsigned char)*p, (unsigned char)*(p + 1), (unsigned char)*(p + 2), (unsigned char)*(p + 3));

 		len = sprintf(ip,"%u.%u.%u.%u",(unsigned char)*p, (unsigned char)*(p + 1), (unsigned char)*(p + 2), (unsigned char)*(p + 3));
 		ip[len] = '\0';

    close(clifd);
    return ip;
}


char *get_host_ip_by_name(char* name,char *ip)
{
	struct hostent *host;
	char *Pip = ip;
	int i,len;
    //通过gethostbyname()得到服务端的主机信息
    if((host = gethostbyname(name)) == NULL)
    {
        printf("gethostbyname() error\n");
        exit(1);
    }
    
    else 
    {
        printf("\nname: %s\naddrtype; %d\naddrlength: %d\n",
            host->h_name, host->h_addrtype, host->h_length);
        printf("\nip address: %s\n",
            inet_ntoa(*(struct in_addr*)host->h_addr_list[0]));
            
            
        /* 打印IP地址 */
        for(i=0;;i++){
            if(host->h_addr_list[i] != NULL){/* 不是IP地址数组的结尾 */
                printf("IP:%s\n",inet_ntoa(*(struct in_addr*)host->h_addr_list[i])); /*打印IP地址*/
            }   else{/*达到结尾*/
                break;  /*退出for循环*/
            }
        }

        /* 打印域名地址 */
        for(i=0;;i++){/*循环*/
            if(host->h_aliases[i] != NULL){/* 没有到达域名数组的结尾 */
                printf("alias %d:%s\n",i,host->h_aliases[i]); /* 打印域名 */
            }   else{/*结尾*/
                break;  /*退出循环*/
            }
        }            
    }
    len = sprintf(ip,"%s",inet_ntoa(*(struct in_addr*)host->h_addr_list[0]));
 		ip[len] = '\0';
    return Pip;

}