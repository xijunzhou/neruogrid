#ifndef _COMM_H_
#define _COMM_H_


int ChangeToBcd(char iChTemp);
void ReverseByArray(char *s, int len);
void ReverseByString(char * s);
long bcds_to_int32(unsigned char bcds[],int len);
long dlt645_data_to_int32(unsigned char bcds[],int len);

void StringToBcd(unsigned char *Schar,unsigned char bcd[],int f_rev);
unsigned char *BcdToString(unsigned char *Pchar,const unsigned char data[],int nbyte);
double trend(double * x, double * y, int n, int x_in);

#endif
