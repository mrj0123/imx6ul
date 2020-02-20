#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include "a.h"
#include <ctype.h>
#include "inifile.h"

//获取当前程序目录
int fn_GetCurrentPath(char buf[],char *pFileName)
{
#ifdef WIN32
	GetModuleFileName(NULL,buf,MAX_PATH);
#else
	char pidfile[64];
	int bytes;
	int fd;

	sprintf(pidfile, "/proc/%d/cmdline", getpid());

	fd = open(pidfile, O_RDONLY, 0);
	bytes = read(fd, buf, 256);
	close(fd);
	buf[MAX_PATH] = '\0';

#endif
	char * p = &buf[strlen(buf)];
	do
	{
		*p = '\0';
		p--;
#ifdef WIN32
	} while( '\\' != *p );
#else
	} while( '/' != *p );
#endif

	p++;

	//配置文件目录
	memcpy(p,pFileName,strlen(pFileName));
	return 0;
}

//从INI文件读取字符串类型数据
char *fn_GetIniKeyString(char *title,char *key,char *filename)
{
	FILE *fp;
	char szLine[1024];
	static char tmpstr[1024];
	int rtnval;
	int i = 0;
	int flag = 0;
	char *tmp;
	//myPtf("打开配置文件%s\n",filename);
	if((fp = fopen(filename, "r")) == NULL)
	{
		myPtf("have   no   such   file %s\n",filename);
		return "";
	}
	char line1[256];
	char line2[256];
	char line3[256];

	while(!feof(fp))
	{
		memset(line1,0,256);
		fgets(line1, sizeof(line1), fp);//IP地址
		line1[strlen(line1)-1]='\0';//去掉fgets自动增加的换行符


		memset(tmpstr,0,1024);
		strcpy(tmpstr,"[");
		strcat(tmpstr,title);
		strcat(tmpstr,"]");
		int pos=0;
		int len=0;
		if( strncmp(tmpstr,line1,strlen(tmpstr)) == 0 )
		{//含有title

			memset(line2,0,256);
			fgets(line2, sizeof(line2), fp);//IP地址
			line2[strlen(line2)-1]='\0';//去掉fgets自动增加的换行符
			memset(line3,0,256);
			fgets(line3, sizeof(line3), fp);//IP地址
			line3[strlen(line3)-1]='\0';//去掉fgets自动增加的换行符
			//判断是否含有关键字
			if(strstr(line2,key)!=NULL)
			{//含有关键字key
				//myPtf("%s含有关键字%s\n",line2,key);
				//查找字符串中出现关键字的位置
				tmp = strchr(line2, '=');
				if(tmp)
				{
					pos=tmp-line2+1;
					len=strlen(line2);
				        //myPtf("字符串%s出现关键字%s的位置%d,%d\n",line2,key,pos,len);
					memset(tmpstr,0,1024);
					sprintf(tmpstr,"%s",&line2[pos]);

				}
				break;
			}//含有关键字
			else if(strstr(line3,key)!=NULL)
			{//含有关键字key
				//myPtf("%s含有关键字%s\n",line3,key);
				//查找字符串中出现关键字的位置
				tmp = strchr(line3, '=');
				if( tmp )
				{
					pos=tmp-line3+1;
					len=strlen(line3);
				        //myPtf("字符串%s出现关键字%s的位置%d,%d\n",line3,key,pos,len);
					memset(tmpstr,0,1024);
					sprintf(tmpstr,"%s",&line3[pos]);
				}

				break;
			}//含有关键字key

			break;
		}//含有title



		//linebuf=malloc(256);
		//linebuf=fn_GetStringBySpecailChar(line);
		//myPtf("行处理返回%s",linebuf);
		/*rtnval = fgetc(fp);

		if(rtnval == EOF)
		{
			break;
		}
		else
		{
			szLine[i++] = rtnval;
		}

		if(rtnval == '\n')
		{
#ifndef WIN32
			i--;
#endif
			szLine[--i] = '\0';
			i = 0;
			tmp = strchr(szLine, '=');

			if(( tmp != NULL )&&(flag == 1))
			{
				if(strstr(szLine,key)!=NULL)
				{
					//注释行
					if ('#' == szLine[0])
					{
					}

					//else if ( '\/' == szLine[0] && '\/' == szLine[1] )
					//{

					//}
					else
					{
						//找打key对应变量
						strcpy(tmpstr,tmp+1);
						fclose(fp);
						return tmpstr;
					}
				}
			}
			else
			{
				strcpy(tmpstr,"[");
				strcat(tmpstr,title);
				strcat(tmpstr,"]");
				if( strncmp(tmpstr,szLine,strlen(tmpstr)) == 0 )
				{
					//找到title
					flag = 1;
				}
			}
		}*/
	}

	fclose(fp);
	//myPtf("找到%s\n",tmpstr);
	return &tmpstr[0];
}

//从INI文件读取整类型数据
int fn_GetIniKeyInt(char *title,char *key,char *filename)
{
	return atoi(fn_GetIniKeyString(title,key,filename));
}
