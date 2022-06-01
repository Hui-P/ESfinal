/*
 * @Author: Hui
 * @Date: 2022-05-27 16:47:20
 * @LastEditors: Hui
 * @LastEditTime: 2022-05-28 17:12:53
 * @Description: APP主函数，创建、使用函数、负责逻辑部分
 * 
 * Copyright (c) 2022 by HUI, All Rights Reserved. 
 */
#include <includes.h>

/*********************************************/
/*任务创建任务*/
static  OS_TCB   AppTaskStartTCB;							//优先级2
static  CPU_STK  AppTaskStartStk[APP_TASK_START_STK_SIZE];	//128u
static  void  AppTaskStart  (void *p_arg);
//////////////////////////////////////////////
/*按键扫描任务*/
//任务优先级
#define KEY_TASK_PRIO		4
//任务堆栈大小	
#define KEY_STK_SIZE 		512
//任务控制块
static	OS_TCB KeyTaskTCB;
//任务堆栈	
static	CPU_STK KEY_TASK_STK[KEY_STK_SIZE];
static	void	key_task(void *p_arg);

//////////////////////////////////////////////
/*LED闪烁任务*/
//任务优先级
#define LED_TASK_PRIO		7
//任务堆栈大小
#define LED_STK_SIZE		128
//任务控制块
static	OS_TCB	LedTaskTCB;
//任务堆栈
static	CPU_STK	LED_TASK_STK[LED_STK_SIZE];
//任务函数
static	void 	led_task(void *p_arg);
//////////////////////////////////////////////
/*OLED显示任务*/
//任务优先级
#define SHOW_TASK_PRIO		6
//任务堆栈大小
#define SHOW_STK_SIZE		128
//任务控制块
static	OS_TCB	ShowTaskTCB;
//任务堆栈
static	CPU_STK	SHOW_TASK_STK[SHOW_STK_SIZE];
//任务函数
static	void	show_task(void *p_arg);

//////////////////////////////////////////////
/* 串口中断接收 */
u8 RxCounter = 0;//定义计数值
u8 RxBuffer[12];//定义缓冲区为12(接收设置温度4位数)
u8 TXBuffer[4];//发送缓冲区为4(发送炉温和室温8位数)
u8 IR_receive = 0;//接收标志
u16 TXtemp;

#define ASC2KB(x)	 (u8)(x%(0x30))	//ASCII 转10进制
#define KB2ASC(x)	 (u8)(0x30+x)	//10进制转 ASCII

//////////////////////////////////////////////
/*定时器1--ADC数据*/
OS_TMR tmr1;
void tmr1_callback(void *p_tmr, void *p_arg);
/*定时器2--定时刷新显示数据*/
OS_TMR tmr2;
static	void tmr2_callback(void *p_tmr,void *p_arg);
///////////////////////////////////////////////

u16 to_number(u8 a[],u8 len)//数组转换成整数
{
    u16 num=0;
		for(u8 i= len;i<= 3+len;i++)
		{
			num=num*10+ASC2KB(a[i]);
		}
    
	return num;
}

///////////////////////////////////////////////
/* 全局变量 */
u16 RT,OT;		//室温，炉温
u16 ST = 100;	//设置温度

int setting_flag = 1;	//设置模式标志位
u16 point = 1;				//光标位置(数值）
u8 point_sit = 4;			//光标位置
u16 SET_Temp;			//保存设置温度的临时变量
/*********************************************/
#define MAX_TEMP 1350	//最大温度

//OS_SEM SETTING_SEM;		//设置切换

/*********************************************/
/* main 函数 */
int  main (void)
{
    OS_ERR  err;

    OSInit(&err);                                               /* Init uC/OS-III.                                      */

    OSTaskCreate((OS_TCB     *)&AppTaskStartTCB,                /* Create the start task                                */
                 (CPU_CHAR   *)"App Task Start",
                 (OS_TASK_PTR ) AppTaskStart,
                 (void       *) 0,
                 (OS_PRIO     ) APP_TASK_START_PRIO,
                 (CPU_STK    *)&AppTaskStartStk[0],
                 (CPU_STK_SIZE) APP_TASK_START_STK_SIZE / 10,
                 (CPU_STK_SIZE) APP_TASK_START_STK_SIZE,
                 (OS_MSG_QTY  ) 5u,
                 (OS_TICK     ) 0u,
                 (void       *) 0,
                 (OS_OPT      )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
                 (OS_ERR     *)&err);
	
				 
    OSStart(&err);                                              /* Start multitasking (i.e. give control to uC/OS-III). */
		
		
}

/*********************************************/
/* 创建其他任务的 AppTaskStart 函数 */
static  void  AppTaskStart (void *p_arg)
{
    CPU_INT32U  cpu_clk_freq;
    CPU_INT32U  cnts;
    OS_ERR      err;

	CPU_SR_ALLOC();
	
   (void)p_arg;
	
    CPU_Init();
    BSP_Init();                                                 /* 外设都在这初始化                           */

    cpu_clk_freq = BSP_CPU_ClkFreq();                           /* Determine SysTick reference freq.                    */
    cnts = cpu_clk_freq / (CPU_INT32U)OSCfg_TickRate_Hz;        /* Determine nbr SysTick increments                     */
    OS_CPU_SysTickInit(cnts);                                   /* Init uC/OS periodic time src (SysTick).              */

    Mem_Init();                                                 /* Initialize Memory Management Module                  */

#if OS_CFG_STAT_TASK_EN > 0u
    OSStatTaskCPUUsageInit(&err);                               /* Compute CPU capacity with no task running            */
#endif

#ifdef CPU_CFG_INT_DIS_MEAS_EN
    CPU_IntDisMeasMaxCurReset();
#endif
#if	OS_CFG_SCHED_ROUND_ROBIN_EN  //当使用时间片轮转的时候
    
    OSSchedRoundRobinCfg(DEF_ENABLED, 1, &err);//使能时间片轮转调度功能,时间片长度为1个系统时钟节拍，既1*5=5ms
#endif

/**********************创建定时器*************************/
	//创建定时器1
	OSTmrCreate((OS_TMR		*)&tmr1,					//定时器1
                (CPU_CHAR	*)"tmr1",					//定时器名字
                (OS_TICK	 )0,						//不延时
                (OS_TICK	 )2,          				//3s
                (OS_OPT		 )OS_OPT_TMR_PERIODIC, 		//周期模式
                (OS_TMR_CALLBACK_PTR)tmr1_callback,		//定时器1回调函数
                (void	    *)0,						//参数为0
                (OS_ERR	    *)&err);					//返回的错误码		
	
	//创建定时器2	
	OSTmrCreate((OS_TMR		*)&tmr2,					//定时器2
                (CPU_CHAR	*)"tmr2",					//定时器名字
                (OS_TICK	 )2,						//延时0.1s
                (OS_TICK	 )1,          				//0.1s
                (OS_OPT		 )OS_OPT_TMR_PERIODIC,		//周期模式
                (OS_TMR_CALLBACK_PTR)tmr2_callback,		//定时器2回调函数
                (void	    *)0,						//参数为0
                (OS_ERR	    *)&err);					//返回的错误码	
				
/**********************创建信号量*************************/
	OS_CRITICAL_ENTER();	//进入临界区

//	OSSemCreate((OS_SEM* ) &SETTING_SEM,
//				(CPU_CHAR* )"setting_sem",
//				(OS_SEM_CTR)0,
//				(OS_ERR*   )&err);
	
/**********************创建任务*************************/

	/*LED闪烁*/
	OSTaskCreate((OS_TCB 	* )&LedTaskTCB,		
				 (CPU_CHAR	* )"LED task", 		
                 (OS_TASK_PTR )led_task, 			
                 (void		* )0,					
                 (OS_PRIO	  )LED_TASK_PRIO,     
                 (CPU_STK   * )&LED_TASK_STK[0],	
                 (CPU_STK_SIZE)LED_STK_SIZE/10,	
                 (CPU_STK_SIZE)LED_STK_SIZE,		
                 (OS_MSG_QTY  )0,					
                 (OS_TICK	  )0,					
                 (void   	* )0,					
                 (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR,
                 (OS_ERR 	* )&err);
				 
	/*创建LCD显示任务*/
	OSTaskCreate((OS_TCB 	* )&ShowTaskTCB,		
				 (CPU_CHAR	* )"SHOW task", 		
                 (OS_TASK_PTR )show_task, 			
                 (void		* )0,					
                 (OS_PRIO	  )SHOW_TASK_PRIO,     
                 (CPU_STK   * )&SHOW_TASK_STK[0],	
                 (CPU_STK_SIZE)SHOW_STK_SIZE/10,	
                 (CPU_STK_SIZE)SHOW_STK_SIZE,		
                 (OS_MSG_QTY  )0,					
                 (OS_TICK	  )0,					
                 (void   	* )0,					
                 (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR,
                 (OS_ERR 	* )&err);
				 
	//创建按键扫描任务
	OSTaskCreate((OS_TCB 	* )&KeyTaskTCB,		
				 (CPU_CHAR	* )"KEY task", 		
                 (OS_TASK_PTR )key_task, 			
                 (void		* )0,					
                 (OS_PRIO	  )KEY_TASK_PRIO,     
                 (CPU_STK   * )&KEY_TASK_STK[0],	
                 (CPU_STK_SIZE)KEY_STK_SIZE/10,	
                 (CPU_STK_SIZE)KEY_STK_SIZE,		
                 (OS_MSG_QTY  )0,					
                 (OS_TICK	  )0,					
                 (void   	* )0,					
                 (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR,
                 (OS_ERR 	* )&err);


/*******************************************************/
				 
	OSTmrStart(&tmr1,&err);					//定时器开启
	OSTmrStart(&tmr2,&err);			 
	OSTaskDel(& AppTaskStartTCB, & err);	//删除AppTaskStart任务自身
	OS_CRITICAL_EXIT();		//退出临界区
}

/************************任务函数****************************/
/* LED */
void led_task(void *p_arg)
{
	OS_ERR err;
	(void)p_arg;
	
	
	while(1)
	{
		if(OT > ST)			//炉温高于设置温度
		{
			LED_RED;		//红灯
		}
		else if(OT < ST)	//炉温低于设置温度
		{
			LED_BLUE;		//蓝灯
		}
		else				//炉温等于设置温度
		{
			LED_GREEN;		//绿灯
		}
		OSTimeDly ( 100, OS_OPT_TIME_DLY, & err );
		//OSTimeDlyHMSM(0,0,0,5,OS_OPT_TIME_PERIODIC,&err);   //延时5ms
	}
}
/*******************************************************/
/* OLED 显示 */
void show_task(void *p_arg)
{
	OS_ERR err;
	//CPU_SR_ALLOC();
	
	while(1)
	{
		//OS_CRITICAL_ENTER();	//进入临界区
		if(setting_flag == 1)	//默认显示模式
		{
			OLED_ShowString(1, 1, "RUNNING   ");
			OLED_ShowString(2, 1, "PV:        ");
			OLED_ShowString(3, 1, "RT:  ");
			OLED_ShowString(4, 1, "SV:  ");
			
			OLED_ShowNum(2, 6,OT,4);
			OLED_ShowNum(3, 6,RT,4);
			OLED_ShowNum(4, 6,ST,4);
		}
		else
		{
			OLED_ShowString(1, 1, "SETTING   ");
			OLED_ShowString(2, 1, "RG:0-1350 C");
			OLED_ShowString(3, 1, "          ");
			OLED_ShowString(4, 1, "SV:  ");
		
			OLED_ShowNum(4, 6,SET_Temp,4);
			//OSTimeDly ( 20, OS_OPT_TIME_DLY, & err );
			
			
		}
		
		//OS_CRITICAL_EXIT();		//退出临界区
		OSTimeDlyHMSM(0,0,0,50,OS_OPT_TIME_PERIODIC,&err);   //延时200ms
	}
}
/*******************************************************/
/* 按键函数 */
void key_task(void *p_arg)
{
	OS_ERR err;
	int key;
	(void)p_arg;

	
	while(1)
	{
		CPU_SR_ALLOC();
		OS_CRITICAL_ENTER();	//进入临界区
		
		key = Key_Scan();
		
		SET_Temp = ST;	//同步数值
		
		switch (key)
		{
			case 1:
				SET_Temp = SET_Temp - (SET_Temp / point) % 10 * point + 1 * point;
			break;
			case 2:
				SET_Temp = SET_Temp - (SET_Temp / point) % 10 * point + 2 * point;
			break;
			case 3:
				SET_Temp = SET_Temp - (SET_Temp / point) % 10 * point + 3 * point;
			break;
			case 4:
				SET_Temp = SET_Temp - (SET_Temp / point) % 10 * point + 4 * point;
			break;
			case 5:
				SET_Temp = SET_Temp - (SET_Temp / point) % 10 * point + 5 * point;
			break;
			case 6:
				SET_Temp = SET_Temp - (SET_Temp / point) % 10 * point + 6 * point;
			break;
			case 7:
				SET_Temp = SET_Temp - (SET_Temp / point) % 10 * point + 7 * point;
			break;
			case 8:
				SET_Temp = SET_Temp - (SET_Temp / point) % 10 * point + 8 * point;
			break;
			case 9:
				SET_Temp = SET_Temp - (SET_Temp / point) % 10 * point + 9 * point;
			break;
			case 0:
				SET_Temp = SET_Temp - (SET_Temp / point) % 10 * point;
			break;
			
			case 11:	//设置键
				setting_flag = -setting_flag;
				if(setting_flag == -1)
				{
					/* 使能串口不接收中断 */
					USART_ITConfig(DEBUG_USART, USART_IT_RXNE, DISABLE);
//					OSSemPost(&SETTING_SEM,OS_OPT_POST_1,&err);
				}else
				{
					printf("%03d",ST%1000);	//发送
					printf("%01d",ST/1000);	//发送
					USART_ITConfig(DEBUG_USART, USART_IT_RXNE, ENABLE);
				}
			break;
			
			case 12:	//光标位移
				point *= 10;
				point_sit--;
				if(point == 10000)
				{
					point = 1;
					point_sit = 4;
				}
			break;
			
		}
				
		if(SET_Temp >= MAX_TEMP)
		{
			SET_Temp = MAX_TEMP;
		}		
				
		//OLED_ShowNum(4, 6,SET_Temp,4);
		ST = SET_Temp;	//同步数值
		
		
		OSTimeDlyHMSM(0,0,0,50,OS_OPT_TIME_PERIODIC,&err);   //延时20ms
		OS_CRITICAL_EXIT();		//退出临界区
	}
}

/************************回调函数****************************/
/*定时器1的回调函数-----数据*/
void tmr1_callback(void *p_tmr, void *p_arg)
{
	CPU_SR_ALLOC();
	OS_CRITICAL_ENTER();	//进入临界区

	
	
	OS_CRITICAL_EXIT();		//退出临界区
}
/*定时器2的回调函数-----显示数据*/
void tmr2_callback(void *p_tmr, void *p_arg)
{
	if(setting_flag != 1)
	{
		OLED_ShowString(4, 5+point_sit, " ");	//光标闪烁

	}else
	{	
		CPU_SR_ALLOC();
		OS_CRITICAL_ENTER();	//进入临界区
		
		
		if(IR_receive == 1)//接收到数据
		{
			IR_receive = 0;
			SET_Temp = to_number(RxBuffer,8);			//取出 下位机的室内温度 （4位数）
			OT = to_number(RxBuffer,4);			//取出 下位机的炉内温度 （4位数）
			RT =  to_number(RxBuffer,0);	//取出 下位机的设置温度 （4位数）
			if(SET_Temp >= MAX_TEMP)				//超量程判断
				SET_Temp = MAX_TEMP;
			ST = SET_Temp;						//同步数值
			
		}
		OS_CRITICAL_EXIT();		//退出临界区
	}
}
/**********************中断服务函数*************************/
void DEBUG_USART_IRQHandler(void)
{

	if(USART_GetITStatus(DEBUG_USART,USART_IT_RXNE)!=RESET)
	{		
		OSIntEnter(); 	                                     //进入中断
			RxBuffer[RxCounter] = USART_ReceiveData( DEBUG_USART );
			RxCounter++;
			if(RxCounter >= 12)									//定义接收的字节个数
			{
				RxCounter = 0;
				IR_receive = 1;									//数据接收完成标志位

			}	
				
		OSIntExit();	                                       //退出中断		
	}	 
}	




