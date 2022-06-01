/*
 * @Author: Hui
 * @Date: 2022-05-26 12:54:19
 * @LastEditors: Hui
 * @LastEditTime: 2022-05-28 22:48:59
 * @Description: 
 * 
 * Copyright (c) 2022 by HUI, All Rights Reserved. 
 */
#ifndef __KEY_H
#define	__KEY_H

#include "stm32f4xx.h"

/*����ķ�ʽ��ͨ��ֱ�Ӳ����⺯����ʽ��ȡIO*/
#define KEY0 		GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_0) //PB0
#define KEY1 		GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_1)	//PB1
#define KEY2 		GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_2) //PB2
#define KEY3 		GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_3) //PB3


/*���淽ʽ��ͨ��λ��������ʽ��ȡIO*/
/*
#define KEY0 		PEin(4)   	//PE4
#define KEY1 		PEin(3)		//PE3 
#define KEY2 		PEin(2)		//P32
#define WK_UP 	PAin(0)		//PA0
*/


#define KEY0_PRES 	1
#define KEY1_PRES	2
#define KEY2_PRES	3
#define KEY3_PRES	4

void Key_GPIO_Config(void);	//IO��ʼ��
u8 KEY_Scan(u8);  		//����ɨ�躯��	

#endif /* __LED_H */

