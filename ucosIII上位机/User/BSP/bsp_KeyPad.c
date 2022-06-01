/*
 * @Author: Hui
 * @Date: 2022-05-27 19:15:15
 * @LastEditors: Hui
 * @LastEditTime: 2022-05-28 15:29:21
 * @Description: GPIO���󰴼����������óɲ�������IO��
 * 
 * Copyright (c) 2022 by HUI, All Rights Reserved. 
 */

#include "bsp_KEYPAD.h"

//8������ 4��Ϊ�� 4��Ϊ��
//������˿ڶ���
#define X1_GPIO_PORT GPIOD           
#define X2_GPIO_PORT GPIOD   
#define X3_GPIO_PORT GPIOD         
#define X4_GPIO_PORT GPIOD
//������˿ڶ���
#define Y1_GPIO_PORT GPIOD         
#define Y2_GPIO_PORT GPIOD 
#define Y3_GPIO_PORT GPIOD         
#define Y4_GPIO_PORT GPIOD

//��������Ŷ���
#define X1_GPIO_PIN GPIO_Pin_4
#define X2_GPIO_PIN GPIO_Pin_5
#define X3_GPIO_PIN GPIO_Pin_6
#define X4_GPIO_PIN GPIO_Pin_7

//���������Ŷ���
#define Y1_GPIO_PIN GPIO_Pin_0
#define Y2_GPIO_PIN GPIO_Pin_1
#define Y3_GPIO_PIN GPIO_Pin_2
#define Y4_GPIO_PIN GPIO_Pin_3

//�����ʱ�Ӷ���
#define X1_RCC RCC_AHB1Periph_GPIOD
#define X2_RCC RCC_AHB1Periph_GPIOD
#define X3_RCC RCC_AHB1Periph_GPIOD
#define X4_RCC RCC_AHB1Periph_GPIOD

//������ʱ�Ӷ���
#define Y1_RCC RCC_AHB1Periph_GPIOD
#define Y2_RCC RCC_AHB1Periph_GPIOD
#define Y3_RCC RCC_AHB1Periph_GPIOD
#define Y4_RCC RCC_AHB1Periph_GPIOD


/// ����ȷ����ʱ
void key_Delay(__IO u32 nCount)
{
	for(; nCount != 0; nCount--);
} 


//����������õ�8�����ſ������ɲ���������ʵ����Ҫ

unsigned char Y1,Y2,Y3,Y4;
void Key_Init(void)
{
   GPIO_InitTypeDef GPIO_InitStructure;   
   RCC_AHB1PeriphClockCmd(X1_RCC|X2_RCC|X3_RCC|X4_RCC|Y1_RCC|Y2_RCC|Y3_RCC|Y4_RCC, ENABLE);
   //GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);/*!< JTAG-DP Disabled and SW-DP Enabled */
	

       
/*****************************4�����*********************************************/
   GPIO_InitStructure.GPIO_Pin =  X1_GPIO_PIN ;
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_OUT;         //����IO��ģʽ
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;    //50MHz
	GPIO_InitStructure.GPIO_OType=GPIO_OType_PP;				//����
	GPIO_InitStructure.GPIO_PuPd=GPIO_PuPd_UP;					//����

	
   GPIO_Init(X1_GPIO_PORT, &GPIO_InitStructure);
   
   GPIO_InitStructure.GPIO_Pin =  X2_GPIO_PIN ;
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_OUT;         //����IO��ģʽ
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;    //50MHz
	GPIO_InitStructure.GPIO_OType=GPIO_OType_PP;				//����
	GPIO_InitStructure.GPIO_PuPd=GPIO_PuPd_UP;					//����
   GPIO_Init(X2_GPIO_PORT, &GPIO_InitStructure);
   
   GPIO_InitStructure.GPIO_Pin =  X3_GPIO_PIN ;
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_OUT;         //����IO��ģʽ
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;    //50MHz
	GPIO_InitStructure.GPIO_OType=GPIO_OType_PP;				//����
	GPIO_InitStructure.GPIO_PuPd=GPIO_PuPd_UP;					//����
   GPIO_Init(X3_GPIO_PORT, &GPIO_InitStructure);
       
   GPIO_InitStructure.GPIO_Mode=GPIO_Mode_OUT;         //����IO��ģʽ
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;    //50MHz
	GPIO_InitStructure.GPIO_OType=GPIO_OType_PP;				//����
	GPIO_InitStructure.GPIO_PuPd=GPIO_PuPd_UP;	
   GPIO_InitStructure.GPIO_Pin = X4_GPIO_PIN ;   
   GPIO_Init(X4_GPIO_PORT, &GPIO_InitStructure);
   
/**************************************4������*************************************/
   GPIO_InitStructure.GPIO_Pin =  Y1_GPIO_PIN ;   
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
   GPIO_InitStructure.GPIO_PuPd 	= GPIO_PuPd_DOWN;//����   
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
   GPIO_Init(Y1_GPIO_PORT, &GPIO_InitStructure);       
   
   GPIO_InitStructure.GPIO_Pin =  Y2_GPIO_PIN ;   
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
   GPIO_InitStructure.GPIO_PuPd 	= GPIO_PuPd_DOWN;//����          
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
   GPIO_Init(Y2_GPIO_PORT, &GPIO_InitStructure);       
   
   GPIO_InitStructure.GPIO_Pin =  Y3_GPIO_PIN ;   
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
   GPIO_InitStructure.GPIO_PuPd 	= GPIO_PuPd_DOWN;//����          
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
   GPIO_Init(Y3_GPIO_PORT, &GPIO_InitStructure);       
       
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
   GPIO_InitStructure.GPIO_PuPd 	= GPIO_PuPd_DOWN;//����          
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
   GPIO_InitStructure.GPIO_Pin = Y4_GPIO_PIN;      
   GPIO_Init(Y4_GPIO_PORT, &GPIO_InitStructure);       
}

int Key_Scan(void)
{
   unsigned char KeyVal;
   GPIO_SetBits(X1_GPIO_PORT,X1_GPIO_PIN);  //����X1�����
   GPIO_SetBits(X2_GPIO_PORT,X2_GPIO_PIN);  //����X2�����
   GPIO_SetBits(X3_GPIO_PORT,X3_GPIO_PIN);  //����X3�����
   GPIO_SetBits(X4_GPIO_PORT,X4_GPIO_PIN);  //����X4�����


        if((GPIO_ReadInputDataBit(Y1_GPIO_PORT,Y1_GPIO_PIN)|GPIO_ReadInputDataBit(Y2_GPIO_PORT,Y2_GPIO_PIN)|
			GPIO_ReadInputDataBit(Y3_GPIO_PORT,Y3_GPIO_PIN)|GPIO_ReadInputDataBit(Y4_GPIO_PORT,Y4_GPIO_PIN))==0x0000)  
        return 15; //���X1��X4ȫΪ����û�а�������  
         else
         {       
            //key_Delay(10);    //��ʱȥ����
         if((GPIO_ReadInputDataBit(Y1_GPIO_PORT,Y1_GPIO_PIN)|GPIO_ReadInputDataBit(Y2_GPIO_PORT,Y2_GPIO_PIN)|
			 GPIO_ReadInputDataBit(Y3_GPIO_PORT,Y3_GPIO_PIN)|GPIO_ReadInputDataBit(Y4_GPIO_PORT,Y4_GPIO_PIN))==0x0000)
            return 15;
         }
         
     GPIO_ResetBits(X1_GPIO_PORT,X1_GPIO_PIN);
     GPIO_ResetBits(X2_GPIO_PORT,X2_GPIO_PIN);
     GPIO_ResetBits(X3_GPIO_PORT,X3_GPIO_PIN);
     GPIO_SetBits(X4_GPIO_PORT,X4_GPIO_PIN);
     
    Y1=GPIO_ReadInputDataBit(Y1_GPIO_PORT,Y1_GPIO_PIN);Y2=GPIO_ReadInputDataBit(Y2_GPIO_PORT,Y2_GPIO_PIN);
    Y3=GPIO_ReadInputDataBit(Y3_GPIO_PORT,Y3_GPIO_PIN);Y4=GPIO_ReadInputDataBit(Y4_GPIO_PORT,Y4_GPIO_PIN);
     if(Y1==1&&Y2==0&&Y3==0&&Y4==0)
            KeyVal=11;	//*
     if(Y1==0&&Y2==1&&Y3==0&&Y4==0)
            KeyVal=0;
     if(Y1==0&&Y2==0&&Y3==0&&Y4==1)
            KeyVal='D';
     if(Y1==0&&Y2==0&&Y3==1&&Y4==0)
            KeyVal=12;	//#
   
     while(((GPIO_ReadInputDataBit(Y1_GPIO_PORT,Y1_GPIO_PIN))|(GPIO_ReadInputDataBit(Y2_GPIO_PORT,Y2_GPIO_PIN))|
		 (GPIO_ReadInputDataBit(Y3_GPIO_PORT,Y3_GPIO_PIN))|(GPIO_ReadInputDataBit(Y4_GPIO_PORT,Y4_GPIO_PIN))) > 0);
    //�ȴ������ͷ�
     GPIO_SetBits(X1_GPIO_PORT,X1_GPIO_PIN);
     GPIO_ResetBits(X2_GPIO_PORT,X2_GPIO_PIN);
     GPIO_ResetBits(X3_GPIO_PORT,X3_GPIO_PIN);
     GPIO_ResetBits(X4_GPIO_PORT,X4_GPIO_PIN);
   
    Y1=GPIO_ReadInputDataBit(Y1_GPIO_PORT,Y1_GPIO_PIN);Y2=GPIO_ReadInputDataBit(Y2_GPIO_PORT,Y2_GPIO_PIN);
    Y3=GPIO_ReadInputDataBit(Y3_GPIO_PORT,Y3_GPIO_PIN);Y4=GPIO_ReadInputDataBit(Y4_GPIO_PORT,Y4_GPIO_PIN);
     if(Y1==1&&Y2==0&&Y3==0&&Y4==0)
            KeyVal=1;
     if(Y1==0&&Y2==1&&Y3==0&&Y4==0)
            KeyVal=2;
     if(Y1==0&&Y2==0&&Y3==1&&Y4==0)
            KeyVal=3;
     if(Y1==0&&Y2==0&&Y3==0&&Y4==1)
            KeyVal='A';
      
      while(((GPIO_ReadInputDataBit(Y1_GPIO_PORT,Y1_GPIO_PIN))|(GPIO_ReadInputDataBit(Y2_GPIO_PORT,Y2_GPIO_PIN))|
		  (GPIO_ReadInputDataBit(Y3_GPIO_PORT,Y3_GPIO_PIN))|(GPIO_ReadInputDataBit(Y4_GPIO_PORT,Y4_GPIO_PIN))) > 0);
               

     GPIO_ResetBits(X1_GPIO_PORT,X1_GPIO_PIN);
     GPIO_SetBits(X2_GPIO_PORT,X2_GPIO_PIN);
     GPIO_ResetBits(X3_GPIO_PORT,X3_GPIO_PIN);
     GPIO_ResetBits(X4_GPIO_PORT,X4_GPIO_PIN);
        
     Y1=GPIO_ReadInputDataBit(Y1_GPIO_PORT,Y1_GPIO_PIN);Y2=GPIO_ReadInputDataBit(Y2_GPIO_PORT,Y2_GPIO_PIN);
     Y3=GPIO_ReadInputDataBit(Y3_GPIO_PORT,Y3_GPIO_PIN);Y4=GPIO_ReadInputDataBit(Y4_GPIO_PORT,Y4_GPIO_PIN);
     if(Y1==1&&Y2==0&&Y3==0&&Y4==0)
            KeyVal=4;
     if(Y1==0&&Y2==1&&Y3==0&&Y4==0)
            KeyVal=5;
     if(Y1==0&&Y2==0&&Y3==1&&Y4==0)
            KeyVal=6;
     if(Y1==0&&Y2==0&&Y3==0&&Y4==1)
            KeyVal='B';
   
      while(((GPIO_ReadInputDataBit(Y1_GPIO_PORT,Y1_GPIO_PIN))|(GPIO_ReadInputDataBit(Y2_GPIO_PORT,Y2_GPIO_PIN))|
		  (GPIO_ReadInputDataBit(Y3_GPIO_PORT,Y3_GPIO_PIN))|(GPIO_ReadInputDataBit(Y4_GPIO_PORT,Y4_GPIO_PIN))) > 0);
               
     GPIO_ResetBits(X1_GPIO_PORT,X1_GPIO_PIN);
     GPIO_ResetBits(X2_GPIO_PORT,X2_GPIO_PIN);
     GPIO_SetBits(X3_GPIO_PORT,X3_GPIO_PIN);
     GPIO_ResetBits(X4_GPIO_PORT,X4_GPIO_PIN);   

     Y1=GPIO_ReadInputDataBit(Y1_GPIO_PORT,Y1_GPIO_PIN);Y2=GPIO_ReadInputDataBit(Y2_GPIO_PORT,Y2_GPIO_PIN);
     Y3=GPIO_ReadInputDataBit(Y3_GPIO_PORT,Y3_GPIO_PIN);Y4=GPIO_ReadInputDataBit(Y4_GPIO_PORT,Y4_GPIO_PIN);
     if(Y1==1&&Y2==0&&Y3==0&&Y4==0)
            KeyVal=7;
     if(Y1==0&&Y2==1&&Y3==0&&Y4==0)
            KeyVal=8;
     if(Y1==0&&Y2==0&&Y3==1&&Y4==0)
            KeyVal=9;
     if(Y1==0&&Y2==0&&Y3==0&&Y4==1)
            KeyVal='C';
	  while(((GPIO_ReadInputDataBit(Y1_GPIO_PORT,Y1_GPIO_PIN))|(GPIO_ReadInputDataBit(Y2_GPIO_PORT,Y2_GPIO_PIN))|
		  (GPIO_ReadInputDataBit(Y3_GPIO_PORT,Y3_GPIO_PIN))|(GPIO_ReadInputDataBit(Y4_GPIO_PORT,Y4_GPIO_PIN))) > 0);
          
	  return KeyVal;

}

/************************************
��������Ϊ��             |1  2  3| A
                         |4  5  6| B
                         |7  8  9| C
                         |*  0  #| D
************************************/
