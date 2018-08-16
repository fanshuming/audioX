//´®¿ÚÏà¹ØµÄÍ·ÎÄ¼ş  
#include<termios.h>    /*PPSIX ÖÕ¶Ë¿ØÖÆ¶¨Òå*/ 
#include<stdio.h>      /*±ê×¼ÊäÈëÊä³ö¶¨Òå*/  
#include<stdlib.h>     /*±ê×¼º¯Êı¿â¶¨Òå*/  
#include<unistd.h>     /*Unix ±ê×¼º¯Êı¶¨Òå*/  
#include<sys/types.h>   
#include<sys/stat.h>     
#include<fcntl.h>      /*ÎÄ¼ş¿ØÖÆ¶¨Òå*/  
#include<errno.h>      /*´íÎóºÅ¶¨Òå*/  
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

//ºê¶¨Òå  
#define TRUE   0  
#define FALSE  -1  


/*******************************************************************
* ¹¦ÄÜ£º            ÉèÖÃ´®¿ÚÊı¾İÎ»£¬Í£Ö¹Î»ºÍĞ§ÑéÎ»
* Èë¿Ú²ÎÊı£º        fd         ´®¿ÚÎÄ¼şÃèÊö·û
*                   speed      ´®¿ÚËÙ¶È
*                   flow_ctrl  Êı¾İÁ÷¿ØÖÆ
*                   databits   Êı¾İÎ»   È¡ÖµÎª 7 »òÕß8
*                   light_onbits   Í£Ö¹Î»   È¡ÖµÎª 1 »òÕß2
*                   parity     Ğ§ÑéÀàĞÍ È¡ÖµÎªN,E,O,,S
*³ö¿Ú²ÎÊı£º         ÕıÈ··µ»ØÎª1£¬´íÎó·µ»ØÎª0
*******************************************************************/
int UARTx_Set(int fd, int speed, int flow_ctrl, int databits, int light_onbits, int parity)
{

    int   i;
        int   speed_arr[] = {B115200, B57600, B38400, B19200, B9600, B4800, B2400, B1200, B300 };
        int   name_arr[]  = { 115200,  57600,  38400,  19200,  9600,  4800,  2400,  1200,  300 };

    struct termios options;

        /* tcgetattr(fd,&options)µÃµ½ÓëfdÖ¸Ïò¶ÔÏóµÄÏà¹Ø²ÎÊı£¬²¢½«ËüÃÇ±£´æÓÚoptions,
         * ¸Ãº¯Êı,»¹¿ÉÒÔ²âÊÔÅäÖÃÊÇ·ñÕıÈ·£¬¸Ã´®¿ÚÊÇ·ñ¿ÉÓÃµÈ¡£
         * Èôµ÷ÓÃ³É¹¦£¬º¯Êı·µ»ØÖµÎª0£¬Èôµ÷ÓÃÊ§°Ü£¬º¯Êı·µ»ØÖµÎª1.
         */
    if( tcgetattr( fd,&options)  !=  0)
    {
        perror("SetupSerial 1");    
        return(FALSE); 
    }

    //ÉèÖÃ´®¿ÚÊäÈë²¨ÌØÂÊºÍÊä³ö²¨ÌØÂÊ
    for ( i= 0;  i < sizeof(speed_arr) / sizeof(int);  i++)
    {
        if  (speed == name_arr[i])
        {       
            cfsetispeed(&options, speed_arr[i]); 
            cfsetospeed(&options, speed_arr[i]);  
            printf("uart_speed=%d ", speed );   
        }
    }     

    //ĞŞ¸Ä¿ØÖÆÄ£Ê½£¬±£Ö¤³ÌĞò²»»áÕ¼ÓÃ´®¿Ú
    options.c_cflag |= CLOCAL;
    //ĞŞ¸Ä¿ØÖÆÄ£Ê½£¬Ê¹µÃÄÜ¹»´Ó´®¿ÚÖĞ¶ÁÈ¡ÊäÈëÊı¾İ
    options.c_cflag |= CREAD;

    //ÉèÖÃÊı¾İÁ÷¿ØÖÆ
    switch(flow_ctrl)
    {
        case 0 ://²»Ê¹ÓÃÁ÷¿ØÖÆ
            options.c_cflag &= ~CRTSCTS;
                break;   

        case 1 ://Ê¹ÓÃÓ²¼şÁ÷¿ØÖÆ
                options.c_cflag |= CRTSCTS;
                break;
        case 2 ://Ê¹ÓÃÈí¼şÁ÷¿ØÖÆ
                options.c_cflag |= IXON | IXOFF | IXANY;
                break;
    }
    printf("flow_ctrl=%d ",flow_ctrl);


    //ÉèÖÃÊı¾İÎ»
    options.c_cflag &= ~CSIZE; //ÆÁ±ÎÆäËû±êÖ¾Î»
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


    //ÉèÖÃĞ£ÑéÎ»
    switch (parity)
    {  
        case 'n':
        case 'N': //ÎŞÆæÅ¼Ğ£ÑéÎ»¡£
            options.c_cflag &= ~PARENB; 
            options.c_iflag &= ~INPCK;  
            printf("parity=N ");  
            break; 
        case 'o':  
        case 'O'://ÉèÖÃÎªÆæĞ£Ñé
            options.c_cflag |= (PARODD | PARENB); 
            options.c_iflag |= INPCK;             
            break; 
        case 'e': 
        case 'E'://ÉèÖÃÎªÅ¼Ğ£Ñé
            options.c_cflag |= PARENB; 
            options.c_cflag &= ~PARODD;       
            options.c_iflag |= INPCK;       
            break;
        case 's':
        case 'S': //ÉèÖÃÎª¿Õ¸ñ
            options.c_cflag &= ~PARENB;
            options.c_cflag &= ~CSTOPB;
            break; 
        default:  
            fprintf(stderr,"Unsupported parity\n");   
            return (FALSE); 
    } 


    // ÉèÖÃÍ£Ö¹Î» 
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


    //ĞŞ¸ÄÊä³öÄ£Ê½£¬Ô­Ê¼Êı¾İÊä³ö /*Output*/
    options.c_oflag &= ~OPOST;

    //options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);//ÎÒ¼ÓµÄ /*Input*/ 
    options.c_lflag &= ~(ISIG | ICANON);  

    //ÉèÖÃµÈ´ıÊ±¼äºÍ×îĞ¡½ÓÊÕ×Ö·û
    options.c_cc[VTIME] = 0; /* ¶ÁÈ¡Ò»¸ö×Ö·ûµÈ´ı1*(1/10)s */  
    options.c_cc[VMIN] = 1;  /* ¶ÁÈ¡×Ö·ûµÄ×îÉÙ¸öÊıÎª1 */

    //Èç¹û·¢ç??Êı¾İÒç³ö£¬½ÓÊÕÊı¾İ£¬µ«ÊÇ²»ÔÙ¶ÁÈ¡
    tcflush(fd,TCIFLUSH);

    //¼¤»îÅäÖÃ (½«ĞŞ¸ÄºóµÄtermiosÊı¾İÉèÖÃµ½´®¿ÚÖĞ£©
    if (tcsetattr(fd,TCSANOW,&options) != 0)  
    {
        perror("com set error!\n");  
        return (FALSE); 
    }

    return (TRUE); 
}


/*******************************************************************
* Ãû³Æ£º           UART0_Recv
* ¹¦ÄÜ£º           ½ÓÊÕ´®¿ÚÊı¾İ
* Èë¿Ú²ÎÊı£º       fd          :ÎÄ¼şÃèÊö·û    
*                  rcv_buf     :½ÓÊÕ´®¿ÚÖĞÊı¾İ´æÈërcv_buf»º³åÇøä¸??*                  data_len    :Ò»Ö¡Êı¾İµÄ³¤¶È
* ³ö¿Ú²ÎÊı£º       ÕıÈ··µ»ØÎª1£¬´íÎó·µ»ØÎª0
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

    //Ê¹ÓÃselectÊµÏÖ´®¿ÚµÄ¶àÂ·Í¨ĞÅ
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
* Ãû³Æ£º            UART0_Send
* ¹¦ÄÜ£º            ·¢ËÍÊı¾İ
* Èë¿Ú²ÎÊı£º         fd         :ÎÄ¼şÃèÊö·û    
*                   send_buf    :å??·Å´®¿Ú·¢ËÍÊı¾İ
*                   data_len    :Ò»Ö¡Êı¾İµÄ¸öæ??
* ³ö¿Ú²ÎÊı£º        ÕıÈ··µ»ØÎª1£¬´íÎó·µ»ØÎª0
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
* ¹¦ÄÜ£º         ´ò¿ª´®¿Ú²¢·µ»Ø´®¿ÚÉè±¸ÎÄ¼şÃèÊö
* Èë¿Ú²ÎÊı£º     fd:ÎÄ¼şÃèÊö·û   port:´®¿ÚºÅ(ttyS0,ttyS1,ttyS2)
* ³ö¿Ú²ÎÊı£º     ÕıÈ··µ»ØÎª1ï¼????Îó·µ»ØÎª0
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

    //ÅĞ¶Ï´®¿ÚµÄ×´Ì¬ÊÇ·ñÎª×èÈû×´Ì¬
    if( fcntl(fd,F_SETFL,0)<0 ){
        printf("fcntl failed! \n");
        return(FALSE);
    }     
    else{
        printf("fcntl=%d \n",fcntl(fd, F_SETFL,0));
    }

    //²âÊÔÊÇ·ñÎªç??¶ËÉè±¸    
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
* Ãû³Æ£º    main
*******************************************************************/

int main(int argc, char **argv)  
//int main(void)  
{  
    int fd;                            //ÎÄ¼şÃèÊö·û  
    int err;                           //·µ»Øµ÷ÓÃº¯ÊıµÄ×´Ì¬  
    int len;                          
    int i;  

    char rcv_buf[256];         
    char send_buf[20]="uartx_test\n";  

    //fd = UART0_Open(fd,argv[1]); //´ò¿ª´®¿Ú£¬·µ»ØÎÄ¼şÃèÊö·û  
    //fd = UARTx_Open(fd); //´ò¿ª´®¿Ú£¬·µ»ØÎÄ¼şÃèÊö·û  
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

