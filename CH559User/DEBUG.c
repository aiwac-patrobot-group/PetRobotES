/********************************** (C) COPYRIGHT *******************************
* File Name          : DEBUG.C
* Author             : WCH
* Version            : V1.3
* Date               : 2016/06/24
* Description        : CH559 DEBUG Interface
                     (1)???0??????,?????;              				   
*******************************************************************************/

#include <stdio.h>
#include "CH559.H"

#define	 FREQ_SYS	12000000	                                                   //????12MHz
#ifndef  BUAD
#define  BUAD    57600
#endif

/*******************************************************************************
* Function Name  : CfgFsys( )
* Description    : CH559?????????,????????12MHz,?????FREQ_SYS??
                   ??PLL_CFG?CLOCK_CFG????,????:
                   Fsys = (Fosc * ( PLL_CFG & MASK_PLL_MULT ))/(CLOCK_CFG & MASK_SYS_CK_DIV);
                   ??????????
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/ 
void	CfgFsys( )  
{
    SAFE_MOD = 0x55;                                                           //??????   ,???????????????
    SAFE_MOD = 0xAA;                                                 
//  CLOCK_CFG |= bOSC_EN_XT;                                                   //??????                                         
//  CLOCK_CFG &= ~bOSC_EN_INT;                                              
// 	CLOCK_CFG &= ~MASK_SYS_CK_DIV;
//  CLOCK_CFG |= 6;                                                            //??????48MHz
//  CLOCK_CFG |= 8;                                                            //??????36MHz
//  CLOCK_CFG |= 10;                                                           //??????28.8MHz
//  CLOCK_CFG |= 12;                                                           //??????24MHz
//  CLOCK_CFG |= 16;                                                           //??????18MHz  	
    SAFE_MOD = 0xFF;                                                           //??????  
//  ??????,?????FREQ_SYS,???????????
}

/*******************************************************************************
* Function Name  : mDelayus(UNIT16 n)
* Description    : us????
* Input          : UNIT16 n
* Output         : None
* Return         : None
*******************************************************************************/ 
void	mDelayuS( UINT16 n )  // ?uS?????
{
	while ( n ) {  // total = 12~13 Fsys cycles, 1uS @Fsys=12MHz
		++ SAFE_MOD;  // 2 Fsys cycles, for higher Fsys, add operation here
#ifdef	FREQ_SYS
#if		FREQ_SYS >= 14000000
		++ SAFE_MOD;
#endif
#if		FREQ_SYS >= 16000000
		++ SAFE_MOD;
#endif
#if		FREQ_SYS >= 18000000
		++ SAFE_MOD;
#endif
#if		FREQ_SYS >= 20000000
		++ SAFE_MOD;
#endif
#if		FREQ_SYS >= 22000000
		++ SAFE_MOD;
#endif
#if		FREQ_SYS >= 24000000
		++ SAFE_MOD;
#endif
#if		FREQ_SYS >= 26000000
		++ SAFE_MOD;
#endif
#if		FREQ_SYS >= 28000000
		++ SAFE_MOD;
#endif
#if		FREQ_SYS >= 30000000
		++ SAFE_MOD;
#endif
#if		FREQ_SYS >= 32000000
		++ SAFE_MOD;
#endif
#if		FREQ_SYS >= 34000000
		++ SAFE_MOD;
#endif
#if		FREQ_SYS >= 36000000
		++ SAFE_MOD;
#endif
#if		FREQ_SYS >= 38000000
		++ SAFE_MOD;
#endif
#if		FREQ_SYS >= 40000000
		++ SAFE_MOD;
#endif
#if		FREQ_SYS >= 42000000
		++ SAFE_MOD;
#endif
#if		FREQ_SYS >= 44000000
		++ SAFE_MOD;
#endif
#if		FREQ_SYS >= 46000000
		++ SAFE_MOD;
#endif
#if		FREQ_SYS >= 48000000
		++ SAFE_MOD;
#endif
#if		FREQ_SYS >= 50000000
		++ SAFE_MOD;
#endif
#if		FREQ_SYS >= 52000000
		++ SAFE_MOD;
#endif
#if		FREQ_SYS >= 54000000
		++ SAFE_MOD;
#endif
#if		FREQ_SYS >= 56000000
		++ SAFE_MOD;
#endif
#endif
		-- n;
	}
}

/*******************************************************************************
* Function Name  : mDelayms(UNIT16 n)
* Description    : ms????
* Input          : UNIT16 n
* Output         : None
* Return         : None
*******************************************************************************/
void	mDelaymS( UINT16 n )                                                  // ?mS?????
{
	while ( n ) 
	{
		mDelayuS( 1000 );
		-- n;
	}
}                                         

/*******************************************************************************
* Function Name  : CH559UART0Alter()
* Description    : CH559??0????,?????P0.2?P0.3
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void CH559UART0Alter()
{
    PORT_CFG |= bP0_OC;
    P0_DIR |= bTXD_;
    P0_PU |= bTXD_ | bRXD_;
    PIN_FUNC |= bUART0_PIN_X;                                                  //?????P0.2?P0.3
}

/*******************************************************************************
* Function Name  : mInitSTDIO()
* Description    : CH559??0???,????T1?UART0???????,?????T2
                   ????????
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void	mInitSTDIO( )
{
	
	
	

    UINT32 x;
    UINT8 x2; 

    SM0 = 0;
    SM1 = 1;  
	//??0????1

    SM2  = 0;                                                                  //???????                                                                    																		   
    RCLK = 0;                                                                  //UART0????
    TCLK = 0;                                                                  //UART0????
	//??Timer1????????  
	
    PCON |= SMOD;
	
	// ??? =  FREQ_SYS / 12  /x(???????? = 256 - TH1)  / 16 
	//12 : ???? = 12* ????
	//16 :??????16?
    x = 10 * FREQ_SYS / BUAD / 16;                                               //??????,??x??????                            
    x2 = x % 10;
    x /= 10;
    if ( x2 >= 5 ) x ++;                                                       //????

    TMOD = TMOD & ~ bT1_GATE & ~ bT1_CT & ~ MASK_T1_MOD | bT1_M1;              //0X20,Timer1??8????????
    T2MOD = T2MOD | bTMR_CLK | bT1_CLK;                                        //Timer1????
    //TH1 = 0-x;                                                                 //12MHz??,buad/12?????????   ,??,???1???????????
   TH1 = 0xF3;
	 TR1 = 1;                                                                   //?????1
    TI = 1;
    REN = 1;                                                                   //??0????
		

		/*
		SCON = 0x50;	 //开启串口方式1，并使能串口接收
		TMOD = 0x20;
		T2MOD = T2MOD | bTMR_CLK | bT1_CLK;  
		PCON = 0x80;
		TH1 =  0xF3;
		
		
    ES = 1;//??????
    EA = 1;//?????
    TR1 = 1;//?????	
		
		*/
		
		
		
		
		
		
		
}

/*******************************************************************************
* Function Name  : CH559UART0RcvByte()
* Description    : CH559UART0??????
* Input          : None
* Output         : None
* Return         : SBUF
*******************************************************************************/
UINT8  CH559UART0RcvByte( )
{
    while(RI == 0);                                                            //????,???????
    RI = 0;
    return SBUF;
}

/*******************************************************************************
* Function Name  : CH559UART0SendByte(UINT8 SendDat)
* Description    : CH559UART0??????
* Input          : UINT8 SendDat;??????
* Output         : None
* Return         : None
*******************************************************************************/
void CH559UART0SendByte(UINT8 SendDat)
{
	SBUF = SendDat;                                                              //????,?????????2???,?????TI=0
	while(TI ==0);
	TI = 1;
}