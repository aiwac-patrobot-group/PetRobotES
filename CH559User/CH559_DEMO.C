/**
 * Author:Tang Yuan
 * Date:2019.3.21
 * Description:宠物机器人，加入超声波模块,喂食功能
*/
 
#include "DEBUG.C"
#include "DEBUG.H"
#include <string.h>
#include <stdio.h>
#include <intrins.h>    
#include "stdarg.h"

#define uchar unsigned  char
#define uint  unsigned   int 
#define Maxtimer2HalfSencondNum 1000


// 运动模块
// 运动马达1		左边的
sbit motor_1_a = P1^2;
sbit motor_1_b = P1^3;
// 运动马达2		右边的
sbit motor_2_a = P1^1;
sbit motor_2_b = P1^0;


// 注意: 灯 1、2   乱写的引脚   记得改
sbit led_1 = P0^1;   //j9
sbit led_2 = P0^2;	 //j10


// 超声波模块
bit startUDetectionFlag = 0; // 超声波探测开关

//模块的数据脚确定
// 超声波模块1，前面的
sbit  Echo_1 = P0^3;   		// Echo
sbit  Trig_1 = P0^2;		// Trig
// 超声波模块2，后面的
sbit  Echo_2 = P2^4;   		// Echo
sbit  Trig_2 = P2^5;		// Trig
// 超声波模块3，左边的
sbit  Echo_3 = P0^0;   		// Echo
sbit  Trig_3 = P0^1;		// Trig
// 超声波模块4，右边的
sbit  Echo_4 = P1^6;   		// Echo
sbit  Trig_4 = P1^7;		// Trig

UINT8  outDetection = 0;	// 远距离次数
uint  time_ultar = 0;		// 一次超声波探测时间   
UINT8 checkTime = 5;   		// 每次检测5次
UINT8 checkRightTime = 5;   // 有效检测次数
uint distanceSum =0;   		// 一次探测的距离总和
uint distanceCount = 0;


// 定时器2
//用单一模块自己用变量存时间，再与这个定时器的变量比较，差值即为经过的时间（归零需注意）
//用单一模块自己用变量存时间，再与这个定时器的变量比较，差值即为经过的时间（归零需注意）
UINT8 timer2IRSNum = 0;  //一次中断触发就++，满10归零,表0.5S到了
uint timer2HalfSencondNum = 0; //0.5秒就记一次，240个0.5S归零
bit timer2IncreaseNum= 0;  //  timer2HalfSencondNum 每次增长就置1
bit oneSecondeFlag = 0;
bit oneSecondeUDetectionFlag = 0;  // 超声波需要用的时间标签
uint timer2HalfSencondNumForSport = 0; //运动模块用来比较收到命令经过的时间


UINT8 DAT = 1;
bit getDataFlag = 0 ;		//收到合适的数据
UINT8 timeSport = 0 ;		// 一次运动的时间计数																													// 保护时间计数
UINT8 order[10];			// 收到的数据																													// 收到的命令
UINT8 bufNum = 0;		


/////////////////////////////////
//注意:需要重新赋值引脚

// 投食 
// 收到投食的命令	,0:无需投食操作，1： 开始投食，2： 结束投食
UINT8 needFeed = 0 ;		
// 扫食马达														
sbit motor_SweepFood_a = P2^3;
sbit motor_SweepFood_b = P2^2;
// 推出食槽马达		
sbit motor_pushFood_a = P2^0;
sbit motor_pushFood_b = P2^1;

sbit needStopPush = P0^4;
sbit needStopPull = P0^5;

																												// 当前命令的字节数
// 运动方式记录变量
UINT8 sportClass = 'i';  //  真正会判断的指令
UINT8 newSportClass = 'i';  // 获取新下发的指令，在执行的时候会跟sportClass比较再赋值给sportClass																												// 运动 的类型
																										// 记录前一次 的运动类型，防止快速转换电平down机

// 自定义打印函数 变量
bit printfBusy;  // 发送状态位
unsigned char idata Put_buf[70];		// 打印的最大字符


/*******************************************************************************
* Function Name  : void TIM2Inital(void)
* Description    : timer2  定时开启，每50ms 中断
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void TIM2Inital(void)
{
	RCAP2H = (65536-50000)/256;//晶振12M 50ms 16bit 自动重载
	RCAP2L = (65536-50000)%256;
	ET2=1;                     //打开定时器中断
	EA=1;                      //打开总中断
	TR2=1;                     //打开定时器开关
}

void TIM2(void) interrupt INT_NO_TMR2 //定时器2中断
{
	timer2IRSNum++;
	if (timer2IRSNum == 10)// 经过0.5秒
	{
		timer2IRSNum =0; // 0.5秒归零
		timer2HalfSencondNum++;
		timer2IncreaseNum = 1;
		
		
		
		//查看是否到 1s 
		if (((timer2HalfSencondNum %2) == 0))
		{
			oneSecondeFlag = 1;
			oneSecondeUDetectionFlag = 1;
		}
	}
	
	

	
	
	if (timer2HalfSencondNum == Maxtimer2HalfSencondNum)// 500s归零
	{
		timer2HalfSencondNum = 0;
	}
	
	
	
	TF2=0;
}



/*******************************************************************************
* Function Name  : my_printf(char* fmt,...) 
* Description    : 
*					自己的打印函数，可以用于串口数据上传到A33,外面自己定义通信协议
*					之所以自定义打印函数，因为printf 和  SBUF 连用不太方便，打印有问题
* Input          : 跟printf使用差不多的，注意 换行的"\n"  需要换成  "\r\n"
* Output         : None
* Return         :
*******************************************************************************/
void my_printf(char* fmt,...)
{
	unsigned char i,len;
	va_list ap;
	va_start(ap,fmt);
	len=vsprintf((char*)Put_buf,fmt,ap);
	va_end(ap);
	
	for(i=0;i<len;i++)			//把缓存内的字符发送出去
	{	
		while(printfBusy);             	//等待前面的数据发送完成
		printfBusy=1;	
		SBUF=Put_buf[i];		//发送一个字节
	}
	while(printfBusy);
	printfBusy=1;
	SBUF = '\r';
	while(printfBusy);
	printfBusy=1;
	SBUF = '\n';
	
	memset(Put_buf,0,sizeof(Put_buf));	//清空缓存
}


/*******************************************************************************
* Function Name  : delayus 
* Description    : us  延时
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void delayus(unsigned int us)
{
	for(;us;us--)
	{
		_nop_();
	}
}

/*******************************************************************************
* Function Name  : delayms 
* Description    : ms  延时
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void delayms(unsigned int ms)
{
	unsigned char i=100,j;
	for(;ms;ms--)
	{
		while(--i)
		{
			j=10;
			while(--j);
		}
	}
}




/*******************************************************************************
* Function Name  : startFeedPet 
* Description    : 进行喂食，把食槽推出来
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void startFeedPet(void)
{
	if (needFeed == 1)  // 有喂食的命令
	{
		
		// 保证停止运动
		motor_1_a = 0;
		motor_1_b = 0;
		motor_2_a = 0;
		motor_2_b = 0;	
		delayms(20);
		
//////////////////////////////////////////
// 注意：马达的 0 1 赋值可能反了，记得调下
//////////////////////////////////////////
		if (needStopPush != 0 )  // 不为0，还可以推
		{
			// 先把食物扫进食槽
			motor_SweepFood_a = 1;
			motor_SweepFood_b = 0;
			
			my_printf("Sweep  Start = 1\r\n");
			delayms(5000);   //  决定把多少食物扫进食槽 貌似时间是12秒
			my_printf("Sweep  Stop = 1\r\n");
			
			motor_SweepFood_a = 0;
			motor_SweepFood_b = 0;
			
			delayms(20);
			
		
			// 把食槽推出
			motor_pushFood_a = 1;
			motor_pushFood_b = 0;
			//delayms(5000);
			
			needStopPush = 1;
			while (needStopPush)
			{
				delayms(10);
				my_printf("needStopPush = 1\r\n");
			}
			
			motor_pushFood_a = 0;
			motor_pushFood_b = 0;
		}
		

		needFeed = 0;
	}
}

/*******************************************************************************
* Function Name  : stopFeedPet 
* Description    : 结束喂食，把食槽拉回来
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void stopFeedPet(void)
{
	if (needFeed == 2)  // 有停止喂食的命令
	{
		// 保证停止运动
		motor_1_a = 0;
		motor_1_b = 0;
		motor_2_a = 0;
		motor_2_b = 0;
				
//////////////////////////////////////////
// 注意：马达的 0 1 赋值可能反了，记得调下
//////////////////////////////////////////
		
		
		
		if (needStopPull != 0)  // 不为0 ，说明还可以拉
		{
			// 把食槽收回
			motor_pushFood_a = 0;
			motor_pushFood_b = 1;
			
			needStopPull = 1;
			while (needStopPull)  // 当拉回食槽的结束位置的时候会  needStopPull == 0
			{ 
				delayms(10);
				my_printf("needStopPull = 1\r\n");
			}
			
			motor_pushFood_a = 0;
			motor_pushFood_b = 0;
		}


		needFeed = 0;
	}
}




/*******************************************************************************
* Function Name  : void conutDetection(void) 
* Description    : 计算检测到的距离
* Input          : None
* Output         : None
* Return         : S 探测到的距离，如果太远就是设置800CM 
*******************************************************************************/
uint conutDetection(void)
{
	uint S=0;
	if(outDetection !=0 )		    //超出测量  
	{
		outDetection=0;
		//my_printf("Y\r\n");
	  
		return 800;
	}
		
	time_ultar = 0;
	// 计算差值
	time_ultar=(TH0-0xbb)*256+(TL0 - 0x11);  // 一个计数，1/10M 秒   1us,让它溢出，得65ms

	S= (uint)(time_ultar*1.87)/100;     //算出来是CM

	return S;
}




/*******************************************************************************
* Function Name  : mTimer0Interrupt 
* Description    : 超过测距范围
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void mTimer0Interrupt( void ) interrupt INT_NO_TMR0 		 //T0中断用来计数器溢出,超过测距范围
{
	outDetection = outDetection + 1 ;		//中断溢出标志
}
	
	
	
/*******************************************************************************
* Function Name  : StartUltrasoundDetection() 
* Description    : 超声波模块启动信号
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void  StartUltrasoundDetection() 		         
{
	// 发起超声波探测
	 // my_printf(" StartUltrasoundDetection\r\n");
	Trig_1=1;			               
	delayus(20);
	Trig_1=0;
	
	TH0=0xbb;			// 定时器清零,只打算探测4米以内的距离,只累计值 17647  个时钟
	TL0=0x11; 
}



/*******************************************************************************
* Function Name  : UltrasonicInit() 
* Description    : 超声波模块时钟0设置，允许T0中断等
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void  UltrasonicInit() 		         
{
		TMOD |=0x01;  //定时器0工作方式1
		ET0=1;
		PT0 = 1;     // 定时器0 优先中断
		
		Trig_1 = 0;
		TR0=0;		 // 关闭计数器0
	
}


/*******************************************************************************
* Function Name  : CH559UART0InterruptInit()
* Description    : CH559UART0中断初始化
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void CH559UART0InterruptInit()
{
		
	SM0 = 0;
	SM1 = 1; 																	//串口0使用模式1

	SM2 = 0;  	
	TMOD = 0x20;               													//使用Timer1作为波特率发生器
																				// 

	PCON |= SMOD;
	T2MOD |= 0xa0;																

	TH1 = 0xb2; 																// 波特率 9600
	TR1 = 1;                                                                 	//启动定时器1

	REN = 1; 
	ES = 1;                                                                   	//开启UART0中断
	EA = 1;                                                                    	//总中断开启
	TI = 1;																		// 发送中断标志位，发送完硬件置1
	PS =1;																		// 尝试提高串口优先级
}




/*******************************************************************************
* Function Name  : sendState()
* Description    : 发送当前机器人情况 
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void sendState()
{
	while(1);	
	
}

/*******************************************************************************
* Function Name  : executeOrder()
* Description    : 执行命令  
* Input          : 
*					command1: 命令类型
*					command2: 具体命令操作
* Output         : None
* Return         : None
*******************************************************************************/
void executeOrder( UINT8 command1, UINT8 command2)
{	
	//my_printf("d2：%bu ,d2：%bu\r\n",command1,command2);
	
	
													  
	if ( command1 == '1') // 运动相关的命令
	{
		switch (command2)
		{
			//my_printf("enter command2：%bu\r\n",command2);
			// a:前 b:后  c:左 d:右 e:前 + 右  g：后 + 右  h：后 + 左  i：停	
			case 'a':
			case 'b':
			case 'c':
			case 'd':
			case 'e':
			case 'f':
			case 'g':
			case 'h':
			case 'i':
			
				newSportClass  = command2;
				timer2HalfSencondNumForSport = timer2HalfSencondNum;
				timeSport = 0;
				break;
			default:
				newSportClass  = 'i';
				
				break;			
		}		
		
	}else if (command1 == '2')  // 超声波相关的命令
	{
		switch (command2)
		{
			case 'a':	  // 开启超声波探测	
				startUDetectionFlag = 1;
				break;
			case 'b':	 // 关闭超声波探测
				startUDetectionFlag = 0;
				break;	
			default:
				break;			
		}	
		
		
	}else if (command1 == '4')  // 投食相关的命令
	{
		switch (command2)
		{
			case 'a':	  // 进行投食	
				newSportClass  = 'i';  
				needFeed = 1;   // 为啦安全 ，确保停止
				break;
			case 'b':	 // 结束投食
				needFeed = 2;			
				newSportClass  = 'i'; // 为啦安全 ，确保停止
				break;	
			default:
				newSportClass  = 'i'; // 为啦安全 ，确保停止
				break;			
		}	
		
	}
		// case 'j':
			// led_1  = 1;
			// break;
		// case 'k':
			// led_1  = 0;
			// break;
		// case 'l':
			// led_2  = 1;
			// break;
		// case 'm':
			// led_2  = 0;
			// break;
		// case 'o':
			// sendState();
			// break;



}


/*******************************************************************************
* Function Name  : CH559UART0Interrupt()
* Description    : CH559UART0中断处理函数，判断一条命令接收情况
*******************************************************************************/
 void CH559UART0Interrupt( )  interrupt INT_NO_UART0  using 1                   //看门狗中断服务程序,使用寄存器组1
 {
    if(TI)
    {
		TI = 0;                                                                // 清空发送中断                        
		printfBusy = 0;
    }
		
    if(RI)
    {      		
		DAT = SBUF;
		//SBUF = DAT;	

  

		if (1)
		{
						//  监测到开头为# 才进行接收和解析
			if ( DAT == '#')
			{
				getDataFlag = 1;
				bufNum = 0;
			}
			
			if ( getDataFlag == 1)  // 收到协议开头  '#‘,进行字符缓存
			{
				order[bufNum] = DAT;	
				// SBUF = 		order[bufNum];
				// SBUF = bufNum;     
			}
			
			if (  bufNum == 3 )	 	//  收到4 字节  ,判断是不是一条命令,
			{
				//my_printf("order:%bu%bu%bu%bu  \r\n",order[0],order[1], order[2],order[3]);
				if ( order[bufNum] == '.' )  //"."表一个命令的结束
				{
					// 满足协议要求的字节流，如"#1a."
					if ( order[bufNum-3] == '#')
					{
						executeOrder(order[bufNum-2], order[bufNum-1]);  					// 使用'.'的前两位位，为命令	 如  1a				
					}				
				}
		
				// 收到4个字节后，无论是不是命令，都该清除标志
				bufNum = 0;
				
				// order[0] = '0';
				// order[1] = '0';
				// order[2] = '0';
				// order[3] = '0';
				memset(order,0,sizeof(order));	//清空缓存
				getDataFlag = 0;
				DAT = '/';
			}
			else{
				bufNum++;  // 下一个字符放额位置	
			}
						
		}
		
		RI = 0;																																// 清空接收中断
     }
		
 }

 
 /*******************************************************************************
* Function Name  :initMotorGPIO(void)
* Description    : 初始化  驱动马达 所需的IO口
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void  initMotorGPIO(void)
{
	// p1.0、1		INM_2       右边的
	// p1.2、3		INB_1		左边的
	P1_DIR |= (0x0f);
	
	// p2.0、1		INB_3		推出食槽马达
	// p2.2、3		INB_4		扫食马达
	P2_DIR |= (0x0f);
	
	
}

/*******************************************************************************
* Function Name  :conductSport(void)
* Description    : 执行运动功能
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void conductSport(void)
{
	
	//  判断未收到命令超2s没，推荐APP 1.5s下发一次数据
	TR2=0; 
	if (timer2HalfSencondNum > timer2HalfSencondNumForSport)  // 两数未超圈，即在当前 timer2HalfSencondNum  未归零
	{
		if ((timer2HalfSencondNum - timer2HalfSencondNumForSport) >3 )  //  未收到命令2s
		{
			my_printf("sportClass = 'i';\r\n");
			newSportClass = 'i';
			
			timer2HalfSencondNumForSport = timer2HalfSencondNum;
		}
	}
	if (timer2HalfSencondNum < timer2HalfSencondNumForSport)  //在这2s内 timer2HalfSencondNum  归零了
	{
		if ((timer2HalfSencondNum+Maxtimer2HalfSencondNum - timer2HalfSencondNumForSport) >3 )  //  未收到命令2s
		{
			my_printf("sportClass = 'i'   go  to 0;\r\n");
			newSportClass = 'i';
			timer2HalfSencondNumForSport = timer2HalfSencondNum;
			
		}
	}
	TR2=1; 
	
	
	if (newSportClass == sportClass) // 重复的指令，直接继续上一次的运动
	{
		mDelaymS(200);		// 等0.2秒
		return;
	}
	else{
		sportClass = newSportClass;  // 更新当前的运动操作
		
		
		// 进入待命状态 缓缓
		motor_1_a = 0;
		motor_1_b = 0;
		motor_2_a = 0;
		motor_2_b = 0;
		mDelaymS(50);
	}
	
	if ( sportClass == 'i')
	{
		
		//  两轮待命
		motor_1_a = 0;
		motor_1_b = 0;
		motor_2_a = 0;
		motor_2_b = 0;

		
		mDelaymS(200);		// 等0.2秒
	}
	else if ( sportClass == 'a')
	{

		//  前进
		motor_1_a = 1;
		motor_1_b = 0;
		motor_2_a = 1;
		motor_2_b = 0;
		
		//my_printf("+\r\n");
		mDelaymS(200);		// 等0.2秒
		
	}
	else if ( sportClass == 'b')
	{			
		
		//  后退
		motor_1_a = 0;
		motor_1_b = 1;
		motor_2_a = 0;
		motor_2_b = 1;
		
		mDelaymS(200);		// 等0.2秒
		
	}
	else if ( sportClass == 'c')
	{
			
		//  左
		
		/*
		motor_1_a = 1;
		motor_1_b = 1;
		motor_2_a = 1;
		motor_2_b = 0;
		
		mDelaymS(200);		// 等0.2秒					
		*/
		
		
		// //  左  稍大的圈
		// //  前进
		// motor_1_a = 1;
		// motor_1_b = 0;
		// motor_2_a = 1;
		// motor_2_b = 0;
		
		// mDelaymS(50);		
		
		//  左
		motor_1_a = 0;
		motor_1_b = 0;
		motor_2_a = 1;
		motor_2_b = 0;
		
		mDelaymS(200);		
		
	}
	else if ( sportClass == 'd')
	{
		/*	
		//  右
		motor_1_a = 1;
		motor_1_b = 0;
		motor_2_a = 1;
		motor_2_b = 1;
		
		mDelaymS(200);		// 等0.2秒						
		*/
		
		// // 右大点的圈
		
		// //  前进
		// motor_1_a = 1;
		// motor_1_b = 0;
		// motor_2_a = 1;
		// motor_2_b = 0;
		
		// mDelaymS(50);		
		
		//  右
		motor_1_a = 1;
		motor_1_b = 0;
		motor_2_a = 0;
		motor_2_b = 0;
		
		mDelaymS(200);		
		
	}
	else if ( sportClass == 'e')
	{
			
		// 前 + 右
		
		//  前进
		motor_1_a = 1;
		motor_1_b = 0;
		motor_2_a = 1;
		motor_2_b = 0;
		
		mDelaymS(100);		
		
		//  右
		motor_1_a = 1;
		motor_1_b = 0;
		motor_2_a = 0;
		motor_2_b = 0;
		
		mDelaymS(100);		

		
	}
	else if ( sportClass == 'f')
	{
			
		// 前 + 左
		
		//  前进
		motor_1_a = 1;
		motor_1_b = 0;
		motor_2_a = 1;
		motor_2_b = 0;
		
		mDelaymS(100);		
		
		//  左
		motor_1_a = 0;
		motor_1_b = 0;
		motor_2_a = 1;
		motor_2_b = 0;
		
		mDelaymS(100);		

		
	}
	else if ( sportClass == 'g')
	{

		// 后 + 右
		
		//  后退
		motor_1_a = 0;
		motor_1_b = 1;
		motor_2_a = 0;
		motor_2_b = 1;
		
		mDelaymS(100);		
		
		//  右
		motor_1_a = 0;
		motor_1_b = 1;
		motor_2_a = 0;
		motor_2_b = 0;
		
		mDelaymS(100);		

		
	}
	else if ( sportClass == 'h')
	{

		// 后 + 左
		
		//  后退
		motor_1_a = 0;
		motor_1_b = 1;
		motor_2_a = 0;
		motor_2_b = 1;
		
		mDelaymS(100);		
		
		//  左
		motor_1_a = 0;
		motor_1_b = 0;
		motor_2_a = 0;
		motor_2_b = 1;
		
		mDelaymS(100);		
						
		
	}else 
	{
		//  两轮待命
		motor_1_a = 0;
		motor_1_b = 0;
		motor_2_a = 0;
		motor_2_b = 0;
		mDelaymS(200);		// 等0.2秒
		
		
	}


	
	
	// timeSport++;
		
	// // 一定时间未收到运动指令，自动停止，需要APP  1.5S以内发送一次运动数据
	// if (startUDetectionFlag == 0)  // 未进行超声波探测，差不多 1.6S
	// {
		// //  监测长时间无命令，停止  8*0.2 秒
		// if ( timeSport >= 8)  
		// {
			// timeSport = 0;
			// sportClass = 'i';
			// //my_printf("---------------------------------------\r\n");
			// my_printf("-\r\n");
			 // // SBUF = '1';
			 // // delayms(200);
			 // // SBUF = 0x0d;
			 // // delayms(200); 
			 // // SBUF = 0x0a;
			 
		// }
				 
	// }
	// else
	// {
		
		// //  监测长时间无命令，停止  7*0.25 秒      差不多1.7
		// if ( timeSport >= 7)
		// {
			// timeSport = 0;
			// sportClass = 'i';
			// //my_printf("---------------------------------------\r\n");
			// my_printf("-\r\n");
		// }
			
	// }

		
	
}


/*******************************************************************************
* Function Name  :initUltrasoundGOIO
* Description    : 初始化超声波模块需要的引脚    Echo赋值 0  Trig 赋值1
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void initUltrasoundGOIO( void )
{
	
	// 前
	// Echo 0   Echo_1 = P0^3;
	P0_DIR &= ~(0x08);
	// Trig 1     Trig_1 = P0^2
	P0_DIR |= (0x04);
	
	// 后
	// Echo 0   Echo_2 = P2^4;
	P2_DIR  &= ~(0x10);
	// Trig 1    Trig_2 = P2^5;
	P2_DIR |=(0x20);
	
	// 左
	// Echo 0   Echo_3 = P0^0;
	P0_DIR &= ~(0x01);
	// Trig 1   Trig_3 = P0^1;
	P0_DIR |= (0x02);
	
	// 右
	// Echo 0   Echo_4 = P1^6;
	P1_DIR &= ~(0x40);
	// Trig 1   Trig_4 = P1^7;
	P1_DIR |= (0x80);
}


/*******************************************************************************
* Function Name  : UltrasoundDetection_1()
* Description    : 
*					进入超声波  1号（前）模块  探测 ，距离在400cm以内需要上传情况。
*					超过300cm（定时器计时和超时的情况汇总后的情况） 定为无效的探测，不需要上报
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void UltrasoundDetection_1()
{
   
	checkTime = 3;  //  需要进行多次探测的次数
	checkRightTime = 0;  // 在checkTime中距离较近的探测次数
	distanceSum = 0;	// 有效探测的距离和
	distanceCount = 800;  // 默认距离较远
	
	while (checkTime > 0)
	{
		
		checkTime--;
		
		 //my_printf("a  checkTime:%d\r\n",checkTime);
		
		//my_printf("qqqqq\r\n");
  

		// 开始超声波探测
		
		TH0=0xbb;			// 定时器清零,只打算探测4米以内的距离,只累计值 17647  个时钟
		TL0=0x11; 
		outDetection = 0;
		
		Trig_1=1;			               
		delayus(20);
		Trig_1=0;
		
			
		while(!Echo_1);		//当Echo为 0 时等待
		TR0=1;			    //开启计数
		while(Echo_1)			//当Echo为1计数并等待   还应该设置超出距离，无法低电平的 情况
		{
			if(outDetection !=0 )		    //定时器超时，超出测量  ,跳过等待，节约时间
			{
				TR0=0;		//关闭计数
				break;
			}						
		}		
		TR0=0;				//关闭计数
		//my_printf("qqqqq\r\n");

		if(outDetection !=0 )	  // 超时直接跳过本次探测
		{
			//mDelaymS(10);
			outDetection = 0;
			continue;
		} 
		  
		 distanceCount =  conutDetection();			//计算
		 //my_printf("distanceCount :%d\r\n",distanceCount );
		if (distanceCount < 300)  // 本次探测距离较近，可记录;距离较远的情况包括1:超时 2： 不超时，但超过400cm 
		{
			checkRightTime++;
			//my_printf("checkRightTime:%bu  \r\n",checkRightTime);
			distanceSum = distanceSum + distanceCount;
		}

		mDelaymS(10);		//设短点，好加快一次总的操作循环

	}
	
	// 在安全距离外，不用打印
	if ( checkRightTime > 0 ) // 进行了效检测到距离，探测到障碍物较近
	{
		//my_printf("t:%bu  d:%d \r\n",checkRightTime,(distanceSum/checkRightTime));
		my_printf("1,1,%03d\r\n",(distanceSum/checkRightTime));  // 进行上行数据传输
	}
	////////////////////////

}


/*******************************************************************************
* Function Name  : UltrasoundDetection_2()
* Description    : 
*					进入超声波  2号（后）模块  探测 ，距离在400cm以内需要上传情况。
*					超过300cm（定时器计时和超时的情况汇总后的情况） 定为无效的探测，不需要上报
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void UltrasoundDetection_2()
{

	checkTime = 3;  //  需要进行多次探测的次数
	checkRightTime = 0;  // 在checkTime中距离较近的探测次数
	distanceSum = 0;	// 有效探测的距离和
	distanceCount = 800;  // 默认距离较远
	
	while (checkTime--)
	{
		// my_printf("a\r\n");
		
		//my_printf("qqqqq\r\n");


		// 开始超声波探测
		
		TH0=0xbb;			// 定时器清零,只打算探测4米以内的距离,只累计值 17647  个时钟
		TL0=0x11; 
		outDetection = 0;
		
		Trig_2=1;			               
		delayus(20);
		Trig_2=0;
		
			
		while(!Echo_2);		//当Echo为 0 时等待
		TR0=1;			    //开启计数
		while(Echo_2)			//当Echo为1计数并等待   还应该设置超出距离，无法低电平的 情况
		{
			if(outDetection !=0 )		    //定时器超时，超出测量  ,跳过等待，节约时间
			{
				TR0=0;		//关闭计数
				break;
			}						
		}		
		TR0=0;				//关闭计数
		//my_printf("qqqqq\r\n");

		if(outDetection !=0 )	  // 超时直接跳过本次探测
		{
			mDelaymS(10);
			outDetection = 0;
			continue;
		} 
		  
		 distanceCount =  conutDetection();			//计算
		// my_printf("distanceCount :%d\r\n",distanceCount );
		if (distanceCount < 300)  // 本次探测距离较近，可记录;距离较远的情况包括1:超时 2： 不超时，但超过400cm 
		{
			checkRightTime++;
			//my_printf("checkRightTime:%bu  \r\n",checkRightTime);
			distanceSum = distanceSum + distanceCount;
		}

		mDelaymS(10);		//设短点，好加快一次总的操作循环

	}
	
	// 在安全距离外，不用打印
	if ( checkRightTime > 0 ) // 进行了效检测到距离，探测到障碍物较近
	{
		//my_printf("t:%bu  d:%d \r\n",checkRightTime,(distanceSum/checkRightTime));
		my_printf("1,2,%03d\r\n",(distanceSum/checkRightTime));  // 进行上行数据传输
	}
	////////////////////////

}


/*******************************************************************************
* Function Name  : UltrasoundDetection_3()
* Description    : 
*					进入超声波  3号（左）模块  探测 ，距离在400cm以内需要上传情况。
*					超过300cm（定时器计时和超时的情况汇总后的情况） 定为无效的探测，不需要上报
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void UltrasoundDetection_3()
{

	checkTime = 3;  //  需要进行多次探测的次数
	checkRightTime = 0;  // 在checkTime中距离较近的探测次数
	distanceSum = 0;	// 有效探测的距离和
	distanceCount = 800;  // 默认距离较远
	
	while (checkTime--)
	{
		// my_printf("a\r\n");
		
		//my_printf("qqqqq\r\n");


		// 开始超声波探测
		
		TH0=0xbb;			// 定时器清零,只打算探测4米以内的距离,只累计值 17647  个时钟
		TL0=0x11; 
		outDetection = 0;
		
		Trig_3=1;			               
		delayus(20);
		Trig_3=0;
		
			
		while(!Echo_3);		//当Echo为 0 时等待
		TR0=1;			    //开启计数
		while(Echo_3)			//当Echo为1计数并等待   还应该设置超出距离，无法低电平的 情况
		{
			if(outDetection !=0 )		    //定时器超时，超出测量  ,跳过等待，节约时间
			{
				TR0=0;		//关闭计数
				break;
			}						
		}		
		TR0=0;				//关闭计数
		//my_printf("qqqqq\r\n");

		if(outDetection !=0 )	  // 超时直接跳过本次探测
		{
			mDelaymS(10);
			outDetection = 0;
			continue;
		} 
		  
		 distanceCount =  conutDetection();			//计算
		// my_printf("distanceCount :%d\r\n",distanceCount );
		if (distanceCount < 300)  // 本次探测距离较近，可记录;距离较远的情况包括1:超时 2： 不超时，但超过400cm 
		{
			checkRightTime++;
			//my_printf("checkRightTime:%bu  \r\n",checkRightTime);
			distanceSum = distanceSum + distanceCount;
		}

		mDelaymS(10);		//设短点，好加快一次总的操作循环

	}
	
	// 在安全距离外，不用打印
	if ( checkRightTime > 0 ) // 进行了效检测到距离，探测到障碍物较近
	{
		//my_printf("t:%bu  d:%d \r\n",checkRightTime,(distanceSum/checkRightTime));
		my_printf("1,3,%03d\r\n",(distanceSum/checkRightTime));  // 进行上行数据传输
	}
	////////////////////////

}

/*******************************************************************************
* Function Name  : UltrasoundDetection_4()
* Description    : 
*					进入超声波  4号（右）模块  探测 ，距离在400cm以内需要上传情况。
*					超过300cm（定时器计时和超时的情况汇总后的情况） 定为无效的探测，不需要上报
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void UltrasoundDetection_4()
{

	checkTime = 3;  //  需要进行多次探测的次数
	checkRightTime = 0;  // 在checkTime中距离较近的探测次数
	distanceSum = 0;	// 有效探测的距离和
	distanceCount = 800;  // 默认距离较远
	
	while (checkTime--)
	{
		// my_printf("a\r\n");
		
		//my_printf("qqqqq\r\n");


		// 开始超声波探测
		
		TH0=0xbb;			// 定时器清零,只打算探测4米以内的距离,只累计值 17647  个时钟
		TL0=0x11; 
		outDetection = 0;
		
		Trig_4=1;			               
		delayus(20);
		Trig_4=0;
		
			
		while(!Echo_4);		//当Echo为 0 时等待
		TR0=1;			    //开启计数
		while(Echo_4)			//当Echo为1计数并等待   还应该设置超出距离，无法低电平的 情况
		{
			if(outDetection !=0 )		    //定时器超时，超出测量  ,跳过等待，节约时间
			{
				TR0=0;		//关闭计数
				break;
			}						
		}		
		TR0=0;				//关闭计数
		//my_printf("qqqqq\r\n");

		if(outDetection !=0 )	  // 超时直接跳过本次探测
		{
			mDelaymS(10);
			outDetection = 0;
			continue;
		} 
		  
		 distanceCount =  conutDetection();			//计算
		// my_printf("distanceCount :%d\r\n",distanceCount );
		if (distanceCount < 300)  // 本次探测距离较近，可记录;距离较远的情况包括1:超时 2： 不超时，但超过400cm 
		{
			checkRightTime++;
			//my_printf("checkRightTime:%bu  \r\n",checkRightTime);
			distanceSum = distanceSum + distanceCount;
		}

		mDelaymS(10);		//设短点，好加快一次总的操作循环

	}
	
	// 在安全距离外，不用打印
	if ( checkRightTime > 0 ) // 进行了效检测到距离，探测到障碍物较近
	{
		//my_printf("t:%bu  d:%d \r\n",checkRightTime,(distanceSum/checkRightTime));
		my_printf("1,4,%03d\r\n",(distanceSum/checkRightTime));  // 进行上行数据传输
	}
	////////////////////////

}



/*******************************************************************************
* Function Name  : UltrasoundDetection()
* Description    : 进入超声波探测 ,共4个模块
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void UltrasoundDetection()
{
	// 检测是否需要进行超声波探测
	if (startUDetectionFlag == 0)
	{
		return ;		
	}
	
	TR2=0;  // 保证在读取值得时候不会出现  脏数据
	if (oneSecondeUDetectionFlag != 1)
	{
		TR2=1;
		return;
	}
	oneSecondeUDetectionFlag = 0;
	TR2=1;
	
	
	UltrasoundDetection_1();	// 前
	UltrasoundDetection_2();	// 后
	UltrasoundDetection_3();	// 左
	UltrasoundDetection_4();	// 右

	
}

void main(void) 
{
	//////////////////////////////////////
	/// 注意 ：端口的输出方向需要重新确定
	
	
	// 端口的方向，输出
	// P0_DIR = 0x00;
	// P0_DIR |= 0xff;
	// P1_DIR = 0x00;
	// P1_DIR |= 0xff;  
	
	// P0_DIR = 0x00;
	// P0_DIR |= 0x20;    // 
	
	// P1_DIR = 0x00;
	// P1_DIR |= 0x0f;   // 运动模块的IO方向
	

	// 灯初始为0，关闭
	// led_1 = 0;   
	// led_2 = 0;
	
	initMotorGPIO();		// 初始化运动相关的IO口
	initUltrasoundGOIO();		// 初始化  超声波模块的IO口
	CH559UART0InterruptInit();	//  串口中断初始化
	
	UltrasonicInit();  // 超声波模块相关的寄存器初始化
	 

	mDelaymS(20);
	
	TIM2Inital();		//  初始化定时器2 ，定时器2是本代码的定时功能的基准定时器


			 // motor_SweepFood_a = 1;
		 // motor_SweepFood_b = 0;
		// // 推出食槽马达		
		 // motor_pushFood_a = 1;
		 // motor_pushFood_b = 0;


		 // motor_1_a = 1;
		 // motor_1_b = 0;
		// // 运动马达2		右边的
		 // motor_2_a = 1;
		 // motor_2_b = 0;
		 
		 
		 my_printf("Hello World\r\n");
	while(1)
	{
		
		if (oneSecondeFlag == 1)
		{
			my_printf("Hello World  1s  SencondNum:%d!\r\n",timer2HalfSencondNum);
			oneSecondeFlag = 0;
		}
		
		// 进行超声波探测
		UltrasoundDetection();
		//my_printf("=\r\n");
		// 进行运动操作
		conductSport();

		
		// 喂食操作
		startFeedPet();
		stopFeedPet();

	}
	
}