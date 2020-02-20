#ifndef LOG_H
#define LOG_H


#define SAVE_LOG_FLAG_IN_DISK 0

//int write_log (FILE* pFile, const char *format, ...);
int write_log_s (const char *format, ...);
int append_log_s (const char *format, ...);
int write_log_c (const char *format, ...);
int append_log_c (const char *format, ...);
int write_log (const char *format, ...);



#endif // LOG_H
