//串口相关的头文件  
#include<termios.h>    /*PPSIX 终端控制定义*/ 
#include<stdio.h>      /*标准输入输出定义*/  
#include<stdlib.h>     /*标准函数库定义*/  
#include<unistd.h>     /*Unix 标准函数定义*/  
#include<sys/types.h>   
#include<sys/stat.h>     
#include<fcntl.h>      /*文件控制定义*/  
#include<errno.h>      /*错误号定义*/  
#include<string.h> 
#include "tty_com.h"

char head_up_buf[6]={0x5A,0x10,0x10,0x02,0x40,0x00};
char head_down_buf[6]={0x5A,0x10,0x10,0x02,0x40,0x01};
char foot_up_buf[6]={0x5A,0x10,0x10,0x02,0x40,0x02};
char foot_down_buf[6]={0x5A,0x10,0x10,0x02,0x40,0x03};
char leg_up_buf[6]={0x5A,0x10,0x10,0x02,0x40,0x04};
char leg_down_buf[6]={0x5A,0x10,0x10,0x02,0x40,0x05};
char lumbar_up_buf[6]={0x5A,0x10,0x10,0x02,0x40,0x06};
char lumbar_down_buf[6]={0x5A,0x10,0x10,0x02,0x40,0x07};
char stop_buf[6]={0x5A,0x10,0x10,0x02,0x30,0x0F};
char flat_buf[6]={0x5A,0x10,0x10,0x02,0x30,0x10};
char antisnore_buf[6]={0x5A,0x10,0x10,0x02,0x30,0x16};
char lounge_buf[6]={0x5A,0x10,0x10,0x02,0x30,0x17};
char zero_gravity_buf[6]={0x5A,0x10,0x10,0x02,0x30,0x13};
char incline_buf[6]={0x5A,0x10,0x10,0x02,0x30,0x18};
char lounge_program_buf[6]={0x5A,0x10,0x10,0x02,0x30,0x27};
char zero_gravity_program_buf[6]={0x5A,0x10,0x10,0x02,0x30,0x23};
char incline_program_buf[6]={0x5A,0x10,0x10,0x02,0x30,0x28};
char massage_on_buf[6]={0x5A,0x10,0x10,0x02,0x30,0x51};
char wave_one_buf[6]={0x5A,0x10,0x10,0x02,0x30,0x52};
char wave_two_buf[6]={0x5A,0x10,0x10,0x02,0x30,0x53};
char wave_three_buf[6]={0x5A,0x10,0x10,0x02,0x30,0x54};
char wave_four_buf[6]={0x5A,0x10,0x10,0x02,0x30,0x55};
char full_body_one_buf[6]={0x5A,0x10,0x10,0x02,0x30,0x56};
char full_body_two_buf[6]={0x5A,0x10,0x10,0x02,0x30,0x57};
char massage_up_buf[6]={0x5A,0x10,0x10,0x02,0x30,0x60};
char massage_down_buf[6]={0x5A,0x10,0x10,0x02,0x30,0x61};
char massage_stop_buf[6]={0x5A,0x10,0x10,0x02,0x30,0x6F};
char light_on_buf[6]={0x5A,0x10,0x10,0x02,0x30,0x73};
char lights_on_buf[6]={0x5A,0x10,0x10,0x02,0x30,0x73};
char light_off_buf[6]={0x5A,0x10,0x10,0x02,0x30,0x74};
char lights_off_buf[6]={0x5A,0x10,0x10,0x02,0x30,0x74};

//宏定义  
#define TRUE   0  
#define FALSE  -1  


/*******************************************************************
* 功能：            设置串口数据位，停止位和效验位
* 入口参数：        fd         串口文件描述符
*                   speed      串口速度
*                   flow_ctrl  数据流控制
*                   databits   数据位   取值为 7 或者8
*                   light_onbits   停止位   取值为 1 或者2
*                   parity     效验类型 取值为N,E,O,,S
*出口参数：         正确返回为1，错误返回为0
*******************************************************************/
int UARTx_Set(int fd, int speed, int flow_ctrl, int databits, int light_onbits, int parity)
{

    int   i;
        int   speed_arr[] = {B115200, B57600, B38400, B19200, B9600, B4800, B2400, B1200, B300 };
        int   name_arr[]  = { 115200,  57600,  38400,  19200,  9600,  4800,  2400,  1200,  300 };

    struct termios options;

        /* tcgetattr(fd,&options)得到与fd指向对象的相关参数，并将它们保存于options,
         * 该函数,还可以测试配置是否正确，该串口是否可用等。
         * 若调用成功，函数返回值为0，若调用失败，函数返回值为1.
         */
    if( tcgetattr( fd,&options)  !=  0)
    {
        perror("SetupSerial 1");    
        return(FALSE); 
    }

    //设置串口输入波特率和输出波特率
    for ( i= 0;  i < sizeof(speed_arr) / sizeof(int);  i++)
    {
        if  (speed == name_arr[i])
        {       
            cfsetispeed(&options, speed_arr[i]); 
            cfsetospeed(&options, speed_arr[i]);  
            printf("uart_speed=%d ", speed );   
        }
    }     

    //修改控制模式，保证程序不会占用串口
    options.c_cflag |= CLOCAL;
    //修改控制模式，使得能够从串口中读取输入数据
    options.c_cflag |= CREAD;

    //设置数据流控制
    switch(flow_ctrl)
    {
        case 0 ://不使用流控制
            options.c_cflag &= ~CRTSCTS;
                break;   

        case 1 ://使用硬件流控制
                options.c_cflag |= CRTSCTS;
                break;
        case 2 ://使用软件流控制
                options.c_cflag |= IXON | IXOFF | IXANY;
                break;
    }
    printf("flow_ctrl=%d ",flow_ctrl);


    //设置数据位
    options.c_cflag &= ~CSIZE; //屏蔽其他标志位
    switch (databits)
    {  
        case 5    :
            options.c_cflag |= CS5;
            break;
        case 6    :
            options.c_cflag |= CS6;
            break;
        case 7    :    
            options.c_cflag |= CS7;
            break;
        case 8:    
            options.c_cflag |= CS8;
            break;  
        default:   
            fprintf(stderr,"Unsupported data size\n");
            return (FALSE); 
    }
    printf("databits=%d ",databits);


    //设置校验位
    switch (parity)
    {  
        case 'n':
        case 'N': //无奇偶校验位。
            options.c_cflag &= ~PARENB; 
            options.c_iflag &= ~INPCK;  
            printf("parity=N ");  
            break; 
        case 'o':  
        case 'O'://设置为奇校验
            options.c_cflag |= (PARODD | PARENB); 
            options.c_iflag |= INPCK;             
            break; 
        case 'e': 
        case 'E'://设置为偶校验
            options.c_cflag |= PARENB; 
            options.c_cflag &= ~PARODD;       
            options.c_iflag |= INPCK;       
            break;
        case 's':
        case 'S': //设置为空格
            options.c_cflag &= ~PARENB;
            options.c_cflag &= ~CSTOPB;
            break; 
        default:  
            fprintf(stderr,"Unsupported parity\n");   
            return (FALSE); 
    } 


    // 设置停止位 
    switch (light_onbits)
    {  
        case 1:   
            options.c_cflag &= ~CSTOPB; 
            break; 
        case 2:   
            options.c_cflag |= CSTOPB; 
            break;
        default:   
            fprintf(stderr,"Unsupported light_on bits\n"); 
            return (FALSE);
    }
    printf("light_onbits=%d \n",light_onbits);


    //修改输出模式，原始数据输出 /*Output*/
    options.c_oflag &= ~OPOST;

    //options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);//我加的 /*Input*/ 
    options.c_lflag &= ~(ISIG | ICANON);  

    //设置等待时间和最小接收字符
    options.c_cc[VTIME] = 0; /* 读取一个字符等待1*(1/10)s */  
    options.c_cc[VMIN] = 1;  /* 读取字符的最少个数为1 */

    //如果发�??数据溢出，接收数据，但是不再读取
    tcflush(fd,TCIFLUSH);

    //激活配置 (将修改后的termios数据设置到串口中）
    if (tcsetattr(fd,TCSANOW,&options) != 0)  
    {
        perror("com set error!\n");  
        return (FALSE); 
    }

    return (TRUE); 
}


/*******************************************************************
* 名称：           UART0_Recv
* 功能：           接收串口数据
* 入口参数：       fd          :文件描述符    
*                  rcv_buf     :接收串口中数据存入rcv_buf缓冲区涓??*                  data_len    :一帧数据的长度
* 出口参数：       正确返回为1，错误返回为0
*******************************************************************/
int UARTx_Recv(int fd, char *rcv_buf,int data_len)
{
    int len,fs_sel;
    fd_set fs_read;

    struct timeval time;

    FD_ZERO(&fs_read);
    FD_SET(fd,&fs_read);

    time.tv_sec = 10;
    time.tv_usec = 0;

    //使用select实现串口的多路通信
    fs_sel = select(fd+1,&fs_read,NULL,NULL,&time);
    if(fs_sel)
    {
            len = read(fd,rcv_buf,data_len);
        printf("I am right!(version1.2) len = %d fs_sel = %d\n",len,fs_sel);  
            return len;
    }
    else
    {
        printf("Sorry,I am wrong!\n");  
            return FALSE;
    }     
}

/*******************************************************************
* 名称：            UART0_Send
* 功能：            发送数据
* 入口参数：         fd         :文件描述符    
*                   send_buf    :�??放串口发送数据
*                   data_len    :一帧数据的个�??
* 出口参数：        正确返回为1，错误返回为0
*******************************************************************/
int UARTx_Send(int fd, char *send_buf,int data_len)
{
    int len = 0;

    len = write(fd,send_buf,data_len);
    printf("len:%d,data_len:%d\n",len,data_len);
    if (len == data_len ){
        return len;
    }     
    else{
        tcflush(fd,TCOFLUSH);
        return FALSE;
    }
}


/*****************************************************************
* 功能：         打开串口并返回串口设备文件描述
* 入口参数：     fd:文件描述符   port:串口号(ttyS0,ttyS1,ttyS2)
* 出口参数：     正确返回为1锛????误返回为0
*****************************************************************/
int UARTx_Open(int fd, const char * ttyX)
{
    //fd = open("/dev/ttyS0", O_RDWR | O_NOCTTY | O_NDELAY);
    fd = open(ttyX, O_RDWR | O_NOCTTY | O_NDELAY);

    if(fd == FALSE)
    {
        perror("open_port: Unable to open /dev/ttyS0 -");
        return(FALSE);
    }

    //判断串口的状态是否为阻塞状态
    if( fcntl(fd,F_SETFL,0)<0 ){
        printf("fcntl failed! \n");
        return(FALSE);
    }     
    else{
        printf("fcntl=%d \n",fcntl(fd, F_SETFL,0));
    }

    //测试是否为�??端设备    
    if(0 == isatty(STDIN_FILENO) ){
        printf("standard input is not a terminal device \n");
        return(FALSE);
    }
    else{
        printf("isatty success!\n");
    }
    printf("fd->open=%d \n",fd);

    //UARTx_Set(fd,115200,0,8,1,'N');
    //UARTx_Set(fd,57600,0,8,1,'N');

    return (fd);
}


/******************************************************
* 名称：    main
*******************************************************************/

int main(int argc, char **argv)  
//int main(void)  
{  
    int fd;                            //文件描述符  
    int err;                           //返回调用函数的状态  
    int len;                          
    int i;  

    char rcv_buf[256];         
    char send_buf[20]="uartx_test\n";  

    //fd = UART0_Open(fd,argv[1]); //打开串口，返回文件描述符  
    //fd = UARTx_Open(fd); //打开串口，返回文件描述符  
    fd = UARTx_Open(fd,argv[1]);
    if(FALSE == fd){        
        exit(1);    
    }

    err = UARTx_Set(fd,115200,0,8,1,'N');  
    if (FALSE == err){  
        printf("Set Port Error\n"); 
        exit(1); 
    }
   





    for(i=0;i<1000;i++){
        len = UARTx_Send(fd, head_up_buf, 6);
        if(len > 0)
		printf(" %x %x %x %x %x %x send head up data successful len=%d \n",head_up_buf[0],head_up_buf[1],head_up_buf[2],head_up_buf[3],head_up_buf[4],head_up_buf[5],len);
        }
/*

   for(i=0;i<50;i++){
        len = UARTx_Send(fd, light_on_buf, strlen(light_on_buf));
        if(len > 0)
                printf(" %x %x %x %x %x %x send light on data successful len=%d \n",light_on_buf[0],light_on_buf[1],light_on_buf[2],light_on_buf[3],light_on_buf[4],light_on_buf[5],len);
        }




    for( i = 0; i < 100; i++ )
    {
        len = UARTx_Send(fd, stop_buf, strlen(light_off_buf));
        if(len > 0)
            printf(" %x %x %x %x %x %x send data successful len=%d \n",light_off_buf[0],light_off_buf[1],light_off_buf[2],light_off_buf[3],light_off_buf[4],light_off_buf[5],len); 
        else
            printf("send data failed!\n");
    }

*/

    len = 0 ;
    while(1) 
    {
        len = read(fd, rcv_buf, 256);  

        if(len > 0){
	    printf("rcv_buf:%s\n",rcv_buf);
            rcv_buf[len] = '\n';
            UARTx_Send(fd,rcv_buf,len+1);
            printf("len=%d uart_RX_ok \n", len); 

            len = 0 ;
            memset(rcv_buf, 0, sizeof(rcv_buf) );
        } 
        sleep(2); 
	break;
    } 

        printf("Close...\n");
    close(fd);   

    return 0;
}

