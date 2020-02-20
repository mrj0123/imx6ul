


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

//������ʾ
int fn_ShowErr(int ErrorCode);
//��INI�ļ���ȡ�ַ�����������
char *fn_GetIniKeyString(char *title,char *key,char *filename);
//��INI�ļ���ȡ����������
int fn_GetIniKeyInt(char *title,char *key,char *filename);
//��ȡ��ǰ����Ŀ¼
int fn_GetCurrentPath(char buf[],char *pFileName);

int g2u(char *inbuf,size_t inlen,char *outbuf,size_t outlen);
//��ȡ����ĵ�ǰʱ�䣨���������ڣ���ʱ�������ȷ��0.1����
int getCurrentTime();

char * GetKBList(int * pKBNumber);

//��ʼ���ѻ���ˮ,
//���ض�ȡ�ܼ�¼��
int fn_InitialRecords();
//��¼һ����ˮ����󣬰�����¼���ڴ����Լ��ļ��У�
//�ɹ�����0��ʧ�ܷ���-1��
int fn_AddRecord(char * data);
//ȡ����ǰ��һ���Ժ��һ����ˮ�������ϴ�
////���ض�ȡ�����μ�¼��
int fn_GetRecord(char * data,int number);
//ɾ����ǰ��һ����ˮ֮���һ����ˮ���ϴ��ɹ������
//����ɾ���ɹ����ʣ����ˮ����ȫ��ɾ������0��ɾ��ʧ�ܷ���-1
int fn_DelRecord(int number);
//�洢��ˮ���ϴ�������ˮ������������ֶ��ˣ���Ҫ��д�洢
int fn_WriteRecords();
//ɾ���ѻ���ˮ
int fn_ClearRecords();
//��ȡ�ѻ���ˮ����
int fn_GetOffline_FlowNum();
//д���ݰ���pData��ʾд�����ݣ�PackageNum=1��ʾ��һ������Ҫ����ԭ���ļ��������ʾ׷���ļ�
int fn_WriteAccoPack(char * pData,int PackageNum,int dataLen);
//�����ݰ������ض������������ݣ�pAccNum��ʾ����������ָ�뷽ʽ���ء�
char * fn_ReadAccoPack(int * pAccNum);

//д�������ļ�
int fn_WriteConfig(char * config,int len);
//��ȡ�����ļ�
int fn_ReadConfig(char * config,int len);

//����Ա�����в����˺�
int fn_FindKeyByAccountId(char * pUsers,int Num,int accountId);
//����Ա�����в��ҿ���
int fn_FindKeyByCardId(char * pUsers,int Num,int cardId);

//void myPtf(const char* format, ...);
