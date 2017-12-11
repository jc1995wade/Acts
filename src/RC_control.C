/*���͵���*/
#include "reg51.h"
#include "RC_control.h"
#include "mfrc522.h"
#include<intrins.h>         //��������ͷ�ļ�	
#include<string.h>          //�ַ�������ͷ�ļ�  
#include<stdio.h>           // �����������ͷ�ļ�
#include "main.h"
unsigned char code data1[16] = {0x00,0x00,0x00,0xAA,\
																0xFF,0xFF,0xFF,0x55,\
																0x00,0x00,0x00,0xAA,
																0x01,0xFE,0x01,0xFE};//
//M1����ĳһ��дΪ���¸�ʽ����ÿ�Ϊ�Ž�����ͬ�����޸����뼴��
//4�ֽ����루���ֽ���ǰ����4�ֽ�����ȡ����4�ֽ����룫1�ֽڿ��ַ��1�ֽڿ��ַȡ����1�ֽڿ��ַ��1�ֽڿ��ַȡ��
//unsigned char code data2[16] = {0x00,0x00,0x00,0x55,0xFF,0xFF,0xFF,0xAA,0x00,0x00,0x00,0x55,0x01,0xFE,0x01,0xFE};//
unsigned char code data2[16] = {0x00,0x00,0x00,0x00,\
																0xFF,0xFF,0xFF,0xff,\
																0x00,0x00,0x00,0x00,\
																0x01,0xFE,0x01,0xFE};
//M1����ĳһ��дΪ���¸�ʽ����ÿ�Ϊ�Ž�����ͬ�����޸����뼴��
//4�ֽ����루���ֽ���ǰ����4�ֽ�����ȡ����4�ֽ����룫1�ֽڿ��ַ��1�ֽڿ��ַȡ����1�ֽڿ��ַ��1�ֽڿ��ַȡ��
unsigned char code DefaultKey[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}; //����
unsigned char TYPE_DATA[2];//����
unsigned char ID_DATA[4];  //ID��ַ 
unsigned char value_DATA[4];//�������ֵ					                            					                      
unsigned char g_ucTempbuf[20];//

//unsigned char code  DIS1[] = {"  �Ž�����ϵͳ  "};
//unsigned char code  DIS2[] = {"ID��"};
//unsigned char code  DIS3[] = {"�����ӻ�ӭ����"};
//unsigned char code  DIS4[] = {"������ˢ����    "};
//unsigned char code  DIS6[] = {"0123456789ABCDEF"};
//unsigned char code  DIS7[]=  {"�����          "}; 
unsigned char tt=0,key_data,key_olddata,kaimengtime;  


unsigned char mode = 0;   // 0��д  2�����Ž���  3ʹ���Ž���
//#define WRITE_MODE 3      // �Ƿ�ִ��д�뺯��

unsigned int  a,b;


void RC_init()
{
		 InitializeSystem( );
     PcdReset();
     PcdAntennaOff(); //�ر�����
     PcdAntennaOn();  //��������
	   SendString("RFID ");
	   SendString("OK!\n");
}

void RC_control( )
{    
     unsigned char status;
 		 static unsigned char door_st = 0;
         status = PcdRequest(PICC_REQALL, g_ucTempbuf); //Ѱ��
				 if(status != MI_OK)//û��
				 {
					  door_st = 1;
						ID_DATA[0]=0x00;
						ID_DATA[1]=0x00;
						ID_DATA[2]=0x00;
						ID_DATA[3]=0x00;
						b=0;
					 
				 }
				 
         if (status == MI_OK)//�п�
         { 	 			 
						 
						 TYPE_DATA[0]=g_ucTempbuf[0];
						 TYPE_DATA[1]=g_ucTempbuf[1];
						 status = PcdAnticoll(g_ucTempbuf);       //����ײ �����ؿ�ƬID�� 4�ֽ�
		         if (status == MI_OK)
		         { 
							  
								 status = PcdSelect(g_ucTempbuf);    //ѡ����Ƭ  ,���뿨ƬID��
				         if (status == MI_OK)
				         { 
											ID_DATA[0]=g_ucTempbuf[0];
											ID_DATA[1]=g_ucTempbuf[1];
											ID_DATA[2]=g_ucTempbuf[2];
											ID_DATA[3]=g_ucTempbuf[3];
#ifdef WRITE_MODE
/*
 *  д�� 
 *  ��ǰ����mode�Ž�ģʽ: mode=3Ϊ���Ž���д��aa�����ÿ�����
 *	mode=3Ϊ���Ž���д��aa�����ÿ�����				 			 
 */			 
		mode = WRITE_MODE;
		writek();             
#endif
											readk();//����
											SendData(value_DATA[0]);
											SendData(value_DATA[1]);
											SendData(value_DATA[2]);
											SendData(value_DATA[3]);
											if(value_DATA[3]== 0xAA && door_st == 1)
											{
													Lock = ~Lock;
													Delay(2000);//��ʱ����
												  door_st = 0;
											}
				         } 	  
		         }
	       }  
		   
				 PcdHalt();//���Ƭ��������״̬����ΪѰ��������Ѱ�����еĿ��������ߵĿ��������������ȥ��	    
}


void InitializeSystem()//��������ڳ�ʼ��(���ж˿�)
{     
   P0 = 0xFF;
	 P1 = 0xFF;
	 P2 = 0xFF; 
	 P3 = 0xFF;

}


void Delay(unsigned int time)//��ʱ����
{
  unsigned int i,k  ;
  for(i=0;i<255;i++)
    for(k=0;k<time;k++)
      _nop_();	  
}

/*
void time0() interrupt 1// �жϷ����ӳ��򣬷�������������3�Σ�����ʱ��-1
{

	TH0=(65536-50000)/256;//����ʱ��װ��ֵ �߰�λ
	TL0=(65536-50000)%256;//����ʱ��װ��ֵ �Ͱ�λ
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

void readk()//������b��ֵ��Ϊ��־
{	
	unsigned char status;
	status = PcdAuthState(PICC_AUTHENT1A, 1, DefaultKey, g_ucTempbuf);//PICC_AUTHENT1A ��֤����ģʽ60A���� 61B���룬��1�����ַ��DefaultKey��ʼ���룬g_ucTempbuf ID��
	if (status == MI_OK) //��֤����
	{	 																																														  
    status = PcdRead(1, g_ucTempbuf);//�����ַ��1�������ݣ�����ֵ����	g_ucTempbuf
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


void writek()//���Ž���д��ֵ��00����aa AA���Կ��� 00�Ĳ����Կ���
{
    unsigned char status;
    if( mode==3)//д��0xaa��ֵ
		{	
			mode=0;
		  status = PcdAuthState(PICC_AUTHENT1A, 1, DefaultKey, g_ucTempbuf);//PICC_AUTHENT1A ��֤����ģʽ60A���� 61B���룬��1�����ַ��DefaultKey��ʼ���룬g_ucTempbuf ID��
			if (status == MI_OK) //��֤����
			  {																																								  	 
				  status = PcdWrite(1, data1);//д������
				  if (status == MI_OK)
					{ 
						SendString("write ok\n");						
				  } 
			  }
		}
		
		if(mode==2) //д��0x00                   
		{
			  mode=0;
	      status = PcdAuthState(PICC_AUTHENT1A, 1, DefaultKey, g_ucTempbuf);//PICC_AUTHENT1A ��֤����ģʽ60A���� 61B���룬��1�����ַ��DefaultKey��ʼ���룬g_ucTempbuf ID��
		    if (status == MI_OK) //��֤����
		    {																																								  	 
				  status = PcdWrite(1, data2);//д������
				  if (status == MI_OK)
					{ 
						//pro(); 
					} 
		    }
		}
} 

