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
#define KEY_STK_SIZE 		128
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
u8 RxBuffer[4];//���建����Ϊ4(���������¶�4λ��)
u8 TXBuffer[8];//���ͻ�����Ϊ8(����¯�º�����8λ��)
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
	u8 temp;
	
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
    cnts = cpu_clk_freq / (CPU_INT32U)OSCfg_TickRate_Hz;        /*ϵͳ��������ΪOS_CFG_TICK_RATE_HZ���ú���os_app_cfg.h���ж��壬Ĭ����1000�� ��ôϵͳ��ʱ�ӽ������ھ�Ϊ1ms                    */
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
                (OS_TICK	 )0,						//����ʱ��10HZʱ����ϵͳʱ��Ƶ�ʣ�Ҳ���Ǳ���OS_CFG_TMR_TASK_RATE_HZ��ֵ�������� HzΪ��λ�ġ� ��������ʱ�������Ƶ�ʣ�OS_CFG_TMR_TASK_RATE_HZ������Ϊ10Hz��
                (OS_TICK	 )1,          				//0.1s���ڶ�ʱ���ؽ�����
                (OS_OPT		 )OS_OPT_TMR_PERIODIC, 		//����ģʽ
                (OS_TMR_CALLBACK_PTR)tmr1_callback,		//��ʱ��1�ص�����
                (void	    *)0,						//����Ϊ0
                (OS_ERR	    *)&err);					//���صĴ�����		
	
	//������ʱ��2	
	OSTmrCreate((OS_TMR		*)&tmr2,					//��ʱ��2
                (CPU_CHAR	*)"tmr2",					//��ʱ������
                (OS_TICK	 )2,						//��ʱ
                (OS_TICK	 )1,          				//0.1s���ڶ�ʱ���ؽ�����
                (OS_OPT		 )OS_OPT_TMR_PERIODIC,		//����ģʽ
                (OS_TMR_CALLBACK_PTR)tmr2_callback,		//��ʱ��2�ص�����
                (void	    *)0,						//����Ϊ0
                (OS_ERR	    *)&err);					//���صĴ�����	
				
/**********************�����ź���*************************/
	OS_CRITICAL_ENTER();	//�����ٽ���
	
/**********************��������*************************/

	/*LED��˸*/
	OSTaskCreate((OS_TCB 	* )&LedTaskTCB,		
				 (CPU_CHAR	* )"LED task", 		
                 (OS_TASK_PTR )led_task, 			
                 (void		* )0,					
                 (OS_PRIO	  )LED_TASK_PRIO,     
                 (CPU_STK   * )&LED_TASK_STK[0],	
                 (CPU_STK_SIZE)LED_STK_SIZE/10,			//����ջ��ȵ�����λ�á����ֵ��ʾ�����ջ����֮ǰʣ���ջ����
                 (CPU_STK_SIZE)LED_STK_SIZE,		
                 (OS_MSG_QTY  )0,						//���Է��͵�����������Ϣ��		
                 (OS_TICK	  )0,						//����֮��ѭ��ʱ��ʱ��Ƭ��ʱ����,0Ĭ��ֵ
                 (void   	* )0,						//���û��ṩ���ڴ�λ�õ�ָ��,����������չ
                 (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR,	//�����ض�ѡ��(���������ջ���|���񴴽�ʱ���ջ)
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
		OSTimeDly ( 100, OS_OPT_TIME_DLY, & err );			//���ģʽ��100��ʱ�������
		//OSTimeDlyHMSM(0,0,0,5,OS_OPT_TIME_PERIODIC,&err);   //��ʱ5ms
	}
}
/*******************************************************/
/* OLED ��ʾ */
void show_task(void *p_arg)
{
	OS_ERR err;
	while(1)
	{
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
		
		
		OSTimeDlyHMSM(0,0,0,20,OS_OPT_TIME_PERIODIC,&err);   //��ʱ200ms
	}
}
/*******************************************************/
/* �������� */
void key_task(void *p_arg)
{
	OS_ERR err;
	u8 key;
	(void)p_arg;
	
	while(1)
	{
		key = KEY_Scan(0);
		
		SET_Temp = ST;	//ͬ����ֵ
		
		switch(key)
		{
			case KEY0_PRES:     //��key0���µĻ��򿪷��������ź���
				//��������ģʽʱ�������պ��������𲿷ֿ��Էŵ���ʱ��tmr2���
//				OSSemPost(&SETTING_SEM,OS_OPT_POST_1,&err);
				setting_flag = -setting_flag;	//��תsetting_flag
				if(setting_flag == -1)
				{
					/* ʹ�ܴ��ڲ������ж� */
					USART_ITConfig(DEBUG_USART, USART_IT_RXNE, DISABLE);
				}else
				{
					USART_ITConfig(DEBUG_USART, USART_IT_RXNE, ENABLE);
				}
				//printf("\r\n KEY0");
				break;
			case KEY1_PRES:	
				
				point *= 10;
				point_sit--;
				if(point == 10000)
				{
					point = 1;
					point_sit = 4;
				}
				//printf("\r\n point:%d",point);
				break;
			case KEY2_PRES:
				
				SET_Temp += point;
				if(SET_Temp >= 1350)
				{
					SET_Temp = 1350;
				}
				//printf("\r\n SET_Temp:%d",SET_Temp);
				break;
			case KEY3_PRES:
				
				SET_Temp -= point;
				if(SET_Temp <= 0)
				{
					SET_Temp = 0;
				}
//				printf("\r\n SET_Temp:%d",SET_Temp);
				break;
		}
		
		ST = SET_Temp;	//ͬ����ֵ
		
		OSTimeDlyHMSM(0,0,0,50,OS_OPT_TIME_PERIODIC,&err);   //��ʱ50ms
	}
}

/************************�ص�����****************************/
/*��ʱ��1�Ļص�����-----��ȡ����*/
void tmr1_callback(void *p_tmr, void *p_arg)
{
	CPU_SR_ALLOC();
	OS_CRITICAL_ENTER();	//�����ٽ���
	RT = Get_Adc_Average(ADC_Channel_16,10);		//10������ȡƽ��
	OT = Get_Adc_Average(ADC_Channel_1,10)*0.3297;	//10��¯��ȡƽ��
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
			SET_Temp = to_number(RxBuffer,0);	//ȡ�� ��λ���������¶� ��4λ����
			if(SET_Temp >= 1350)				//�������ж�
				SET_Temp = 1350;
			ST = SET_Temp;						//ͬ����ֵ
			
		}

			printf("%04d",RT);
			printf("%04d",OT);
			printf("%04d",ST);
		

		
		//printf("SET:%d\n\r",SET_Temp);
		
		OS_CRITICAL_EXIT();		//�˳��ٽ���
	}
}
/**********************�жϷ�����*************************/
void DEBUG_USART_IRQHandler(void)
{
//  uint8_t ucTemp;
	if(USART_GetITStatus(DEBUG_USART,USART_IT_RXNE)!=RESET)
	{		
		OSIntEnter(); 	                                     //�����ж�
		RxBuffer[RxCounter] = USART_ReceiveData( DEBUG_USART );
		RxCounter++;
		if(RxCounter >= 4)									//������յ��ֽڸ���
		{
			RxCounter = 0;
			IR_receive = 1;									//���ݽ�����ɱ�־λ
		}			
		OSIntExit();	                                       //�˳��ж�		
	}	 
}	




