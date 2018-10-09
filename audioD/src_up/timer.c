#include<stdio.h>
#include<signal.h>
#include<unistd.h>
#include<sys/time.h>//itimerval结构体的定义

void set_time(unsigned long int second, unsigned long int usecond)
{
   struct itimerval itv;
   itv.it_interval.tv_sec = second;//5;//10;//自动装载，之后每10秒响应一次
   itv.it_interval.tv_usec = usecond;
   itv.it_value.tv_sec = second; //5;//第一次定时的时间
   itv.it_value.tv_usec = usecond;//0;
   setitimer(ITIMER_REAL,&itv,NULL);
}

void unset_time()  
{  
    struct itimerval value;  
    value.it_value.tv_sec = 0;  
    value.it_value.tv_usec = 0;  
    value.it_interval = value.it_value;  
    setitimer(ITIMER_REAL, &value, NULL);  
}

/*
void alarm_handle(int sig)
{
   printf("have alarm handle\n");
   unset_time();
}

void main(void)
{
   struct itimerval itv;

   signal(SIGALRM,alarm_handle);

   set_time(5,0);

   while(1){
  	printf("xxx\n");
//   	getitimer(ITIMER_REAL,&itv);
//   	printf("pass second is %d/n",(int)itv.it_value.tv_sec);
   	sleep(1);
   }
   
   return;
}
*/
