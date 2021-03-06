/* 包含头文件 */
#include "ioCC2530.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
/* 四个LED灯的端口定义 */
#define D3 P1_0
#define D4 P1_1
#define D5 P1_3
#define D6 P1_4
/* 按键的端口定义 */
#define SW1 P1_2
#define SW2 P0_1

/* 变量定义 */
int light_state=0;//流水灯状态1：light_state=1    流水灯状态2：light_state=2

unsigned int counter=0;      //10ms定时器中断计数值

/**********LED端口初始化************************/
void InitLED(void)
{
    P1SEL &=~0x1B;      //设置D3 D4 D5 D6为普通IO口 
    P1DIR |= 0x1B;      //设置D3 D4 D5 D6为输出方向
    D3=0;
    D4=0;
    D5=0;
    D6=0;  
}

void InitSw(void)
{ 
    P1SEL &= ~0X04;       //设置SW1为普通IO口
    P1DIR &= ~0X04;       //设置SW1为输入引脚    
    P0SEL&=~0X02;         //设置SW2为普通IO口
    P0DIR&=~0X02;         //设置SW2为输入引脚
    
    P1INP &= ~0X04;       //设置SW1为上下拉模式
    P2INP &= ~0x40;       //设置SW1所属端口为上拉    
    IEN2 |=0X10;          //使能SW1端口组中断源     
    P1IEN|=0X04;          //使能SW1端口外部中断    
    PICTL|=0X02;          //下降沿触发；PICTL中断边缘寄存器  
  
    /*.......答题区2 开始.......按钮SW2(P0_1)配置成采用中断的方式进行控制，
    上拉输入模式,下降沿触发中断...................*/

    P0INP &= ~0X02;       //设置SW2为上下拉模式
    P2INP &= ~0x20;       //设置SW2所属端口为上拉
    IEN1 |=0X20;          //使能SW2/P0端口组中断源     
    P0IEN|=0X02;          //使能SW2端口外部中断    
    PICTL|=0X01;          //下降沿触发；PICTL中断边缘寄存器  
    
    /*.......答题区2 结束.......................................*/
 
}

/***********定时器初始化************************/
void InittTimer1(void)
{
    T1IF=0;                  //清除timer1中断标志
    T1STAT &= ~0x01;         //清除通道0中断标志
    
    T1CTL = 0x0A;            //配置32分频，模模式
    
    T1CCTL0 |= 0x04;         //设定timer1通道0比较模式，定时10ms
    T1CC0L = 10000&0xFF;     //把10000的低8位写入T1CC0L
    T1CC0H = (10000>>8)&0xFF;//把10000的高8位写入T1CC0H
    
    T1IE = 1;                //使能定时器1的中断，或者写为IEN1 |= 0x02;
}

/**********串口通信初始化************************/
void InitUART0(void)
{
  /*.......答题区3开始：串口设置....................*/
   
    PERCFG = 0x00; //usart0 使用备用位置1 TX-P0.3 RX-P0.2  
    P2DIR &= ~0xC0;//P0优先作为UART方式  
    P0SEL |= 0x0C; //P0.2 P0.3 用于外设功能
    
    U0CSR |= 0x80; //uart模式 允许发送，禁止接收
    
    U0BAUD = 59;//波特率：9600
    U0GCR = 8; 
    
    U0UCR |= 0x80; //流控无 无奇偶校验 8位数据位  1位停止位
    
  /*.......答题区3结束.......................................*/
}

/*************** 往串口发送指定长度的数据  ***************/
void uart_tx_string(char *data_tx)  
{   
  while(*data_tx!='\0')
  {   
    U0DBUF = *data_tx; // 将要发送的1字节数据写入U0DBUF
    while(UTX0IF == 0);  // 等待TX中断标志，即U0DBUF就绪
    UTX0IF = 0;          // 清零TX中断标志
    data_tx++;
  }
}

void uart_tx_byte(char data_tx)  
{   
    U0DBUF = data_tx;   // 将要发送的1字节数据写入U0DBUF
    while(UTX0IF == 0); // 等待TX中断标志，即U0DBUF就绪
    UTX0IF = 0;         // 清零TX中断标志
} 

/************定时器T1中断服务子程序**************/
#pragma vector = T1_VECTOR //中断服务子程序
__interrupt void T1_ISR(void)
{   
    if(counter>0)
      counter--;
}
/************按键SW1中断服务子程序**************/
#pragma vector=P1INT_VECTOR //第P1组中断
__interrupt void EXTI1_ISR()
{
    counter = 0;//清零
    D3 = D4 = D5 = D6 = 0;
    
    if(P1IFG & 0X04) //按键SW1按下时
    {
        if(SW1==0)//确实是SW1按钮触发了外部中断
          light_state = 1;
    }
  
    //注意产生中断时会把以下值变为1，执行完中断后务必记得下面的操作：要清0
    //中断标志位清0
    P1IFG&=~(0X1<<2);//清SW1中断标志
    IRCON2&=~(0x1<<3);  //清P1端口组中断标志 第3位为0代表端口1组中断标志位被清除  P1IF=0;
}

/************按键SW2中断服务子程序**************/
#pragma vector=P0INT_VECTOR //第P0组中断
__interrupt void EXTI2_ISR()
{
    counter = 0;//清零
    D3 = D4 = D5 = D6 = 0;
    
    if(P0IFG & 0X02) //按键SW2按下时
    {
        if(SW2==0)//确实是SW2按钮触发了外部中断
          light_state = 2;
    }
  
    //注意产生中断时会把以下值变为1，执行完中断后务必记得下面的操作：要清0
    //中断标志位清0
    P0IFG&=~(0X1<<1);//清SW2中断标志
    IRCON&=~(0x1<<5);  //清P0端口组中断标志 第5位为0代表端口0组中断标志位被清除  P0IF=0;
}

/************main函数入口**************************/
void main(void)
{
    //时钟初始化,速度设置为32MHz
    CLKCONCMD &= 0X80;
    while(CLKCONSTA&0X40);
    
    InitLED();
    InitSw();
    InittTimer1();      //初始化Timer1
    InitUART0();        //UART0初始化

    EA = 1;             //使能全局中断
    
    /*.......答题区1开始：D3,D4,D5,D6灯全亮1秒后全灭...*/
       
        D3 = D4 = D5 = D6 = 1;
        counter = 100;
        while(counter);
        D3 = D4 = D5 = D6 = 0;
    
     /*.......答题区1结束...***********************************/
        
    while(1)
    {
      
      if(light_state == 1)//按下按键SW1
      {
      /*.......答题区4 开始******************************/
      /*由考生设置按下按键SW1后:
          （1）按照D4->D3->D6->D5的顺序,每隔1秒依次点亮一个灯,
          当D5灯灭掉之后，等待1秒后将顺序颠倒过来，
          （2）即按照D5->D6->D3->D4的顺序，每隔1秒依次点亮一个灯,
          当D4灯灭掉之后，等待1秒后，再次颠倒，
          后面就是一直按照这样的流程循环进行。
          （3）每次循环完一次之后，发送“流水灯1完成”到电脑串口显示。
      */
        //D4->D3->D6->D5
        D4 = 1;
        counter = 100;
        while(counter);
        D4 = 0;
        D3 = 1;
        counter = 100;
        while(counter);
        D3 = 0;
        D6 = 1;
        counter = 100;
        while(counter);
        D6 = 0;
        D5 = 1;
        counter = 100;
        while(counter);
        D5 = 0;
        counter = 100;
        while(counter);
        
        //D5->D6->D3->D4
        D5 = 1;
        counter = 100;
        while(counter);
        D5 = 0;
        D6 = 1;
        counter = 100;
        while(counter);
        D6 = 0;
        D3 = 1;
        counter = 100;
        while(counter);
        D3 = 0;
        D4 = 1;
        counter = 100;
        while(counter);
        D4 = 0;
        counter = 100;
        while(counter);
        
        //发送到串口显示
        uart_tx_string("流水灯1完成");
        
      /*.......答题区4结束...***********************************/
      }
      else if(light_state == 2)//按下按键SW2
      {
      /*.......答题区5 开始.........*****************************/
      /*由考生设置按下按键SW2后:
          （1）亮灯顺序为先是D4,D6同时点亮，持续2秒，然后灭掉，
          接着D3,D5同时点亮，持续2秒，然后灭掉，
          过1秒后4个灯同时点亮，持续2秒，之后灭掉，保持1秒；
          （2）接着就是先是D3,D5同时点亮，持续2秒，然后灭掉，
          接着D4,D6同时点亮，持续2秒，然后灭掉，
          过1秒后4个灯同时点亮，持续2秒，之后灭掉
          （3）每次循环完一次之后，发送“流水灯2完成”到电脑串口显示。
      */
        //D4,D6先亮
        D4 = 1;
        D6 = 1;
        counter = 200;
        while(counter);
        D4 = 0;
        D6 = 0;
        D3 = 1;
        D5 = 1;
        counter = 200;
        while(counter);
        D3 = 0;
        D5 = 0;
        counter =100;
        while(counter);
        D4 = 1;
        D6 = 1;
        D3 = 1;
        D5 = 1;
        counter = 200;
        while(counter);
        D4 = 0;
        D6 = 0;
        D3 = 0;
        D5 = 0;
        counter = 100;
        while(counter);
        
        //D3,D5先亮
        D3 = 1;
        D5 = 1;
        counter = 200;
        while(counter);
        D3 = 0;
        D5 = 0;
        D4 = 1;
        D6 = 1;
        counter = 200;
        while(counter);
        D4 = 0;
        D6 = 0;
        counter =100;
        while(counter);
        D4 = 1;
        D6 = 1;
        D3 = 1;
        D5 = 1;
        counter = 200;
        while(counter);
        D4 = 0;
        D6 = 0;
        D3 = 0;
        D5 = 0;
        
        //发送到串口显示
        uart_tx_string("流水灯2完成");
        
       /*.......答题区5结束...***********************************/
        
        light_state = 0;//清零
      }
    }
}
