/*---------------------------------------------------------------------*/
/* --- �����Ž��ײ���� -----------------------------------------------*/
/* --- ʱ�䣺2014.11.14 -----------------------------------------------*/
/* --- ���ļ�¼���� ---------------------------------------------------*/
/* --- ��ϵ�绰��010-68949731/32 -------------------------------------------*/
/* --- Web: www.r8c.com --------------------------------------------*/
/*---------------------------------------------------------------------*/


#include "reg51.h"
#include "intrins.h"


#include "main.h"
#include "RC_control.h"


typedef unsigned int WORD;

#define FOSC 18432000L          //ϵͳƵ��
#define BAUD 9600             //���ڲ�����

#define NONE_PARITY     0       //��У��
#define ODD_PARITY      1       //��У��
#define EVEN_PARITY     2       //żУ��
#define MARK_PARITY     3       //���У��
#define SPACE_PARITY    4       //�հ�У��

#define PARITYBIT EVEN_PARITY   //����У��λ

sfr AUXR  = 0x8e;               //�����Ĵ���
sfr S2CON = 0x9a;               //UART2 ���ƼĴ���
sfr S2BUF = 0x9b;               //UART2 ���ݼĴ���
sfr T2H   = 0xd6;               //��ʱ��2��8λ
sfr T2L   = 0xd7;               //��ʱ��2��8λ
sfr IE2   = 0xaf;               //�жϿ��ƼĴ���2

#define S2RI  0x01              //S2CON.0
#define S2TI  0x02              //S2CON.1
#define S2RB8 0x04              //S2CON.2
#define S2TB8 0x08              //S2CON.3

sfr P_SW2   = 0xBA;             //���蹦���л��Ĵ���2
#define S2_S0 0x01              //P_SW2.0

bit busy;
bit Flag;
BYTE GetC;

void main()
{

	  BYTE i = 0;
    P_SW2 &= ~S2_S0;            //S2_S0=0 (P1.0/RxD2, P1.1/TxD2)
//  P_SW2 |= S2_S0;             //S2_S0=1 (P4.6/RxD2_2, P4.7/TxD2_2)

#if (PARITYBIT == NONE_PARITY)
    S2CON = 0x50;               //8λ�ɱ䲨����
#elif (PARITYBIT == ODD_PARITY) || (PARITYBIT == EVEN_PARITY) || (PARITYBIT == MARK_PARITY)
    S2CON = 0xda;               //9λ�ɱ䲨����,У��λ��ʼΪ1
#elif (PARITYBIT == SPACE_PARITY)
    S2CON = 0xd2;               //9λ�ɱ䲨����,У��λ��ʼΪ0
#endif

    T2L = (65536 - (FOSC/4/BAUD));   //���ò�������װֵ
    T2H = (65536 - (FOSC/4/BAUD))>>8;
    AUXR = 0x14;                //T2Ϊ1Tģʽ, ��������ʱ��2
    IE2 = 0x01;                 //ʹ�ܴ���2�ж�
    EA = 1;
		
   // SendString("STC15F2K60S2\r\nUart2 Test !\r\n");
	Lock=0;
	RC_init();  // RC522��ʼ��
  //SendString("OK\n");
  while(1)
  {
	  RC_control(); // ʶ��RFID
		
		
	  if(Flag)
	  {

		 switch(GetC)
		 {
	      	case 'a': Lock=1;  
					break;
	   		case 'b': Lock=0;  
					break;
			
		    default: Lock=0;   break;
		 }
		 

	   SendData(GetC);
		 Flag=0;
	 }	 
	}
}

/*----------------------------
UART2 �жϷ������
-----------------------------*/
void Uart2() interrupt 8 using 1
{
  
    if (S2CON & S2RI)
    {
        S2CON &= ~S2RI;         //���S2RIλ
        GetC  = S2BUF;             //P0��ʾ��������
		    Flag=1;
    }
    if (S2CON & S2TI)
    {
        S2CON &= ~S2TI;         //���S2TIλ
        busy = 0;               //��æ��־
    }

}

/*----------------------------
���ʹ�������
----------------------------*/

void SendData(BYTE dat)
{
    while (busy);               //�ȴ�ǰ������ݷ������
    ACC = dat;                  //��ȡУ��λP (PSW.0)
    if (P)                      //����P������У��λ
    {
#if (PARITYBIT == ODD_PARITY)
        S2CON &= ~S2TB8;        //����У��λΪ0
#elif (PARITYBIT == EVEN_PARITY)
        S2CON |= S2TB8;         //����У��λΪ1
#endif
    }
    else
    {
#if (PARITYBIT == ODD_PARITY)
        S2CON |= S2TB8;         //����У��λΪ1
#elif (PARITYBIT == EVEN_PARITY)
        S2CON &= ~S2TB8;        //����У��λΪ0
#endif
    }
    busy = 1;
    S2BUF = ACC;                //д���ݵ�UART2���ݼĴ���
}

/*----------------------------
�����ַ���
----------------------------*/

void SendString(char *s)
{
    while (*s)                  //����ַ���������־
    {
        SendData(*s++);         //���͵�ǰ�ַ�
    }
}
 