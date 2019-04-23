/**
 * Author:Tang Yuan
 * Date:2019.3.21
 * Description:��������ˣ����볬����ģ��,ιʳ����
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


// �˶�ģ��
// �˶�����1		��ߵ�
sbit motor_1_a = P1^2;
sbit motor_1_b = P1^3;
// �˶�����2		�ұߵ�
sbit motor_2_a = P1^1;
sbit motor_2_b = P1^0;


// ע��: �� 1��2   ��д������   �ǵø�
sbit led_1 = P0^1;   //j9
sbit led_2 = P0^2;	 //j10


// ������ģ��
bit startUDetectionFlag = 0; // ������̽�⿪��

//ģ������ݽ�ȷ��
// ������ģ��1��ǰ���
sbit  Echo_1 = P0^3;   		// Echo
sbit  Trig_1 = P0^2;		// Trig
// ������ģ��2�������
sbit  Echo_2 = P2^4;   		// Echo
sbit  Trig_2 = P2^5;		// Trig
// ������ģ��3����ߵ�
sbit  Echo_3 = P0^0;   		// Echo
sbit  Trig_3 = P0^1;		// Trig
// ������ģ��4���ұߵ�
sbit  Echo_4 = P1^6;   		// Echo
sbit  Trig_4 = P1^7;		// Trig

UINT8  outDetection = 0;	// Զ�������
uint  time_ultar = 0;		// һ�γ�����̽��ʱ��   
UINT8 checkTime = 5;   		// ÿ�μ��5��
UINT8 checkRightTime = 5;   // ��Ч������
uint distanceSum =0;   		// һ��̽��ľ����ܺ�
uint distanceCount = 0;


// ��ʱ��2
//�õ�һģ���Լ��ñ�����ʱ�䣬���������ʱ���ı����Ƚϣ���ֵ��Ϊ������ʱ�䣨������ע�⣩
//�õ�һģ���Լ��ñ�����ʱ�䣬���������ʱ���ı����Ƚϣ���ֵ��Ϊ������ʱ�䣨������ע�⣩
UINT8 timer2IRSNum = 0;  //һ���жϴ�����++����10����,��0.5S����
uint timer2HalfSencondNum = 0; //0.5��ͼ�һ�Σ�240��0.5S����
bit timer2IncreaseNum= 0;  //  timer2HalfSencondNum ÿ����������1
bit oneSecondeFlag = 0;
bit oneSecondeUDetectionFlag = 0;  // ��������Ҫ�õ�ʱ���ǩ
uint timer2HalfSencondNumForSport = 0; //�˶�ģ�������Ƚ��յ��������ʱ��


UINT8 DAT = 1;
bit getDataFlag = 0 ;		//�յ����ʵ�����
UINT8 timeSport = 0 ;		// һ���˶���ʱ�����																													// ����ʱ�����
UINT8 order[10];			// �յ�������																													// �յ�������
UINT8 bufNum = 0;		


/////////////////////////////////
//ע��:��Ҫ���¸�ֵ����

// Ͷʳ 
// �յ�Ͷʳ������	,0:����Ͷʳ������1�� ��ʼͶʳ��2�� ����Ͷʳ
UINT8 needFeed = 0 ;		
// ɨʳ����														
sbit motor_SweepFood_a = P2^3;
sbit motor_SweepFood_b = P2^2;
// �Ƴ�ʳ������		
sbit motor_pushFood_a = P2^0;
sbit motor_pushFood_b = P2^1;

sbit needStopPush = P0^4;
sbit needStopPull = P0^5;

																												// ��ǰ������ֽ���
// �˶���ʽ��¼����
UINT8 sportClass = 'i';  //  �������жϵ�ָ��
UINT8 newSportClass = 'i';  // ��ȡ���·���ָ���ִ�е�ʱ����sportClass�Ƚ��ٸ�ֵ��sportClass																												// �˶� ������
																										// ��¼ǰһ�� ���˶����ͣ���ֹ����ת����ƽdown��

// �Զ����ӡ���� ����
bit printfBusy;  // ����״̬λ
unsigned char idata Put_buf[70];		// ��ӡ������ַ�


/*******************************************************************************
* Function Name  : void TIM2Inital(void)
* Description    : timer2  ��ʱ������ÿ50ms �ж�
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void TIM2Inital(void)
{
	RCAP2H = (65536-50000)/256;//����12M 50ms 16bit �Զ�����
	RCAP2L = (65536-50000)%256;
	ET2=1;                     //�򿪶�ʱ���ж�
	EA=1;                      //�����ж�
	TR2=1;                     //�򿪶�ʱ������
}

void TIM2(void) interrupt INT_NO_TMR2 //��ʱ��2�ж�
{
	timer2IRSNum++;
	if (timer2IRSNum == 10)// ����0.5��
	{
		timer2IRSNum =0; // 0.5�����
		timer2HalfSencondNum++;
		timer2IncreaseNum = 1;
		
		
		
		//�鿴�Ƿ� 1s 
		if (((timer2HalfSencondNum %2) == 0))
		{
			oneSecondeFlag = 1;
			oneSecondeUDetectionFlag = 1;
		}
	}
	
	

	
	
	if (timer2HalfSencondNum == Maxtimer2HalfSencondNum)// 500s����
	{
		timer2HalfSencondNum = 0;
	}
	
	
	
	TF2=0;
}



/*******************************************************************************
* Function Name  : my_printf(char* fmt,...) 
* Description    : 
*					�Լ��Ĵ�ӡ�������������ڴ��������ϴ���A33,�����Լ�����ͨ��Э��
*					֮�����Զ����ӡ��������Ϊprintf ��  SBUF ���ò�̫���㣬��ӡ������
* Input          : ��printfʹ�ò��ģ�ע�� ���е�"\n"  ��Ҫ����  "\r\n"
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
	
	for(i=0;i<len;i++)			//�ѻ����ڵ��ַ����ͳ�ȥ
	{	
		while(printfBusy);             	//�ȴ�ǰ������ݷ������
		printfBusy=1;	
		SBUF=Put_buf[i];		//����һ���ֽ�
	}
	while(printfBusy);
	printfBusy=1;
	SBUF = '\r';
	while(printfBusy);
	printfBusy=1;
	SBUF = '\n';
	
	memset(Put_buf,0,sizeof(Put_buf));	//��ջ���
}


/*******************************************************************************
* Function Name  : delayus 
* Description    : us  ��ʱ
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
* Description    : ms  ��ʱ
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
* Description    : ����ιʳ����ʳ���Ƴ���
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void startFeedPet(void)
{
	if (needFeed == 1)  // ��ιʳ������
	{
		
		// ��ֹ֤ͣ�˶�
		motor_1_a = 0;
		motor_1_b = 0;
		motor_2_a = 0;
		motor_2_b = 0;	
		delayms(20);
		
//////////////////////////////////////////
// ע�⣺����� 0 1 ��ֵ���ܷ��ˣ��ǵõ���
//////////////////////////////////////////
		if (needStopPush != 0 )  // ��Ϊ0����������
		{
			// �Ȱ�ʳ��ɨ��ʳ��
			motor_SweepFood_a = 1;
			motor_SweepFood_b = 0;
			
			my_printf("Sweep  Start = 1\r\n");
			delayms(5000);   //  �����Ѷ���ʳ��ɨ��ʳ�� ò��ʱ����12��
			my_printf("Sweep  Stop = 1\r\n");
			
			motor_SweepFood_a = 0;
			motor_SweepFood_b = 0;
			
			delayms(20);
			
		
			// ��ʳ���Ƴ�
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
* Description    : ����ιʳ����ʳ��������
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void stopFeedPet(void)
{
	if (needFeed == 2)  // ��ֹͣιʳ������
	{
		// ��ֹ֤ͣ�˶�
		motor_1_a = 0;
		motor_1_b = 0;
		motor_2_a = 0;
		motor_2_b = 0;
				
//////////////////////////////////////////
// ע�⣺����� 0 1 ��ֵ���ܷ��ˣ��ǵõ���
//////////////////////////////////////////
		
		
		
		if (needStopPull != 0)  // ��Ϊ0 ��˵����������
		{
			// ��ʳ���ջ�
			motor_pushFood_a = 0;
			motor_pushFood_b = 1;
			
			needStopPull = 1;
			while (needStopPull)  // ������ʳ�۵Ľ���λ�õ�ʱ���  needStopPull == 0
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
* Description    : �����⵽�ľ���
* Input          : None
* Output         : None
* Return         : S ̽�⵽�ľ��룬���̫Զ��������800CM 
*******************************************************************************/
uint conutDetection(void)
{
	uint S=0;
	if(outDetection !=0 )		    //��������  
	{
		outDetection=0;
		//my_printf("Y\r\n");
	  
		return 800;
	}
		
	time_ultar = 0;
	// �����ֵ
	time_ultar=(TH0-0xbb)*256+(TL0 - 0x11);  // һ��������1/10M ��   1us,�����������65ms

	S= (uint)(time_ultar*1.87)/100;     //�������CM

	return S;
}




/*******************************************************************************
* Function Name  : mTimer0Interrupt 
* Description    : ������෶Χ
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void mTimer0Interrupt( void ) interrupt INT_NO_TMR0 		 //T0�ж��������������,������෶Χ
{
	outDetection = outDetection + 1 ;		//�ж������־
}
	
	
	
/*******************************************************************************
* Function Name  : StartUltrasoundDetection() 
* Description    : ������ģ�������ź�
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void  StartUltrasoundDetection() 		         
{
	// ��������̽��
	 // my_printf(" StartUltrasoundDetection\r\n");
	Trig_1=1;			               
	delayus(20);
	Trig_1=0;
	
	TH0=0xbb;			// ��ʱ������,ֻ����̽��4�����ڵľ���,ֻ�ۼ�ֵ 17647  ��ʱ��
	TL0=0x11; 
}



/*******************************************************************************
* Function Name  : UltrasonicInit() 
* Description    : ������ģ��ʱ��0���ã�����T0�жϵ�
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void  UltrasonicInit() 		         
{
		TMOD |=0x01;  //��ʱ��0������ʽ1
		ET0=1;
		PT0 = 1;     // ��ʱ��0 �����ж�
		
		Trig_1 = 0;
		TR0=0;		 // �رռ�����0
	
}


/*******************************************************************************
* Function Name  : CH559UART0InterruptInit()
* Description    : CH559UART0�жϳ�ʼ��
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void CH559UART0InterruptInit()
{
		
	SM0 = 0;
	SM1 = 1; 																	//����0ʹ��ģʽ1

	SM2 = 0;  	
	TMOD = 0x20;               													//ʹ��Timer1��Ϊ�����ʷ�����
																				// 

	PCON |= SMOD;
	T2MOD |= 0xa0;																

	TH1 = 0xb2; 																// ������ 9600
	TR1 = 1;                                                                 	//������ʱ��1

	REN = 1; 
	ES = 1;                                                                   	//����UART0�ж�
	EA = 1;                                                                    	//���жϿ���
	TI = 1;																		// �����жϱ�־λ��������Ӳ����1
	PS =1;																		// ������ߴ������ȼ�
}




/*******************************************************************************
* Function Name  : sendState()
* Description    : ���͵�ǰ��������� 
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
* Description    : ִ������  
* Input          : 
*					command1: ��������
*					command2: �����������
* Output         : None
* Return         : None
*******************************************************************************/
void executeOrder( UINT8 command1, UINT8 command2)
{	
	//my_printf("d2��%bu ,d2��%bu\r\n",command1,command2);
	
	
													  
	if ( command1 == '1') // �˶���ص�����
	{
		switch (command2)
		{
			//my_printf("enter command2��%bu\r\n",command2);
			// a:ǰ b:��  c:�� d:�� e:ǰ + ��  g���� + ��  h���� + ��  i��ͣ	
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
		
	}else if (command1 == '2')  // ��������ص�����
	{
		switch (command2)
		{
			case 'a':	  // ����������̽��	
				startUDetectionFlag = 1;
				break;
			case 'b':	 // �رճ�����̽��
				startUDetectionFlag = 0;
				break;	
			default:
				break;			
		}	
		
		
	}else if (command1 == '4')  // Ͷʳ��ص�����
	{
		switch (command2)
		{
			case 'a':	  // ����Ͷʳ	
				newSportClass  = 'i';  
				needFeed = 1;   // Ϊ����ȫ ��ȷ��ֹͣ
				break;
			case 'b':	 // ����Ͷʳ
				needFeed = 2;			
				newSportClass  = 'i'; // Ϊ����ȫ ��ȷ��ֹͣ
				break;	
			default:
				newSportClass  = 'i'; // Ϊ����ȫ ��ȷ��ֹͣ
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
* Description    : CH559UART0�жϴ����������ж�һ������������
*******************************************************************************/
 void CH559UART0Interrupt( )  interrupt INT_NO_UART0  using 1                   //���Ź��жϷ������,ʹ�üĴ�����1
 {
    if(TI)
    {
		TI = 0;                                                                // ��շ����ж�                        
		printfBusy = 0;
    }
		
    if(RI)
    {      		
		DAT = SBUF;
		//SBUF = DAT;	

  

		if (1)
		{
						//  ��⵽��ͷΪ# �Ž��н��պͽ���
			if ( DAT == '#')
			{
				getDataFlag = 1;
				bufNum = 0;
			}
			
			if ( getDataFlag == 1)  // �յ�Э�鿪ͷ  '#��,�����ַ�����
			{
				order[bufNum] = DAT;	
				// SBUF = 		order[bufNum];
				// SBUF = bufNum;     
			}
			
			if (  bufNum == 3 )	 	//  �յ�4 �ֽ�  ,�ж��ǲ���һ������,
			{
				//my_printf("order:%bu%bu%bu%bu  \r\n",order[0],order[1], order[2],order[3]);
				if ( order[bufNum] == '.' )  //"."��һ������Ľ���
				{
					// ����Э��Ҫ����ֽ�������"#1a."
					if ( order[bufNum-3] == '#')
					{
						executeOrder(order[bufNum-2], order[bufNum-1]);  					// ʹ��'.'��ǰ��λλ��Ϊ����	 ��  1a				
					}				
				}
		
				// �յ�4���ֽں������ǲ���������������־
				bufNum = 0;
				
				// order[0] = '0';
				// order[1] = '0';
				// order[2] = '0';
				// order[3] = '0';
				memset(order,0,sizeof(order));	//��ջ���
				getDataFlag = 0;
				DAT = '/';
			}
			else{
				bufNum++;  // ��һ���ַ��Ŷ�λ��	
			}
						
		}
		
		RI = 0;																																// ��ս����ж�
     }
		
 }

 
 /*******************************************************************************
* Function Name  :initMotorGPIO(void)
* Description    : ��ʼ��  �������� �����IO��
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void  initMotorGPIO(void)
{
	// p1.0��1		INM_2       �ұߵ�
	// p1.2��3		INB_1		��ߵ�
	P1_DIR |= (0x0f);
	
	// p2.0��1		INB_3		�Ƴ�ʳ������
	// p2.2��3		INB_4		ɨʳ����
	P2_DIR |= (0x0f);
	
	
}

/*******************************************************************************
* Function Name  :conductSport(void)
* Description    : ִ���˶�����
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void conductSport(void)
{
	
	//  �ж�δ�յ����2sû���Ƽ�APP 1.5s�·�һ������
	TR2=0; 
	if (timer2HalfSencondNum > timer2HalfSencondNumForSport)  // ����δ��Ȧ�����ڵ�ǰ timer2HalfSencondNum  δ����
	{
		if ((timer2HalfSencondNum - timer2HalfSencondNumForSport) >3 )  //  δ�յ�����2s
		{
			my_printf("sportClass = 'i';\r\n");
			newSportClass = 'i';
			
			timer2HalfSencondNumForSport = timer2HalfSencondNum;
		}
	}
	if (timer2HalfSencondNum < timer2HalfSencondNumForSport)  //����2s�� timer2HalfSencondNum  ������
	{
		if ((timer2HalfSencondNum+Maxtimer2HalfSencondNum - timer2HalfSencondNumForSport) >3 )  //  δ�յ�����2s
		{
			my_printf("sportClass = 'i'   go  to 0;\r\n");
			newSportClass = 'i';
			timer2HalfSencondNumForSport = timer2HalfSencondNum;
			
		}
	}
	TR2=1; 
	
	
	if (newSportClass == sportClass) // �ظ���ָ�ֱ�Ӽ�����һ�ε��˶�
	{
		mDelaymS(200);		// ��0.2��
		return;
	}
	else{
		sportClass = newSportClass;  // ���µ�ǰ���˶�����
		
		
		// �������״̬ ����
		motor_1_a = 0;
		motor_1_b = 0;
		motor_2_a = 0;
		motor_2_b = 0;
		mDelaymS(50);
	}
	
	if ( sportClass == 'i')
	{
		
		//  ���ִ���
		motor_1_a = 0;
		motor_1_b = 0;
		motor_2_a = 0;
		motor_2_b = 0;

		
		mDelaymS(200);		// ��0.2��
	}
	else if ( sportClass == 'a')
	{

		//  ǰ��
		motor_1_a = 1;
		motor_1_b = 0;
		motor_2_a = 1;
		motor_2_b = 0;
		
		//my_printf("+\r\n");
		mDelaymS(200);		// ��0.2��
		
	}
	else if ( sportClass == 'b')
	{			
		
		//  ����
		motor_1_a = 0;
		motor_1_b = 1;
		motor_2_a = 0;
		motor_2_b = 1;
		
		mDelaymS(200);		// ��0.2��
		
	}
	else if ( sportClass == 'c')
	{
			
		//  ��
		
		/*
		motor_1_a = 1;
		motor_1_b = 1;
		motor_2_a = 1;
		motor_2_b = 0;
		
		mDelaymS(200);		// ��0.2��					
		*/
		
		
		// //  ��  �Դ��Ȧ
		// //  ǰ��
		// motor_1_a = 1;
		// motor_1_b = 0;
		// motor_2_a = 1;
		// motor_2_b = 0;
		
		// mDelaymS(50);		
		
		//  ��
		motor_1_a = 0;
		motor_1_b = 0;
		motor_2_a = 1;
		motor_2_b = 0;
		
		mDelaymS(200);		
		
	}
	else if ( sportClass == 'd')
	{
		/*	
		//  ��
		motor_1_a = 1;
		motor_1_b = 0;
		motor_2_a = 1;
		motor_2_b = 1;
		
		mDelaymS(200);		// ��0.2��						
		*/
		
		// // �Ҵ���Ȧ
		
		// //  ǰ��
		// motor_1_a = 1;
		// motor_1_b = 0;
		// motor_2_a = 1;
		// motor_2_b = 0;
		
		// mDelaymS(50);		
		
		//  ��
		motor_1_a = 1;
		motor_1_b = 0;
		motor_2_a = 0;
		motor_2_b = 0;
		
		mDelaymS(200);		
		
	}
	else if ( sportClass == 'e')
	{
			
		// ǰ + ��
		
		//  ǰ��
		motor_1_a = 1;
		motor_1_b = 0;
		motor_2_a = 1;
		motor_2_b = 0;
		
		mDelaymS(100);		
		
		//  ��
		motor_1_a = 1;
		motor_1_b = 0;
		motor_2_a = 0;
		motor_2_b = 0;
		
		mDelaymS(100);		

		
	}
	else if ( sportClass == 'f')
	{
			
		// ǰ + ��
		
		//  ǰ��
		motor_1_a = 1;
		motor_1_b = 0;
		motor_2_a = 1;
		motor_2_b = 0;
		
		mDelaymS(100);		
		
		//  ��
		motor_1_a = 0;
		motor_1_b = 0;
		motor_2_a = 1;
		motor_2_b = 0;
		
		mDelaymS(100);		

		
	}
	else if ( sportClass == 'g')
	{

		// �� + ��
		
		//  ����
		motor_1_a = 0;
		motor_1_b = 1;
		motor_2_a = 0;
		motor_2_b = 1;
		
		mDelaymS(100);		
		
		//  ��
		motor_1_a = 0;
		motor_1_b = 1;
		motor_2_a = 0;
		motor_2_b = 0;
		
		mDelaymS(100);		

		
	}
	else if ( sportClass == 'h')
	{

		// �� + ��
		
		//  ����
		motor_1_a = 0;
		motor_1_b = 1;
		motor_2_a = 0;
		motor_2_b = 1;
		
		mDelaymS(100);		
		
		//  ��
		motor_1_a = 0;
		motor_1_b = 0;
		motor_2_a = 0;
		motor_2_b = 1;
		
		mDelaymS(100);		
						
		
	}else 
	{
		//  ���ִ���
		motor_1_a = 0;
		motor_1_b = 0;
		motor_2_a = 0;
		motor_2_b = 0;
		mDelaymS(200);		// ��0.2��
		
		
	}


	
	
	// timeSport++;
		
	// // һ��ʱ��δ�յ��˶�ָ��Զ�ֹͣ����ҪAPP  1.5S���ڷ���һ���˶�����
	// if (startUDetectionFlag == 0)  // δ���г�����̽�⣬��� 1.6S
	// {
		// //  ��ⳤʱ�������ֹͣ  8*0.2 ��
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
		
		// //  ��ⳤʱ�������ֹͣ  7*0.25 ��      ���1.7
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
* Description    : ��ʼ��������ģ����Ҫ������    Echo��ֵ 0  Trig ��ֵ1
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void initUltrasoundGOIO( void )
{
	
	// ǰ
	// Echo 0   Echo_1 = P0^3;
	P0_DIR &= ~(0x08);
	// Trig 1     Trig_1 = P0^2
	P0_DIR |= (0x04);
	
	// ��
	// Echo 0   Echo_2 = P2^4;
	P2_DIR  &= ~(0x10);
	// Trig 1    Trig_2 = P2^5;
	P2_DIR |=(0x20);
	
	// ��
	// Echo 0   Echo_3 = P0^0;
	P0_DIR &= ~(0x01);
	// Trig 1   Trig_3 = P0^1;
	P0_DIR |= (0x02);
	
	// ��
	// Echo 0   Echo_4 = P1^6;
	P1_DIR &= ~(0x40);
	// Trig 1   Trig_4 = P1^7;
	P1_DIR |= (0x80);
}


/*******************************************************************************
* Function Name  : UltrasoundDetection_1()
* Description    : 
*					���볬����  1�ţ�ǰ��ģ��  ̽�� ��������400cm������Ҫ�ϴ������
*					����300cm����ʱ����ʱ�ͳ�ʱ��������ܺ������� ��Ϊ��Ч��̽�⣬����Ҫ�ϱ�
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void UltrasoundDetection_1()
{
   
	checkTime = 3;  //  ��Ҫ���ж��̽��Ĵ���
	checkRightTime = 0;  // ��checkTime�о���Ͻ���̽�����
	distanceSum = 0;	// ��Ч̽��ľ����
	distanceCount = 800;  // Ĭ�Ͼ����Զ
	
	while (checkTime > 0)
	{
		
		checkTime--;
		
		 //my_printf("a  checkTime:%d\r\n",checkTime);
		
		//my_printf("qqqqq\r\n");
  

		// ��ʼ������̽��
		
		TH0=0xbb;			// ��ʱ������,ֻ����̽��4�����ڵľ���,ֻ�ۼ�ֵ 17647  ��ʱ��
		TL0=0x11; 
		outDetection = 0;
		
		Trig_1=1;			               
		delayus(20);
		Trig_1=0;
		
			
		while(!Echo_1);		//��EchoΪ 0 ʱ�ȴ�
		TR0=1;			    //��������
		while(Echo_1)			//��EchoΪ1�������ȴ�   ��Ӧ�����ó������룬�޷��͵�ƽ�� ���
		{
			if(outDetection !=0 )		    //��ʱ����ʱ����������  ,�����ȴ�����Լʱ��
			{
				TR0=0;		//�رռ���
				break;
			}						
		}		
		TR0=0;				//�رռ���
		//my_printf("qqqqq\r\n");

		if(outDetection !=0 )	  // ��ʱֱ����������̽��
		{
			//mDelaymS(10);
			outDetection = 0;
			continue;
		} 
		  
		 distanceCount =  conutDetection();			//����
		 //my_printf("distanceCount :%d\r\n",distanceCount );
		if (distanceCount < 300)  // ����̽�����Ͻ����ɼ�¼;�����Զ���������1:��ʱ 2�� ����ʱ��������400cm 
		{
			checkRightTime++;
			//my_printf("checkRightTime:%bu  \r\n",checkRightTime);
			distanceSum = distanceSum + distanceCount;
		}

		mDelaymS(10);		//��̵㣬�üӿ�һ���ܵĲ���ѭ��

	}
	
	// �ڰ�ȫ�����⣬���ô�ӡ
	if ( checkRightTime > 0 ) // ������Ч��⵽���룬̽�⵽�ϰ���Ͻ�
	{
		//my_printf("t:%bu  d:%d \r\n",checkRightTime,(distanceSum/checkRightTime));
		my_printf("1,1,%03d\r\n",(distanceSum/checkRightTime));  // �����������ݴ���
	}
	////////////////////////

}


/*******************************************************************************
* Function Name  : UltrasoundDetection_2()
* Description    : 
*					���볬����  2�ţ���ģ��  ̽�� ��������400cm������Ҫ�ϴ������
*					����300cm����ʱ����ʱ�ͳ�ʱ��������ܺ������� ��Ϊ��Ч��̽�⣬����Ҫ�ϱ�
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void UltrasoundDetection_2()
{

	checkTime = 3;  //  ��Ҫ���ж��̽��Ĵ���
	checkRightTime = 0;  // ��checkTime�о���Ͻ���̽�����
	distanceSum = 0;	// ��Ч̽��ľ����
	distanceCount = 800;  // Ĭ�Ͼ����Զ
	
	while (checkTime--)
	{
		// my_printf("a\r\n");
		
		//my_printf("qqqqq\r\n");


		// ��ʼ������̽��
		
		TH0=0xbb;			// ��ʱ������,ֻ����̽��4�����ڵľ���,ֻ�ۼ�ֵ 17647  ��ʱ��
		TL0=0x11; 
		outDetection = 0;
		
		Trig_2=1;			               
		delayus(20);
		Trig_2=0;
		
			
		while(!Echo_2);		//��EchoΪ 0 ʱ�ȴ�
		TR0=1;			    //��������
		while(Echo_2)			//��EchoΪ1�������ȴ�   ��Ӧ�����ó������룬�޷��͵�ƽ�� ���
		{
			if(outDetection !=0 )		    //��ʱ����ʱ����������  ,�����ȴ�����Լʱ��
			{
				TR0=0;		//�رռ���
				break;
			}						
		}		
		TR0=0;				//�رռ���
		//my_printf("qqqqq\r\n");

		if(outDetection !=0 )	  // ��ʱֱ����������̽��
		{
			mDelaymS(10);
			outDetection = 0;
			continue;
		} 
		  
		 distanceCount =  conutDetection();			//����
		// my_printf("distanceCount :%d\r\n",distanceCount );
		if (distanceCount < 300)  // ����̽�����Ͻ����ɼ�¼;�����Զ���������1:��ʱ 2�� ����ʱ��������400cm 
		{
			checkRightTime++;
			//my_printf("checkRightTime:%bu  \r\n",checkRightTime);
			distanceSum = distanceSum + distanceCount;
		}

		mDelaymS(10);		//��̵㣬�üӿ�һ���ܵĲ���ѭ��

	}
	
	// �ڰ�ȫ�����⣬���ô�ӡ
	if ( checkRightTime > 0 ) // ������Ч��⵽���룬̽�⵽�ϰ���Ͻ�
	{
		//my_printf("t:%bu  d:%d \r\n",checkRightTime,(distanceSum/checkRightTime));
		my_printf("1,2,%03d\r\n",(distanceSum/checkRightTime));  // �����������ݴ���
	}
	////////////////////////

}


/*******************************************************************************
* Function Name  : UltrasoundDetection_3()
* Description    : 
*					���볬����  3�ţ���ģ��  ̽�� ��������400cm������Ҫ�ϴ������
*					����300cm����ʱ����ʱ�ͳ�ʱ��������ܺ������� ��Ϊ��Ч��̽�⣬����Ҫ�ϱ�
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void UltrasoundDetection_3()
{

	checkTime = 3;  //  ��Ҫ���ж��̽��Ĵ���
	checkRightTime = 0;  // ��checkTime�о���Ͻ���̽�����
	distanceSum = 0;	// ��Ч̽��ľ����
	distanceCount = 800;  // Ĭ�Ͼ����Զ
	
	while (checkTime--)
	{
		// my_printf("a\r\n");
		
		//my_printf("qqqqq\r\n");


		// ��ʼ������̽��
		
		TH0=0xbb;			// ��ʱ������,ֻ����̽��4�����ڵľ���,ֻ�ۼ�ֵ 17647  ��ʱ��
		TL0=0x11; 
		outDetection = 0;
		
		Trig_3=1;			               
		delayus(20);
		Trig_3=0;
		
			
		while(!Echo_3);		//��EchoΪ 0 ʱ�ȴ�
		TR0=1;			    //��������
		while(Echo_3)			//��EchoΪ1�������ȴ�   ��Ӧ�����ó������룬�޷��͵�ƽ�� ���
		{
			if(outDetection !=0 )		    //��ʱ����ʱ����������  ,�����ȴ�����Լʱ��
			{
				TR0=0;		//�رռ���
				break;
			}						
		}		
		TR0=0;				//�رռ���
		//my_printf("qqqqq\r\n");

		if(outDetection !=0 )	  // ��ʱֱ����������̽��
		{
			mDelaymS(10);
			outDetection = 0;
			continue;
		} 
		  
		 distanceCount =  conutDetection();			//����
		// my_printf("distanceCount :%d\r\n",distanceCount );
		if (distanceCount < 300)  // ����̽�����Ͻ����ɼ�¼;�����Զ���������1:��ʱ 2�� ����ʱ��������400cm 
		{
			checkRightTime++;
			//my_printf("checkRightTime:%bu  \r\n",checkRightTime);
			distanceSum = distanceSum + distanceCount;
		}

		mDelaymS(10);		//��̵㣬�üӿ�һ���ܵĲ���ѭ��

	}
	
	// �ڰ�ȫ�����⣬���ô�ӡ
	if ( checkRightTime > 0 ) // ������Ч��⵽���룬̽�⵽�ϰ���Ͻ�
	{
		//my_printf("t:%bu  d:%d \r\n",checkRightTime,(distanceSum/checkRightTime));
		my_printf("1,3,%03d\r\n",(distanceSum/checkRightTime));  // �����������ݴ���
	}
	////////////////////////

}

/*******************************************************************************
* Function Name  : UltrasoundDetection_4()
* Description    : 
*					���볬����  4�ţ��ң�ģ��  ̽�� ��������400cm������Ҫ�ϴ������
*					����300cm����ʱ����ʱ�ͳ�ʱ��������ܺ������� ��Ϊ��Ч��̽�⣬����Ҫ�ϱ�
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void UltrasoundDetection_4()
{

	checkTime = 3;  //  ��Ҫ���ж��̽��Ĵ���
	checkRightTime = 0;  // ��checkTime�о���Ͻ���̽�����
	distanceSum = 0;	// ��Ч̽��ľ����
	distanceCount = 800;  // Ĭ�Ͼ����Զ
	
	while (checkTime--)
	{
		// my_printf("a\r\n");
		
		//my_printf("qqqqq\r\n");


		// ��ʼ������̽��
		
		TH0=0xbb;			// ��ʱ������,ֻ����̽��4�����ڵľ���,ֻ�ۼ�ֵ 17647  ��ʱ��
		TL0=0x11; 
		outDetection = 0;
		
		Trig_4=1;			               
		delayus(20);
		Trig_4=0;
		
			
		while(!Echo_4);		//��EchoΪ 0 ʱ�ȴ�
		TR0=1;			    //��������
		while(Echo_4)			//��EchoΪ1�������ȴ�   ��Ӧ�����ó������룬�޷��͵�ƽ�� ���
		{
			if(outDetection !=0 )		    //��ʱ����ʱ����������  ,�����ȴ�����Լʱ��
			{
				TR0=0;		//�رռ���
				break;
			}						
		}		
		TR0=0;				//�رռ���
		//my_printf("qqqqq\r\n");

		if(outDetection !=0 )	  // ��ʱֱ����������̽��
		{
			mDelaymS(10);
			outDetection = 0;
			continue;
		} 
		  
		 distanceCount =  conutDetection();			//����
		// my_printf("distanceCount :%d\r\n",distanceCount );
		if (distanceCount < 300)  // ����̽�����Ͻ����ɼ�¼;�����Զ���������1:��ʱ 2�� ����ʱ��������400cm 
		{
			checkRightTime++;
			//my_printf("checkRightTime:%bu  \r\n",checkRightTime);
			distanceSum = distanceSum + distanceCount;
		}

		mDelaymS(10);		//��̵㣬�üӿ�һ���ܵĲ���ѭ��

	}
	
	// �ڰ�ȫ�����⣬���ô�ӡ
	if ( checkRightTime > 0 ) // ������Ч��⵽���룬̽�⵽�ϰ���Ͻ�
	{
		//my_printf("t:%bu  d:%d \r\n",checkRightTime,(distanceSum/checkRightTime));
		my_printf("1,4,%03d\r\n",(distanceSum/checkRightTime));  // �����������ݴ���
	}
	////////////////////////

}



/*******************************************************************************
* Function Name  : UltrasoundDetection()
* Description    : ���볬����̽�� ,��4��ģ��
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void UltrasoundDetection()
{
	// ����Ƿ���Ҫ���г�����̽��
	if (startUDetectionFlag == 0)
	{
		return ;		
	}
	
	TR2=0;  // ��֤�ڶ�ȡֵ��ʱ�򲻻����  ������
	if (oneSecondeUDetectionFlag != 1)
	{
		TR2=1;
		return;
	}
	oneSecondeUDetectionFlag = 0;
	TR2=1;
	
	
	UltrasoundDetection_1();	// ǰ
	UltrasoundDetection_2();	// ��
	UltrasoundDetection_3();	// ��
	UltrasoundDetection_4();	// ��

	
}

void main(void) 
{
	//////////////////////////////////////
	/// ע�� ���˿ڵ����������Ҫ����ȷ��
	
	
	// �˿ڵķ������
	// P0_DIR = 0x00;
	// P0_DIR |= 0xff;
	// P1_DIR = 0x00;
	// P1_DIR |= 0xff;  
	
	// P0_DIR = 0x00;
	// P0_DIR |= 0x20;    // 
	
	// P1_DIR = 0x00;
	// P1_DIR |= 0x0f;   // �˶�ģ���IO����
	

	// �Ƴ�ʼΪ0���ر�
	// led_1 = 0;   
	// led_2 = 0;
	
	initMotorGPIO();		// ��ʼ���˶���ص�IO��
	initUltrasoundGOIO();		// ��ʼ��  ������ģ���IO��
	CH559UART0InterruptInit();	//  �����жϳ�ʼ��
	
	UltrasonicInit();  // ������ģ����صļĴ�����ʼ��
	 

	mDelaymS(20);
	
	TIM2Inital();		//  ��ʼ����ʱ��2 ����ʱ��2�Ǳ�����Ķ�ʱ���ܵĻ�׼��ʱ��


			 // motor_SweepFood_a = 1;
		 // motor_SweepFood_b = 0;
		// // �Ƴ�ʳ������		
		 // motor_pushFood_a = 1;
		 // motor_pushFood_b = 0;


		 // motor_1_a = 1;
		 // motor_1_b = 0;
		// // �˶�����2		�ұߵ�
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
		
		// ���г�����̽��
		UltrasoundDetection();
		//my_printf("=\r\n");
		// �����˶�����
		conductSport();

		
		// ιʳ����
		startFeedPet();
		stopFeedPet();

	}
	
}