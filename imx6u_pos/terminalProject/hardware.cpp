#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include "hardware.h"


char * GetHardwareSerialNo(int *reclen)
{
	FILE *ptr = NULL;
	char cmd[128] = "cat /sys/fsl_otp/HW_OCOTP_CFG1";
    //int status = 0;
	char buf[150];
    //int count;
	char *newbuf=NULL;

	newbuf=(char *)malloc(256);
	memset(newbuf,0,256);

	if((ptr = popen(cmd, "r"))==NULL)
	{
		return 0;
	}
	*reclen=0;
	memset(buf, 0, sizeof(buf));
	
	if((fgets(buf, sizeof(buf),ptr))!= NULL)//获取进程和子进程的总数
	{
		sprintf(newbuf,"%s",&buf[2]);
		newbuf[strlen(newbuf)-1]='\0';
		*reclen=strlen(newbuf);
		for(int i=0;i<strlen(newbuf);i++)
		{
			
			if(newbuf[i]>96&&newbuf[i]<123)
			   {
			    newbuf[i]=newbuf[i]-32;
			    continue;
			   }
		}		
	}
    fclose(ptr);
	return newbuf;

}

