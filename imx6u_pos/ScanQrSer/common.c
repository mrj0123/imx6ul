#include "a.h"
#include "common.h"

int code_convert(char *from_charset,char *to_charset,char *inbuf,int inlen,char *outbuf,int outlen)
{
        iconv_t cd;
        int rc;
        char **pin = &inbuf;
        char **pout = &outbuf;

        cd = iconv_open(to_charset,from_charset);
        if (cd==0)
        {
            perror("iconv_open");
            return -1;
        }

        size_t a=inlen;
        size_t b=outlen;

        memset(outbuf,0,outlen);
        //if (iconv(cd,pin,(size_t *)&inlen,pout,(size_t *)&outlen) == -1)
        if (iconv(cd,pin,&a,pout,&b) == -1)
        {
            perror("iconv");
            return -1;
        }
        iconv_close(cd);
        return 0;
}

int u2g(char *inbuf,int inlen,char *outbuf,int outlen)
{
        return code_convert("utf-8","gb2312",inbuf,inlen,outbuf,outlen);
}

int g2u(char *inbuf,size_t inlen,char *outbuf,size_t outlen)
{
        return code_convert("gb2312","utf-8",inbuf,inlen,outbuf,outlen);
        memcpy(outbuf,inbuf,inlen);
        return 0;
}

//错误提示
int fn_ShowErr(int ErrorCode)
{
    switch(ErrorCode)
    {
        //显示错误信息
        case -1:
            myPtf("errorcode -1\n");
            break;
        case -2:
            myPtf("errorcode -2\n");
            break;
        default:
            myPtf("errorcode 其它\n");
            break;
    }
    return 1;
}

//获取开机毫秒数
unsigned long GetTickCount()
{
    /*
    struct timespec ts;

    clock_gettime(CLOCK_MONOTONIC, &ts);

    return (ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
    */
    return 0;
}


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

	if((fp = fopen(filename, "r")) == NULL)
	{
		myPtf("have   no   such   file \n");
		return "";
	}
	while(!feof(fp))
	{
		rtnval = fgetc(fp);
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
					/*
					else if ( '\/' == szLine[0] && '\/' == szLine[1] )
					{

					}*/
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
		}
	}
	fclose(fp);
	return "";
}

//从INI文件读取整类型数据
int fn_GetIniKeyInt(char *title,char *key,char *filename)
{
	return atoi(fn_GetIniKeyString(title,key,filename));
}
