#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


#define SYSFS_BRIGHT_LED0           "/sys/class/pca9633/adc_read"  
#define SYSFS_BRIGHTRESS_OPEN     "0"   
#define SYSFS_BRIGHTRESS_CLOSE     "255"



int main(void)
{
  
   int fd,i;
   double value1,value2;
   char  adc_value[4];
   fd= open(SYSFS_BRIGHT_LED0,O_RDWR);
   if(fd<0)
   { 
     perror("[yebin]----open the file error----\r\n");
     exit(1);
   }else
     perror("[yebin]----open the file OK!!----\r\n");



     while(1)
    {

	
      lseek(fd,0,SEEK_SET);
	read(fd,adc_value,4);
 
	value1=atoi(adc_value);
	if(i>0)
	value2 = (value1+value2)/2;
	else
	value2 = value1;
	i++;
	if(i>39)
      {
          printf("%f\n",value2);
	    i=0;
	}
	
	usleep(10000);
    }	  

    close(fd); 
   return 0;
}



