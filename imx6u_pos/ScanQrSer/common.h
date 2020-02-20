#include <string.h>
#ifdef WIN32
#include <Windows.h>
#include <stdio.h>
#else
#endif

#define  MAX_PATH 260

#include "a.h"

//错误提示
int fn_ShowErr(int ErrorCode);
//从INI文件读取字符串类型数据
char *fn_GetIniKeyString(char *title,char *key,char *filename);
//从INI文件读取整类型数据
int fn_GetIniKeyInt(char *title,char *key,char *filename);
//获取当前程序目录
int fn_GetCurrentPath(char buf[],char *pFileName);

int g2u(char *inbuf,size_t inlen,char *outbuf,size_t outlen);
