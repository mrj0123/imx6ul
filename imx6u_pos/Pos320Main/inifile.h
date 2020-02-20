//获取当前程序目录
int fn_GetCurrentPath(char buf[],char *pFileName);
//从INI文件读取字符串类型数据
char *fn_GetIniKeyString(char *title,char *key,char *filename);

//从INI文件读取整类型数据
int fn_GetIniKeyInt(char *title,char *key,char *filename);
