#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/****
  * Function: int ChangeToBcd(char iChTemp)
  *@Describe: 字符转换成BCD
  *@Param_1 : char iChTemp 要转换的字符串
  *@Param_2 :
  *@Param_3 : 
  *@Return  : 
  */
int ChangeToBcd(char iChTemp)
{
	int m;
	if( iChTemp>='0' && iChTemp<='9' )
	{
		m=iChTemp-'0';
	}
	else if( iChTemp>='A' && iChTemp<='Z' )
	{
		m=iChTemp-'A'+10;
	}
	else if( iChTemp>='a' && iChTemp<='z' )
	{
		m=iChTemp-'a'+10;
	}
	return m;
}

/****
  * Function: void ReverseByArray(char *s, int len)
  *@Describe: 数组翻转
  *@Param_1 : s[] 要转换的数组
  *@Param_2 : len 数组大小
  *@Param_3 : 
  *@Return  : 
  */
void ReverseByArray(char *s, int len)
{
    char t;
    int i;
    for (i = 0; i < (len + 1) / 2; i++)
    {
        t = s[i];
        s[i] = s[len - i - 1];
        s[len - i - 1] = t;
    }
}

/****
  * Function: void ReverseByString(char * s)
  *@Describe: 字符串翻转
  *@Param_1 : *s 要转换的字符串
  *@Param_2 : 
  *@Param_3 : 
  *@Return  : 
  */
void ReverseByString(char * s)
{
    int len = 0;
    int i,t;
    while (s[len] != '\0')
        len++;
    for (i = 0; i < (len + 1) / 2; i++)
    {
        t = s[i];
        s[i] = s[len - i - 1];
        s[len - i - 1] = t;
    }
}



/****
  * Function: long bcds_to_int32(unsigned char bcds[],int len)
  *@Describe:将BCD数组转换成长整型数据。数组的数据均先低位后高位 
             DLT645_DATA_OFFSET 为数据DLT645的独特计算规则
             举例：bcds[] = 12 34 56 78   返回值为十进制的：78563412
  *@Param_1 : bcds[],被转换的BCD数组
  *@Param_2 : len，BCD数组长度
  *@Param_3 : 
  *@Return  : 返回 转换后的长整形数
  */
long bcds_to_int32(unsigned char bcds[],int len)
{
	int i;
	long rate,res;
	res = 0;
	rate = 1;
	for(i=0;i<len;i++)
	{		
		res += (((bcds[i]) & 0x0f)) * rate;
		rate = rate * 10;
		res += (((bcds[i]) >> 4)) * rate;
		rate = rate * 10;
	}
	return res;
}



long dlt645_data_to_int32(unsigned char bcds[],int len)
{
	int i;
	long rate,res;
	res = 0;
	rate = 1;
	for(i=0;i<len;i++)
	{		
		res += (((bcds[i] - 0x33) & 0x0f)) * rate;
		rate = rate * 10;
		res += (((bcds[i] - 0x33) >> 4)) * rate;
		rate = rate * 10;
	}
	return res;
}

/****
  * Function: void StringToBcd(char *Schar,char bcd[],int f_rev)
  *@Describe: 字符串转换为BCD数组
  *@Param_1 : char *Schar 要转换的字符串
  *@Param_2 : char bcd[] 转换存入的BCD数组
  *@Param_3 : int f_rev  数组存储方式，正序：0 逆序：1
  *@Return  : 
  */
void StringToBcd(unsigned char *Schar,unsigned char bcd[],int f_rev) 
{
	int i,len;
	len = strlen(Schar);
	for(i = 0; i < len; i+=2)
	{
		if(f_rev ==1)
			{
				bcd[(len-i)/2 - 1] = ChangeToBcd(*Schar++) << 4;
				bcd[(len-i)/2 - 1] += ChangeToBcd(*Schar++);
			}
		else
			{
				bcd[i/2] = ChangeToBcd(*Schar++) << 4;
				bcd[i/2] += ChangeToBcd(*Schar++);
			}
	}

}

/****
  * Function: char *BcdToString(char *Pchar,const char data[],int nbyte)
  *@Describe: BCD数组转换成字符串
  *@Param_1 : 
  *@Param_2 : 
  *@Param_3 : 
  *@Return  : 成功，返回 0；失败，返回 -1.
  */
unsigned char *BcdToString(unsigned char *Pchar,const unsigned char data[],int nbyte)
{
	int i;
	unsigned char *src = Pchar;
	for(i = 0; i < nbyte ; i++,Pchar)
	{
   if((data[i] >> 4) >= 0 &&  (data[i] >> 4) <= 9)
           *Pchar++ = ((data[i] >> 4) & 0x0f) + '0';
   else if((data[i] >> 4) >= 10 &&  (data[i] >> 4) <= 15)
           *Pchar++ = ((data[i] >> 4) & 0x0f) + 'a' - 10;
   if((data[i] & 0x0f) >= 0 &&  (data[i] & 0x0f) <= 9)
           *Pchar++ = (data[i] & 0x0f) + '0';
   else if((data[i] & 0x0f) >= 10 &&  (data[i] & 0x0f) <= 15)
           *Pchar++ = (data[i] & 0x0f) + 'a' - 10;	
	}
	*Pchar = '\0';
//	printf("\n-------------------------%s",src);
	return src;
}

/*
一元线性回归方程  y = bx + a
输入：double x[i]
			double y[i]
			double x_in
			
输出：double y_out


*/
 
double trend(double * x, double * y, int n, int x_in) 
{
    double a,b;
    double lxx,lxy,sumx,sumy,mx,my;

    int i;
 
    // 变量的初始化
    lxx = lxy = sumx = sumy = mx = my =0.0;
 		
    // 计算x、y的和
    for (i = 0; i < n; i++) {
        sumx += x[i];
        sumy += y[i];
    }       	
    printf("\n sumx=%f sumy=%f",sumx,sumy);
    if(sumx == 0.0 || sumy == 0.0)
    	return 0.0;
    	
    // 计算x、y的平均值
    mx = sumx / n;
    my = sumy / n;    
    printf("mx=%f my=%f\n",mx,my); 
    
    // 计算xx 和 xy平方和
    for (i = 0; i < n; i++) {
        lxy += (x[i] - mx) * (y[i] - my);
        lxx += (x[i] - mx) * (x[i] - mx);
    }
    
    //如果xx平方和等于0，则y_out = y[0]
    if(lxx == 0.0)
    	return y[0];
    
     // 计算系数 a、 b
    b = lxy/lxx; 
    a = my-b*mx; 
    
    printf("\n lxy=%f lxx=%f sumxy=%f",lxy,lxx);
    printf("\n a=%f b=%f",a,b);
    return (b*x_in + a);
}

/*
参考
计算x，y的平均数，x平方、y平方的平均数，xy乘积的平均数，y=a+bx中，a、b的值，相关系数r和标准偏差s(y)
*/

void analysis(double * x, double * y, int n) {
    double d1, d2, d3,a,b;
    double sumx,sumy,sumxx,sumyy,sumxy,mx,my,mxx,myy,mxy;
    int i;
 
    // 变量的初始化
    d1 = d2 = d3 =sumx=sumy=sumxx=sumyy=sumxy=0.0;
 
    // 计算x、y的平均值
    for (i = 0; i < n; i++) {
        sumx += x[i];
        sumy += y[i];
    }
    mx = sumx / n;
    my = sumy / n;
    printf("mx=%f my=%f\n",mx,my);
    // 计算x、y平和x*y的平均值
    for (i = 0; i < n; i++) {
        sumxx += x[i]*x[i];
        sumyy += y[i]*y[i];
        sumxy += x[i]*y[i];
    }
    mxx = sumxx / n;
    myy = sumyy / n;
    mxy = sumxy / n;
    printf("mxx=%f myy=%f mxy=%f\n",mxx,myy,mxy);
    //
    a=(n*sumxy-sumx*sumy)/(n*sumxx-sumx*sumx);          //此处a  b  应该反了！！！！！！！！！！
    b=(sumxx*sumy-sumx*sumxy)/(n*sumxx-sumx*sumx);
    printf("a=%f b=%f\n",a,b);
    // 计算相关系数的数据组成部分
    for (i = 0; i < n; i++) {
        d1 += (x[i] - mx) * (y[i] - my);
        d2 += (x[i] - mx) * (x[i] - mx);
        d3 += (y[i] - my) * (y[i] - my);
    }
     
    double r = d1 / sqrt(d2 * d3);
    //
    printf("相关系数r=%f\n",r);
    //
    double *yy=(double*)malloc(sizeof(double)*n);
    double sumerrorsquare=0,error;
    for(i=0;i<n;i++) {
        yy[i]=a*x[i]+b;
        sumerrorsquare+=(yy[i]-y[i])*(yy[i]-y[i]);
    }
    error=sqrt(sumerrorsquare/(n-1));
    printf("标准偏差s(y)=%f\n",error);
}