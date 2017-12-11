/*铭贤电子*/
#include "reg51.h"
#include "RC_control.h"
#include "mfrc522.h"
#include<intrins.h>         //函数处理头文件	
#include<string.h>          //字符串处理头文件  
#include<stdio.h>           // 输入输出函数头文件
#include "main.h"
unsigned char code data1[16] = {0x00,0x00,0x00,0xAA,\
																0xFF,0xFF,0xFF,0x55,\
																0x00,0x00,0x00,0xAA,
																0x01,0xFE,0x01,0xFE};//
//M1卡的某一块写为如下格式，则该块为门禁，不同的门修改门码即可
//4字节门码（低字节在前）＋4字节门码取反＋4字节门码＋1字节块地址＋1字节块地址取反＋1字节块地址＋1字节块地址取反
//unsigned char code data2[16] = {0x00,0x00,0x00,0x55,0xFF,0xFF,0xFF,0xAA,0x00,0x00,0x00,0x55,0x01,0xFE,0x01,0xFE};//
unsigned char code data2[16] = {0x00,0x00,0x00,0x00,\
																0xFF,0xFF,0xFF,0xff,\
																0x00,0x00,0x00,0x00,\
																0x01,0xFE,0x01,0xFE};
//M1卡的某一块写为如下格式，则该块为门禁，不同的门修改门码即可
//4字节门码（低字节在前）＋4字节门码取反＋4字节门码＋1字节块地址＋1字节块地址取反＋1字节块地址＋1字节块地址取反
unsigned char code DefaultKey[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}; //密码
unsigned char TYPE_DATA[2];//类型
unsigned char ID_DATA[4];  //ID地址 
unsigned char value_DATA[4];//卡里面的值					                            					                      
unsigned char g_ucTempbuf[20];//

//unsigned char code  DIS1[] = {"  门禁管理系统  "};
//unsigned char code  DIS2[] = {"ID："};
//unsigned char code  DIS3[] = {"曙光电子欢迎光临"};
//unsigned char code  DIS4[] = {"进门请刷卡！    "};
//unsigned char code  DIS6[] = {"0123456789ABCDEF"};
//unsigned char code  DIS7[]=  {"请进！          "}; 
unsigned char tt=0,key_data,key_olddata,kaimengtime;  


unsigned char mode = 0;   // 0不写  2禁用门禁卡  3使能门禁卡
//#define WRITE_MODE 3      // 是否执行写入函数

unsigned int  a,b;


void RC_init()
{
		 InitializeSystem( );
     PcdReset();
     PcdAntennaOff(); //关闭天线
     PcdAntennaOn();  //开启天线
	   SendString("RFID ");
	   SendString("OK!\n");
}

void RC_control( )
{    
     unsigned char status;
 		 static unsigned char door_st = 0;
         status = PcdRequest(PICC_REQALL, g_ucTempbuf); //寻卡
				 if(status != MI_OK)//没卡
				 {
					  door_st = 1;
						ID_DATA[0]=0x00;
						ID_DATA[1]=0x00;
						ID_DATA[2]=0x00;
						ID_DATA[3]=0x00;
						b=0;
					 
				 }
				 
         if (status == MI_OK)//有卡
         { 	 			 
						 
						 TYPE_DATA[0]=g_ucTempbuf[0];
						 TYPE_DATA[1]=g_ucTempbuf[1];
						 status = PcdAnticoll(g_ucTempbuf);       //防冲撞 ，返回卡片ID号 4字节
		         if (status == MI_OK)
		         { 
							  
								 status = PcdSelect(g_ucTempbuf);    //选定卡片  ,输入卡片ID号
				         if (status == MI_OK)
				         { 
											ID_DATA[0]=g_ucTempbuf[0];
											ID_DATA[1]=g_ucTempbuf[1];
											ID_DATA[2]=g_ucTempbuf[2];
											ID_DATA[3]=g_ucTempbuf[3];
#ifdef WRITE_MODE
/*
 *  写卡 
 *  提前设置mode门禁模式: mode=3为给门禁卡写入aa允许该卡开门
 *	mode=3为给门禁卡写入aa允许该卡开门				 			 
 */			 
		mode = WRITE_MODE;
		writek();             
#endif
											readk();//读卡
											SendData(value_DATA[0]);
											SendData(value_DATA[1]);
											SendData(value_DATA[2]);
											SendData(value_DATA[3]);
											if(value_DATA[3]== 0xAA && door_st == 1)
											{
													Lock = ~Lock;
													Delay(2000);//延时函数
												  door_st = 0;
											}
				         } 	  
		         }
	       }  
		   
				 PcdHalt();//命令卡片进入休眠状态，因为寻卡命令是寻找所有的卡包括休眠的卡，这条命令可以去掉	    
}


void InitializeSystem()//输入输出口初始化(所有端口)
{     
   P0 = 0xFF;
	 P1 = 0xFF;
	 P2 = 0xFF; 
	 P3 = 0xFF;

}


void Delay(unsigned int time)//延时函数
{
  unsigned int i,k  ;
  for(i=0;i<255;i++)
    for(k=0;k<time;k++)
      _nop_();	  
}

/*
void time0() interrupt 1// 中断服务子程序，蜂鸣器声音进入3次，开门时间-1
{

	TH0=(65536-50000)/256;//给定时器装初值 高八位
	TL0=(65536-50000)%256;//给定时器装初值 低八位
	tt++;
	if(tt==3)
	{	
		tt=0;
	}
	if(kaimengtime>0)
  {	
		kaimengtime--;  
	}	
}
*/

void readk()//读卡，b的值及为标志
{	
	unsigned char status;
	status = PcdAuthState(PICC_AUTHENT1A, 1, DefaultKey, g_ucTempbuf);//PICC_AUTHENT1A 验证密码模式60A密码 61B密码，“1”块地址，DefaultKey初始密码，g_ucTempbuf ID号
	if (status == MI_OK) //验证密码
	{	 																																														  
    status = PcdRead(1, g_ucTempbuf);//读块地址“1”的数据，返回值存在	g_ucTempbuf
		if (status == MI_OK)
		{ 
				value_DATA[0]=g_ucTempbuf[0];
				value_DATA[1]=g_ucTempbuf[1];
				value_DATA[2]=g_ucTempbuf[2];
				value_DATA[3]=g_ucTempbuf[3];	
			
	
			
				b=value_DATA[3];																			     
		}   
  }
}	


void writek()//给门禁卡写入值，00或者aa AA可以开门 00的不可以开门
{
    unsigned char status;
    if( mode==3)//写入0xaa充值
		{	
			mode=0;
		  status = PcdAuthState(PICC_AUTHENT1A, 1, DefaultKey, g_ucTempbuf);//PICC_AUTHENT1A 验证密码模式60A密码 61B密码，“1”块地址，DefaultKey初始密码，g_ucTempbuf ID号
			if (status == MI_OK) //验证密码
			  {																																								  	 
				  status = PcdWrite(1, data1);//写入门码
				  if (status == MI_OK)
					{ 
						SendString("write ok\n");						
				  } 
			  }
		}
		
		if(mode==2) //写入0x00                   
		{
			  mode=0;
	      status = PcdAuthState(PICC_AUTHENT1A, 1, DefaultKey, g_ucTempbuf);//PICC_AUTHENT1A 验证密码模式60A密码 61B密码，“1”块地址，DefaultKey初始密码，g_ucTempbuf ID号
		    if (status == MI_OK) //验证密码
		    {																																								  	 
				  status = PcdWrite(1, data2);//写入门码
				  if (status == MI_OK)
					{ 
						//pro(); 
					} 
		    }
		}
} 


