/*---------------------------------------------------------------------*/
/* --- 智能门禁底层程序 -----------------------------------------------*/
/* --- 时间：2014.11.14 -----------------------------------------------*/
/* --- 更改记录：无 ---------------------------------------------------*/
/* --- 联系电话：010-68949731/32 -------------------------------------------*/
/* --- Web: www.r8c.com --------------------------------------------*/
/*---------------------------------------------------------------------*/


#include "reg51.h"
#include "intrins.h"


#include "main.h"
#include "RC_control.h"


typedef unsigned int WORD;

#define FOSC 18432000L          //系统频率
#define BAUD 9600             //串口波特率

#define NONE_PARITY     0       //无校验
#define ODD_PARITY      1       //奇校验
#define EVEN_PARITY     2       //偶校验
#define MARK_PARITY     3       //标记校验
#define SPACE_PARITY    4       //空白校验

#define PARITYBIT EVEN_PARITY   //定义校验位

sfr AUXR  = 0x8e;               //辅助寄存器
sfr S2CON = 0x9a;               //UART2 控制寄存器
sfr S2BUF = 0x9b;               //UART2 数据寄存器
sfr T2H   = 0xd6;               //定时器2高8位
sfr T2L   = 0xd7;               //定时器2低8位
sfr IE2   = 0xaf;               //中断控制寄存器2

#define S2RI  0x01              //S2CON.0
#define S2TI  0x02              //S2CON.1
#define S2RB8 0x04              //S2CON.2
#define S2TB8 0x08              //S2CON.3

sfr P_SW2   = 0xBA;             //外设功能切换寄存器2
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
    S2CON = 0x50;               //8位可变波特率
#elif (PARITYBIT == ODD_PARITY) || (PARITYBIT == EVEN_PARITY) || (PARITYBIT == MARK_PARITY)
    S2CON = 0xda;               //9位可变波特率,校验位初始为1
#elif (PARITYBIT == SPACE_PARITY)
    S2CON = 0xd2;               //9位可变波特率,校验位初始为0
#endif

    T2L = (65536 - (FOSC/4/BAUD));   //设置波特率重装值
    T2H = (65536 - (FOSC/4/BAUD))>>8;
    AUXR = 0x14;                //T2为1T模式, 并启动定时器2
    IE2 = 0x01;                 //使能串口2中断
    EA = 1;
		
   // SendString("STC15F2K60S2\r\nUart2 Test !\r\n");
	Lock=0;
	RC_init();  // RC522初始化
  //SendString("OK\n");
  while(1)
  {
	  RC_control(); // 识别RFID
		
		
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
UART2 中断服务程序
-----------------------------*/
void Uart2() interrupt 8 using 1
{
  
    if (S2CON & S2RI)
    {
        S2CON &= ~S2RI;         //清除S2RI位
        GetC  = S2BUF;             //P0显示串口数据
		    Flag=1;
    }
    if (S2CON & S2TI)
    {
        S2CON &= ~S2TI;         //清除S2TI位
        busy = 0;               //清忙标志
    }

}

/*----------------------------
发送串口数据
----------------------------*/

void SendData(BYTE dat)
{
    while (busy);               //等待前面的数据发送完成
    ACC = dat;                  //获取校验位P (PSW.0)
    if (P)                      //根据P来设置校验位
    {
#if (PARITYBIT == ODD_PARITY)
        S2CON &= ~S2TB8;        //设置校验位为0
#elif (PARITYBIT == EVEN_PARITY)
        S2CON |= S2TB8;         //设置校验位为1
#endif
    }
    else
    {
#if (PARITYBIT == ODD_PARITY)
        S2CON |= S2TB8;         //设置校验位为1
#elif (PARITYBIT == EVEN_PARITY)
        S2CON &= ~S2TB8;        //设置校验位为0
#endif
    }
    busy = 1;
    S2BUF = ACC;                //写数据到UART2数据寄存器
}

/*----------------------------
发送字符串
----------------------------*/

void SendString(char *s)
{
    while (*s)                  //检测字符串结束标志
    {
        SendData(*s++);         //发送当前字符
    }
}
 