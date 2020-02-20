#ifndef NETCOMMON_H
#define NETCOMMON_H





long GetTick(char *str_time);
//获得命令返回值
//ret:0异常 1正常
int GetCommandReturnValue(char *cmd,char *outvalue);
int replaceStr(char *s1, char *s2, char *s3);//替换字符串函数返回n表示替换了多少次 
void reStr(char *s1, char *s2, char *s3);//替换字符串 
int checkStr(char *s1, char *s2);//匹配字符串相不相等


#endif // NETCOMMON_H
