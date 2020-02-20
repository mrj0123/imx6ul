
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#include <time.h>
#include <ctype.h>

#include "common.h"


long GetTick(char *str_time)  
{  
    struct tm stm;  
    int iY, iM, iD, iH, iMin, iS;  
  
    memset(&stm,0,sizeof(stm));  
  
    iY = atoi(str_time);  
    iM = atoi(str_time+5);  
    iD = atoi(str_time+8);  
    iH = atoi(str_time+11);  
    iMin = atoi(str_time+14);  
    iS = atoi(str_time+17);  
  
    stm.tm_year=iY-1900;  
    stm.tm_mon=iM-1;  
    stm.tm_mday=iD;  
    stm.tm_hour=iH;  
    stm.tm_min=iMin;  
    stm.tm_sec=iS;  
  
    myPtf("=====time format=====%d-%02d-%02d %02d:%02d:%02d=============\n", iY, iM, iD, iH, iMin, iS); 
  
    return mktime(&stm);  
}  
//获得命令返回值
int GetCommandReturnValue(char *cmd,char *outvalue)
{
	FILE *ptr = NULL;
	char mycmd[1024];
	memset(mycmd,0,1024);
	sprintf(mycmd,"%s",cmd);	

	int status = 0;
	char buf[1024];
	int count;
	

	
	memset(outvalue,0,1024);

	if((ptr = popen(mycmd, "r"))==NULL)
	{
		return 0;
	}
	
	memset(buf, 0, sizeof(buf));
	
	if((fgets(buf, sizeof(buf),ptr))!= NULL)//获取进程和子进程的总数
	{
		sprintf(outvalue,"%s",buf);
		outvalue[strlen(outvalue)-1]='\0';
		
	}
	fclose(ptr);
	return 1;

}

int checkStr(char *s1, char *s2) //匹配字符串相不相等
{
	for (int i = 0; i < strlen(s2); i++)
	{
		if (s1[i] != s2[i]) return 0;
	}
	return 1;
}
void reStr(char *s1, char *s2, char *s3)//替换字符串
{
	int s1len = strlen(s1),
	s2len = strlen(s2),
	s3len = strlen(s3);
	int n1 = s1len - s2len;
	int n2 = s2len - s3len;
	if (n2 > 0)
	{
		//后面的字符串全部向前移n2个位置
		for (int i = s2len; i < s1len; i++)
		{
			s1[i - n2] = s1[i];
		}
		s1[s1len - n2] = '\0';
	}
	else if (n2 < 0)//后面的字符串全部向后移n2个位置
	{
		for (int i = s1len; i >= s2len; i--)
		{
			s1[i - n2] = s1[i];
		}
		s1[s1len - n2 + 1] = '\0';
	}
	//然后在把要替换的字符串插进去
	for (int i=0;i<s3len;i++)
	{
		s1[i] = s3[i];
	}
}
int replaceStr(char *s1, char *s2, char *s3)//替换字符串函数返回n表示替换了多少次 
{ 
	int n = 0; 
	for (int i = 0; i < strlen(s1); i++)
	{
		if (s1[i] == s2[0] && checkStr(&s1[i], s2) == 1)
		{
			reStr(&s1[i], s2, s3);
			n++;
		}
	}
	return n;
}
