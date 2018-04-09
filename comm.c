#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/****
  * Function: int ChangeToBcd(char iChTemp)
  *@Describe: �ַ�ת����BCD
  *@Param_1 : char iChTemp Ҫת�����ַ���
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
  *@Describe: ���鷭ת
  *@Param_1 : s[] Ҫת��������
  *@Param_2 : len �����С
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
  *@Describe: �ַ�����ת
  *@Param_1 : *s Ҫת�����ַ���
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
  *@Describe:��BCD����ת���ɳ��������ݡ���������ݾ��ȵ�λ���λ 
             DLT645_DATA_OFFSET Ϊ����DLT645�Ķ��ؼ������
             ������bcds[] = 12 34 56 78   ����ֵΪʮ���Ƶģ�78563412
  *@Param_1 : bcds[],��ת����BCD����
  *@Param_2 : len��BCD���鳤��
  *@Param_3 : 
  *@Return  : ���� ת����ĳ�������
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
  *@Describe: �ַ���ת��ΪBCD����
  *@Param_1 : char *Schar Ҫת�����ַ���
  *@Param_2 : char bcd[] ת�������BCD����
  *@Param_3 : int f_rev  ����洢��ʽ������0 ����1
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
  *@Describe: BCD����ת�����ַ���
  *@Param_1 : 
  *@Param_2 : 
  *@Param_3 : 
  *@Return  : �ɹ������� 0��ʧ�ܣ����� -1.
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
һԪ���Իع鷽��  y = bx + a
���룺double x[i]
			double y[i]
			double x_in
			
�����double y_out


*/
 
double trend(double * x, double * y, int n, int x_in) 
{
    double a,b;
    double lxx,lxy,sumx,sumy,mx,my;

    int i;
 
    // �����ĳ�ʼ��
    lxx = lxy = sumx = sumy = mx = my =0.0;
 		
    // ����x��y�ĺ�
    for (i = 0; i < n; i++) {
        sumx += x[i];
        sumy += y[i];
    }       	
    printf("\n sumx=%f sumy=%f",sumx,sumy);
    if(sumx == 0.0 || sumy == 0.0)
    	return 0.0;
    	
    // ����x��y��ƽ��ֵ
    mx = sumx / n;
    my = sumy / n;    
    printf("mx=%f my=%f\n",mx,my); 
    
    // ����xx �� xyƽ����
    for (i = 0; i < n; i++) {
        lxy += (x[i] - mx) * (y[i] - my);
        lxx += (x[i] - mx) * (x[i] - mx);
    }
    
    //���xxƽ���͵���0����y_out = y[0]
    if(lxx == 0.0)
    	return y[0];
    
     // ����ϵ�� a�� b
    b = lxy/lxx; 
    a = my-b*mx; 
    
    printf("\n lxy=%f lxx=%f sumxy=%f",lxy,lxx);
    printf("\n a=%f b=%f",a,b);
    return (b*x_in + a);
}

/*
�ο�
����x��y��ƽ������xƽ����yƽ����ƽ������xy�˻���ƽ������y=a+bx�У�a��b��ֵ�����ϵ��r�ͱ�׼ƫ��s(y)
*/

void analysis(double * x, double * y, int n) {
    double d1, d2, d3,a,b;
    double sumx,sumy,sumxx,sumyy,sumxy,mx,my,mxx,myy,mxy;
    int i;
 
    // �����ĳ�ʼ��
    d1 = d2 = d3 =sumx=sumy=sumxx=sumyy=sumxy=0.0;
 
    // ����x��y��ƽ��ֵ
    for (i = 0; i < n; i++) {
        sumx += x[i];
        sumy += y[i];
    }
    mx = sumx / n;
    my = sumy / n;
    printf("mx=%f my=%f\n",mx,my);
    // ����x��yƽ��x*y��ƽ��ֵ
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
    a=(n*sumxy-sumx*sumy)/(n*sumxx-sumx*sumx);          //�˴�a  b  Ӧ�÷��ˣ�������������������
    b=(sumxx*sumy-sumx*sumxy)/(n*sumxx-sumx*sumx);
    printf("a=%f b=%f\n",a,b);
    // �������ϵ����������ɲ���
    for (i = 0; i < n; i++) {
        d1 += (x[i] - mx) * (y[i] - my);
        d2 += (x[i] - mx) * (x[i] - mx);
        d3 += (y[i] - my) * (y[i] - my);
    }
     
    double r = d1 / sqrt(d2 * d3);
    //
    printf("���ϵ��r=%f\n",r);
    //
    double *yy=(double*)malloc(sizeof(double)*n);
    double sumerrorsquare=0,error;
    for(i=0;i<n;i++) {
        yy[i]=a*x[i]+b;
        sumerrorsquare+=(yy[i]-y[i])*(yy[i]-y[i]);
    }
    error=sqrt(sumerrorsquare/(n-1));
    printf("��׼ƫ��s(y)=%f\n",error);
}