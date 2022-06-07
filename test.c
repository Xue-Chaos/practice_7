/* ����ͷ�ļ� */
#include "ioCC2530.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
/* �ĸ�LED�ƵĶ˿ڶ��� */
#define D3 P1_0
#define D4 P1_1
#define D5 P1_3
#define D6 P1_4
/* �����Ķ˿ڶ��� */
#define SW1 P1_2


/* �������� */
int light_state=0;//��ˮ��״̬1��light_state=1    ��ˮ��״̬2��light_state=2

unsigned int counter=0;      //10ms��ʱ���жϼ���ֵ

/**********LED�˿ڳ�ʼ��************************/
void InitLED(void)
{
    P1SEL &=~0x1B;      //����D3 D4 D5 D6Ϊ��ͨIO�� 
    P1DIR |= 0x1B;      //����D3 D4 D5 D6Ϊ�������
    D3=0;
    D4=0;
    D5=0;
    D6=0;  
}

void InitSw(void)
{ 
    P1SEL &= ~0X04;       //����SW1Ϊ��ͨIO��
    P1DIR &= ~0X04;       //����SW1Ϊ��������    
    P0SEL&=~0X02;         //����SW2Ϊ��ͨIO��
    P0DIR&=~0X02;         //����SW2Ϊ��������
    
    P1INP &= ~0X04;       //����SW1Ϊ������ģʽ
    P2INP &= ~0x40;       //����SW1�����˿�Ϊ����    
    IEN2 |=0X10;          //ʹ��SW1�˿����ж�Դ     
    P1IEN|=0X04;          //ʹ��SW1�˿��ⲿ�ж�    
    PICTL|=0X02;          //�½��ش�����PICTL�жϱ�Ե�Ĵ���  
  
    /*.......������2 ��ʼ.......��ťSW2(P0_1)���óɲ����жϵķ�ʽ���п��ƣ�
    ��������ģʽ,�½��ش����ж�...................*/
    P0SEL &= ~0x02;
    P0DIR &= ~0x02;
    P0INP &= ~0x02;
    P2INP &= ~0x20;
    PICTL |= 0x01;
    IEN1 |= 0x20;
    P0IEN |= 0x02;
    
    /*.......������2 ����.......................................*/
 
}

/***********��ʱ����ʼ��************************/
void InittTimer1(void)
{
    T1IF=0;                  //���timer1�жϱ�־
    T1STAT &= ~0x01;         //���ͨ��0�жϱ�־
    
    T1CTL = 0x0A;            //����32��Ƶ��ģģʽ
    
    T1CCTL0 |= 0x04;         //�趨timer1ͨ��0�Ƚ�ģʽ����ʱ10ms
    T1CC0L = 10000&0xFF;     //��10000�ĵ�8λд��T1CC0L
    T1CC0H = (10000>>8)&0xFF;//��10000�ĸ�8λд��T1CC0H
    
    T1IE = 1;                //ʹ�ܶ�ʱ��1���жϣ�����дΪIEN1 |= 0x02;
}

/**********����ͨ�ų�ʼ��************************/
void InitUART0(void)
{

}

/*************** �����ڷ���ָ�����ȵ�����  ***************/
void uart_tx_string(char *data_tx)  
{   
  while(*data_tx!='\0')
  {   
    U0DBUF = *data_tx; // ��Ҫ���͵�1�ֽ�����д��U0DBUF
    while(UTX0IF == 0);  // �ȴ�TX�жϱ�־����U0DBUF����
    UTX0IF = 0;          // ����TX�жϱ�־
    data_tx++;
  }
}

void uart_tx_byte(char data_tx)  
{   
    U0DBUF = data_tx;   // ��Ҫ���͵�1�ֽ�����д��U0DBUF
    while(UTX0IF == 0); // �ȴ�TX�жϱ�־����U0DBUF����
    UTX0IF = 0;         // ����TX�жϱ�־
} 

/************��ʱ��T1�жϷ����ӳ���**************/
#pragma vector = T1_VECTOR //�жϷ����ӳ���
__interrupt void T1_ISR(void)
{   
    if(counter>0)
      counter--;
}
/************����SW1�жϷ����ӳ���**************/
#pragma vector=P1INT_VECTOR //��P1���ж�
__interrupt void EXTI1_ISR()
{
    counter = 0;//����
    D3 = D4 = D5 = D6 = 0;
    
    if(P1IFG & 0X04) //����SW1����ʱ
    {
        if(SW1==0)//ȷʵ��SW1��ť�������ⲿ�ж�
          light_state = 1;
    }
  
    //ע������ж�ʱ�������ֵ��Ϊ1��ִ�����жϺ���ؼǵ�����Ĳ�����Ҫ��0
    //�жϱ�־λ��0
    P1IFG&=~(0X1<<2);//��SW1�жϱ�־
    IRCON2&=~(0x1<<3);  //��P1�˿����жϱ�־ ��3λΪ0�����˿�1���жϱ�־λ�����  P1IF=0;
}

/************����SW2�жϷ����ӳ���**************/
#pragma vector=P0INT_VECTOR //��P0���ж�
__interrupt void EXTI2_ISR()
{
    counter = 0;//����
    D3 = D4 = D5 = D6 = 0;
    
    if(P0IFG & 0X02) //����SW2����ʱ
    {
        if(SW2==0)//ȷʵ��SW2��ť�������ⲿ�ж�
          light_state = 2;
    }
  
    //ע������ж�ʱ�������ֵ��Ϊ1��ִ�����жϺ���ؼǵ�����Ĳ�����Ҫ��0
    //�жϱ�־λ��0
    P0IFG&=~(0X1<<1);//��SW2�жϱ�־
    IRCON&=~(0x1<<5);  //��P0�˿����жϱ�־ ��5λΪ0�����˿�0���жϱ�־λ�����  P0IF=0;
}

/************main�������**************************/
void main(void)
{
    //ʱ�ӳ�ʼ��,�ٶ�����Ϊ32MHz
    CLKCONCMD &= 0X80;
    while(CLKCONSTA&0X40);
    
    InitLED();
    InitSw();
    InittTimer1();      //��ʼ��Timer1
    InitUART0();        //UART0��ʼ��

    EA = 1;             //ʹ��ȫ���ж�
    
    /*.......������1��ʼ��D3,D4,D5,D6��ȫ��1���ȫ��...*/


     /*.......������1����...***********************************/
        
    while(1)
    {
      
      if(light_state == 1)//���°���SW1
      {
      /*.......������4 ��ʼ******************************/
      /*�ɿ������ð��°���SW1��:
          ��1������D4->D3->D6->D5��˳��,ÿ��1�����ε���һ����,
          ��D5�����֮�󣬵ȴ�1���˳��ߵ�������
          ��2��������D5->D6->D3->D4��˳��ÿ��1�����ε���һ����,
          ��D4�����֮�󣬵ȴ�1����ٴεߵ���
          �������һֱ��������������ѭ�����С�
          ��3��ÿ��ѭ����һ��֮�󣬷��͡���ˮ��1��ɡ������Դ�����ʾ��
      */
        //D4->D3->D6->D5
        
        
        //D5->D6->D3->D4
       
        
        //���͵�������ʾ
     
        
      /*.......������4����...***********************************/
      }
      else if(light_state == 2)//���°���SW2
      {
      /*.......������5 ��ʼ.........*****************************/
      /*�ɿ������ð��°���SW2��:
          ��1������˳��Ϊ����D4,D6ͬʱ����������2�룬Ȼ�������
          ����D3,D5ͬʱ����������2�룬Ȼ�������
          ��1���4����ͬʱ����������2�룬֮�����������1�룻
          ��2�����ž�������D3,D5ͬʱ����������2�룬Ȼ�������
          ����D4,D6ͬʱ����������2�룬Ȼ�������
          ��1���4����ͬʱ����������2�룬֮�����
          ��3��ÿ��ѭ����һ��֮�󣬷��͡���ˮ��2��ɡ������Դ�����ʾ��
      */
        //D4,D6����
        
        //D3,D5���� 
        
        //���͵�������ʾ
        
       /*.......������5����...***********************************/
        
        light_state = 0;//����
      }
    }
}