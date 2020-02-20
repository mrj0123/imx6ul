


#ifdef WIN32
#include <Windows.h>
#else
#endif

#define  MAX_PATH 260


#define DEV_BUS         0
#define DEV_VENDOR      1
#define DEV_PORDUCT     2
#define DEV_VERSION     3
#define DEV_NAME        4
#define DEV_PHYS        5
#define DEV_SYSFS       6
#define DEV_HANDLERS    7

#define DEV_ITEMLEN     8
#define DEV_INFOLEN     256

//错误提示
int fn_ShowErr(int ErrorCode);
//从INI文件读取字符串类型数据
char *fn_GetIniKeyString(char *title,char *key,char *filename);
//从INI文件读取整类型数据
int fn_GetIniKeyInt(char *title,char *key,char *filename);
//获取当前程序目录
int fn_GetCurrentPath(char buf[],char *pFileName);

int g2u(char *inbuf,size_t inlen,char *outbuf,size_t outlen);
//获取当天的当前时间（不包括日期）的时间戳，精确到0.1毫秒
int getCurrentTime();

char * GetKBList(int * pKBNumber);

//初始化脱机流水,
//返回读取总记录数
int fn_InitialRecords();
//记录一条流水在最后，包括记录到内存中以及文件中，
//成功返回0，失败返回-1；
int fn_AddRecord(char * data);
//取出当前第一条以后的一批流水，用于上传
////返回读取本批次记录数
int fn_GetRecord(char * data,int number);
//删除当前第一条流水之后的一批流水，上传成功后调用
//返回删除成功后的剩余流水数，全部删除返回0，删除失败返回-1
int fn_DelRecord(int number);
//存储流水，上传部分流水后，如果发现网又断了，需要回写存储
int fn_WriteRecords();
//删除脱机流水
int fn_ClearRecords();
//获取脱机流水条数
int fn_GetOffline_FlowNum();
//写数据包，pData表示写入数据，PackageNum=1表示第一包，需要覆盖原来文件，其余表示追加文件
int fn_WriteAccoPack(char * pData,int PackageNum,int dataLen);
//读数据包，返回读到的数据内容，pAccNum表示数据条数，指针方式返回。
char * fn_ReadAccoPack(int * pAccNum);

//写入配置文件
int fn_WriteConfig(char * config,int len);
//读取配置文件
int fn_ReadConfig(char * config,int len);

//在人员名单中查找账号
int fn_FindKeyByAccountId(char * pUsers,int Num,int accountId);
//在人员名单中查找卡号
int fn_FindKeyByCardId(char * pUsers,int Num,int cardId);

//void myPtf(const char* format, ...);
