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
** Descriptions:		IKMSIK_W5500模块－客户端模式实验(模拟SPI)			
**--------------------------------------------------------------------------------------------------------
** Modified by:			
** Modified date:		
** Version:				
** Descriptions:		
** Rechecked by:			
**********************************************************************************************************/
/****-----请参考IKMSIK_W5500客户端模式例程(模拟SPI)PDF文档进行实验------***********/

#include "W5500.h"				
#include <string.h>
typedef char I8;
typedef int I16;
typedef long I32;
typedef unsigned char U8; 

sbit PUMP = P3 ^ 6;  // LED
//sbit PUMP = P2 ^ 3;  // 电泵PUMP
sbit Buzzer    =  P5 ^ 4;           	// 蜂鸣器

static	 unsigned int   Timer4_Count=1;
unsigned char xdata SRCCID[] = {"SRC00000000000002"};

void delay_ms(unsigned int x) ;			//延时函数(ms)
void Timer4Init();
unsigned char CheckBCC(unsigned char len, unsigned char *recv);
void ResponseData(unsigned char len,unsigned char *RES_DATA);
void Buzzer_Actions_Status(unsigned char status);
void Pump_Actions_Status(unsigned char status);//开关水泵


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
	//50 毫秒@11.0592MHz
	T4T3M &= 0xDF;		//定时器时钟12T模式
	T4L = 0x00;		//设置定时初值
	T4H = 0x4C;		//设置定时初值
	T4T3M |= 0x80;		//定时器4开始计时
	
		IE2 |= 0x40;		//开定时器4中断
		EA=1; 	//总中断开启
}


//10s中断自动上报信息
void Timer4_interrupt() interrupt 20    //定时中断入口
{
	
		if(Timer4_Count>=200){  //200 * 50ms = 10s
			  	unsigned char j=0;
					U8 xdata RES_DATA[37]= {0};
          unsigned char RES_LEN= 37;
					unsigned char  light_status = PUMP ? 0x02 : 0x01;
					unsigned char buzzy_status = Buzzer ? 0x02 : 0x01;
					Timer4_Count = 0;

				  RES_DATA[0] = 0X23;//数据头
					RES_DATA[1] = 0X23;
					RES_DATA[2] = 0X10;//命令标识  下发0x8006  对于的上传是0x1006
					RES_DATA[3] = 0X06;
					RES_DATA[4] = 0xFE;//应答标识
						
				 for(j=0;j<17;j++){//CID赋值
						RES_DATA[j+5] = SRCCID[j];
				 }
				 
				RES_DATA[22] = 0X01;//不加密
				RES_DATA[23] = 0X00;//长度两位 高位00
				RES_DATA[24] = 0X0B;//低位0B 一共11位

				RES_DATA[25] = 0X14;//年 0x14+2000 = 2020 
				RES_DATA[26] = 0X05;//月 
				RES_DATA[27] = 0X18;//日 
				RES_DATA[28] = 0X15;//时 
				RES_DATA[29] = 0X24;//分
				RES_DATA[30] = 0X08;//秒
				
				RES_DATA[31] = 0X02;//基础数据上报


			RES_DATA[32] = 12;//DATA_Temphui[0];
			RES_DATA[33] = 	14;//DATA_Temphui[1];
			RES_DATA[34] = light_status;
			RES_DATA[35] = buzzy_status,
			RES_DATA[RES_LEN-1] = CheckBCC(RES_LEN, RES_DATA);
			
			if(S0_State == (S_INIT|S_CONN))
			{
				S0_Data&=~S_TRANSMITOK;
				memcpy(Tx_Buffer, RES_DATA, 37);	
				Write_SOCK_Data_Buffer(0, Tx_Buffer, 37);//指定Socket(0~7)发送数据处理,端口0发送20字节数据
			}
					
		}else{
			
		    Timer4_Count++;
		}
		
}



/*******************************************************************************
* 函数名  : W5500_Initialization
* 描述    : W5500初始货配置
* 输入    : 无
* 输出    : 无
* 返回值  : 无
* 说明    : 无
*******************************************************************************/
void W5500_Initialization(void)
{
	W5500_Init();		//初始化W5500寄存器函数
	Detect_Gateway();	//检查网关服务器 
	Socket_Init(0);		//指定Socket(0~7)初始化,初始化端口0
}

/*******************************************************************************
* 函数名  : Load_Net_Parameters
* 描述    : 装载网络参数
* 输入    : 无
* 输出    : 无
* 返回值  : 无
* 说明    : 网关、掩码、物理地址、本机IP地址、端口号、目的IP地址、目的端口号、端口工作模式
*******************************************************************************/
void Load_Net_Parameters(void)
{
	Gateway_IP[0] = 192;//加载网关参数
	Gateway_IP[1] = 168;
	Gateway_IP[2] = 0;
	Gateway_IP[3] = 1;

	Sub_Mask[0]=255;//加载子网掩码
	Sub_Mask[1]=255;
	Sub_Mask[2]=255;
	Sub_Mask[3]=0;

	Phy_Addr[0]=0x0c;//加载物理地址
	Phy_Addr[1]=0x29;
	Phy_Addr[2]=0xab;
	Phy_Addr[3]=0x7c;
	Phy_Addr[4]=0x00;
	Phy_Addr[5]=0x01;
	
	IP_Addr[0]=192;     //加载本机IP地址
	IP_Addr[1]=168;
	IP_Addr[2]=0;
	IP_Addr[3]=101;

	S0_Port[0] = 0x13;  //加载端口0的端口号5000 
	S0_Port[1] = 0x88;

	S0_DIP[0]=47;      //加载端口0的目的IP地址 47.104.19.111
	S0_DIP[1]=104;
	S0_DIP[2]=19;
	S0_DIP[3]=111;
	
	S0_DPort[0] = 0x0F; //加载端口0的目的端口号4001
	S0_DPort[1] = 0xA1;

	S0_Mode=TCP_CLIENT; //加载端口0的工作模式,TCP客户端模式
}

/*******************************************************************************
* 函数名  : W5500_Socket_Set
* 描述    : W5500端口初始化配置
* 输入    : 无
* 输出    : 无
* 返回值  : 无
* 说明    : 分别设置4个端口,根据端口工作模式,将端口置于TCP服务器、TCP客户端或UDP模式.
*			从端口状态字节Socket_State可以判断端口的工作情况
*******************************************************************************/
void W5500_Socket_Set(void)
{
	if(S0_State==0)//端口0初始化配置
	{
		if(S0_Mode==TCP_SERVER)//TCP服务器模式 
		{
			if(Socket_Listen(0)==TRUE)
				S0_State=S_INIT;
			else
				S0_State=0;
		}
		else if(S0_Mode==TCP_CLIENT)   //TCP客户端模式 
		{
			if(Socket_Connect(0)==TRUE)
				S0_State=S_INIT;
			else
				S0_State=0;
		}
		else          //UDP模式 
		{
			if(Socket_UDP(0)==TRUE)
				S0_State=S_INIT|S_CONN;
			else
				S0_State=0;
		}
	}
}

/*******************************************************************************
* 函数名  : Process_Socket_Data
* 描述    : W5500接收并发送接收到的数据
* 输入    : s:端口号
* 输出    : 无
* 返回值  : 无
* 说明    : 本过程先调用S_rx_process()从W5500的端口接收数据缓冲区读取数据,
*			然后将读取的数据从Rx_Buffer拷贝到Temp_Buffer缓冲区进行处理。
*			处理完毕，将数据从Temp_Buffer拷贝到Tx_Buffer缓冲区。调用S_tx_process()
*			发送数据。
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


//校验和
	if(CheckBCC(len, RES_DATA) == RES_DATA[len-1]){
	
		 unsigned int dataCmdFlag =(RES_DATA[2] << 8) | RES_DATA[3];         //命令标识
		 unsigned char dataCmdAck = RES_DATA[4];          //应答标识
		 unsigned char j=0;
		 unsigned char dataEncryptFlag = RES_DATA[22];    //加密方式
		 unsigned char dataUintLength = (RES_DATA[23] << 8) | RES_DATA[24];  //数据长度
		 unsigned char xdata  dataTimestamp[6] = {0x00,0x00,0x00,0x00,0x00,0x00};  //时间数据

	 //校验CID是否正确
		 for(j=5;j<22;j++){
			  if(SRCCID[j-5] != RES_DATA[j]){
				 return;
			 }
		 }
		
		 //校验长度是否正确
		 if ((26 + dataUintLength) != len) {
				return ;
		 }
		 
		 
		 //保存时间
		 for(j=0;j<6;j++){
			 dataTimestamp[j] = RES_DATA[25+j];
		 }
		 
		 if(dataCmdFlag == 0x8001){//连接认证
			 
		 }else if(dataCmdFlag ==0x8002){//实时信息主动上报
			 
		 }else if(dataCmdFlag ==0x8003){//补发
			 
		 }else if(dataCmdFlag ==0x8004){//设备登出
			 
		 }else if(dataCmdFlag ==0x8005){//心跳
			 
		 }else if(dataCmdFlag ==0x8006){//远程控制

			 if(RES_DATA[31] == 0x02){//基础数据查询	温度、湿度、灯、喇叭；请见【信息体定义】
					unsigned char  light_status = PUMP ? 0x02 : 0x01;
					unsigned char buzzy_status = Buzzer ? 0x02 : 0x01;
					unsigned char xdata ds[37] = {0};
					unsigned char dslen =37;
			  	unsigned char j=0;
				
				  ds[0] = 0X23;//数据头
					ds[1] = 0X23;
					ds[2] = 0X10;//命令标识  下发0x8006  对于的上传是0x1006
					ds[3] = 0X06;
					
					if(dataCmdAck == 0xFE){//应答标识
						ds[4] = 0x01;//成功
						
					}
			
				 for(j=0;j<17;j++){//CID赋值
						ds[j+5] = SRCCID[j];
				 }
				ds[22] = 0X01;//不加密
				ds[23] = 0X00;//长度两位 高位00
				ds[24] = 0X0B;//低位0B 一共11位

				ds[25] = 0X14;//年 0x14+2000 = 2020 
				ds[26] = 0X05;//月 
				ds[27] = 0X18;//日 
				ds[28] = 0X15;//时 
				ds[29] = 0X24;//分
				ds[30] = 0X08;//秒
				
				ds[31] = 0X02;//基础查询   编码


					ds[32] = 22;//DATA_Temphui[0]; //基础数据4个字节的数据
					ds[33] = 33;//DATA_Temphui[1];
					ds[34] = light_status;
					ds[35] = buzzy_status;
					
			
					
				 ds[dslen-1] = CheckBCC(dslen, ds);//计算校验和  放最后一位
					//	SendAckData(dslen,ds);
						
					if(S0_State == (S_INIT|S_CONN))
					{
						S0_Data&=~S_TRANSMITOK;
						memcpy(Tx_Buffer, ds, dslen);	
						Write_SOCK_Data_Buffer(0, Tx_Buffer, dslen);//指定Socket(0~7)发送数据处理,端口0发送20字节数据
					}

				 
				 
			 }else if(RES_DATA[31] == 0x03){//基础控制	灯、喇叭；请见【信息体定义】
				 			 
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
						RES_DATA[4] = 0x01;//成功
					
					}
					if(dataCmdAck == 0xFE){//应答标识
						RES_DATA[4] = 0x01;//成功
						
					}
			
				 for(j=0;j<17;j++){//CID赋值
						RES_DATA[j+5] = SRCCID[j];
				 }
				RES_DATA[22] = 0X01;//不加密
				RES_DATA[23] = 0X00;//长度两位 高位00
				RES_DATA[24] = 0X09;//低位09 一共9位    6位的时间+1位的命令标识 + 2位的数据

				RES_DATA[25] = 0X14;//年 0x14+2000 = 2020 
				RES_DATA[26] = 0X05;//月 
				RES_DATA[27] = 0X18;//日 
				RES_DATA[28] = 0X15;//时 
				RES_DATA[29] = 0X24;//分
				RES_DATA[30] = 0X08;//秒
				
				RES_DATA[31] = 0X03;//基础控制  灯、喇叭；请见【信息体定义】
				
//				RES_DATA[32] = RES_DATA[32];// 这两位不用改动  
//				RES_DATA[33] = RES_DATA[33];
				
				
						RES_DATA[len-1] = CheckBCC(len, RES_DATA);//这一帧数据 35个字节 len=35
					//	SendAckData(len,RES_DATA);
						
									if(S0_State == (S_INIT|S_CONN))
					{
						S0_Data&=~S_TRANSMITOK;
						memcpy(Tx_Buffer, RES_DATA, len);	
						Write_SOCK_Data_Buffer(0, Tx_Buffer, len);//指定Socket(0~7)发送数据处理,端口0发送20字节数据
					}

			 
			 }else if(RES_DATA[31] == 0x7F){//重启
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
功能描述：主函数
入口参数：无
返回值：无
************************/
int main(void)
{
	Buzzer = 0;

	/////////////////////////////////////////////////
//注意: STC15W4K32S4系列的芯片,上电后所有与PWM相关的IO口均为
//      高阻态,需将这些口设置为准双向口或强推挽模式方可正常使用
//相关IO: P0.6/P0.7/P1.6/P1.7/P2.1/P2.2
//        P2.3/P2.7/P3.7/P4.2/P4.4/P4.5
/////////////////////////////////////////////////
	P0M1 = 0;	P0M0 = 0;	//设置P0.0~P0.7为准双向口
	P1M1 = 0;	P1M0 = 0;	//设置P1.0~P1.7为准双向口
	P2M1 = 0;	P2M0 = 0xff;	//设置P2.0~P2.7为推挽输出
	P3M1 = 0;	P3M0 = 0;	//设置P3.0~P3.7为准双向口
	P4M1 = 0;	P4M0 = 0;	//设置P4.0~P4.7为准双向口
	P5M1 = 0;	P5M0 = 0;	//设置P5.0~P5.7为准双向口
	
	
	Load_Net_Parameters();		//装载网络参数	
	W5500_Hardware_Reset();		//硬件复位W5500
	W5500_Initialization();		//W5500初始货配置
	Timer4Init();
	
	  WDT_CONTR = 0x06;       //看门狗定时器溢出时间计算公式: (12 * 32768 * PS) / FOSC (秒)
                            //设置看门狗定时器分频数为32,溢出时间如下:
                            //11.0592M : 1.14s
                            //18.432M  : 0.68s
                            //20M      : 0.63s
    WDT_CONTR |= 0x20;      //启动看门狗  STC单片机的看门狗一旦启动后，就没法关闭

    while(1) {
			
			WDT_CONTR |= 0x10;  //喂狗程序
			
		W5500_Socket_Set();//W5500端口初始化配置
		
		W5500_Interrupt_Process();//W5500中断处理程序框架

		if((S0_Data & S_RECEIVE) == S_RECEIVE)//如果Socket0接收到数据
		{
			S0_Data&=~S_RECEIVE;
			Process_Socket_Data(0);//W5500接收并发送接收到的数据
		}
	}
}

/**************************************
功能描述：延时函数
入口参数：unsigned int x ，该值为1时，延时1ms
返回值：无
***************************************/
void delay_ms(unsigned int x) 
{  
	unsigned int j,i;   
	for(j=0;j<x;j++)   
	{    
		for(i=0;i<1100;i++);   
	}  
}


