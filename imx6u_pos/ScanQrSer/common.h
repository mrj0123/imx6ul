#include <string.h>
#ifdef WIN32
#include <Windows.h>
#include <stdio.h>
#else
#endif

#define  MAX_PATH 260

#include "a.h"

//������ʾ
int fn_ShowErr(int ErrorCode);
//��INI�ļ���ȡ�ַ�����������
char *fn_GetIniKeyString(char *title,char *key,char *filename);
//��INI�ļ���ȡ����������
int fn_GetIniKeyInt(char *title,char *key,char *filename);
//��ȡ��ǰ����Ŀ¼
int fn_GetCurrentPath(char buf[],char *pFileName);

int g2u(char *inbuf,size_t inlen,char *outbuf,size_t outlen);
