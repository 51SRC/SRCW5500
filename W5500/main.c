/****************************************Copyright (c)****************************************************
**                                        
**                                 
**
**--------------File Info---------------------------------------------------------------------------------
** File name:			
** Last modified Date:          
** Last Version:		   
** Descriptions:							
**--------------------------------------------------------------------------------------------------------
** Created by:			FiYu
** Created date:		2017-02-20
** Version:			    1.0
** Descriptions:		IKMSIK_W5500ģ�飭�ͻ���ģʽʵ��(ģ��SPI)			
**--------------------------------------------------------------------------------------------------------
** Modified by:			
** Modified date:		
** Version:				
** Descriptions:		
** Rechecked by:			
**********************************************************************************************************/
/****-----��ο�IKMSIK_W5500�ͻ���ģʽ����(ģ��SPI)PDF�ĵ�����ʵ��------***********/

#include "W5500.h"				
#include <string.h>
typedef char I8;
typedef int I16;
typedef long I32;
typedef unsigned char U8; 

sbit PUMP = P3 ^ 6;  // LED
//sbit PUMP = P2 ^ 3;  // ���PUMP
sbit Buzzer    =  P5 ^ 4;           	// ������

static	 unsigned int   Timer4_Count=1;
unsigned char xdata SRCCID[] = {"SRC00000000000002"};

void delay_ms(unsigned int x) ;			//��ʱ����(ms)
void Timer4Init();
unsigned char CheckBCC(unsigned char len, unsigned char *recv);
void ResponseData(unsigned char len,unsigned char *RES_DATA);
void Buzzer_Actions_Status(unsigned char status);
void Pump_Actions_Status(unsigned char status);//����ˮ��


unsigned char CheckBCC(unsigned char len, unsigned char *recv){
	  unsigned char bcc = 0x00;
		unsigned char i=0;
    for(i=0;i<len-1;i++)
    {
        bcc^=recv[i];
    };
    return bcc;

}

void Timer4Init(void)		
{
	//50 ����@11.0592MHz
	T4T3M &= 0xDF;		//��ʱ��ʱ��12Tģʽ
	T4L = 0x00;		//���ö�ʱ��ֵ
	T4H = 0x4C;		//���ö�ʱ��ֵ
	T4T3M |= 0x80;		//��ʱ��4��ʼ��ʱ
	
		IE2 |= 0x40;		//����ʱ��4�ж�
		EA=1; 	//���жϿ���
}


//10s�ж��Զ��ϱ���Ϣ
void Timer4_interrupt() interrupt 20    //��ʱ�ж����
{
	
		if(Timer4_Count>=200){  //200 * 50ms = 10s
			  	unsigned char j=0;
					U8 xdata RES_DATA[37]= {0};
          unsigned char RES_LEN= 37;
					unsigned char  light_status = PUMP ? 0x02 : 0x01;
					unsigned char buzzy_status = Buzzer ? 0x02 : 0x01;
					Timer4_Count = 0;

				  RES_DATA[0] = 0X23;//����ͷ
					RES_DATA[1] = 0X23;
					RES_DATA[2] = 0X10;//�����ʶ  �·�0x8006  ���ڵ��ϴ���0x1006
					RES_DATA[3] = 0X06;
					RES_DATA[4] = 0xFE;//Ӧ���ʶ
						
				 for(j=0;j<17;j++){//CID��ֵ
						RES_DATA[j+5] = SRCCID[j];
				 }
				 
				RES_DATA[22] = 0X01;//������
				RES_DATA[23] = 0X00;//������λ ��λ00
				RES_DATA[24] = 0X0B;//��λ0B һ��11λ

				RES_DATA[25] = 0X14;//�� 0x14+2000 = 2020 
				RES_DATA[26] = 0X05;//�� 
				RES_DATA[27] = 0X18;//�� 
				RES_DATA[28] = 0X15;//ʱ 
				RES_DATA[29] = 0X24;//��
				RES_DATA[30] = 0X08;//��
				
				RES_DATA[31] = 0X02;//���������ϱ�


			RES_DATA[32] = 12;//DATA_Temphui[0];
			RES_DATA[33] = 	14;//DATA_Temphui[1];
			RES_DATA[34] = light_status;
			RES_DATA[35] = buzzy_status,
			RES_DATA[RES_LEN-1] = CheckBCC(RES_LEN, RES_DATA);
			
			if(S0_State == (S_INIT|S_CONN))
			{
				S0_Data&=~S_TRANSMITOK;
				memcpy(Tx_Buffer, RES_DATA, 37);	
				Write_SOCK_Data_Buffer(0, Tx_Buffer, 37);//ָ��Socket(0~7)�������ݴ���,�˿�0����20�ֽ�����
			}
					
		}else{
			
		    Timer4_Count++;
		}
		
}



/*******************************************************************************
* ������  : W5500_Initialization
* ����    : W5500��ʼ������
* ����    : ��
* ���    : ��
* ����ֵ  : ��
* ˵��    : ��
*******************************************************************************/
void W5500_Initialization(void)
{
	W5500_Init();		//��ʼ��W5500�Ĵ�������
	Detect_Gateway();	//������ط����� 
	Socket_Init(0);		//ָ��Socket(0~7)��ʼ��,��ʼ���˿�0
}

/*******************************************************************************
* ������  : Load_Net_Parameters
* ����    : װ���������
* ����    : ��
* ���    : ��
* ����ֵ  : ��
* ˵��    : ���ء����롢�����ַ������IP��ַ���˿ںš�Ŀ��IP��ַ��Ŀ�Ķ˿ںš��˿ڹ���ģʽ
*******************************************************************************/
void Load_Net_Parameters(void)
{
	Gateway_IP[0] = 192;//�������ز���
	Gateway_IP[1] = 168;
	Gateway_IP[2] = 0;
	Gateway_IP[3] = 1;

	Sub_Mask[0]=255;//������������
	Sub_Mask[1]=255;
	Sub_Mask[2]=255;
	Sub_Mask[3]=0;

	Phy_Addr[0]=0x0c;//���������ַ
	Phy_Addr[1]=0x29;
	Phy_Addr[2]=0xab;
	Phy_Addr[3]=0x7c;
	Phy_Addr[4]=0x00;
	Phy_Addr[5]=0x01;
	
	IP_Addr[0]=192;     //���ر���IP��ַ
	IP_Addr[1]=168;
	IP_Addr[2]=0;
	IP_Addr[3]=101;

	S0_Port[0] = 0x13;  //���ض˿�0�Ķ˿ں�5000 
	S0_Port[1] = 0x88;

	S0_DIP[0]=47;      //���ض˿�0��Ŀ��IP��ַ 47.104.19.111
	S0_DIP[1]=104;
	S0_DIP[2]=19;
	S0_DIP[3]=111;
	
	S0_DPort[0] = 0x0F; //���ض˿�0��Ŀ�Ķ˿ں�4001
	S0_DPort[1] = 0xA1;

	S0_Mode=TCP_CLIENT; //���ض˿�0�Ĺ���ģʽ,TCP�ͻ���ģʽ
}

/*******************************************************************************
* ������  : W5500_Socket_Set
* ����    : W5500�˿ڳ�ʼ������
* ����    : ��
* ���    : ��
* ����ֵ  : ��
* ˵��    : �ֱ�����4���˿�,���ݶ˿ڹ���ģʽ,���˿�����TCP��������TCP�ͻ��˻�UDPģʽ.
*			�Ӷ˿�״̬�ֽ�Socket_State�����ж϶˿ڵĹ������
*******************************************************************************/
void W5500_Socket_Set(void)
{
	if(S0_State==0)//�˿�0��ʼ������
	{
		if(S0_Mode==TCP_SERVER)//TCP������ģʽ 
		{
			if(Socket_Listen(0)==TRUE)
				S0_State=S_INIT;
			else
				S0_State=0;
		}
		else if(S0_Mode==TCP_CLIENT)   //TCP�ͻ���ģʽ 
		{
			if(Socket_Connect(0)==TRUE)
				S0_State=S_INIT;
			else
				S0_State=0;
		}
		else          //UDPģʽ 
		{
			if(Socket_UDP(0)==TRUE)
				S0_State=S_INIT|S_CONN;
			else
				S0_State=0;
		}
	}
}

/*******************************************************************************
* ������  : Process_Socket_Data
* ����    : W5500���ղ����ͽ��յ�������
* ����    : s:�˿ں�
* ���    : ��
* ����ֵ  : ��
* ˵��    : �������ȵ���S_rx_process()��W5500�Ķ˿ڽ������ݻ�������ȡ����,
*			Ȼ�󽫶�ȡ�����ݴ�Rx_Buffer������Temp_Buffer���������д���
*			������ϣ������ݴ�Temp_Buffer������Tx_Buffer������������S_tx_process()
*			�������ݡ�
*******************************************************************************/
void Process_Socket_Data(SOCKET s)
{
	unsigned short size;
	size=Read_SOCK_Data_Buffer(s, Rx_Buffer);
	
		ResponseData(size,Rx_Buffer);		

//	memcpy(Tx_Buffer, Rx_Buffer, size);			
//	Write_SOCK_Data_Buffer(s, Tx_Buffer, size);
}




void ResponseData(unsigned char len,unsigned char *RES_DATA) {
	
	if(len <26){
		return ;
	}


//У���
	if(CheckBCC(len, RES_DATA) == RES_DATA[len-1]){
	
		 unsigned int dataCmdFlag =(RES_DATA[2] << 8) | RES_DATA[3];         //�����ʶ
		 unsigned char dataCmdAck = RES_DATA[4];          //Ӧ���ʶ
		 unsigned char j=0;
		 unsigned char dataEncryptFlag = RES_DATA[22];    //���ܷ�ʽ
		 unsigned char dataUintLength = (RES_DATA[23] << 8) | RES_DATA[24];  //���ݳ���
		 unsigned char xdata  dataTimestamp[6] = {0x00,0x00,0x00,0x00,0x00,0x00};  //ʱ������

	 //У��CID�Ƿ���ȷ
		 for(j=5;j<22;j++){
			  if(SRCCID[j-5] != RES_DATA[j]){
				 return;
			 }
		 }
		
		 //У�鳤���Ƿ���ȷ
		 if ((26 + dataUintLength) != len) {
				return ;
		 }
		 
		 
		 //����ʱ��
		 for(j=0;j<6;j++){
			 dataTimestamp[j] = RES_DATA[25+j];
		 }
		 
		 if(dataCmdFlag == 0x8001){//������֤
			 
		 }else if(dataCmdFlag ==0x8002){//ʵʱ��Ϣ�����ϱ�
			 
		 }else if(dataCmdFlag ==0x8003){//����
			 
		 }else if(dataCmdFlag ==0x8004){//�豸�ǳ�
			 
		 }else if(dataCmdFlag ==0x8005){//����
			 
		 }else if(dataCmdFlag ==0x8006){//Զ�̿���

			 if(RES_DATA[31] == 0x02){//�������ݲ�ѯ	�¶ȡ�ʪ�ȡ��ơ����ȣ��������Ϣ�嶨�塿
					unsigned char  light_status = PUMP ? 0x02 : 0x01;
					unsigned char buzzy_status = Buzzer ? 0x02 : 0x01;
					unsigned char xdata ds[37] = {0};
					unsigned char dslen =37;
			  	unsigned char j=0;
				
				  ds[0] = 0X23;//����ͷ
					ds[1] = 0X23;
					ds[2] = 0X10;//�����ʶ  �·�0x8006  ���ڵ��ϴ���0x1006
					ds[3] = 0X06;
					
					if(dataCmdAck == 0xFE){//Ӧ���ʶ
						ds[4] = 0x01;//�ɹ�
						
					}
			
				 for(j=0;j<17;j++){//CID��ֵ
						ds[j+5] = SRCCID[j];
				 }
				ds[22] = 0X01;//������
				ds[23] = 0X00;//������λ ��λ00
				ds[24] = 0X0B;//��λ0B һ��11λ

				ds[25] = 0X14;//�� 0x14+2000 = 2020 
				ds[26] = 0X05;//�� 
				ds[27] = 0X18;//�� 
				ds[28] = 0X15;//ʱ 
				ds[29] = 0X24;//��
				ds[30] = 0X08;//��
				
				ds[31] = 0X02;//������ѯ   ����


					ds[32] = 22;//DATA_Temphui[0]; //��������4���ֽڵ�����
					ds[33] = 33;//DATA_Temphui[1];
					ds[34] = light_status;
					ds[35] = buzzy_status;
					
			
					
				 ds[dslen-1] = CheckBCC(dslen, ds);//����У���  �����һλ
					//	SendAckData(dslen,ds);
						
					if(S0_State == (S_INIT|S_CONN))
					{
						S0_Data&=~S_TRANSMITOK;
						memcpy(Tx_Buffer, ds, dslen);	
						Write_SOCK_Data_Buffer(0, Tx_Buffer, dslen);//ָ��Socket(0~7)�������ݴ���,�˿�0����20�ֽ�����
					}

				 
				 
			 }else if(RES_DATA[31] == 0x03){//��������	�ơ����ȣ��������Ϣ�嶨�塿
				 			 
					 unsigned char pump = RES_DATA[32];
					 unsigned char buzzy = RES_DATA[33];
			 
					 if( pump==0x02){
							Pump_Actions_Status(0);
						}else if( pump==0x01){
							Pump_Actions_Status(1);
						}
					 
					 if( buzzy==0x02){
							Buzzer_Actions_Status(0);
					 }else if( buzzy==0x01){
							Buzzer_Actions_Status(1);
					 }
					 
					 
					 
					 
					RES_DATA[0] = 0X23;
					RES_DATA[1] = 0X23;
					RES_DATA[2] = 0X10;
					RES_DATA[3] = 0X06;

					if(dataCmdAck == 0xFE){
						RES_DATA[4] = 0x01;//�ɹ�
					
					}
					if(dataCmdAck == 0xFE){//Ӧ���ʶ
						RES_DATA[4] = 0x01;//�ɹ�
						
					}
			
				 for(j=0;j<17;j++){//CID��ֵ
						RES_DATA[j+5] = SRCCID[j];
				 }
				RES_DATA[22] = 0X01;//������
				RES_DATA[23] = 0X00;//������λ ��λ00
				RES_DATA[24] = 0X09;//��λ09 һ��9λ    6λ��ʱ��+1λ�������ʶ + 2λ������

				RES_DATA[25] = 0X14;//�� 0x14+2000 = 2020 
				RES_DATA[26] = 0X05;//�� 
				RES_DATA[27] = 0X18;//�� 
				RES_DATA[28] = 0X15;//ʱ 
				RES_DATA[29] = 0X24;//��
				RES_DATA[30] = 0X08;//��
				
				RES_DATA[31] = 0X03;//��������  �ơ����ȣ��������Ϣ�嶨�塿
				
//				RES_DATA[32] = RES_DATA[32];// ����λ���øĶ�  
//				RES_DATA[33] = RES_DATA[33];
				
				
						RES_DATA[len-1] = CheckBCC(len, RES_DATA);//��һ֡���� 35���ֽ� len=35
					//	SendAckData(len,RES_DATA);
						
									if(S0_State == (S_INIT|S_CONN))
					{
						S0_Data&=~S_TRANSMITOK;
						memcpy(Tx_Buffer, RES_DATA, len);	
						Write_SOCK_Data_Buffer(0, Tx_Buffer, len);//ָ��Socket(0~7)�������ݴ���,�˿�0����20�ֽ�����
					}

			 
			 }else if(RES_DATA[31] == 0x7F){//����
				 	IAP_CONTR = 0X20;
			 }
			 
			 
			 
		 }
		 
		
	}
	
}




void Pump_Actions_Status(unsigned char status){

	if(status){
		PUMP = 0;
	}else{
		PUMP = 1;
	}

}


void Buzzer_Actions_Status(unsigned char status){

	if(status){
		Buzzer = 0;
	}else{
		Buzzer = 1;
	}

}

/***********************
����������������
��ڲ�������
����ֵ����
************************/
int main(void)
{
	Buzzer = 0;

	/////////////////////////////////////////////////
//ע��: STC15W4K32S4ϵ�е�оƬ,�ϵ��������PWM��ص�IO�ھ�Ϊ
//      ����̬,�轫��Щ������Ϊ׼˫��ڻ�ǿ����ģʽ��������ʹ��
//���IO: P0.6/P0.7/P1.6/P1.7/P2.1/P2.2
//        P2.3/P2.7/P3.7/P4.2/P4.4/P4.5
/////////////////////////////////////////////////
	P0M1 = 0;	P0M0 = 0;	//����P0.0~P0.7Ϊ׼˫���
	P1M1 = 0;	P1M0 = 0;	//����P1.0~P1.7Ϊ׼˫���
	P2M1 = 0;	P2M0 = 0xff;	//����P2.0~P2.7Ϊ�������
	P3M1 = 0;	P3M0 = 0;	//����P3.0~P3.7Ϊ׼˫���
	P4M1 = 0;	P4M0 = 0;	//����P4.0~P4.7Ϊ׼˫���
	P5M1 = 0;	P5M0 = 0;	//����P5.0~P5.7Ϊ׼˫���
	
	
	Load_Net_Parameters();		//װ���������	
	W5500_Hardware_Reset();		//Ӳ����λW5500
	W5500_Initialization();		//W5500��ʼ������
	Timer4Init();
	
	  WDT_CONTR = 0x06;       //���Ź���ʱ�����ʱ����㹫ʽ: (12 * 32768 * PS) / FOSC (��)
                            //���ÿ��Ź���ʱ����Ƶ��Ϊ32,���ʱ������:
                            //11.0592M : 1.14s
                            //18.432M  : 0.68s
                            //20M      : 0.63s
    WDT_CONTR |= 0x20;      //�������Ź�  STC��Ƭ���Ŀ��Ź�һ�������󣬾�û���ر�

    while(1) {
			
			WDT_CONTR |= 0x10;  //ι������
			
		W5500_Socket_Set();//W5500�˿ڳ�ʼ������
		
		W5500_Interrupt_Process();//W5500�жϴ��������

		if((S0_Data & S_RECEIVE) == S_RECEIVE)//���Socket0���յ�����
		{
			S0_Data&=~S_RECEIVE;
			Process_Socket_Data(0);//W5500���ղ����ͽ��յ�������
		}
	}
}

/**************************************
������������ʱ����
��ڲ�����unsigned int x ����ֵΪ1ʱ����ʱ1ms
����ֵ����
***************************************/
void delay_ms(unsigned int x) 
{  
	unsigned int j,i;   
	for(j=0;j<x;j++)   
	{    
		for(i=0;i<1100;i++);   
	}  
}


