
#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <stdlib.h>
#include "log.h"
#include "a.h"

int write_log (const char *format, ...) {
    va_list arg;
    int done=0;
   if(SAVE_LOG_FLAG_IN_DISK==1)
   {	
    va_start (arg, format);
    //done = vfprintf (stdout, format, arg);
    char filename[256];
    filename[0]='\0';	

    time_t time_log = time(NULL);
    struct tm* tm_log = localtime(&time_log);
    sprintf(filename,"%s/%d_%02d_%02d.txt",USER_HOME_DIRECTORY,tm_log->tm_year + 1900, tm_log->tm_mon + 1, tm_log->tm_mday);
    FILE* pFile= fopen(filename, "a");
    fprintf(pFile, "[%04d-%02d-%02d %02d:%02d:%02d] ", tm_log->tm_year + 1900, tm_log->tm_mon + 1, tm_log->tm_mday, tm_log->tm_hour, tm_log->tm_min, tm_log->tm_sec);

    done = vfprintf (pFile, format, arg);
    va_end (arg);

    fflush(pFile);
    fclose(pFile);
    }
    return done;
}

//服务器端日志
int write_log_s (const char *format, ...) {
    va_list arg;
    int done=0;
if(SAVE_LOG_FLAG_IN_DISK==1)
   {	
    va_start (arg, format);
    //done = vfprintf (stdout, format, arg);
    char filename[256];
    filename[0]='\0';	

    time_t time_log = time(NULL);
    struct tm* tm_log = localtime(&time_log);
    sprintf(filename,"%s/%d_%02d_%02d_s.txt",USER_HOME_DIRECTORY,tm_log->tm_year + 1900, tm_log->tm_mon + 1, tm_log->tm_mday);
    FILE* pFile= fopen(filename, "a");
    fprintf(pFile, "[%04d-%02d-%02d %02d:%02d:%02d] ", tm_log->tm_year + 1900, tm_log->tm_mon + 1, tm_log->tm_mday, tm_log->tm_hour, tm_log->tm_min, tm_log->tm_sec);

    done = vfprintf (pFile, format, arg);
    va_end (arg);

    fflush(pFile);
    fclose(pFile);
}
    return done;
}

int append_log_s (const char *format, ...) {
    va_list arg;
    int done=0;
if(SAVE_LOG_FLAG_IN_DISK==1)
   {	
    va_start (arg, format);
    //done = vfprintf (stdout, format, arg);
    char filename[256];
    filename[0]='\0';	

    time_t time_log = time(NULL);
    struct tm* tm_log = localtime(&time_log);
    sprintf(filename,"%s/%d_%02d_%02d_s.txt",USER_HOME_DIRECTORY,tm_log->tm_year + 1900, tm_log->tm_mon + 1, tm_log->tm_mday);
    FILE* pFile= fopen(filename, "a");
    done = vfprintf (pFile, format, arg);
    va_end (arg);

    fflush(pFile);
    fclose(pFile);
}
    return done;
}
int append_log_c (const char *format, ...) {
    va_list arg;
    int done=0;
if(SAVE_LOG_FLAG_IN_DISK==1)
   {	
    va_start (arg, format);
    //done = vfprintf (stdout, format, arg);
    char filename[256];
    filename[0]='\0';	

    time_t time_log = time(NULL);
    struct tm* tm_log = localtime(&time_log);
    sprintf(filename,"%s/%d_%02d_%02d_c.txt",USER_HOME_DIRECTORY,tm_log->tm_year + 1900, tm_log->tm_mon + 1, tm_log->tm_mday);
    FILE* pFile= fopen(filename, "a");
    done = vfprintf (pFile, format, arg);
    va_end (arg);

    fflush(pFile);
    fclose(pFile);
	}
    return done;
}
//客户端日志
int write_log_c (const char *format, ...) {
    va_list arg;
    int done=0;
if(SAVE_LOG_FLAG_IN_DISK==1)
   {	
    va_start (arg, format);
    //done = vfprintf (stdout, format, arg);
    char filename[256];
    filename[0]='\0';	

    time_t time_log = time(NULL);
    struct tm* tm_log = localtime(&time_log);
    sprintf(filename,"%s/%d_%02d_%02d_c.txt",USER_HOME_DIRECTORY,tm_log->tm_year + 1900, tm_log->tm_mon + 1, tm_log->tm_mday);
    FILE* pFile= fopen(filename, "a");
    fprintf(pFile, "[%04d-%02d-%02d %02d:%02d:%02d] ", tm_log->tm_year + 1900, tm_log->tm_mon + 1, tm_log->tm_mday, tm_log->tm_hour, tm_log->tm_min, tm_log->tm_sec);

    done = vfprintf (pFile, format, arg);
    va_end (arg);

    fflush(pFile);
    fclose(pFile);
}
    return done;
}
/*
日志用法
#include "log.h"
//注意log.h的日志开关
 write_log("%s\n", "version=1.01 running");
*/
