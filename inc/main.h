#ifndef _MAIN_
#define _MAIN_
typedef unsigned char BYTE;
sbit Lock = P2^0;
void SendString(char *s);
void SendData(BYTE dat);

#endif

