/*
 * @Author: Hui
 * @Date: 2022-05-27 16:47:20
 * @LastEditors: Hui
 * @LastEditTime: 2022-05-28 17:12:53
 * @Description: APP��������������ʹ�ú����������߼�����
 * 
 * Copyright (c) 2022 by HUI, All Rights Reserved. 
 */
#include <includes.h>

/*********************************************/
/*���񴴽�����*/
static  OS_TCB   AppTaskStartTCB;							//���ȼ�2
static  CPU_STK  AppTaskStartStk[APP_TASK_START_STK_SIZE];	//128u
static  void  AppTaskStart  (void *p_arg);
//////////////////////////////////////////////
/*����ɨ������*/
//�������ȼ�
#define KEY_TASK_PRIO		4
//�����ջ��С	
#define KEY_STK_SIZE 		512
//������ƿ�
static	OS_TCB KeyTaskTCB;
//�����ջ	
static	CPU_STK KEY_TASK_STK[KEY_STK_SIZE];
static	void	key_task(void *p_arg);

//////////////////////////////////////////////
/*LED��˸����*/
//�������ȼ�
#define LED_TASK_PRIO		7
//�����ջ��С
#define LED_STK_SIZE		128
//������ƿ�
static	OS_TCB	LedTaskTCB;
//�����ջ
static	CPU_STK	LED_TASK_STK[LED_STK_SIZE];
//������
static	void 	led_task(void *p_arg);
//////////////////////////////////////////////
/*OLED��ʾ����*/
//�������ȼ�
#define SHOW_TASK_PRIO		6
//�����ջ��С
#define SHOW_STK_SIZE		128
//������ƿ�
static	OS_TCB	ShowTaskTCB;
//�����ջ
static	CPU_STK	SHOW_TASK_STK[SHOW_STK_SIZE];
//������
static	void	show_task(void *p_arg);

//////////////////////////////////////////////
/* �����жϽ��� */
u8 RxCounter = 0;//�������ֵ
u8 RxBuffer[12];//���建����Ϊ12(���������¶�4λ��)
u8 TXBuffer[4];//���ͻ�����Ϊ4(����¯�º�����8λ��)
u8 IR_receive = 0;//���ձ�־
u16 TXtemp;

#define ASC2KB(x)	 (u8)(x%(0x30))	//ASCII ת10����
#define KB2ASC(x)	 (u8)(0x30+x)	//10����ת ASCII

//////////////////////////////////////////////
/*��ʱ��1--ADC����*/
OS_TMR tmr1;
void tmr1_callback(void *p_tmr, void *p_arg);
/*��ʱ��2--��ʱˢ����ʾ����*/
OS_TMR tmr2;
static	void tmr2_callback(void *p_tmr,void *p_arg);
///////////////////////////////////////////////

u16 to_number(u8 a[],u8 len)//����ת��������
{
    u16 num=0;
		for(u8 i= len;i<= 3+len;i++)
		{
			num=num*10+ASC2KB(a[i]);
		}
    
	return num;
}

///////////////////////////////////////////////
/* ȫ�ֱ��� */
u16 RT,OT;		//���£�¯��
u16 ST = 100;	//�����¶�

int setting_flag = 1;	//����ģʽ��־λ
u16 point = 1;				//���λ��(��ֵ��
u8 point_sit = 4;			//���λ��
u16 SET_Temp;			//���������¶ȵ���ʱ����
/*********************************************/
#define MAX_TEMP 1350	//����¶�

//OS_SEM SETTING_SEM;		//�����л�

/*********************************************/
/* main ���� */
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
/* ������������� AppTaskStart ���� */
static  void  AppTaskStart (void *p_arg)
{
    CPU_INT32U  cpu_clk_freq;
    CPU_INT32U  cnts;
    OS_ERR      err;

	CPU_SR_ALLOC();
	
   (void)p_arg;
	
    CPU_Init();
    BSP_Init();                                                 /* ���趼�����ʼ��                           */

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
#if	OS_CFG_SCHED_ROUND_ROBIN_EN  //��ʹ��ʱ��Ƭ��ת��ʱ��
    
    OSSchedRoundRobinCfg(DEF_ENABLED, 1, &err);//ʹ��ʱ��Ƭ��ת���ȹ���,ʱ��Ƭ����Ϊ1��ϵͳʱ�ӽ��ģ���1*5=5ms
#endif

/**********************������ʱ��*************************/
	//������ʱ��1
	OSTmrCreate((OS_TMR		*)&tmr1,					//��ʱ��1
                (CPU_CHAR	*)"tmr1",					//��ʱ������
                (OS_TICK	 )0,						//����ʱ
                (OS_TICK	 )2,          				//3s
                (OS_OPT		 )OS_OPT_TMR_PERIODIC, 		//����ģʽ
                (OS_TMR_CALLBACK_PTR)tmr1_callback,		//��ʱ��1�ص�����
                (void	    *)0,						//����Ϊ0
                (OS_ERR	    *)&err);					//���صĴ�����		
	
	//������ʱ��2	
	OSTmrCreate((OS_TMR		*)&tmr2,					//��ʱ��2
                (CPU_CHAR	*)"tmr2",					//��ʱ������
                (OS_TICK	 )2,						//��ʱ0.1s
                (OS_TICK	 )1,          				//0.1s
                (OS_OPT		 )OS_OPT_TMR_PERIODIC,		//����ģʽ
                (OS_TMR_CALLBACK_PTR)tmr2_callback,		//��ʱ��2�ص�����
                (void	    *)0,						//����Ϊ0
                (OS_ERR	    *)&err);					//���صĴ�����	
				
/**********************�����ź���*************************/
	OS_CRITICAL_ENTER();	//�����ٽ���

//	OSSemCreate((OS_SEM* ) &SETTING_SEM,
//				(CPU_CHAR* )"setting_sem",
//				(OS_SEM_CTR)0,
//				(OS_ERR*   )&err);
	
/**********************��������*************************/

	/*LED��˸*/
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
				 
	/*����LCD��ʾ����*/
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
				 
	//��������ɨ������
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
				 
	OSTmrStart(&tmr1,&err);					//��ʱ������
	OSTmrStart(&tmr2,&err);			 
	OSTaskDel(& AppTaskStartTCB, & err);	//ɾ��AppTaskStart��������
	OS_CRITICAL_EXIT();		//�˳��ٽ���
}

/************************������****************************/
/* LED */
void led_task(void *p_arg)
{
	OS_ERR err;
	(void)p_arg;
	
	
	while(1)
	{
		if(OT > ST)			//¯�¸��������¶�
		{
			LED_RED;		//���
		}
		else if(OT < ST)	//¯�µ��������¶�
		{
			LED_BLUE;		//����
		}
		else				//¯�µ��������¶�
		{
			LED_GREEN;		//�̵�
		}
		OSTimeDly ( 100, OS_OPT_TIME_DLY, & err );
		//OSTimeDlyHMSM(0,0,0,5,OS_OPT_TIME_PERIODIC,&err);   //��ʱ5ms
	}
}
/*******************************************************/
/* OLED ��ʾ */
void show_task(void *p_arg)
{
	OS_ERR err;
	//CPU_SR_ALLOC();
	
	while(1)
	{
		//OS_CRITICAL_ENTER();	//�����ٽ���
		if(setting_flag == 1)	//Ĭ����ʾģʽ
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
		
		//OS_CRITICAL_EXIT();		//�˳��ٽ���
		OSTimeDlyHMSM(0,0,0,50,OS_OPT_TIME_PERIODIC,&err);   //��ʱ200ms
	}
}
/*******************************************************/
/* �������� */
void key_task(void *p_arg)
{
	OS_ERR err;
	int key;
	(void)p_arg;

	
	while(1)
	{
		CPU_SR_ALLOC();
		OS_CRITICAL_ENTER();	//�����ٽ���
		
		key = Key_Scan();
		
		SET_Temp = ST;	//ͬ����ֵ
		
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
			
			case 11:	//���ü�
				setting_flag = -setting_flag;
				if(setting_flag == -1)
				{
					/* ʹ�ܴ��ڲ������ж� */
					USART_ITConfig(DEBUG_USART, USART_IT_RXNE, DISABLE);
//					OSSemPost(&SETTING_SEM,OS_OPT_POST_1,&err);
				}else
				{
					printf("%03d",ST%1000);	//����
					printf("%01d",ST/1000);	//����
					USART_ITConfig(DEBUG_USART, USART_IT_RXNE, ENABLE);
				}
			break;
			
			case 12:	//���λ��
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
		ST = SET_Temp;	//ͬ����ֵ
		
		
		OSTimeDlyHMSM(0,0,0,50,OS_OPT_TIME_PERIODIC,&err);   //��ʱ20ms
		OS_CRITICAL_EXIT();		//�˳��ٽ���
	}
}

/************************�ص�����****************************/
/*��ʱ��1�Ļص�����-----����*/
void tmr1_callback(void *p_tmr, void *p_arg)
{
	CPU_SR_ALLOC();
	OS_CRITICAL_ENTER();	//�����ٽ���

	
	
	OS_CRITICAL_EXIT();		//�˳��ٽ���
}
/*��ʱ��2�Ļص�����-----��ʾ����*/
void tmr2_callback(void *p_tmr, void *p_arg)
{
	if(setting_flag != 1)
	{
		OLED_ShowString(4, 5+point_sit, " ");	//�����˸

	}else
	{	
		CPU_SR_ALLOC();
		OS_CRITICAL_ENTER();	//�����ٽ���
		
		
		if(IR_receive == 1)//���յ�����
		{
			IR_receive = 0;
			SET_Temp = to_number(RxBuffer,8);			//ȡ�� ��λ���������¶� ��4λ����
			OT = to_number(RxBuffer,4);			//ȡ�� ��λ����¯���¶� ��4λ����
			RT =  to_number(RxBuffer,0);	//ȡ�� ��λ���������¶� ��4λ����
			if(SET_Temp >= MAX_TEMP)				//�������ж�
				SET_Temp = MAX_TEMP;
			ST = SET_Temp;						//ͬ����ֵ
			
		}
		OS_CRITICAL_EXIT();		//�˳��ٽ���
	}
}
/**********************�жϷ�����*************************/
void DEBUG_USART_IRQHandler(void)
{

	if(USART_GetITStatus(DEBUG_USART,USART_IT_RXNE)!=RESET)
	{		
		OSIntEnter(); 	                                     //�����ж�
			RxBuffer[RxCounter] = USART_ReceiveData( DEBUG_USART );
			RxCounter++;
			if(RxCounter >= 12)									//������յ��ֽڸ���
			{
				RxCounter = 0;
				IR_receive = 1;									//���ݽ�����ɱ�־λ

			}	
				
		OSIntExit();	                                       //�˳��ж�		
	}	 
}	




